install_data('iptsd-reset-sensor', install_dir: bindir)
install_subdir('config', install_dir: configdir, strip_directory: true)

configure_file(input: 'systemd/iptsd.service.in',
	       output: 'iptsd.service',
	       configuration: conf)

configure_file(input: 'openrc/iptsd.initd.in',
	       output: 'iptsd.initd',
	       configuration: conf)

service_manager = get_option('service_manager')
if service_manager.contains('systemd')
	systemd = dependency('systemd', required: false)

	if systemd.found()
		unitdir = systemd.get_pkgconfig_variable('systemdsystemunitdir')

		udev = dependency('udev')
		udevdir = udev.get_pkgconfig_variable('udevdir')
		rulesdir = join_paths(udevdir, 'rules.d')

		install_data(join_paths(meson.current_build_dir(), 'iptsd.service'),
			     install_dir: unitdir)

		install_data('udev/50-ipts.rules', install_dir: rulesdir)
	else
		warning('systemd integration was enabled, but systemd was not found')
	endif
endif

if service_manager.contains('openrc')
	openrcdir = join_paths(sysconfdir, 'init.d')

	install_data(join_paths(meson.current_build_dir(), 'iptsd.initd'),
		     install_dir: openrcdir,
		     rename: 'iptsd')
endif

if get_option('sample_config')
	install_data('ipts.conf', install_dir: sysconfdir)
endif
