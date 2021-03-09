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
	libcommon,
	libcontacts,
	libipts,
	dependency('fmt', fallback: ['fmt', 'fmt_dep']),
	dependency('inih', fallback: ['inih', 'inih_dep']),
	dependency('Microsoft.GSL', modules:['Microsoft.GSL::GSL'],
		   fallback: ['microsoft-gsl', 'microsoft_gsl_dep']),
	dependency('spdlog', fallback: ['spdlog', 'spdlog_dep']),
]

executable('iptsd', sources, dependencies: deps,
	   include_directories: includes, install: true)
