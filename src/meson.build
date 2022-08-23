cpp = meson.get_compiler('cpp')

configure_file(input: 'configure.h.in', output: 'configure.h', configuration: conf)

includes = [
  include_directories('.')
]

cxxflags = [
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

optflags = [
  '-ffast-math',
]

target_cpu = target_machine.cpu_family()

if target_cpu == 'x86_64'
  optflags += '-march=x86-64-v3'
endif

if target_cpu == 'aarch64'
  optflags += '-march=armv8.2-a+crypto+fp16+rcpc+dotprod' # Surface Pro X
endif

cxxflags = cpp.get_supported_arguments(cxxflags)
add_project_arguments(cxxflags, language: 'cpp')

if get_option('optimization') in ['2', '3']
  optflags = cpp.get_supported_arguments(optflags)
  add_project_arguments(optflags, language: 'cpp')
endif

if get_option('access_checks')
  add_project_arguments('-D_GLIBCXX_ASSERTIONS', language: 'cpp')
endif

if get_option('force_access_checks')
  add_project_arguments('-DIPTSD_CONFIG_FORCE_ACCESS_CHECKS', language: 'cpp')
endif

add_project_arguments('-DSPDLOG_FMT_EXTERNAL', language: 'cpp')

# Dependencies
fmt = dependency('fmt', fallback: ['fmt', 'fmt_dep'], default_options: ['default_library=static'])
inih = dependency('inih', fallback: ['inih', 'inih_dep'], default_options: ['default_library=static'])
spdlog = dependency('spdlog', fallback: ['spdlog', 'spdlog_dep'], default_options: ['default_library=static'])
gsl = dependency('Microsoft.GSL', modules: ['Microsoft.GSL::GSL'],
                 fallback: ['microsoft-gsl', 'microsoft_gsl_dep'],
                 default_options: ['default_library=static'])

forcefallback = get_option('wrap_mode') == 'forcefallback'

hidrd_item = cpp.find_library('hidrd_item', required: false)
if forcefallback or not hidrd_item.found()
  hidrd_item = dependency('hidrd_item', default_options: ['default_library=static'])
endif

hidrd_usage = cpp.find_library('hidrd_usage', required: false)
if forcefallback or not hidrd_usage.found()
  hidrd_usage = dependency('hidrd_usage', default_options: ['default_library=static'])
endif

# Find libstdc++fs for older GCC
stdcppfs = cpp.find_library('stdc++fs')

# Find drm library
drm = dependency('libdrm')

subdir('common')
subdir('math')

subdir('container')
subdir('drm')
subdir('hid')
subdir('ipts')

subdir('contacts')
subdir('daemon')

if get_option('debug_tools').length() > 0
  gtkmm = dependency('gtkmm-3.0', required: false)
  cairomm = dependency('cairomm-1.0', required: false)
  cli11 = dependency('CLI11', required: false, fallback: ['cli11', 'CLI11_dep'], default_options: ['default_library=static'])

  subdir('gfx')
  subdir('debug')
endif
