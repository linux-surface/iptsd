sources = [
	'processor.cpp',
]

deps = [
	libcommon,
]

our_includes = [
	include_directories('.'),
]

lib = static_library('libcontacts', sources, include_directories: includes)
libcontacts = declare_dependency(dependencies: deps, link_with: lib,
				 include_directories: our_includes)
