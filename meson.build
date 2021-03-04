project('iptsd', 'cpp',
	version: '0.3.1',
	license: 'GPL-2.0-or-later',
	default_options: [
		'cpp_std=c++2a',
		'warning_level=3',
		'werror=true',
		'sysconfdir=/etc',
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
