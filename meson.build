project('iptsd', 'cpp',
	version: '0.3.1',
	license: 'GPL-2.0-or-later',
	default_options: [
		'cpp_std=c++17',
		'warning_level=3',
		'werror=true',
		'sysconfdir=/etc',
	])

cpp = meson.get_compiler('cpp')

prefix = get_option('prefix')
bindir = join_paths(prefix, get_option('bindir'))
datadir = join_paths(prefix, get_option('datadir'))
sysconfdir = get_option('sysconfdir')

configdir = join_paths(datadir, 'ipts')
configfile = join_paths(sysconfdir, 'ipts.conf')

conf = configuration_data()
conf.set('bindir', bindir)
conf.set('datadir', datadir)
conf.set('sysconfdir', sysconfdir)

conf.set('configdir', configdir)
conf.set('configfile', configfile)

configure_file(input: 'configure.h.in',
	       output: 'configure.h',
	       configuration: conf)

sources = [
	'src/cone.cpp',
	'src/config.cpp',
	'src/contact.cpp',
	'src/control.cpp',
	'src/data.cpp',
	'src/devices.cpp',
	'src/finger.cpp',
	'src/heatmap.cpp',
	'src/hid.cpp',
	'src/main.cpp',
	'src/reader.cpp',
	'src/payload.cpp',
	'src/singletouch.cpp',
	'src/stylus.cpp',
	'src/touch.cpp',
	'src/touch-processing.cpp',
	'src/uinput-device.cpp',
	'src/utils.cpp',
]

sources_dbg = [
	'debug/debug.cpp',
	'src/control.cpp',
	'src/utils.cpp',
]

includes = [
	 include_directories('src'),
]

deps = [
	dependency('inih', fallback: ['inih', 'inih_dep']),
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

executable('iptsd', sources,
	   dependencies: deps,
	   install: true,
	   include_directories: includes)

subdir('etc')

if get_option('debug_tool')
	executable('ipts-dbg', sources_dbg,
		   install: true,
		   include_directories: includes)
endif
