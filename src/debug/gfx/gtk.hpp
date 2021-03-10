#pragma once

#include "gdk.hpp"

#include <common/types.hpp>

#include <functional>
#include <gtk/gtk.h>
#include <utility>

namespace iptsd::gfx::gtk {

namespace detail {

template <class T> struct raw_type {
	using type = typename T::raw_t;

	static inline auto wrap(typename T::raw_t raw) -> T
	{
		return T::wrap_raw(raw);
	}

	static inline auto unwrap(T wrapped) -> typename T::raw_t
	{
		return wrapped.raw();
	}
};

template <> struct raw_type<void> {
	using type = void;
};

template <> struct raw_type<bool> {
	using type = gboolean;

	static inline auto wrap(gboolean raw) -> bool
	{
		return raw;
	}

	static inline auto unwrap(bool raw) -> gboolean
	{
		return raw;
	}
};

template <class T> using raw_t = typename raw_type<T>::type;

template <class T> inline auto wrap(raw_t<T> raw) -> T
{
	return raw_type<T>::wrap(raw);
}

template <class T> inline auto unwrap(T wrapped) -> raw_t<T>
{
	return raw_type<T>::unwrap(wrapped);
}

namespace signal {

template <class Ret, class... Args>
inline auto do_call_wrapped(std::function<Ret(Args...)> &fn, raw_t<Args>... args) -> raw_t<Ret>
{
	if constexpr (std::is_same_v<Ret, void>) {
		fn(wrap<Args>(std::forward<raw_t<Args>>(args))...);
	} else {
		return unwrap(fn(wrap<Args>(std::forward<raw_t<Args>>(args))...));
	}

	// TODO: exception handling
}

template <class Ret, class... Args>
inline auto call_wrapped(raw_t<Args>... args, gpointer data) -> raw_t<Ret>
{
	auto *fn = static_cast<std::function<Ret(Args...)> *>(data);

	return do_call_wrapped(*fn, std::forward<raw_t<Args>>(args)...);
}

template <class Ret, class... Args> inline void destroy_wrapped(gpointer data, GClosure *closure)
{
	auto *fn = static_cast<std::function<Ret(Args...)> *>(data);
	auto _destroy = std::unique_ptr<std::function<Ret(Args...)>>(fn);
}

template <class Ret, class... Args>
inline auto do_connect(gpointer instance, char const *signal,
		       std::function<Ret(Args...)> &&callback)
{
	auto cb = std::make_unique<std::function<Ret(Args...)>>(
		std::forward<std::function<Ret(Args...)>>(callback));
	auto *cw = &call_wrapped<Ret, Args...>;
	auto flags = GConnectFlags(0);

	return g_signal_connect_data(instance, signal, G_CALLBACK(cw), cb.release(),
				     destroy_wrapped<Ret, Args...>, flags);
}

} /* namespace signal */
} /* namespace detail */

enum class ApplicationFlags {
	None = G_APPLICATION_FLAGS_NONE,
};

enum class WindowPosition {
	Center = GTK_WIN_POS_CENTER,
};

class Application {
public:
	using raw_t = GtkApplication *;

public:
	static auto create(char const *id, ApplicationFlags flags = ApplicationFlags::None)
		-> Application;
	static auto wrap_raw(gpointer raw) -> Application;

public:
	Application(GtkApplication *raw);
	Application(Application const &other);
	Application(Application &&other);
	~Application();

	auto operator=(Application const &rhs) -> Application &;
	auto operator=(Application &&rhs) -> Application &;

	auto raw() -> raw_t;
	auto operator*() -> raw_t;

	auto run(int argc, char **argv) -> int;

	template <class F> auto connect(char const *signal, F callback) -> ulong;

	// TODO: ...

private:
	GtkApplication *m_raw;
};

class Widget {
public:
	using raw_t = GtkWidget *;

public:
	static auto wrap_raw(gpointer raw) -> Widget;

public:
	Widget(GtkWidget *raw);
	Widget(Widget const &other);
	Widget(Widget &&other);
	virtual ~Widget();

	explicit operator bool() const;

	auto operator=(Widget const &rhs) -> Widget &;
	auto operator=(Widget &&rhs) -> Widget &;

