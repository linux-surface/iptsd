// SPDX-License-Identifier: GPL-2.0-or-later

#include <common/types.hpp>
#include <contacts/finder.hpp>
#include <container/image.hpp>
#include <gfx/visualization.hpp>
#include <ipts/device.hpp>
#include <ipts/parser.hpp>

#include <CLI/CLI.hpp>
#include <SDL2/SDL.h>
#include <cairomm/cairomm.h>
#include <filesystem>
#include <gsl/gsl>
#include <gsl/span>
#include <spdlog/spdlog.h>
#include <vector>

namespace iptsd::debug::show {

static void iptsd_show_handle_input(const Cairo::RefPtr<Cairo::Context> &cairo, index2_t rsize,
				    gfx::Visualization &vis, contacts::ContactFinder &finder,
				    const ipts::Heatmap &data)
{
	// Make sure that all buffers have the correct size
	finder.resize(index2_t {data.dim.width, data.dim.height});

	// Normalize and invert the heatmap data.
	std::transform(data.data.begin(), data.data.end(), finder.data().begin(), [&](auto v) {
		f32 val = static_cast<f32>(v - data.dim.z_min) /
			  static_cast<f32>(data.dim.z_max - data.dim.z_min);

		return 1.0f - val;
	});

	// Search for a contact
	const std::vector<contacts::Contact> &contacts = finder.search();

	// Draw the raw heatmap
	vis.draw_heatmap(cairo, rsize, finder.data());

	// Draw the contacts
	vis.draw_contacts(cairo, rsize, contacts);
}

static int main(gsl::span<char *> args)
{
	CLI::App app {};
	std::filesystem::path path;

	app.add_option("DEVICE", path, "The hidraw device to read from.")
		->type_name("FILE")
		->required();

	CLI11_PARSE(app, args.size(), args.data());

	ipts::Device device {path};
	config::Config config {device.vendor(), device.product()};

	// Check if a config was found
	if (config.width == 0 || config.height == 0)
		throw std::runtime_error("No display config for this device was found!");

	gfx::Visualization vis {config};
	contacts::ContactFinder finder {config.contacts()};

	// Get the buffer size from the HID descriptor
	std::size_t buffer_size = device.buffer_size();
	std::vector<u8> buffer(buffer_size);

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window = nullptr;
	SDL_Renderer *renderer = nullptr;

	// Create an SDL window
	SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_ALLOW_HIGHDPI,
				    &window, &renderer);

	index2_t rsize {};
	SDL_GetRendererOutputSize(renderer, &rsize.x, &rsize.y);

	// Create a texture that will be rendered later
	SDL_Texture *rendertex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
						   SDL_TEXTUREACCESS_STREAMING, rsize.x, rsize.y);

	// Create a texture for drawing
	auto drawtex = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, rsize.x, rsize.y);
	auto cairo = Cairo::Context::create(drawtex);

	ipts::Parser parser {};
	parser.on_heatmap = [&](const auto &data) {
		iptsd_show_handle_input(cairo, rsize, vis, finder, data);
	};

	// Count errors, if we receive 50 continuous errors, chances are pretty good that
	// something is broken beyond repair and the program should exit.
	i32 errors = 0;

	// Enable multitouch mode
	device.set_mode(true);

	while (true) {
		if (errors >= 50) {
			spdlog::error("Encountered 50 continuous errors, aborting...");
			break;
		}

		SDL_Event event;
		bool quit = false;

		// Check for SDL quit event
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				quit = true;
		}

		if (quit)
			break;

		try {
			ssize_t size = device.read(buffer);

			// Does this report contain touch data?
			if (!device.is_touch_data(buffer[0]))
				continue;

			parser.parse(gsl::span<u8>(buffer.data(), size));

			void *pixels = nullptr;
			int pitch = 0;

			// Copy drawtex to rendertex
			SDL_LockTexture(rendertex, nullptr, &pixels, &pitch);
			std::memcpy(pixels, drawtex->get_data(), rsize.span() * 4L);
			SDL_UnlockTexture(rendertex);

			// Display rendertex
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, rendertex, nullptr, nullptr);
			SDL_RenderPresent(renderer);
		} catch (std::exception &e) {
			spdlog::warn(e.what());
			errors++;
			continue;
		}

		// Reset error count
		errors = 0;
	}

	// Disable multitouch mode
	device.set_mode(false);

	SDL_DestroyTexture(rendertex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}

} // namespace iptsd::debug::show

int main(int argc, char *argv[])
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return iptsd::debug::show::main(gsl::span(argv, argc));
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
