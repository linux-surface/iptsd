deps = [
	cairomm,
	iptsd_common,
	iptsd_container,
	iptsd_math,
]

libgfx = static_library('gfx', 'visualization.cpp', dependencies: deps,
			include_directories: includes)

iptsd_gfx = declare_dependency(dependencies: deps, link_with: libgfx)
