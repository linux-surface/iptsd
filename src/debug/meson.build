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
	'visualization.cpp',
]

deps = [
	libcommon,
	libipts,
	libcontacts,
	dependency('CLI11', fallback: ['CLI11', 'CLI11_dep']),
	dependency('fmt', fallback: ['fmt', 'fmt_dep']),
	dependency('spdlog', fallback: ['spdlog', 'spdlog_dep']),
	dependency('cairo'),
]

executable('ipts-proto-plot', sources, dependencies: deps,
	   include_directories: includes, install: true)

sources = [
	'proto-rt.cpp',
	'visualization.cpp',
]

deps = [
	libcommon,
	libipts,
	libcontacts,
	dependency('fmt', fallback: ['fmt', 'fmt_dep']),
	dependency('spdlog', fallback: ['spdlog', 'spdlog_dep']),
	dependency('gtk+-3.0'),
]

executable('ipts-proto-rt', sources, dependencies: deps,
	   include_directories: includes, install: true)
