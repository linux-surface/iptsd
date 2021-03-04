sources = [
	'debug.cpp',
]

deps = [
	libipts,
	dependency('CLI11', fallback: ['CLI11', 'CLI11_dep']),
	dependency('fmt', fallback: ['fmt', 'fmt_dep']),
]

executable('ipts-dbg', sources, dependencies: deps,
	   include_directories: includes, install: true)
