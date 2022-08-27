tools = get_option('debug_tools')

dbg_calibrate = tools.contains('calibrate')
dbg_dump = tools.contains('dump')
dbg_plot = tools.contains('plot')
dbg_show = tools.contains('show')

if dbg_calibrate and cli11.found()
  deps = dependencies + [
    iptsd_config,
    iptsd_contacts,
    iptsd_ipts,
  ]

  executable('iptsd-calibrate', 'calibrate.cpp', dependencies: deps, include_directories: includes, install: true)
elif dbg_calibrate
  warning('Debug tool "calibrate" is enabled but has missing deps')
endif

if dbg_dump and cli11.found()
  deps = dependencies + [
    cli11,
    iptsd_ipts,
  ]

  executable('iptsd-dump', 'dump.cpp', dependencies: deps, include_directories: includes, install: true)
elif dbg_dump
  warning('Debug tool "dump" is enabled but has missing deps')
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

if dbg_show and cairomm.found() and sdl.found()
  deps = dependencies + [
    cairomm,
    cli11,
    iptsd_config,
    iptsd_contacts,
    iptsd_gfx,
    iptsd_ipts,
    sdl,
  ]

  executable('iptsd-show', 'show.cpp', dependencies: deps, include_directories: includes, install: true)
elif dbg_show
  warning('Debug tool "show" is enabled but has missing deps')
endif
