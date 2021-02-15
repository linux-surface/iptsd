sources = [
	'cone.cpp',
	'config.cpp',
	'contact.cpp',
	'control.cpp',
	'data.cpp',
	'devices.cpp',
	'finger.cpp',
	'heatmap.cpp',
	'hid.cpp',
	'main.cpp',
	'reader.cpp',
	'payload.cpp',
	'singletouch.cpp',
	'stylus.cpp',
	'touch.cpp',
	'touch-processing.cpp',
	'uinput-device.cpp',
	'utils.cpp',
]

deps = [
	dependency('inih', fallback: ['inih', 'inih_dep']),
]

executable('iptsd', sources, dependencies: deps,
	   install: true, include_directories: includes)
