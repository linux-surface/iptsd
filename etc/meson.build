install_subdir('config', install_dir: configdir, strip_directory: true)

configure_file(input: 'systemd/iptsd@.service.in', output: 'iptsd@.service', configuration: conf)
install_data('iptsd-find-hidraw', install_dir: bindir)

service_manager = get_option('service_manager')
if service_manager.contains('systemd')
  systemd = dependency('systemd', required: false)

  if systemd.found()
    unitdir = systemd.get_variable(pkgconfig: 'systemdsystemunitdir')

    udev = dependency('udev')
    udevdir = udev.get_variable(pkgconfig: 'udevdir')
    rulesdir = join_paths(udevdir, 'rules.d')

    unit = join_paths(meson.current_build_dir(), 'iptsd@.service')

    install_data(unit, install_dir: unitdir)
    install_data('udev/50-ipts.rules', install_dir: rulesdir)
  else
    warning('systemd integration was enabled, but systemd was not found')
  endif
endif

if get_option('sample_config')
  install_data('ipts.conf', install_dir: sysconfdir)
endif
