deps = [
	fmt,
	iptsd_common,
	iptsd_container,
	iptsd_math,
	spdlog,
]

our_includes = [
	include_directories('.'),
]

libcontacts = static_library('contacts', 'processor.cpp', dependencies: deps,
			     include_directories: includes)

iptsd_contacts = declare_dependency(dependencies: deps, link_with: libcontacts,
				    include_directories: our_includes)
