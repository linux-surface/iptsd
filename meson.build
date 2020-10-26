project('iptsd', 'c',
	version: '0.3',
	license: 'GPL-2.0-or-later',
	default_options: ['c_std=gnu99'])

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
	'syscall.c',
	'touch.c',
	'touch-processing.c',
]

deps = [
	cc.find_library('m'),
	dependency('inih'),
]

executable('iptsd', sources, dependencies: deps)

