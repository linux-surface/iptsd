sources = [
  'device.cpp',
  'parser.cpp',
  'reader.cpp',
]

deps = dependencies + [
  iptsd_hid,
]

libipts = static_library('ipts', sources, dependencies: deps, include_directories: includes)
iptsd_ipts = declare_dependency(dependencies: deps, link_with: libipts)
