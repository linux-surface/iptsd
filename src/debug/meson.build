tools = get_option('debug_tools')

dbg_dump = tools.contains('dump')
dbg_finger = tools.contains('finger')
dbg_plot = tools.contains('plot')
dbg_rt = tools.contains('rt')

if dbg_dump and cli11.found()
  deps = dependencies + [
    cli11,
    iptsd_ipts,
  ]

  executable('ipts-dump', 'dump.cpp', dependencies: deps, include_directories: includes, install: true)
elif dbg_dump
  warning('Debug tool "dump" is enabled but has missing deps')
endif

if dbg_finger and cli11.found()
  deps = dependencies + [
    iptsd_config,
    iptsd_contacts,
    iptsd_ipts,
  ]

  executable('iptsd-finger-size', 'finger-size.cpp', dependencies: deps, include_directories: includes, install: true)
elif dbg_finger
  warning('Debug tool "finger" is enabled but has missing deps')
endif

if dbg_plot and cli11.found() and cairomm.found()
  deps = dependencies + [
    cairomm,
    cli11,
    iptsd_contacts,
    iptsd_gfx,
    iptsd_hid,
    iptsd_ipts,
  ]

  executable('ipts-proto-plot', 'proto-plot.cpp', dependencies: deps, include_directories: includes, install: true)
elif dbg_plot
  warning('Debug tool "plot" is enabled but has missing deps')
endif

if dbg_rt and cairomm.found() and sdl.found()
  deps = dependencies + [
    cairomm,
    iptsd_config,
    iptsd_contacts,
    iptsd_gfx,
    iptsd_ipts,
    sdl,
  ]

  executable('ipts-proto-rt', 'proto-rt.cpp', dependencies: deps, include_directories: includes, install: true)
elif dbg_rt
  warning('Debug tool "rt" is enabled but has missing deps')
endif
