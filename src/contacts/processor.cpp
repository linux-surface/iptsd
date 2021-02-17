#include "processor.hpp"

#include "types.hpp"

#include "algorithm/convolution.hpp"
#include "algorithm/distance_transform.hpp"
#include "algorithm/gaussian_fitting.hpp"
#include "algorithm/hessian.hpp"
#include "algorithm/label.hpp"
#include "algorithm/local_maxima.hpp"
#include "algorithm/structure_tensor.hpp"

#include "container/image.hpp"
#include "container/kernel.hpp"
#include "container/ops.hpp"

#include "eval/perf.hpp"

#include "math/num.hpp"
#include "math/vec2.hpp"
#include "math/mat2.hpp"

#include <array>
#include <vector>
#include <queue>


touch_processor::touch_processor(index2_t size)
    : m_perf_reg{}
    , m_perf_t_total{m_perf_reg.create_entry("total")}
    , m_perf_t_prep{m_perf_reg.create_entry("preprocessing")}
    , m_perf_t_st{m_perf_reg.create_entry("structure-tensor")}
    , m_perf_t_stev{m_perf_reg.create_entry("structure-tensor.eigenvalues")}
    , m_perf_t_hess{m_perf_reg.create_entry("hessian")}
    , m_perf_t_rdg{m_perf_reg.create_entry("ridge")}
    , m_perf_t_obj{m_perf_reg.create_entry("objective")}
    , m_perf_t_lmax{m_perf_reg.create_entry("objective.maximas")}
    , m_perf_t_lbl{m_perf_reg.create_entry("labels")}
    , m_perf_t_cscr{m_perf_reg.create_entry("component-score")}
    , m_perf_t_wdt{m_perf_reg.create_entry("distance-transform")}
    , m_perf_t_flt{m_perf_reg.create_entry("filter")}
    , m_perf_t_lmaxf{m_perf_reg.create_entry("filter.maximas")}
    , m_perf_t_gfit{m_perf_reg.create_entry("gaussian-fitting")}
    , m_img_pp{size}
    , m_img_m2_1{size}
    , m_img_m2_2{size}
    , m_img_stev{size}
    , m_img_rdg{size}
    , m_img_obj{size}
    , m_img_lbl{size}
    , m_img_dm1{size}
    , m_img_dm2{size}
    , m_img_flt{size}
    , m_img_gftmp{size}
    , m_wdt_queue{}
    , m_gf_params{}
    , m_maximas{32}
    , m_cstats{32}
    , m_cscore{32}
    , m_kern_pp{alg::conv::kernels::gaussian<f32, 5, 5>(0.9f)}
    , m_kern_st{alg::conv::kernels::gaussian<f32, 5, 5>(1.0f)}
    , m_kern_hs{alg::conv::kernels::gaussian<f32, 5, 5>(1.0f)}
    , m_gf_window{11, 11}
    , m_touchpoints{}
{
    m_wdt_queue = std::priority_queue { std::less<alg::wdt::q_item<f32>>(), [](){
        auto buf = std::vector<alg::wdt::q_item<f32>>{};
        buf.reserve(512);
        return buf;
    }() };

    alg::gfit::reserve(m_gf_params, 32, size);

    m_touchpoints.reserve(32);
}

