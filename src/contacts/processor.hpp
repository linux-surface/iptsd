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


namespace iptsd {

struct TouchPoint {
    f32        confidence;
    f32        scale;
    Vec2<f32>  mean;
    Mat2s<f32> cov;
};


struct ComponentStats {
    u32 size;
    f32 volume;
    f32 incoherence;
    u32 maximas;
};


class TouchProcessor {
public:
    TouchProcessor(index2_t size);

    auto process(Image<f32> const& hm) -> std::vector<TouchPoint> const&;
    auto perf() const -> eval::perf::Registry const&;

private:
    // performance measurements
    eval::perf::Registry m_perf_reg;
    eval::perf::Token m_perf_t_total;
    eval::perf::Token m_perf_t_prep;
    eval::perf::Token m_perf_t_st;
    eval::perf::Token m_perf_t_stev;
    eval::perf::Token m_perf_t_hess;
    eval::perf::Token m_perf_t_rdg;
    eval::perf::Token m_perf_t_obj;
    eval::perf::Token m_perf_t_lmax;
    eval::perf::Token m_perf_t_lbl;
    eval::perf::Token m_perf_t_cscr;
    eval::perf::Token m_perf_t_wdt;
    eval::perf::Token m_perf_t_flt;
    eval::perf::Token m_perf_t_lmaxf;
    eval::perf::Token m_perf_t_gfit;

    // temporary storage
    Image<f32> m_img_pp;
    Image<Mat2s<f32>> m_img_m2_1;
    Image<Mat2s<f32>> m_img_m2_2;
    Image<std::array<f32, 2>> m_img_stev;
    Image<f32> m_img_rdg;
    Image<f32> m_img_obj;
    Image<u16> m_img_lbl;
    Image<f32> m_img_dm1;
    Image<f32> m_img_dm2;
    Image<f32> m_img_flt;
    Image<f64> m_img_gftmp;

    std::priority_queue<alg::wdt::QItem<f32>> m_wdt_queue;
    std::vector<alg::gfit::Parameters<f64>> m_gf_params;

    std::vector<index_t> m_maximas;
    std::vector<ComponentStats> m_cstats;
    std::vector<f32> m_cscore;

    // gauss kernels
    Kernel<f32, 5, 5> m_kern_pp;
    Kernel<f32, 5, 5> m_kern_st;
    Kernel<f32, 5, 5> m_kern_hs;

    // parameters
    index2_t m_gf_window;

    // output
    std::vector<TouchPoint> m_touchpoints;
};


inline auto TouchProcessor::perf() const -> eval::perf::Registry const&
{
    return m_perf_reg;
}

} /* namespace iptsd */
