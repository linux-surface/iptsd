sources = [
	'visualization.cpp',
]

deps = [
	libcommon,
	libcontainer,
	libmath,
	dependency('cairo'),
	dependency('gtk+-3.0'),
]

lib = static_library('gfx', sources, dependencies: deps,
		     include_directories: includes)

libgfx = declare_dependency(dependencies: deps, link_with: lib)
