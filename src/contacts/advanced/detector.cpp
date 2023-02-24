#include "detector.hpp"

#include "../neutral.hpp"

#include <common/types.hpp>

#include "algorithm/convolution.hpp"
#include "algorithm/distance_transform.hpp"
#include "algorithm/gaussian_fitting.hpp"
#include "algorithm/hessian.hpp"
#include "algorithm/label.hpp"
#include "algorithm/local_maxima.hpp"
#include "algorithm/structure_tensor.hpp"

#include <container/image.hpp>
#include <container/kernel.hpp>
#include <container/ops.hpp>

#include <gsl/gsl>
#include <math/num.hpp>
#include <math/vec2.hpp>
#include <math/mat2.hpp>

#include <spdlog/spdlog.h>

#include <array>
#include <vector>
#include <queue>

using namespace iptsd::container;
using namespace iptsd::math;


namespace iptsd::contacts::advanced {

BlobDetector::BlobDetector(index2_t size, BlobDetectorConfig config)
    : config{config}
    , m_hm{size}
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
    std::less<alg::wdt::QItem<f32>> op {};
    std::vector<alg::wdt::QItem<f32>> buf {512};

    m_wdt_queue = std::priority_queue { op, buf };

    alg::gfit::reserve(m_gf_params, 32, size);
}

class WdtCost {
public:
    WdtCost(Image<std::array<f32, 2>> const& m_img_stev_,
        Image<f32> const& m_img_rdg_) : m_img_stev{m_img_stev_}
    , m_img_rdg{m_img_rdg_} {}

    template<index_t DX, index_t DY>
    [[gnu::always_inline]] [[nodiscard]] f32 get_cost(index_t i) const
    {
        f32 constexpr c_dist = 0.1f;
        f32 constexpr c_ridge = 9.0f;
        f32 constexpr c_grad = 1.0f;

        static_assert(DX == -1 || DX == 0 || DX == 1);
        static_assert(DY == -1 || DY == 0 || DY == 1);
        // auto const dist = std::hypotf(gsl::narrow<f32>(d.x), gsl::narrow<f32>(d.y));
        f32 constexpr dist = DX * DY == 0 ? 1 : M_SQRT2;

        auto const [ev1, ev2] = common::unchecked<std::array<f32, 2>>(m_img_stev.get(), i);
        auto const grad = std::max(ev1, 0.0f) + std::max(ev2, 0.0f);
        auto const ridge = common::unchecked<f32>(m_img_rdg.get(), i);

        return c_ridge * ridge + c_grad * grad + c_dist * dist;
    }

private:
    std::reference_wrapper<const Image<std::array<f32, 2>>> m_img_stev;
    std::reference_wrapper<const Image<f32>> m_img_rdg;
};

