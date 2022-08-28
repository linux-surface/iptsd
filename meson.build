project('iptsd', 'cpp', 'c',
        version: '0.5.1',
        license: 'GPL-2.0-or-later',
        meson_version: '>=0.58.0',
        default_options: [
          'cpp_std=c++17',
          'warning_level=3',
          'werror=true',
          'sysconfdir=/etc',
          'buildtype=debugoptimized',
          'b_lto=true',
        ])

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

subdir('etc')
subdir('src')
