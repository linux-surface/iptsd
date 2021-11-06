deps = [
	fmt,
	iptsd_common,
	iptsd_container,
	iptsd_math,
	spdlog,
]

src = [
	'processor.cpp',
	'advanced/processor.cpp',
	'basic/cluster.cpp',
	'basic/heatmap.cpp',
	'basic/processor.cpp',
]

libcontacts = static_library('contacts', src, dependencies: deps,
			     include_directories: includes)

iptsd_contacts = declare_dependency(dependencies: deps, link_with: libcontacts)
