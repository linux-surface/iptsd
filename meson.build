project('iptsd', 'cpp',
        version: '1.4.0',
        license: 'GPL-2.0-or-later',
        meson_version: '>=0.56.0',
        default_options: [
          'cpp_std=c++17',
          'warning_level=3',
          'werror=true',
          'sysconfdir=/etc',
          'optimization=3',
          'debug=true',
          'b_lto=true',
        ])

prefix = get_option('prefix')
bindir = join_paths(prefix, get_option('bindir'))
datadir = join_paths(prefix, get_option('datadir'))
sysconfdir = get_option('sysconfdir')

presetdir = join_paths(datadir, 'iptsd')
configdir = join_paths(sysconfdir, 'iptsd.d')
configfile = join_paths(sysconfdir, 'iptsd.conf')

conf = configuration_data()
conf.set('bindir', bindir)
conf.set('datadir', datadir)
conf.set('sysconfdir', sysconfdir)

conf.set('presetdir', presetdir)
conf.set('configdir', configdir)
conf.set('configfile', configfile)

subdir('etc')
subdir('src')
