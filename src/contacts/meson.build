sources = [
	'processor.cpp',
]

deps = [
	libcommon,
	dependency('fmt', fallback: ['fmt', 'fmt_dep']),
	dependency('spdlog', fallback: ['spdlog', 'spdlog_dep']),
]

our_includes = [
	include_directories('.'),
]

lib = static_library('contacts', sources, dependencies: deps,
		     include_directories: includes)

libcontacts = declare_dependency(dependencies: deps, link_with: lib,
				 include_directories: our_includes)
