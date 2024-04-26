install_subdir(
	'presets',
	install_dir: presetdir,
	strip_directory: true,
)

install_data(
	'scripts/iptsd-find-hidraw',
	install_dir: bindir,
	install_mode: 'rwxr-xr-x',
)

install_data(
	'scripts/iptsd-foreach',
	install_dir: bindir,
	install_mode: 'rwxr-xr-x',
)

conf = configuration_data()
conf.set('bindir', bindir)
conf.set('datadir', datadir)
conf.set('sysconfdir', sysconfdir)

configure_file(
	input: 'systemd/iptsd@.service.in',
	output: 'iptsd@.service',
	configuration: conf,
)

configure_file(
	input: 'udev/50-iptsd.rules.in',
	output: '50-iptsd.rules',
	configuration: conf,
)

service_manager = get_option('service_manager')
if service_manager.contains('systemd')
	systemd = dependency('systemd', required: false)

	if systemd.found()
		unitdir = systemd.get_variable(pkgconfig: 'systemdsystemunitdir')

		udev = dependency('udev')
		udevdir = udev.get_variable(pkgconfig: 'udevdir')
		rulesdir = join_paths(udevdir, 'rules.d')

		unit = join_paths(meson.current_build_dir(), 'iptsd@.service')
		rule = join_paths(meson.current_build_dir(), '50-iptsd.rules')

		install_data(unit, install_dir: unitdir)
		install_data(rule, install_dir: rulesdir)

		install_data(
			'scripts/iptsd-systemd',
			install_dir: bindir,
			install_mode: 'rwxr-xr-x',
		)

		install_data(
			'scripts/iptsd-find-service',
			install_dir: bindir,
			install_mode: 'rwxr-xr-x',
		)
	else
		warning('systemd integration was enabled, but systemd was not found')
	endif
endif

if get_option('sample_config')
	install_data('iptsd.conf', install_dir: sysconfdir)

	if meson.version().version_compare('>=0.60')
		install_emptydir(configdir)
	else
		meson.add_install_script('sh', '-c', 'mkdir -p "$DESTDIR/@0@"'.format(configdir))
	endif
endif
