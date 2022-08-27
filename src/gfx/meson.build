sources = [
  'visualization.cpp'
]

deps = dependencies + [
  cairomm,
  iptsd_config,
  iptsd_contacts,
]

if cairomm.found()
  libgfx = static_library('gfx', sources, dependencies: deps, include_directories: includes)
  iptsd_gfx = declare_dependency(dependencies: deps, link_with: libgfx)
endif
