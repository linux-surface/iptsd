project('iptsd', 'cpp',
	version: '0.3.1',
	license: 'GPL-2.0-or-later',
	default_options: [
		'cpp_std=c++17',
		'warning_level=3',
		'werror=true',
		'sysconfdir=/etc',
	])

cpp = meson.get_compiler('cpp')

prefix = get_option('prefix')
bindir = join_paths(prefix, get_option('bindir'))
datadir = join_paths(prefix, get_option('datadir'))
sysconfdir = get_option('sysconfdir')

configdir = join_paths(datadir, 'ipts')
configfile = join_paths(sysconfdir, 'ipts.conf')

conf = configuration_data()
conf.set('bindir', bindir)
conf.set('datadir', datadir)
conf.set('sysconfdir', sysconfdir)

conf.set('configdir', configdir)
conf.set('configfile', configfile)

configure_file(input: 'configure.h.in',
	       output: 'configure.h',
	       configuration: conf)

sources = [
	'src/cone.cpp',
	'src/config.cpp',
	'src/contact.cpp',
	'src/control.cpp',
	'src/data.cpp',
	'src/devices.cpp',
	'src/finger.cpp',
	'src/heatmap.cpp',
	'src/hid.cpp',
	'src/main.cpp',
	'src/reader.cpp',
	'src/payload.cpp',
	'src/singletouch.cpp',
	'src/stylus.cpp',
	'src/touch.cpp',
	'src/touch-processing.cpp',
	'src/uinput-device.cpp',
	'src/utils.cpp',
]

sources_dbg = [
	'debug/debug.cpp',
	'src/control.cpp',
	'src/utils.cpp',
]

includes = [
	 include_directories('src'),
]

deps = [
	dependency('inih', fallback: ['inih', 'inih_dep']),
]

cxxflags = [
	'-Wundef',
	'-Wuninitialized',
	'-Wno-unused-result',
	'-Wlogical-op',
	'-Wmissing-include-dirs',
	'-Wpointer-arith',
	'-Winit-self',
	'-Wimplicit-fallthrough=2',
	'-Wendif-labels',
	'-Wstrict-aliasing=2',
	'-Woverflow',
	'-Wno-missing-braces',
	'-Wno-missing-field-initializers',
	'-Wno-unused-parameter',
]

cxxflags = cpp.get_supported_arguments(cxxflags)
add_project_arguments(cxxflags, language: 'cpp')

executable('iptsd', sources,
	   dependencies: deps,
	   install: true,
	   include_directories: includes)

install_data('etc/iptsd-reset-sensor', install_dir: bindir)
install_subdir('config', install_dir: configdir, strip_directory: true)

configure_file(input: 'etc/systemd/iptsd.service.in',
	       output: 'iptsd.service',
	       configuration: conf)

configure_file(input: 'etc/openrc/iptsd.initd.in',
	       output: 'iptsd.initd',
	       configuration: conf)

service_manager = get_option('service_manager')
if service_manager.contains('systemd')
	systemd = dependency('systemd')
	unitdir = systemd.get_pkgconfig_variable('systemdsystemunitdir')

	udev = dependency('udev')
	udevdir = udev.get_pkgconfig_variable('udevdir')
	rulesdir = join_paths(udevdir, 'rules.d')

	install_data(join_paths(meson.current_build_dir(), 'iptsd.service'),
		     install_dir: unitdir)

	install_data('etc/udev/50-ipts.rules', install_dir: rulesdir)
endif

if service_manager.contains('openrc')
	dependency('openrc')
	openrcdir = join_paths(sysconfdir, 'init.d')

	install_data(join_paths(meson.current_build_dir(), 'iptsd.initd'),
		     install_dir: openrcdir,
		     rename: 'iptsd')
endif

if get_option('sample_config')
	install_data('etc/ipts.conf', install_dir: sysconfdir)
endif

if get_option('debug_tool')
	executable('ipts-dbg', sources_dbg,
		   install: true,
		   include_directories: includes)
endif
