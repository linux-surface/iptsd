deps = [
  fmt,
  iptsd_common,
  iptsd_container,
  iptsd_math,
  spdlog,
]

src = [
  'finder.cpp',
  'advanced/detector.cpp',
  'basic/cluster.cpp',
  'basic/detector.cpp',
  'basic/heatmap.cpp',
]

libcontacts = static_library('contacts', src, dependencies: deps, include_directories: includes)
iptsd_contacts = declare_dependency(dependencies: deps, link_with: libcontacts)