auto touch_processor::process(container::image<f32> const& hm) -> std::vector<touch_point> const&
{
    auto _tr = m_perf_reg.record(m_perf_t_total);

    // preprocessing
    {
        auto _r = m_perf_reg.record(m_perf_t_prep);

        alg::convolve(m_img_pp, hm, m_kern_pp);

        auto const sum = container::ops::sum(m_img_pp);
        auto const avg = sum / m_img_pp.size().product();

        container::ops::transform(m_img_pp, [&](auto const x) {
            return std::max(x - avg, 0.0f);
        });
    }

    // structure tensor
    {
        auto _r = m_perf_reg.record(m_perf_t_st);

        alg::structure_tensor(m_img_m2_1, m_img_pp);
        alg::convolve(m_img_m2_2, m_img_m2_1, m_kern_st);
    }

    // eigenvalues of structure tensor
    {
        auto _r = m_perf_reg.record(m_perf_t_stev);

        container::ops::transform(m_img_m2_2, m_img_stev, [](auto const s) {
            return s.eigenvalues();
        });
    }

    // hessian
    {
        auto _r = m_perf_reg.record(m_perf_t_hess);

        alg::hessian(m_img_m2_1, m_img_pp);
        alg::convolve(m_img_m2_2, m_img_m2_1, m_kern_hs);
    }

    // ridge measure
    {
        auto _r = m_perf_reg.record(m_perf_t_rdg);

        container::ops::transform(m_img_m2_2, m_img_rdg, [](auto h) {
            auto const [ev1, ev2] = h.eigenvalues();
            return std::max(ev1, 0.0f) + std::max(ev2, 0.0f);
        });
    }

    // objective for labeling
    {
        auto _r = m_perf_reg.record(m_perf_t_obj);

        f32 const wr = 1.5;
        f32 const wh = 1.0;

        for (index_t i = 0; i < m_img_pp.size().product(); ++i) {
            m_img_obj[i] = wh * m_img_pp[i] - wr * m_img_rdg[i];
        }
    }

    // local maximas
    {
        auto _r = m_perf_reg.record(m_perf_t_lmax);

        // TODO: We may want to compute local maximas with a different smoothing factor

        m_maximas.clear();
        alg::find_local_maximas(m_img_pp, 0.05f, std::back_inserter(m_maximas));
    }

    // labels
    u16 num_labels;
    {
        auto _r = m_perf_reg.record(m_perf_t_lbl);

        num_labels = alg::label<4>(m_img_lbl, m_img_obj, 0.0f);
    }

    // component score
    {
        auto _r = m_perf_reg.record(m_perf_t_cscr);

        m_cstats.clear();
        m_cstats.assign(num_labels, component_stats { 0, 0, 0, 0 });

        for (index_t i = 0; i < m_img_pp.size().product(); ++i) {
            auto const label = m_img_lbl[i];

            if (label == 0)
                continue;

            auto const value = m_img_pp[i];
            auto const [ev1, ev2] = m_img_stev[i];

            auto const coherence = ev1 + ev2 != 0.0f ? (ev1 - ev2) / (ev1 + ev2) : 1.0;

            m_cstats.at(label - 1).size += 1;
            m_cstats.at(label - 1).volume += value;
            m_cstats.at(label - 1).incoherence += 1.0f - (coherence * coherence);
        }

        for (auto m : m_maximas) {
            if (m_img_lbl[m] > 0) {
                m_cstats.at(m_img_lbl[m] - 1).maximas += 1;
            }
        }

        m_cscore.assign(num_labels, 0.0f);
        for (index_t i = 0; i < num_labels; ++i) {
            auto const& stats = m_cstats.at(i);

            // size score
            auto const cen_vol = 40.0f;
            auto const spr_vol = 15.0f;
            auto const cov_vol =  0.9f;

            auto const alph_vol = std::log(-(cov_vol + 1.0f) / (cov_vol - 1.0f)) / spr_vol;
            auto const beta_vol = alph_vol * cen_vol;

            auto const s_vol = 1.0f - 1.0f / (1.0f + std::exp(-alph_vol * stats.size + beta_vol));

            // rotation score per size
            auto const cen_rot = 0.4f;
            auto const spr_rot = 0.3f;
            auto const cov_rot = 0.9f;

            auto const alph_rot = std::log(-(cov_rot + 1.0f) / (cov_rot - 1.0f)) / spr_rot;
            auto const beta_rot = alph_rot * cen_rot;

            auto const s_rot = 1.0f / (1.0f + std::exp(-alph_rot * (stats.incoherence / stats.size) + beta_rot));

            // combined score
            m_cscore.at(i) = stats.maximas > 0 ? s_vol * s_rot : 0.0f;
        }
    }

    // TODO: limit inclusion to N (e.g. N=16) local maximas by highest inclusion score
    // TODO: if everything is excluded here, we can skip the rest
    //       (useful to improve performance for touch sensor issues)

    // distance transform
    {
        auto _r = m_perf_reg.record(m_perf_t_wdt);

        auto const th_inc = 0.6f;

        auto const wdt_cost = [&](index_t i, index2_t d) -> f32 {
            f32 const c_dist = 0.1f;
            f32 const c_ridge = 9.0f;
            f32 const c_grad = 1.0f;

            auto const [ev1, ev2] = m_img_stev[i];
            auto const grad = std::max(ev1, 0.0f) + std::max(ev2, 0.0f);
            auto const ridge = m_img_rdg[i];
            auto const dist = std::sqrt(static_cast<f32>(d.x * d.x + d.y * d.y));

            return c_ridge * ridge + c_grad * grad + c_dist * dist;
        };

        auto const wdt_mask = [&](index_t i) -> bool {
            return m_img_pp[i] > 0.0f && m_img_lbl[i] == 0;
        };

        auto const wdt_inc_bin = [&](index_t i) -> bool {
            return m_img_lbl[i] > 0 && m_cscore.at(m_img_lbl[i] - 1) > th_inc;
        };

        auto const wdt_exc_bin = [&](index_t i) -> bool {
            return m_img_lbl[i] > 0 && m_cscore.at(m_img_lbl[i] - 1) <= th_inc;
        };

        alg::weighted_distance_transform<4>(m_img_dm1, wdt_inc_bin, wdt_mask, wdt_cost, m_wdt_queue, 6.0f);
        alg::weighted_distance_transform<4>(m_img_dm2, wdt_exc_bin, wdt_mask, wdt_cost, m_wdt_queue, 6.0f);
    }

    // filter
    {
        auto _r = m_perf_reg.record(m_perf_t_flt);

        for (index_t i = 0; i < m_img_pp.size().product(); ++i) {
            auto const sigma = 1.0f;

            auto w_inc = m_img_dm1[i] / sigma;
            w_inc = std::exp(-w_inc * w_inc);

            auto w_exc = m_img_dm2[i] / sigma;
            w_exc = std::exp(-w_exc * w_exc);

            auto const w_total = w_inc + w_exc;
            auto const w = w_total > 0.0f ? w_inc / w_total : 0.0f;

            m_img_flt[i] = m_img_pp[i] * w;
        }
    }

    // filtered maximas
    {
        auto _r = m_perf_reg.record(m_perf_t_lmaxf);

        // TODO: We may want to compute local maximas with a different smoothing factor

        m_maximas.clear();
        alg::find_local_maximas(m_img_flt, 0.05f, std::back_inserter(m_maximas));
    }

    // gaussian fitting
    if (!m_maximas.empty()) {
        auto _r = m_perf_reg.record(m_perf_t_gfit);

        alg::gfit::reserve(m_gf_params, m_maximas.size(), m_gf_window);

        for (std::size_t i = 0; i < m_maximas.size(); ++i) {
            auto const [x, y] = container::image<f32>::unravel(m_img_pp.size(), m_maximas[i]);

            // TODO: move window inwards instead of clamping?
            auto const bounds = alg::gfit::bbox {
                std::max(x - (m_gf_window.x - 1) / 2, 0),
                std::min(x + (m_gf_window.x - 1) / 2, m_img_pp.size().x - 1),
                std::max(y - (m_gf_window.y - 1) / 2, 0),
                std::min(y + (m_gf_window.y - 1) / 2, m_img_pp.size().y - 1),
            };

            m_gf_params[i].valid  = true;
            m_gf_params[i].scale  = 1.0f;
            m_gf_params[i].mean   = { static_cast<f32>(x), static_cast<f32>(y) };
            m_gf_params[i].prec   = { 1.0f, 0.0f, 1.0f };
            m_gf_params[i].bounds = bounds;
        }

        alg::gfit::fit(m_gf_params, m_img_flt, m_img_gftmp, 3);
    } else {
        for (auto& p : m_gf_params) {
            p.valid = false;
        }
    }

    // generate output
    m_touchpoints.clear();
    for (auto const& p : m_gf_params) {
        if (!p.valid) {
            continue;
        }

        auto const cov = p.prec.inverse();
        if (!cov.has_value()) {
            std::cout << "warning: failed to invert matrix\n";
            continue;
        }

        auto const [ev1, ev2] = cov->eigenvalues();
        auto const sd1 = std::sqrt(std::abs(ev1));
        auto const sd2 = std::sqrt(std::abs(ev2));

        if (sd1 <= math::num<f32>::eps || sd2 <= math::num<f32>::eps) {
            std::cout << "warning: standard deviation too small\n";
            continue;
        }

        auto const aspect = std::max(sd1, sd2) / std::min(sd1, sd2);
        if (aspect > 2.0f) {
            continue;
        }

        auto const x = std::clamp(static_cast<index_t>(p.mean.x), 0, m_img_lbl.size().x - 1);
        auto const y = std::clamp(static_cast<index_t>(p.mean.y), 0, m_img_lbl.size().y - 1);
        auto const cs = m_img_lbl[{ x, y }] > 0 ? m_cscore.at(m_img_lbl[{ x, y }] - 1) : 0.0f;

        m_touchpoints.push_back(touch_point { cs, static_cast<f32>(p.scale), p.mean.cast<f32>(), cov->cast<f32>() });
    }

    return m_touchpoints;
}
