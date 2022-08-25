sources = [
  'finder.cpp',
  'advanced/detector.cpp',
  'basic/cluster.cpp',
  'basic/detector.cpp',
  'basic/heatmap.cpp',
]

deps = dependencies

libcontacts = static_library('contacts', sources, dependencies: deps, include_directories: includes)
iptsd_contacts = declare_dependency(dependencies: deps, link_with: libcontacts)
