deps = [
  cairomm,
  gsl,
  iptsd_common,
  iptsd_contacts,
  iptsd_container,
  iptsd_math,
]

if cairomm.found()
  libgfx = static_library('gfx', 'visualization.cpp', dependencies: deps, include_directories: includes)
  iptsd_gfx = declare_dependency(dependencies: deps, link_with: libgfx)
endif
