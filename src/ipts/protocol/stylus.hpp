// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_PROTOCOL_STYLUS_HPP
#define IPTSD_IPTS_PROTOCOL_STYLUS_HPP

#include <common/types.hpp>

#include <array>

namespace iptsd::ipts::protocol::stylus {

constexpr u16 MAX_X = 9600;
constexpr u16 MAX_Y = 7200;

constexpr u16 MAX_PRESSURE_MPP_1_0 = 1024;
constexpr u16 MAX_PRESSURE_MPP_1_51 = 4096;

/*!
 * A stylus report. This header is followed by one or more samples of the position and state
 * of the stylus. The sample type is determined by the type of the enclosing report frame.
 */
struct [[gnu::packed]] Report {
	//! The number of samples contained in this report.
	u8 samples;

	//! ...
	std::array<u8, 3> reserved;

	//! Something like a serial number of the stylus.
	//! Doesn't appear to be reliable with multiple styli though.
	u32 serial;
};
static_assert(sizeof(Report) == 8);

/*!
 * The state that the stylus is currently being used in.
 *
 * This is templated, because the underlying fields can have different sizes.
 *
 * @tparam Base The base type to use for this bitfield, e.g. u8 or u16.
 */
template <class Base>
struct [[gnu::packed]] State {
	//! Whether the stylus is near the display and sending data.
	//! When moving out of proximity a single sample will be generated with this being 0.
	bool proximity : 1;

	//! Whether the stylus tip is making contact with the display.
	//! Will always be 0 in rubber mode. Use pressure for reliable contact detection.
	bool contact : 1;

	//! Whether the side button of the stylus is being pressed.
	bool button : 1;

	//! Whether the stylus is being used in rubber mode.
	bool rubber : 1;

	//! Fill up the struct to the desired base type size.
	Base reserved : (sizeof(Base) * 8) - 4;
};
static_assert(sizeof(State<u8>) == sizeof(u8));
static_assert(sizeof(State<u16>) == sizeof(u16));

/*!
 * The position and state of an MPP 1.0 stylus.
 *
 * MPP 1.0 styli support 1024 levels of pressure and send no information about orientation.
 */
struct [[gnu::packed]] SampleMPP_1_0 {
	//! ...
	std::array<u8, 4> reserved1;

	//! The state that the stylus is currently in.
	State<u8> state;

	//! The x coordinate of the stylus tip on the display.
	//! Range: 0 to @ref MAX_X
	u16 x;

	//! The y coordinate of the stylus tip on the display.
	//! Range: 0 to @ref MAX_Y
	u16 y;

	//! How hard the stylus is being pressed on the display.
	//! Range: 0 to @ref MAX_PRESSURE_MPP_1_0
	u16 pressure;

	//! ...
	std::array<u8, 1> reserved2;
};
static_assert(sizeof(SampleMPP_1_0) == 12);

/*!
 * The position and state of an MPP 1.51 (or later) stylus.
 *
 * MPP 1.51 styli support 4096 levels of pressure and send information about the tip orientation.
 */
struct [[gnu::packed]] SampleMPP_1_51 {
	//! A number that gets incremented with each sample.
	u16 timestamp;

	//! The state that the stylus is currently in.
	State<u16> state;

	//! The x coordinate of the stylus tip on the display.
	//! Range: 0 to @ref MAX_X
	u16 x;

	//! The y coordinate of the stylus tip on the display.
	//! Range: 0 to @ref MAX_Y
	u16 y;

	//! How hard the stylus is being pressed on the display.
	//! Range: 0 to @ref MAX_PRESSURE_MPP_1_51
	u16 pressure;

	//! The angle between the stylus tip and the display.
	//! Unit: degrees * 100
	u16 altitude;

	//! The direction in which the stylus tip is pointing.
	//! Unit: degrees * 100
	u16 azimuth;

	//! ...
	std::array<u8, 2> reserved;
};
static_assert(sizeof(SampleMPP_1_51) == 16);

}; // namespace iptsd::ipts::protocol::stylus

#endif // IPTSD_IPTS_PROTOCOL_STYLUS_HPP
