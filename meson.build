project(
	'iptsd', 'c',
	version: '0.3.1',
	license: 'GPL-2.0-or-later',
	default_options: [
		'c_std=gnu99',
		'warning_level=3',
		'werror=true',
		'sysconfdir=/etc',
	],
)

cc = meson.get_compiler('c')

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
	output: 'configure.h', configuration: conf)

sources = [
	'src/cone.c',
	'src/config.c',
	'src/contact.c',
	'src/control.c',
	'src/data.c',
	'src/devices.c',
	'src/finger.c',
	'src/heatmap.c',
	'src/hid.c',
	'src/main.c',
	'src/reader.c',
	'src/payload.c',
	'src/singletouch.c',
	'src/stylus.c',
	'src/touch.c',
	'src/touch-processing.c',
	'src/utils.c',
]

sources_dbg = [
	'debug/debug.c',
	'src/control.c',
	'src/utils.c',
]

deps = [
	cc.find_library('m'),
	dependency('inih', fallback: ['inih', 'inih_dep']),
]

cflags = [
	'-Wundef',
	'-Wuninitialized',
	'-Wno-unused-result',
	'-Wlogical-op',
	'-Wmissing-include-dirs',
	'-Wold-style-definition',
	'-Wpointer-arith',
	'-Winit-self',
	'-Wstrict-prototypes',
	'-Wimplicit-fallthrough=2',
	'-Wendif-labels',
	'-Wstrict-aliasing=2',
	'-Woverflow',
	'-Wmissing-prototypes',
	'-Wno-missing-braces',
	'-Wno-missing-field-initializers',
	'-Wno-unused-parameter',
]

cflags = cc.get_supported_arguments(cflags)
add_project_arguments(cflags, language: 'c')

executable('iptsd', sources, dependencies: deps, install: true)
install_data('etc/iptsd-reset-sensor', install_dir: bindir)
install_subdir('config', install_dir: configdir, strip_directory: true)

unitdir='' # If this remains empty, configure_file will skip the install step
if get_option('systemd')
	systemd = dependency('systemd')
	unitdir = systemd.get_pkgconfig_variable('systemdsystemunitdir')

	udev = dependency('udev')
	udevdir = udev.get_pkgconfig_variable('udevdir')
	rulesdir = join_paths(udevdir, 'rules.d')

	install_data('etc/udev/50-ipts.rules', install_dir: rulesdir)
endif

configure_file(
	input: 'etc/systemd/iptsd.service.in',
	output: 'iptsd.service',
	configuration: conf,
	install_dir: unitdir
)

openrcdir=''
if get_option('openrc')
	dependency('openrc')
	install_openrc=true
	openrcdir = join_paths(sysconfdir, 'init.d')
endif

configure_file(
	input: 'etc/openrc/iptsd.initd.in',
	output: 'iptsd.initd',
	configuration: conf
)

# Workaround required to rename iptsd.initd -> iptsd when installing
if openrcdir != ''
	install_data(
		join_paths(meson.current_build_dir(), 'iptsd.initd'),
		install_dir: openrcdir,
		rename: 'iptsd'
	)
endif

if get_option('sample_config')
	install_data('etc/ipts.conf', install_dir: sysconfdir)
endif

if get_option('debug_tool')
	executable('ipts-dbg', sources_dbg, install: true)
endif
