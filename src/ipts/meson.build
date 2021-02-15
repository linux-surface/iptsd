sources = [
	'control.cpp',
]

deps = [
	libcommon,
]

lib = static_library('ipts', sources, include_directories: includes)
libipts = declare_dependency(dependencies: deps, link_with: lib)
