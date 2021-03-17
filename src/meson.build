cpp = meson.get_compiler('cpp')

configure_file(input: 'configure.h.in',
	       output: 'configure.h',
	       configuration: conf)

includes = [
	include_directories('.')
]

cxxflags = [
	'-Wundef',
	'-Wuninitialized',
	'-Wno-unused-result',
	'-Wlogical-op',
	'-Wmissing-include-dirs',
	'-Wpointer-arith',
	'-Winit-self',
	'-Wimplicit-fallthrough=2',
	'-Wendif-labels',
	'-Wstrict-aliasing=2',
	'-Woverflow',
	'-Wno-missing-braces',
	'-Wno-missing-field-initializers',
	'-Wno-unused-parameter',
]

optflags = []

debug = get_option('debug')
optimize = not debug and get_option('optimization') in ['2', '3']

access_checks = get_option('access_checks')
check_access = access_checks.enabled() or (access_checks.auto() and not optimize)

fast_math = get_option('fast_math')
if fast_math.enabled() or (fast_math.auto() and optimize)
	optflags += ['-ffast-math']
endif

cpu_type = get_option('cpu_type')
if cpu_type != ''
	optflags += ['-march=' + cpu_type]
endif

simd_type = get_option('simd_type')
if simd_type != ''
	optflags += ['-m' + simd_type]
endif

cxxflags = cpp.get_supported_arguments(cxxflags)
optflags = cpp.get_supported_arguments(optflags)

add_project_arguments(cxxflags, language: 'cpp')
add_project_arguments(optflags, language: 'cpp')

if check_access
	add_project_arguments('-DIPTSD_CONFIG_ACCESS_CHECKS', language: 'cpp')
endif

add_project_arguments('-DSPDLOG_FMT_EXTERNAL', language: 'cpp')

# Dependencies
fmt = dependency('fmt', fallback: ['fmt', 'fmt_dep'])
inih = dependency('inih', fallback: ['inih', 'inih_dep'])
gsl = dependency('Microsoft.GSL', modules: ['Microsoft.GSL::GSL'],
		 fallback: ['microsoft-gsl', 'microsoft_gsl_dep'])
spdlog = dependency('spdlog', fallback: ['spdlog', 'spdlog_dep'])

subdir('common')
subdir('math')

subdir('container')
subdir('ipts')

subdir('contacts')
subdir('daemon')

if get_option('debug_tool')
	cli11 = dependency('CLI11', fallback: ['CLI11', 'CLI11_dep'])
	cairomm = dependency('cairomm-1.0')
	cairommpng = dependency('cairomm-png-1.0')
	gtkmm = dependency('gtkmm-3.0')

	subdir('gfx')
	subdir('debug')
endif
