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
	fmt,
	gsl,
	inih,
	iptsd_common,
	iptsd_contacts,
	iptsd_ipts,
	iptsd_math,
	iptsd_container,
	spdlog,
	stdcppfs,
]

executable('iptsd', sources, dependencies: deps,
	   include_directories: includes, install: true)
