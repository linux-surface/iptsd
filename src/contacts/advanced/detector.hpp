#pragma once

#include <common/types.hpp>

#include "algorithm/distance_transform.hpp"
#include "algorithm/gaussian_fitting.hpp"

#include <container/image.hpp>
#include <container/kernel.hpp>

#include <contacts/interface.hpp>

#include <math/vec2.hpp>
#include <math/mat2.hpp>

#include <array>
#include <bit>
#include <vector>
#include <queue>

using namespace iptsd::container;
using namespace iptsd::math;


namespace iptsd::contacts::advanced {

struct ComponentStats {
    u32 size;
    f32 volume;
    f32 incoherence;
    u32 maximas;
};

class MyQueue {
public:
    #define QUEUE_MAX 1024
    //alg::wdt::QItem<f32> data[QUEUE_MAX];
    index_t ind[QUEUE_MAX];
    i32 cost[QUEUE_MAX];
    size_t length{};
    // 0=normal heap, 1=left node is real root, 2=right node is real root
    size_t delay_pop{};

    alg::wdt::QItem<f32> top() {
        // IEEE754 floats are bit compatible with being compared as integers
        // We will just assume that there is nothing special like NaN or Inf
        return {ind[delay_pop], std::bit_cast<f32>(cost[delay_pop])};
    }

    void push(alg::wdt::QItem<f32> item) {
        if (length == QUEUE_MAX) {
            throw std::runtime_error("MyQueue overflow");
        }
        i32 item_cost = std::bit_cast<i32>(item.cost);
        index_t item_ind = item.idx;
        if (delay_pop) {
            delay_pop = 0;
            downheap(item_cost, item_ind);
            ++length;
            return;
        }
        size_t idx{length++};
        while (idx) {
            size_t next{(idx - 1) / 2};
            i32 parent = cost[next];
            if (parent < item_cost) {
                cost[idx] = parent;
                ind[idx] = ind[next];
            } else break;
            idx = next;
        }
        ind[idx] = item_ind;
        cost[idx] = item_cost;
    }

    [[nodiscard]] bool empty() const {
        return !length;
    }

    void pop() {
        size_t len = length--;
        if (delay_pop) {
            downheap(cost[len], ind[len]);
        }
        if (!len) {
            delay_pop = 0;
            return;
        }
        delay_pop = 1;
        if (len == 1) return;
        if (cost[2] > cost[1])
            delay_pop = 2;
    }

    void downheap(i32 item_cost, index_t item_ind) {
        size_t len = length + 1;
        size_t idx{0};
        for (;;) {
            size_t left_idx = 2 * idx + 1;
            if (left_idx >= len) break;
            i32 left = cost[left_idx];
            size_t right_idx = 2 * idx + 2;
            if (right_idx >= len) {
                if (left > item_cost) {
                    cost[idx] = left;
                    ind[idx] = ind[left_idx];
                    idx = left_idx;
                }
                break;
            }
            i32 right = cost[right_idx];
            if (left > item_cost) {
                if (right > left) {
                    cost[idx] = right;
                    ind[idx] = ind[right_idx];
                    idx = right_idx;
                } else {
                    cost[idx] = left;
                    ind[idx] = ind[left_idx];
                    idx = left_idx;
                }
            } else if (right > item_cost) {
                cost[idx] = right;
                ind[idx] = ind[right_idx];
                idx = right_idx;
            } else break;
        }
        cost[idx] = item_cost;
        ind[idx] = item_ind;
    }
};


//class MyQueue {
//public:
//#define QUEUE_MAX 3072
//    alg::wdt::QItem<f32> data[QUEUE_MAX];
//    size_t length{};
//
//    alg::wdt::QItem<f32> top() {
//        return data[0];
//    }
//
//    void push(alg::wdt::QItem<f32> item) {
//        size_t idx{length++};
//        while (idx) {
//            size_t next{(idx - 1) / 2};
//            alg::wdt::QItem<f32> parent = data[next];
//            if (parent < item) {
//                data[idx] = parent;
//            } else break;
//            idx = next;
//        }
//        data[idx] = item;
//    }
//
//    [[nodiscard]] bool empty() const {
//        return !length;
//    }
//
//    void pop() {
//        size_t idx{0};
//        size_t len = --length;
//        alg::wdt::QItem<f32> item = data[len];
//        for (;;) {
//            size_t left_idx = 2 * idx + 1;
//            if (left_idx >= len) break;
//            alg::wdt::QItem<f32> left = data[left_idx];
//            size_t right_idx = 2 * idx + 2;
//            if (right_idx >= len) {
//                if (left > item) {
//                    data[idx] = left;
//                    idx = left_idx;
//                }
//                break;
//            }
//            alg::wdt::QItem<f32> right = data[right_idx];
//            if (left > item) {
//                if (right > left) {
//                    data[idx] = right;
//                    idx = right_idx;
//                } else {
//                    data[idx] = left;
//                    idx = left_idx;
//                }
//            } else if (right > item) {
//                data[idx] = right;
//                idx = right_idx;
//            } else break;
//        }
//        data[idx] = item;
//    }
//};

class BlobDetector : public IBlobDetector {
public:
    BlobDetector(index2_t size, BlobDetectorConfig config);

    auto data() -> Image<f32> & override;
    auto search() -> std::vector<Blob> const& override;

private:
    auto process(Image<f32> const& hm) -> std::vector<Blob> const&;

    BlobDetectorConfig config;

    // temporary storage
    Image<f32> m_hm;
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

    MyQueue m_wdt_queue;
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
    std::vector<Blob> m_touchpoints;
};

inline auto BlobDetector::data() -> Image<f32> &
{
    return m_hm;
}

inline auto BlobDetector::search() -> std::vector<Blob> const&
{
	return this->process(this->m_hm);
}

} /* namespace iptsd::contacts::advanced */
