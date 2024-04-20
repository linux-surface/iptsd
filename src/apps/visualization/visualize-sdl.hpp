// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_APPS_VISUALIZATION_VISUALIZE_SDL_HPP
#define IPTSD_APPS_VISUALIZATION_VISUALIZE_SDL_HPP

#include "visualize.hpp"

#include <common/casts.hpp>
#include <common/chrono.hpp>
#include <common/types.hpp>
#include <core/generic/config.hpp>
#include <core/generic/device.hpp>

#include <SDL.h>
#include <cairomm/cairomm.h>
#include <gsl/gsl>

#include <cstring>

namespace iptsd::apps::visualization {

class VisualizeSDL : public Visualize {
private:
	using clock = std::chrono::steady_clock;

private:
	SDL_Window *m_window = nullptr;
	SDL_Renderer *m_renderer = nullptr;

	SDL_Texture *m_rtex = nullptr;
	Cairo::RefPtr<Cairo::ImageSurface> m_tex {};

	clock::time_point m_last_draw {};

public:
	VisualizeSDL(const core::Config &config, const core::DeviceInfo &info)
		: Visualize(config, info)
	{
		SDL_Init(SDL_INIT_VIDEO);
	}

	void on_start() override
	{
		// Create an SDL window
		constexpr u32 flags = SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_ALLOW_HIGHDPI;
		SDL_CreateWindowAndRenderer(0, 0, flags, &m_window, &m_renderer);

		// Get the screen size.
		SDL_GetRendererOutputSize(m_renderer, &m_size.x(), &m_size.y());

		// Create a texture that will be rendered later
		m_rtex = SDL_CreateTexture(m_renderer,
		                           SDL_PIXELFORMAT_ARGB8888,
		                           SDL_TEXTUREACCESS_STREAMING,
		                           m_size.x(),
		                           m_size.y());

		// Create a texture for drawing.
		m_tex = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, m_size.x(), m_size.y());

		// Create context for issuing draw commands.
		m_cairo = Cairo::Context::create(m_tex);
	}

	void on_data(const gsl::span<u8> data) override
	{
		Visualize::on_data(data);

		// Handle window events, such as the X11_NET_WM_PING
		// event that is used to detect stuck programs.
		SDL_PumpEvents();

		constexpr usize FPS = 60;

		// Limit how many times per seconds the screen is redrawn.
		const clock::time_point now = clock::now();
		const clock::time_point next = m_last_draw + (1000ms / FPS);

		if (now < next)
			return;

		this->draw();

		void *pixels = nullptr;
		int pitch = 0;

		// Copy drawtex to rendertex
		SDL_LockTexture(m_rtex, nullptr, &pixels, &pitch);
		std::memcpy(pixels, m_tex->get_data(), casts::to_unsigned(m_size.prod() * 4L));
		SDL_UnlockTexture(m_rtex);

		// Display rendertex
		SDL_RenderClear(m_renderer);
		SDL_RenderCopy(m_renderer, m_rtex, nullptr, nullptr);
		SDL_RenderPresent(m_renderer);

		m_last_draw = now;
	}

	void on_stop() override
	{
		SDL_DestroyTexture(m_rtex);
		SDL_DestroyRenderer(m_renderer);
		SDL_DestroyWindow(m_window);

		SDL_Quit();
	}
};

} // namespace iptsd::apps::visualization

#endif // IPTSD_APPS_VISUALIZATION_VISUALIZE_SDL_HPP
