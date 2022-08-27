sources = [
  'cone.cpp',
  'devices.cpp',
  'dft.cpp',
  'main.cpp',
  'stylus.cpp',
  'touch.cpp',
  'uinput-device.cpp',
]

deps = dependencies + [
  cli11,
  iptsd_config,
  iptsd_contacts,
  iptsd_ipts,
]

executable('iptsd', sources, dependencies: deps, include_directories: includes, install: true)
