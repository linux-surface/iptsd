sources = [
	'visualization.cpp',
]

deps = [
	libcommon,
	libcontainer,
	libmath,
	dependency('cairomm-1.0'),
]

lib = static_library('gfx', sources, dependencies: deps,
		     include_directories: includes)

libgfx = declare_dependency(dependencies: deps, link_with: lib)
