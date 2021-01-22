/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_CONTACT_HPP_
#define _IPTSD_CONTACT_HPP_

#include "heatmap.hpp"

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <vector>

class Cluster {
public:
	int64_t x;
	int64_t y;
	int64_t xx;
	int64_t yy;
	int64_t xy;
	int64_t w;
	int64_t max_v;

	Cluster(Heatmap *hm, int32_t x, int32_t y);

	void add(int32_t x, int32_t y, int32_t w);
	std::tuple<float, float> mean(void);
	std::tuple<float, float, float> cov(void);

private:
	void get(Heatmap *hm, int32_t x, int32_t y);
};

class Contact {
public:
	float x;
	float y;
	float ev1;
	float ev2;
	float qx1;
	float qy1;
	float qx2;
	float qy2;
	float angle;
	int64_t max_v;
	bool is_palm;

	void from_cluster(Cluster cluster);
	std::tuple<float, float> pca(float x, float y);
	bool near(Contact other);

	static size_t find_contacts(Heatmap *hm, std::vector<Contact> &contacts);
};

#endif /* _IPTSD_CONTACT_HPP_ */
