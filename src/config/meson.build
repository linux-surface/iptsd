sources = [
  'config.cpp'
]

deps = [
  gsl,
  inih,
  iptsd_common,
  iptsd_contacts,
  iptsd_ipts,
]

libconfig = static_library('config', sources, dependencies: deps, include_directories: includes)
iptsd_config = declare_dependency(dependencies: deps, link_with: libconfig)
