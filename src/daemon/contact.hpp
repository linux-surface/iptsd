/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_DAEMON_CONTACT_HPP_
#define _IPTSD_DAEMON_CONTACT_HPP_

#include "heatmap.hpp"

#include <common/types.hpp>

#include <cstddef>
#include <tuple>
#include <vector>

class Cluster {
public:
	i64 x;
	i64 y;
	i64 xx;
	i64 yy;
	i64 xy;
	i64 w;
	i64 max_v;

	Cluster(Heatmap *hm, i32 x, i32 y);

	void add(i32 x, i32 y, i32 w);
	std::tuple<f32, f32> mean(void);
	std::tuple<f32, f32, f32> cov(void);

private:
	void get(Heatmap *hm, i32 x, i32 y);
};

class Contact {
public:
	f32 x;
	f32 y;
	f32 ev1;
	f32 ev2;
	f32 qx1;
	f32 qy1;
	f32 qx2;
	f32 qy2;
	f32 angle;
	i64 max_v;
	bool is_palm;

	void from_cluster(Cluster cluster);
	std::tuple<f32, f32> pca(f32 x, f32 y);
	bool near(Contact other);

	static size_t find_contacts(Heatmap *hm, std::vector<Contact> &contacts);
};

#endif /* _IPTSD_DAEMON_CONTACT_HPP_ */
