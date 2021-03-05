sources = [
	'control.cpp',
	'parser.cpp',
]

deps = [
	libcommon,
]

lib = static_library('ipts', sources, dependencies: deps,
		     include_directories: includes)

libipts = declare_dependency(dependencies: deps, link_with: lib)
