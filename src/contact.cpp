// SPDX-License-Identifier: GPL-2.0-or-later

#include "contact.hpp"

#include "heatmap.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <tuple>

void Cluster::add(int32_t x, int32_t y, int32_t w)
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

std::tuple<float, float> Cluster::mean(void)
{
	float fx = (float)this->x;
	float fy = (float)this->y;
	float fw = (float)this->w;

	return std::tuple<float, float>(fx / fw, fy / fw);
}

std::tuple<float, float, float> Cluster::cov(void)
{
	float fx = (float)this->x;
	float fy = (float)this->y;
	float fxx = (float)this->xx;
	float fyy = (float)this->yy;
	float fxy = (float)this->xy;
	float fw = (float)this->w;

	float r1 = (fxx - (fx * fx / fw)) / fw;
	float r2 = (fyy - (fy * fy / fw)) / fw;
	float r3 = (fxy - (fx * fy / fw)) / fw;

	return std::tuple<float, float, float>(r1, r2, r3);
}

void Cluster::get(Heatmap *hm, int32_t x, int32_t y)
{
	uint8_t v = hm->value(x, y);

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

Cluster::Cluster(Heatmap *hm, int32_t x, int32_t y)
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
	std::tuple<float, float> mean = cluster.mean();
	this->x = std::get<0>(mean);
	this->y = std::get<1>(mean);

	std::tuple<float, float, float> cov = cluster.cov();
	float vx = std::get<0>(cov);
	float vy = std::get<1>(cov);
	float cv = std::get<2>(cov);

	float sqrtd = std::sqrt((vx - vy) * (vx - vy) + 4 * cv * cv);

	this->ev1 = (vx + vy + sqrtd) / 2;
	this->ev2 = (vx + vy - sqrtd) / 2;

	this->qx1 = vx + cv - this->ev2;
	this->qy1 = vy + cv - this->ev2;
	this->qx2 = vx + cv - this->ev1;
	this->qy2 = vy + cv - this->ev1;

	float d1 = std::hypot(this->qx1, this->qy1);
	float d2 = std::hypot(this->qx2, this->qy2);

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

std::tuple<float, float> Contact::pca(float x, float y)
{
	float rx = this->qx1 * x + this->qx2 * y;
	float ry = this->qy1 * x + this->qy2 * y;

	return std::tuple<float, float>(rx, ry);
}

bool Contact::near(Contact o)
{
	std::tuple<float, float> pca = o.pca(this->x - o.x, this->y - o.y);
	float dx = std::get<0>(pca);
	float dy = std::get<1>(pca);

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

	for (int32_t x = 0; x < hm->width; x++) {
		for (int32_t y = 0; y < hm->height; y++) {
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
