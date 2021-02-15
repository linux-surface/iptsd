sources = [
	'cone.cpp',
	'config.cpp',
	'contact.cpp',
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
]

deps = [
	libcommon,
	libipts,
	dependency('inih', fallback: ['inih', 'inih_dep']),
]

executable('iptsd', sources, dependencies: deps,
	   include_directories: includes, install: true)
