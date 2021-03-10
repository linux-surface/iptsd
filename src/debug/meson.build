sources = [
	'debug.cpp',
]

deps = [
	libcommon,
	libipts,
	dependency('CLI11', fallback: ['CLI11', 'CLI11_dep']),
	dependency('fmt', fallback: ['fmt', 'fmt_dep']),
	dependency('spdlog', fallback: ['spdlog', 'spdlog_dep']),
]

executable('ipts-dbg', sources, dependencies: deps,
	   include_directories: includes, install: true)

sources = [
	'proto-plot.cpp',
]

deps = [
	libcommon,
	libipts,
	libcontacts,
	libgfx,
	dependency('cairomm-1.0'),
	dependency('cairomm-png-1.0'),
	dependency('CLI11', fallback: ['CLI11', 'CLI11_dep']),
	dependency('fmt', fallback: ['fmt', 'fmt_dep']),
	dependency('spdlog', fallback: ['spdlog', 'spdlog_dep']),
]

executable('ipts-proto-plot', sources, dependencies: deps,
	   include_directories: includes, install: true)

sources = [
	'proto-rt.cpp',
]

deps = [
	libcommon,
	libipts,
	libcontacts,
	libgfx,
	dependency('fmt', fallback: ['fmt', 'fmt_dep']),
	dependency('gtkmm-3.0'),
	dependency('spdlog', fallback: ['spdlog', 'spdlog_dep']),
]

executable('ipts-proto-rt', sources, dependencies: deps,
	   include_directories: includes, install: true)
