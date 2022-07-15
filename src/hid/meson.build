sources = [
	'descriptor.cpp',
	'device.cpp',
]

deps = [
	gsl,
	hidrd_item,
	hidrd_usage,
	iptsd_common,
]

libhid = static_library('hid', sources, dependencies: deps,
			 include_directories: includes)

iptsd_hid = declare_dependency(dependencies: deps, link_with: libhid)
