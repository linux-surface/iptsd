#include <common/types.hpp>
#include <contacts/advanced/processor.hpp>
#include <contacts/eval/perf.hpp>
#include <container/image.hpp>
#include <gfx/visualization.hpp>
#include <ipts/control.hpp>
#include <ipts/parser.hpp>

#include <atomic>
#include <cairomm/context.h>
#include <cairomm/enums.h>
#include <cairomm/refptr.h>
#include <fstream>
#include <gtkmm/application.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/enums.h>
#include <gtkmm/object.h>
#include <gtkmm/widget.h>
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

	auto draw_event(const Cairo::RefPtr<Cairo::Context> &cr) -> bool;

public:
	Gtk::DrawingArea *m_widget;

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
	m_widget->queue_draw();
}

auto MainContext::draw_event(const Cairo::RefPtr<Cairo::Context> &cr) -> bool
{
	auto const width = m_widget->get_allocated_width();
	auto const height = m_widget->get_allocated_height();

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
	contacts::advanced::TouchProcessor prc {size};

	ipts::Control ctrl;
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

			std::copy(hm.begin(), hm.end(), prc.hm().begin());
			ctx.submit(hm, prc.process());
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

	auto app = Gtk::Application::create(argc, argv);
	Gtk::Window window;

	window.set_position(Gtk::WIN_POS_CENTER);
	window.set_default_size(900, 600);
	window.set_title("IPTS Processor Prototype");
	window.set_resizable(false);

	ctx.m_widget = Gtk::make_managed<Gtk::DrawingArea>();
	window.add(*ctx.m_widget);

	ctx.m_widget->signal_draw().connect([&](const Cairo::RefPtr<Cairo::Context> &cr) -> bool {
		return ctx.draw_event(cr);
	});

	window.show_all();

	int status = app->run(window);

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
