project(
	'iptsd', 'c',
	version: '0.3',
	license: 'GPL-2.0-or-later',
	default_options: [
		'c_std=gnu99',
		'warning_level=2',
		'werror=true',
	],
)

cc = meson.get_compiler('c')
dir_config = join_paths(get_option('prefix'), get_option('datadir'), 'ipts')

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
	'src/payload.c',
	'src/singletouch.c',
	'src/stylus.c',
	'src/touch.c',
	'src/touch-processing.c',
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
install_subdir('config', install_dir: dir_config, strip_directory: true)

