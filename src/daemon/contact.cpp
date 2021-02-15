// SPDX-License-Identifier: GPL-2.0-or-later

#include "contact.hpp"

#include "heatmap.hpp"

#include <common/types.hpp>

#include <cmath>
#include <cstddef>
#include <iterator>
#include <memory>
#include <tuple>

void Cluster::add(i32 x, i32 y, i32 w)
{
	this->x += w * x;
	this->y += w * y;
	this->xx += w * x * x;
	this->yy += w * y * y;
	this->xy += w * x * y;
	this->w += w;

	if (this->max_v < w)
		this->max_v = w;
}

std::tuple<f32, f32> Cluster::mean(void)
{
	f32 fx = (f32)this->x;
	f32 fy = (f32)this->y;
	f32 fw = (f32)this->w;

	return std::tuple<f32, f32>(fx / fw, fy / fw);
}

std::tuple<f32, f32, f32> Cluster::cov(void)
{
	f32 fx = (f32)this->x;
	f32 fy = (f32)this->y;
	f32 fxx = (f32)this->xx;
	f32 fyy = (f32)this->yy;
	f32 fxy = (f32)this->xy;
	f32 fw = (f32)this->w;

	f32 r1 = (fxx - (fx * fx / fw)) / fw;
	f32 r2 = (fyy - (fy * fy / fw)) / fw;
	f32 r3 = (fxy - (fx * fy / fw)) / fw;

	return std::tuple<f32, f32, f32>(r1, r2, r3);
}

void Cluster::get(Heatmap *hm, i32 x, i32 y)
{
	u8 v = hm->value(x, y);

	if (!hm->is_touch(x, y))
		return;

	if (hm->get_visited(x, y))
		return;

	this->add(x, y, v);
	hm->set_visited(x, y, true);

	this->get(hm, x + 1, y);
	this->get(hm, x - 1, y);
	this->get(hm, x, y + 1);
	this->get(hm, x, y - 1);
}

Cluster::Cluster(Heatmap *hm, i32 x, i32 y)
{
	this->x = 0;
	this->y = 0;
	this->xx = 0;
	this->xy = 0;
	this->yy = 0;
	this->w = 0;
	this->max_v = 0;

	this->get(hm, x, y);
}

void Contact::from_cluster(Cluster cluster)
{
	std::tuple<f32, f32> mean = cluster.mean();
	this->x = std::get<0>(mean);
	this->y = std::get<1>(mean);

	std::tuple<f32, f32, f32> cov = cluster.cov();
	f32 vx = std::get<0>(cov);
	f32 vy = std::get<1>(cov);
	f32 cv = std::get<2>(cov);

	f32 sqrtd = std::sqrt((vx - vy) * (vx - vy) + 4 * cv * cv);

	this->ev1 = (vx + vy + sqrtd) / 2;
	this->ev2 = (vx + vy - sqrtd) / 2;

	this->qx1 = vx + cv - this->ev2;
	this->qy1 = vy + cv - this->ev2;
	this->qx2 = vx + cv - this->ev1;
	this->qy2 = vy + cv - this->ev1;

	f32 d1 = std::hypot(this->qx1, this->qy1);
	f32 d2 = std::hypot(this->qx2, this->qy2);

	this->qx1 /= d1;
	this->qy1 /= d1;
	this->qx2 /= d2;
	this->qy2 /= d2;

	this->max_v = cluster.max_v;
	this->is_palm = false;

	this->angle = M_PI_2 - std::atan2(this->qy1, this->qx1);
	if (this->angle < 0)
		this->angle += M_PI;
	if (this->angle > M_PI)
		this->angle -= M_PI;
}

std::tuple<f32, f32> Contact::pca(f32 x, f32 y)
{
	f32 rx = this->qx1 * x + this->qx2 * y;
	f32 ry = this->qy1 * x + this->qy2 * y;

	return std::tuple<f32, f32>(rx, ry);
}

bool Contact::near(Contact o)
{
	std::tuple<f32, f32> pca = o.pca(this->x - o.x, this->y - o.y);
	f32 dx = std::get<0>(pca);
	f32 dy = std::get<1>(pca);

	dx = std::abs(dx);
	dy = std::abs(dy);

	dx /= 3.2 * std::sqrt(o.ev1) + 8;
	dy /= 3.2 * std::sqrt(o.ev2) + 8;

	return dx * dx + dy * dy <= 1;
}

size_t Contact::find_contacts(Heatmap *hm, std::vector<Contact> &contacts)
{
	size_t c = 0;

	if (std::size(contacts) == 0)
		return 0;

	for (size_t i = 0; i < hm->size; i++)
		hm->visited[i] = false;

	for (i32 x = 0; x < hm->width; x++) {
		for (i32 y = 0; y < hm->height; y++) {
			if (!hm->is_touch(x, y))
				continue;

			if (hm->get_visited(x, y))
				continue;

			Cluster cluster = Cluster(hm, x, y);
			contacts[c].from_cluster(cluster);

			// ignore 0 variance contacts
			if (contacts[c].ev2 > 0)
				c++;

			if (c == std::size(contacts))
				return c;
		}
	}

	return c;
}
