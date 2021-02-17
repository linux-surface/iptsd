#pragma once

#include "types.hpp"

#include "algorithm/distance_transform.hpp"
#include "algorithm/gaussian_fitting.hpp"

#include "container/image.hpp"
#include "container/kernel.hpp"

#include "eval/perf.hpp"

#include "math/vec2.hpp"
#include "math/mat2.hpp"

#include <array>
#include <vector>
#include <queue>


struct touch_point {
    f32 confidence;
    f32 scale;
    math::vec2_t<f32>  mean;
    math::mat2s_t<f32> cov;
};


struct component_stats {
    u32 size;
    f32 volume;
    f32 incoherence;
    u32 maximas;
};


class touch_processor {
public:
    touch_processor(index2_t size);

    auto process(container::image<f32> const& hm) -> std::vector<touch_point> const&;
    auto perf() const -> eval::perf::registry const&;

private:
    // performance measurements
    eval::perf::registry m_perf_reg;
    eval::perf::token m_perf_t_total;
    eval::perf::token m_perf_t_prep;
    eval::perf::token m_perf_t_st;
    eval::perf::token m_perf_t_stev;
    eval::perf::token m_perf_t_hess;
    eval::perf::token m_perf_t_rdg;
    eval::perf::token m_perf_t_obj;
    eval::perf::token m_perf_t_lmax;
    eval::perf::token m_perf_t_lbl;
    eval::perf::token m_perf_t_cscr;
    eval::perf::token m_perf_t_wdt;
    eval::perf::token m_perf_t_flt;
    eval::perf::token m_perf_t_lmaxf;
    eval::perf::token m_perf_t_gfit;

    // temporary storage
    container::image<f32> m_img_pp;
    container::image<math::mat2s_t<f32>> m_img_m2_1;
    container::image<math::mat2s_t<f32>> m_img_m2_2;
    container::image<std::array<f32, 2>> m_img_stev;
    container::image<f32> m_img_rdg;
    container::image<f32> m_img_obj;
    container::image<u16> m_img_lbl;
    container::image<f32> m_img_dm1;
    container::image<f32> m_img_dm2;
    container::image<f32> m_img_flt;
    container::image<f64> m_img_gftmp;

    std::priority_queue<alg::wdt::q_item<f32>> m_wdt_queue;
    std::vector<alg::gfit::parameters<f64>> m_gf_params;

    std::vector<index_t> m_maximas;
    std::vector<component_stats> m_cstats;
    std::vector<f32> m_cscore;

    // gauss kernels
    container::kernel<f32, 5, 5> m_kern_pp;
    container::kernel<f32, 5, 5> m_kern_st;
    container::kernel<f32, 5, 5> m_kern_hs;

    // parameters
    index2_t m_gf_window;

    // output
    std::vector<touch_point> m_touchpoints;
};


inline auto touch_processor::perf() const -> eval::perf::registry const&
{
    return m_perf_reg;
}
