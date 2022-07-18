sources = [
  'hidrd.c',
]

deps = [
  hidrd_item,
  hidrd_usage,
]

libhidrd_shim = static_library('hidrd_shim', sources, dependencies: deps)
hidrd_shim = declare_dependency(dependencies: deps, link_with: libhidrd_shim)
