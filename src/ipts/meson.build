sources = [
	'control.cpp',
	'parser.cpp',
]

deps = [
	iptsd_common,
]

libipts = static_library('ipts', sources, dependencies: deps,
			 include_directories: includes)

iptsd_ipts = declare_dependency(dependencies: deps, link_with: libipts)
