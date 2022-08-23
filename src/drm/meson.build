sources = [
  'device.cpp',
]

deps = [
  drm,
  gsl,
  iptsd_common,
]

libdrm = static_library('drm', sources, dependencies: deps, include_directories: includes)
iptsd_drm = declare_dependency(dependencies: deps, link_with: libdrm)
