sources = [
  'cone.cpp',
  'config.cpp',
  'devices.cpp',
  'dft.cpp',
  'main.cpp',
  'stylus.cpp',
  'touch.cpp',
  'uinput-device.cpp',
]

deps = [
  fmt,
  gsl,
  inih,
  iptsd_common,
  iptsd_contacts,
  iptsd_drm,
  iptsd_ipts,
  iptsd_math,
  iptsd_container,
  spdlog,
  stdcppfs,
]

executable('iptsd', sources, dependencies: deps, include_directories: includes, install: true)
