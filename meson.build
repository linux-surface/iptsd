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

sources = [
	'config.c',
	'contact.c',
	'control.c',
	'data.c',
	'devices.c',
	'finger.c',
	'heatmap.c',
	'hid.c',
	'main.c',
	'payload.c',
	'singletouch.c',
	'stylus.c',
	'stylus-processing.c',
	'touch.c',
	'touch-processing.c',
	'utils.c',
]

deps = [
	cc.find_library('m'),
	dependency('inih'),
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

executable('iptsd', sources, dependencies: deps)

