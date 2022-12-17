install_subdir('presets', install_dir: presetdir, strip_directory: true)

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
  install_data('iptsd.conf', install_dir: sysconfdir)

  if meson.version().version_compare('>=0.60')
    install_emptydir(configdir)
  else
    meson.add_install_script('sh', '-c', 'mkdir -p "$DESTDIR/@0@"'.format(configdir))
  endif
endif
