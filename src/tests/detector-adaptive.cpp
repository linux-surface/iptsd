// SPDX-License-Identifier: GPL-2.0-or-later

// Tests for the adaptive neutral mode of the contact detector, with heatmaps shaped like the
// Surface Book 2 13" (IPTS EDS1) touchscreen at rest.

#include <common/types.hpp>
#include <contacts/contact.hpp>
#include <contacts/detection/config.hpp>
#include <contacts/detection/detector.hpp>

#include <spdlog/spdlog.h>

#include <array>
#include <cstdlib>
#include <exception>
#include <vector>

namespace {

using namespace iptsd;
using namespace iptsd::contacts;

constexpr Eigen::Index Rows = 48;
constexpr Eigen::Index Cols = 72;

constexpr f64 Rest = 74.0 / 255.0;
constexpr f64 Max = 1.0;

// The cells of row 31 that read 255 at rest on the Surface Book 2, next to rows 30 and 32
// that read 255 in all but their last cell (recorded with iptsd-dump, 28 Nov 2026).
constexpr std::array<Eigen::Index, 31> HighCells {
	1,  3,  9,  12, 13, 17, 21, 24, 29, 31, 34, 36, 37, 38, 42, 45,
	46, 48, 52, 53, 55, 56, 57, 58, 60, 62, 63, 65, 66, 69, 70,
};

bool check(const bool ok, const char *what)
{
	if (ok)
		spdlog::info("PASS: {}", what);
	else
		spdlog::error("FAIL: {}", what);

	return ok;
}

detection::Detector<f64> make_detector()
{
	detection::Config<f64> config {};
	config.normalize = true;
	config.neutral_value_algorithm = detection::neutral::Algorithm::ADAPTIVE;
	config.activation_threshold = 40.0 / 255.0;
	config.deactivation_threshold = 36.0 / 255.0;
	config.neutral_value_backoff = 16;
	return detection::Detector<f64> {config};
}

// The resting heatmap: rows 30 and 32 at 255 but for their last cell, row 31 with HighCells at
// 255, the rest at 74.
Image<f64> resting()
{
	Image<f64> img = Image<f64>::Constant(Rows, Cols, Rest);
	img.row(30).head(Cols - 1).setConstant(Max);
	img.row(32).head(Cols - 1).setConstant(Max);

	for (const Eigen::Index x : HighCells)
		img(31, x) = Max;

	return img;
}

usize count(detection::Detector<f64> &detector, const Image<f64> &img)
{
	std::vector<Contact<f64>> contacts {};
	detector.detect(img, contacts);
	return contacts.size();
}

// For under a second, all HighCells drop to the resting level together, then return to 255.
// The recording had four such dips in an hour, each 9 frames long.
bool row_31_dips_do_not_leave_a_contact()
{
	auto detector = make_detector();
	const Image<f64> rest = resting();

	Image<f64> dip = rest;
	for (const Eigen::Index x : HighCells)
		dip(31, x) = Rest;

	usize before = 0;
	for (int i = 0; i < 100; i++)
		before += count(detector, rest);

	bool ok = check(before == 0, "the resting heatmap has no contact");

	for (int d = 0; d < 4; d++) {
		for (int i = 0; i < 9; i++)
			count(detector, dip);

		usize after = 0;
		for (int i = 0; i < 100; i++)
			after += count(detector, rest);

		spdlog::info("dip {}: {} contacts in the 100 resting frames after it",
		             d + 1,
		             after);
		ok &= check(after == 0, "a short dip of the cells at 255 leaves no contact behind");
	}

	return ok;
}

// A fingertip: a 3 by 3 blob well above the resting level, away from the rows at 255.
Image<f64> with_finger(Image<f64> img)
{
	for (Eigen::Index y = 10; y <= 12; y++) {
		for (Eigen::Index x = 20; x <= 22; x++)
			img(y, x) = 160.0 / 255.0;
	}

	img(11, 21) = 200.0 / 255.0;
	return img;
}

// A finger that rests on the screen for a long time is still a contact.
bool finger_held_still_stays_a_contact()
{
	auto detector = make_detector();
	const Image<f64> rest = resting();
	const Image<f64> touch = with_finger(rest);

	for (int i = 0; i < 100; i++)
		count(detector, rest);

	usize missed = 0;
	for (int i = 0; i < 2000; i++)
		missed += count(detector, touch) == 1 ? 0 : 1;

	return check(missed == 0, "a finger held still for 2000 frames is detected in every frame");
}

} // namespace

int main()
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		bool ok = row_31_dips_do_not_leave_a_contact();
		ok &= finger_held_still_stays_a_contact();
		return ok ? EXIT_SUCCESS : EXIT_FAILURE;
	} catch (const std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
