sources = [
  'cone.cpp',
  'dft.cpp',
  'main.cpp',
]

deps = dependencies + [
  cli11,
  iptsd_config,
  iptsd_ipts,
]

executable('iptsd', sources, dependencies: deps, include_directories: includes, install: true)
