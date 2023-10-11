cpp = meson.get_compiler('cpp')

configure_file(input: 'configure.h.in', output: 'configure.h', configuration: conf)

includes = [
  include_directories('.')
]

cxxflags = [
  '-Wmissing-include-dirs',
  '-Winit-self',
  '-Wimplicit-fallthrough',
  '-Wdouble-promotion',
  '-Wconversion',
]

optflags = []

target_cpu = target_machine.cpu_family()

if target_cpu == 'x86_64'
  optflags += '-march=x86-64-v3'
endif

if target_cpu == 'aarch64'
  optflags += '-march=armv8.2-a+crypto+fp16+rcpc+dotprod' # Surface Pro X
endif

if get_option('access_checks')
  cxxflags += '-D_GLIBCXX_ASSERTIONS'
endif

if get_option('force_access_checks')
  cxxflags += '-DIPTSD_CONFIG_FORCE_ACCESS_CHECKS'
endif

cxxflags += '-DSPDLOG_FMT_EXTERNAL'

cxxflags = cpp.get_supported_arguments(cxxflags)
add_project_arguments(cxxflags, language: 'cpp')

if get_option('optimization') in ['2', '3']
  optflags = cpp.get_supported_arguments(optflags)
endif

# Build wrapped dependencies as static libraries and disable warnings
dependency_options = [
  'default_library=static',
  'warning_level=0',
  'werror=false',
]

# Dependencies
cli11 = dependency('CLI11',
  fallback: ['cli11', 'CLI11_dep'],
  default_options: dependency_options,
)

eigen = dependency('eigen3',
  fallback: ['eigen', 'eigen_dep'],
  default_options: dependency_options,
  include_type: 'system',
)

fmt = dependency('fmt',
  fallback: ['fmt', 'fmt_dep'],
  default_options: dependency_options,
)

inih = dependency('INIReader',
  fallback: ['inih', 'INIReader_dep'],
  default_options: dependency_options,
)

gsl = dependency('Microsoft.GSL',
  modules: ['Microsoft.GSL::GSL'],
  fallback: ['microsoft-gsl', 'microsoft_gsl_dep'],
  default_options: dependency_options,
)

spdlog = dependency('spdlog',
  fallback: ['spdlog', 'spdlog_dep'],
  default_options: dependency_options + ['tests=false'],
)

# Find libstdc++fs for older GCC
stdcppfs = cpp.find_library('stdc++fs')

# Default dependencies
default_deps = [
  cli11,
  eigen,
  fmt,
  inih,
  gsl,
  spdlog,
  stdcppfs,
]

# The main iptsd daemon
executable('iptsd', 'apps/daemon/main.cpp',
           install: true,
           cpp_args: optflags,
           dependencies: default_deps,
           include_directories: includes,
)

executable('iptsd-check-device', 'apps/check-device/main.cpp',
           install: true,
           dependencies: default_deps,
           include_directories: includes,
)

tools = get_option('debug_tools')

if tools.contains('calibrate')
  executable('iptsd-calibrate', 'apps/calibrate/main.cpp',
             install: true,
             cpp_args: optflags,
             dependencies: default_deps,
             include_directories: includes,
  )
endif

if tools.contains('dump')
  executable('iptsd-dump', 'apps/dump/main.cpp',
             install: true,
             cpp_args: optflags,
             dependencies: default_deps,
             include_directories: includes,
  )
endif

if tools.contains('perf')
  executable('iptsd-perf', 'apps/perf/main.cpp',
             install: true,
             cpp_args: optflags,
             dependencies: default_deps,
             include_directories: includes,
  )
endif

if tools.contains('plot') or tools.contains('show')
  cairo = dependency('cairomm-1.0', required: false)
endif

if tools.contains('plot')
  if cairo.found()
    executable('iptsd-plot', 'apps/visualization/plot.cpp',
               install: true,
               cpp_args: optflags,
               dependencies: default_deps + [cairo],
               include_directories: includes,
    )
  else
    warning('Debug tool "plot" is enabled but cairomm was not found!')
  endif
endif

if tools.contains('show')
  sdl_options = [
    'test=false',
    'use_audio=disabled',
    'use_joystick=disabled',
    'use_haptic=disabled',
    'use_hidapi=disabled',
    'use_sensor=disabled',
    'use_video=enabled',
    'use_video_x11=enabled',
    'use_video_wayland=disabled',
    'use_video_wayland_libdecor=disabled',
    'use_video_opengl=disabled',
    'use_video_openglesv2=enabled',
    'use_video_vulkan=disabled',
    'use_video_offscreen=disabled',
    'system_iconv=disabled',
    'use_dummies=true',
    'prefer_dlopen=true',
  ]

  sdl = dependency('sdl2',
    fallback: ['sdl2', 'sdl2_dep'],
    default_options: dependency_options + sdl_options,
  )

  # Enabling compiler warnings in the consuming project is a great idea, SDL...
  sdl = sdl.partial_dependency(
    compile_args: sdl.type_name() == 'pkgconfig',
    includes: true,
    link_args: true,
    links: true,
    sources: true
  )

  if cairo.found()
    executable('iptsd-show', 'apps/visualization/show.cpp',
               install: true,
               cpp_args: optflags,
               dependencies: default_deps + [cairo, sdl],
               include_directories: includes,
    )
  else
    warning('Debug tool "show" is enabled but cairomm was not found!')
  endif
endif
