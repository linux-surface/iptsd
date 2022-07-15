sources = [
	'control.cpp',
	'device.cpp',
	'parser.cpp',
	'reader.cpp',
]

deps = [
	gsl,
	iptsd_common,
	iptsd_hid,
]

libipts = static_library('ipts', sources, dependencies: deps,
			 include_directories: includes)

iptsd_ipts = declare_dependency(dependencies: deps, link_with: libipts)
