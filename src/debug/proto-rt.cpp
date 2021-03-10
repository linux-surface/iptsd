#include <common/types.hpp>
#include <contacts/eval/perf.hpp>
#include <contacts/processor.hpp>
#include <container/image.hpp>
#include <ipts/control.hpp>
#include <ipts/parser.hpp>

#include <atomic>
#include <fstream>
#include <gfx/cairo.hpp>
#include <gfx/gtk.hpp>
#include <gfx/visualization.hpp>
#include <iostream>
#include <mutex>
#include <spdlog/spdlog.h>
#include <thread>
#include <vector>

using namespace iptsd::gfx;

namespace iptsd::debug::rt {

class MainContext {
public:
	MainContext(index2_t img_size);

	void submit(container::Image<f32> const &img, std::vector<contacts::TouchPoint> const &tps);

	auto draw_event(cairo::Cairo &cr) -> bool;

public:
	gtk::Widget m_widget;

private:
	gfx::Visualization m_vis;

	container::Image<f32> m_img1;
	container::Image<f32> m_img2;

	std::vector<contacts::TouchPoint> m_tps1;
	std::vector<contacts::TouchPoint> m_tps2;

	std::mutex m_lock;

	container::Image<f32> *m_img_frnt;
	container::Image<f32> *m_img_back;

	std::vector<contacts::TouchPoint> *m_tps_frnt;
	std::vector<contacts::TouchPoint> *m_tps_back;
	bool m_swap = false;
};

MainContext::MainContext(index2_t img_size)
	: m_widget {nullptr}, m_vis {img_size}, m_img1 {img_size}, m_img2 {img_size}, m_tps1 {},
	  m_tps2 {}, m_img_frnt {&m_img1}, m_img_back {&m_img2}, m_tps_frnt {&m_tps1},
	  m_tps_back {&m_tps2}
{}

void MainContext::submit(container::Image<f32> const &img,
			 std::vector<contacts::TouchPoint> const &tps)
{
	{
		// set swap to false to prevent read-access in draw
		auto guard = std::lock_guard(m_lock);
		m_swap = false;
	}

	// copy to back-buffer
	*m_img_back = img;
	*m_tps_back = tps;

	{
		// set swap to true to indicate that new data has arrived
		auto guard = std::lock_guard(m_lock);
		m_swap = true;
	}

	// request update
	if (m_widget)
		m_widget.queue_draw();
}

auto MainContext::draw_event(cairo::Cairo &cr) -> bool
{
	auto const width = m_widget.get_allocated_width();
	auto const height = m_widget.get_allocated_height();

	{
		// check and swap buffers, if necessary
		auto guard = std::lock_guard(m_lock);

		if (m_swap) {
			std::swap(m_img_frnt, m_img_back);
			std::swap(m_tps_frnt, m_tps_back);
			m_swap = false;
		}
	}

	m_vis.draw(cr, *m_img_frnt, *m_tps_frnt, width, height);
	return false;
}

static int main(int argc, char *argv[])
{
	const index2_t size {72, 48};
	MainContext ctx {size};
	contacts::TouchProcessor prc {size};

	ipts::Control ctrl;

	auto app = gtk::Application::create("com.github.qzed.digitizer-prototype.rt");

	app.connect("activate", [&](gtk::Application app) -> void {
		auto window = gtk::ApplicationWindow::create(app);

		// fix aspect to 3-to-2
		auto geom =
			gdk::Geometry {0, 0, 0, 0, 0, 0, 0, 0, 1.5f, 1.5f, gdk::Gravity::Center};

		window.set_position(gtk::WindowPosition::Center);
		window.set_default_size(900, 600);
		window.set_title("IPTS Processor Prototype");
		window.set_geometry_hints(geom, gdk::WindowHints::Aspect);

		auto darea = gtk::DrawingArea::create();
		window.add(darea);

		ctx.m_widget = darea;
		darea.connect("draw", [&](gtk::Widget widget, cairo::Cairo cr) -> bool {
			return ctx.draw_event(cr);
		});

		window.show_all();
	});

	std::atomic_bool run(true);

	std::thread updt([&]() -> void {
		using namespace std::chrono_literals;

		ipts::Parser parser(ctrl.info.buffer_size);
		parser.on_heatmap = [&](const auto &data) {
			container::Image<f32> hm {size};

			std::transform(data.data.begin(), data.data.end(), hm.begin(), [&](auto v) {
				f32 val = static_cast<f32>(v - data.z_min) /
					  static_cast<f32>(data.z_max - data.z_min);

				return 1.0f - val;
			});

			ctx.submit(hm, prc.process(hm));
		};

		while (run.load()) {
			u32 doorbell = ctrl.doorbell();

			while (doorbell > ctrl.current_doorbell && run.load()) {
				ctrl.read(parser.buffer());
				parser.parse();
				ctrl.send_feedback();
			}

			std::this_thread::sleep_for(10ms);
		}
	});

	int status = app.run(argc, argv);

	// TODO: should probably hook into destroy event to stop thread before gtk_main() returns

	run.store(false);
	updt.join();

	return status;
}

} // namespace iptsd::debug::rt

int main(int argc, char *argv[])
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return iptsd::debug::rt::main(argc, argv);
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
