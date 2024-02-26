// SDPX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_PROTOCOL_DFT_HPP
#define IPTSD_IPTS_PROTOCOL_DFT_HPP

#include <common/types.hpp>

#include <array>

namespace iptsd::ipts::protocol::dft {

constexpr u8 NUM_COMPONENTS = 9;
constexpr u8 MAX_ROWS = 16;
constexpr u8 PRESSURE_ROWS = 6;

enum class Type : u8 {
	Position = 0x6,
	PositionMPP_2 = 0x7,
	Button = 0x9,
	BinaryMPP_2 = 0xA,
	Pressure = 0xB,
};

struct [[gnu::packed]] Metadata {
	u32 group_counter;
	u8 seq_num;
	Type data_type;
	std::array<u8, 10> reserved;
};
static_assert(sizeof(Metadata) == 16);

struct [[gnu::packed]] Window {
	u32 timestamp;
	u8 num_rows;
	u8 seq_num;
	std::array<u8, 3> reserved1;
	Type data_type;
	std::array<u8, 2> reserved2;
};
static_assert(sizeof(Window) == 12);

struct [[gnu::packed]] Row {
	u32 frequency;
	u32 magnitude;
	std::array<i16, NUM_COMPONENTS> real;
	std::array<i16, NUM_COMPONENTS> imag;
	i8 first;
	i8 last;
	i8 mid;
	i8 zero;
};
static_assert(sizeof(Row) == 48);

} // namespace iptsd::ipts::protocol::dft

#endif // IPTSD_IPTS_PROTOCOL_DFT_HPP