	auto raw() -> raw_t;
	auto operator*() -> raw_t;

	void show();
	void show_all();

	auto get_allocated_width() -> int;
	auto get_allocated_height() -> int;

	void queue_draw();

	template <class F> auto connect(char const *signal, F callback) -> ulong;

	// TODO: ...

protected:
	GtkWidget *m_raw;
};

class Container : public Widget {
public:
	using raw_t = GtkWidget *;

public:
	static auto wrap_raw(gpointer raw) -> Container;

public:
	Container(GtkContainer *raw);
	Container(GtkWidget *raw);

	void add(Widget &widget);
};

class Bin : public Container {
public:
	using raw_t = GtkWidget *;

public:
	static auto wrap_raw(gpointer raw) -> Bin;

public:
	Bin(GtkBin *raw);
	Bin(GtkWidget *raw);
};

class Window : public Bin {
public:
	using raw_t = GtkWidget *;

public:
	static auto wrap_raw(gpointer raw) -> Window;

public:
	Window(GtkWindow *raw);
	Window(GtkWidget *raw);

	void set_position(WindowPosition position);
	void set_title(char const *title);
	void set_default_size(int width, int height);
	void set_geometry_hints(gdk::Geometry const &geom, gdk::WindowHints geom_mask);
};

class ApplicationWindow : public Window {
public:
	using raw_t = GtkWidget *;

public:
	static auto create(Application &app) -> ApplicationWindow;
	static auto wrap_raw(gpointer raw) -> ApplicationWindow;

public:
	ApplicationWindow(GtkApplicationWindow *raw);
	ApplicationWindow(GtkWidget *raw);
};

class DrawingArea : public Widget {
public:
	using raw_t = GtkWidget *;

public:
	static auto create() -> DrawingArea;
	static auto wrap_raw(gpointer raw) -> DrawingArea;

public:
	DrawingArea(GtkDrawingArea *raw);
	DrawingArea(GtkWidget *raw);
};

inline auto Application::create(char const *id, ApplicationFlags flags) -> Application
{
	return {gtk_application_new(id, static_cast<GApplicationFlags>(flags))};
}

inline auto Application::wrap_raw(gpointer raw) -> Application
{
	return {GTK_APPLICATION(g_object_ref(raw))};
}

inline Application::Application(GtkApplication *raw) : m_raw {raw}
{}

inline Application::Application(Application const &other)
	: m_raw {GTK_APPLICATION(g_object_ref(other.m_raw))}
{}

inline Application::Application(Application &&other) : m_raw {std::exchange(other.m_raw, nullptr)}
{}

inline Application::~Application()
{
	if (m_raw) {
		g_object_unref(m_raw);
	}
}

inline auto Application::operator=(Application const &rhs) -> Application &
{
	auto _destroy = Application(std::exchange(m_raw, nullptr));

	m_raw = GTK_APPLICATION(g_object_ref(rhs.m_raw));
	return *this;
}

inline auto Application::operator=(Application &&rhs) -> Application &
{
	m_raw = std::exchange(rhs.m_raw, nullptr);
	return *this;
}

inline auto Application::raw() -> raw_t
{
	return m_raw;
}

inline auto Application::operator*() -> raw_t
{
	return m_raw;
}

inline auto Application::run(int argc, char **argv) -> int
{
	return g_application_run(G_APPLICATION(m_raw), argc, argv);
}

template <class F> inline auto Application::connect(char const *signal, F callback) -> ulong
{
	return detail::signal::do_connect(m_raw, signal, std::function {callback});
}

inline auto Widget::wrap_raw(gpointer raw) -> Widget
{
	return {GTK_WIDGET(g_object_ref(raw))};
}

inline Widget::Widget(GtkWidget *raw) : m_raw {raw}
{}

inline Widget::Widget(Widget const &other) : m_raw {GTK_WIDGET(g_object_ref(other.m_raw))}
{}

inline Widget::Widget(Widget &&other) : m_raw {std::exchange(other.m_raw, nullptr)}
{}

inline Widget::~Widget()
{
	if (m_raw) {
		g_object_unref(m_raw);
	}
}

Widget::operator bool() const
{
	return m_raw != nullptr;
}

auto Widget::operator=(Widget const &rhs) -> Widget &
{
	auto _destroy = Widget(std::exchange(m_raw, nullptr));

	m_raw = GTK_WIDGET(g_object_ref(rhs.m_raw));
	return *this;
}

auto Widget::operator=(Widget &&rhs) -> Widget &
{
	m_raw = std::exchange(rhs.m_raw, nullptr);
	return *this;
}

inline auto Widget::raw() -> raw_t
{
	return m_raw;
}

inline auto Widget::operator*() -> raw_t
{
	return m_raw;
}

inline void Widget::show()
{
	gtk_widget_show(m_raw);
}

inline void Widget::show_all()
{
	gtk_widget_show_all(m_raw);
}

inline auto Widget::get_allocated_width() -> int
{
	return gtk_widget_get_allocated_width(m_raw);
}

inline auto Widget::get_allocated_height() -> int
{
	return gtk_widget_get_allocated_height(m_raw);
}

inline void Widget::queue_draw()
{
	gtk_widget_queue_draw(m_raw);
}

template <class F> inline auto Widget::connect(char const *signal, F callback) -> ulong
{
	return detail::signal::do_connect(m_raw, signal, std::function {callback});
}

inline auto Container::wrap_raw(gpointer raw) -> Container
{
	return {GTK_CONTAINER(g_object_ref(raw))};
}

inline Container::Container(GtkContainer *raw) : Widget {GTK_WIDGET(raw)}
{}

inline Container::Container(GtkWidget *raw) : Widget {raw}
{}

inline void Container::add(Widget &widget)
{
	gtk_container_add(GTK_CONTAINER(m_raw), *widget);
}

inline auto Bin::wrap_raw(gpointer raw) -> Bin
{
	return {GTK_BIN(g_object_ref(raw))};
}

inline Bin::Bin(GtkBin *raw) : Container {GTK_WIDGET(raw)}
{}

inline Bin::Bin(GtkWidget *raw) : Container {raw}
{}

inline auto Window::wrap_raw(gpointer raw) -> Window
{
	return {GTK_WINDOW(g_object_ref(raw))};
}

inline Window::Window(GtkWindow *raw) : Bin {GTK_WIDGET(raw)}
{}

inline Window::Window(GtkWidget *raw) : Bin {raw}
{}

inline void Window::set_position(WindowPosition position)
{
	gtk_window_set_position(GTK_WINDOW(m_raw), static_cast<GtkWindowPosition>(position));
}

inline void Window::set_title(char const *title)
{
	gtk_window_set_title(GTK_WINDOW(m_raw), title);
}

inline void Window::set_default_size(int width, int height)
{
	gtk_window_set_default_size(GTK_WINDOW(m_raw), width, height);
}

inline void Window::set_geometry_hints(gdk::Geometry const &geom, gdk::WindowHints geom_mask)
{
	auto geom_c = geom.c_struct();
	auto mask = static_cast<GdkWindowHints>(geom_mask);

	gtk_window_set_geometry_hints(GTK_WINDOW(m_raw), NULL, &geom_c, mask);
}

inline auto ApplicationWindow::wrap_raw(gpointer raw) -> ApplicationWindow
{
	return {GTK_APPLICATION_WINDOW(g_object_ref(raw))};
}

inline auto ApplicationWindow::create(Application &app) -> ApplicationWindow
{
	return wrap_raw(gtk_application_window_new(*app));
}

inline ApplicationWindow::ApplicationWindow(GtkApplicationWindow *raw) : Window {GTK_WIDGET(raw)}
{}

inline ApplicationWindow::ApplicationWindow(GtkWidget *raw) : Window {raw}
{}

inline auto DrawingArea::wrap_raw(gpointer raw) -> DrawingArea
{
	return {GTK_DRAWING_AREA(g_object_ref(raw))};
}

inline auto DrawingArea::create() -> DrawingArea
{
	return wrap_raw(gtk_drawing_area_new());
}

inline DrawingArea::DrawingArea(GtkDrawingArea *raw) : Widget {GTK_WIDGET(raw)}
{}

inline DrawingArea::DrawingArea(GtkWidget *raw) : Widget {raw}
{}

} /* namespace iptsd::gfx::gtk */
