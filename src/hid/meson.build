subdir('shim')

sources = [
  'descriptor.cpp',
  'device.cpp',
]

deps = dependencies + [
  hidrd_shim,
]

libhid = static_library('hid', sources, dependencies: deps, include_directories: includes)
iptsd_hid = declare_dependency(dependencies: deps, link_with: libhid)