auto BlobDetector::process(Image<f32> const& hm) -> std::vector<Blob> const&
{
    // preprocessing
    {
        alg::convolve(m_img_pp, hm, m_kern_pp);

        auto const nval = neutral(this->config, hm);

        container::ops::transform(m_img_pp, [&](auto const x) {
            return std::max(x - nval, 0.0f);
        });
    }

    // structure tensor
    {
        alg::structure_tensor(m_img_m2_1, m_img_pp);
        alg::convolve(m_img_m2_2, m_img_m2_1, m_kern_st);
    }

    // eigenvalues of structure tensor
    {
        container::ops::transform(m_img_m2_2, m_img_stev, [](auto const s) {
            return s.eigenvalues();
        });
    }

    // hessian
    {
        alg::hessian(m_img_m2_1, m_img_pp);
        alg::convolve(m_img_m2_2, m_img_m2_1, m_kern_hs);
    }

    // ridge measure
    {
        container::ops::transform(m_img_m2_2, m_img_rdg, [](auto h) {
            auto const [ev1, ev2] = h.eigenvalues();
            return std::max(ev1, 0.0f) + std::max(ev2, 0.0f);
        });
    }

    // objective for labeling
    {
        f32 const wr = 1.5;
        f32 const wh = 1.0;

        for (index_t i = 0; i < m_img_pp.size().span(); ++i) {
            m_img_obj[i] = wh * m_img_pp[i] - wr * m_img_rdg[i];
        }
    }

    // local maximas
    {
        // TODO: We may want to compute local maximas with a different smoothing factor

        m_maximas.clear();
        alg::find_local_maximas(m_img_pp, 0.05f, std::back_inserter(m_maximas));
    }

    // labels
    u16 num_labels = 0;
    {
        num_labels = alg::label<4>(m_img_lbl, m_img_obj, 0.0f);
    }

    // component score
    {
        m_cstats.clear();
        m_cstats.assign(num_labels, ComponentStats { 0, 0, 0, 0 });

        for (index_t i = 0; i < m_img_pp.size().span(); ++i) {
            auto const label = m_img_lbl[i];

            if (label == 0)
                continue;

            auto const value = m_img_pp[i];
            auto const [ev1, ev2] = m_img_stev[i];

            auto const coherence = ev1 + ev2 != 0.0f ? (ev1 - ev2) / (ev1 + ev2) : 1.0;

            m_cstats.at(label - 1).size += 1;
            m_cstats.at(label - 1).volume += value;
            m_cstats.at(label - 1).incoherence += 1.0f - gsl::narrow_cast<f32>(coherence * coherence);
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

            auto const s_vol = 1.0f - 1.0f / (1.0f + std::exp(-alph_vol * gsl::narrow<f32>(stats.size) + beta_vol));

            // rotation score per size
            auto const cen_rot = 0.4f;
            auto const spr_rot = 0.3f;
            auto const cov_rot = 0.9f;

            auto const alph_rot = std::log(-(cov_rot + 1.0f) / (cov_rot - 1.0f)) / spr_rot;
            auto const beta_rot = alph_rot * cen_rot;

            auto const s_rot = 1.0f / (1.0f + std::exp(-alph_rot * (stats.incoherence / gsl::narrow<f32>(stats.size)) + beta_rot));

            // combined score
            m_cscore.at(i) = stats.maximas > 0 ? s_vol * s_rot : 0.0f;
        }
    }

    // TODO: limit inclusion to N (e.g. N=16) local maximas by highest inclusion score
    // TODO: if everything is excluded here, we can skip the rest
    //       (useful to improve performance for touch sensor issues)

    // distance transform
    {
        auto const th_inc = 0.6f;
        const WdtCost wdt_cost {m_img_stev, m_img_rdg};

        auto const wdt_mask = [&](index_t i) -> bool {
            return common::unchecked<f32>(m_img_pp, i) > 0.0f && common::unchecked<u16>(m_img_lbl, i) == 0;
        };

        auto const wdt_inc_bin = [&](index_t i) -> bool {
            u16 const lbl = common::unchecked<u16>(m_img_lbl, i);
            return lbl > 0 && common::unchecked<f32>(m_cscore, lbl - 1) > th_inc;
        };

        auto const wdt_exc_bin = [&](index_t i) -> bool {
            u16 const lbl = common::unchecked<u16>(m_img_lbl, i);
            return lbl && common::unchecked<f32>(m_cscore, lbl - 1) <= th_inc;
        };

        alg::weighted_distance_transform<4>(m_img_dm1, wdt_inc_bin, wdt_mask, wdt_cost, m_wdt_queue, 6.0f);
        alg::weighted_distance_transform<4>(m_img_dm2, wdt_exc_bin, wdt_mask, wdt_cost, m_wdt_queue, 6.0f);
    }

    // filter
    {
        for (index_t i = 0; i < m_img_pp.size().span(); ++i) {
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
        // TODO: We may want to compute local maximas with a different smoothing factor

        m_maximas.clear();
        alg::find_local_maximas(m_img_flt, 0.05f, std::back_inserter(m_maximas));
    }

    // gaussian fitting
    if (!m_maximas.empty()) {
        alg::gfit::reserve(m_gf_params, m_maximas.size(), m_gf_window);

        for (std::size_t i = 0; i < m_maximas.size(); ++i) {
            auto const [x, y] = Image<f32>::unravel(m_img_pp.size(), m_maximas[i]);

            // TODO: move window inwards instead of clamping?
            auto const bounds = alg::gfit::BBox {
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
            spdlog::warn("failed to invert matrix");
            continue;
        }

        auto const [ev1, ev2] = cov->eigenvalues();
        auto const sd1 = std::sqrt(std::abs(ev1));
        auto const sd2 = std::sqrt(std::abs(ev2));

        if (sd1 <= math::num<f32>::eps || sd2 <= math::num<f32>::eps) {
            spdlog::warn("standard deviation too small");
            continue;
        }

        m_touchpoints.push_back(Blob { p.mean + 0.5, cov.value() });
    }

    return m_touchpoints;
}

} /* namespace iptsd::contacts::advanced */
