deps = [
	fmt,
	iptsd_common,
	iptsd_container,
	iptsd_math,
	spdlog,
]

libcontacts = static_library('contacts', 'processor.cpp', dependencies: deps,
			     include_directories: includes)

iptsd_contacts = declare_dependency(dependencies: deps, link_with: libcontacts)
