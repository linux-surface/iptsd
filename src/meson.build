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

if get_option('access_checks')
	add_project_arguments('-D_GLIBCXX_ASSERTIONS', language: 'cpp')
endif

if get_option('force_access_checks')
	add_project_arguments('-DIPTSD_CONFIG_FORCE_ACCESS_CHECKS', language: 'cpp')
endif

add_project_arguments('-DSPDLOG_FMT_EXTERNAL', language: 'cpp')

# Dependencies
fmt = dependency('fmt', fallback: ['fmt', 'fmt_dep'])
inih = dependency('inih', fallback: ['inih', 'inih_dep'])
gsl = dependency('Microsoft.GSL', modules: ['Microsoft.GSL::GSL'],
		 fallback: ['microsoft-gsl', 'microsoft_gsl_dep'])
spdlog = dependency('spdlog', fallback: ['spdlog', 'spdlog_dep'])

# hidrd does not ship pkg-config files
hidrd_item = cpp.find_library('hidrd_item')
hidrd_usage = cpp.find_library('hidrd_usage')

# Find libstdc++fs for older GCC
stdcppfs = cpp.find_library('stdc++fs')

subdir('common')
subdir('math')

subdir('container')
subdir('hid')
subdir('ipts')

subdir('contacts')
subdir('daemon')

if get_option('debug_tools').length() > 0
	gtkmm = dependency('gtkmm-3.0', required: false)
	cairomm = dependency('cairomm-1.0', required: false)
	cli11 = dependency('CLI11', required: false, fallback: ['CLI11', 'CLI11_dep'])

	subdir('gfx')
	subdir('debug')
endif
