#pragma once

#include <contacts/types.hpp>

#include <functional>
#include <gtk/gtk.h>
#include <utility>

namespace iptsd::gfx::gdk {

enum class Gravity {
	NorthWest = GDK_GRAVITY_NORTH_WEST,
	North = GDK_GRAVITY_NORTH,
	NorthEast = GDK_GRAVITY_NORTH_EAST,
	West = GDK_GRAVITY_WEST,
	Center = GDK_GRAVITY_CENTER,
	East = GDK_GRAVITY_EAST,
	SouthWest = GDK_GRAVITY_SOUTH_WEST,
	South = GDK_GRAVITY_SOUTH,
	SouthEast = GDK_GRAVITY_SOUTH_EAST,
	Static = GDK_GRAVITY_STATIC,
};

enum class WindowHints {
	Pos = GDK_HINT_POS,
	MinSize = GDK_HINT_MIN_SIZE,
	MaxSize = GDK_HINT_MAX_SIZE,
	BaseSize = GDK_HINT_BASE_SIZE,
	Aspect = GDK_HINT_ASPECT,
	ResiceInc = GDK_HINT_RESIZE_INC,
	WinGravity = GDK_HINT_WIN_GRAVITY,
	UserPos = GDK_HINT_USER_POS,
	UserSize = GDK_HINT_USER_SIZE,
};

struct Geometry {
public:
	i32 min_width;
	i32 min_height;
	i32 max_width;
	i32 max_height;
	i32 base_width;
	i32 base_height;
	i32 width_inc;
	i32 height_inc;
	f64 min_aspect;
	f64 max_aspect;
	Gravity win_gravity;

public:
	auto c_struct() const -> GdkGeometry;
};

inline auto operator|(WindowHints a, WindowHints b) -> WindowHints
{
	return static_cast<WindowHints>(static_cast<GdkWindowHints>(a) |
					static_cast<GdkWindowHints>(b));
}

inline auto operator&(WindowHints a, WindowHints b) -> WindowHints
{
	return static_cast<WindowHints>(static_cast<GdkWindowHints>(a) &
					static_cast<GdkWindowHints>(b));
}

inline auto Geometry::c_struct() const -> GdkGeometry
{
	return {
		min_width,
		min_height,
		max_width,
		max_height,
		base_width,
		base_height,
		width_inc,
		height_inc,
		min_aspect,
		max_aspect,
		static_cast<GdkGravity>(win_gravity),
	};
}

} /* namespace iptsd::gfx::gdk */
