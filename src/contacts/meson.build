sources = [
	'processor.cpp',
]

deps = [
	libcommon,
	dependency('fmt', fallback: ['fmt', 'fmt_dep']),
	dependency('spdlog', fallback: ['spdlog', 'spdlog_dep']),
	dependency('Microsoft.GSL', modules:['Microsoft.GSL::GSL'],
		   fallback: ['microsoft-gsl', 'microsoft_gsl_dep']),
]

our_includes = [
	include_directories('.'),
]

lib = static_library('libcontacts', sources, include_directories: includes)
libcontacts = declare_dependency(dependencies: deps, link_with: lib,
				 include_directories: our_includes)
