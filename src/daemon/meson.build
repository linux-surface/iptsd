sources = [
	'config.cpp',
	'devices.cpp',
	'main.cpp',
	'singletouch.cpp',
	'stylus.cpp',
	'touch-manager.cpp',
	'touch.cpp',
	'uinput-device.cpp',
]

deps = [
	libcontacts,
	libipts,
	dependency('inih', fallback: ['inih', 'inih_dep']),
]

executable('iptsd', sources, dependencies: deps,
	   include_directories: includes, install: true)
