tools = get_option('debug_tools')

dbg_dump = tools.contains('dump')
dbg_plot = tools.contains('plot')
dbg_rt = tools.contains('rt')

if dbg_dump and cli11.found()
  deps = [
    cli11,
    fmt,
    gsl,
    iptsd_common,
    iptsd_ipts,
    spdlog,
    stdcppfs,
  ]

  executable('ipts-dump', 'dump.cpp', dependencies: deps, include_directories: includes, install: true)
elif dbg_rt
  warning('Debug tool "dump" is enabled but has missing deps')
endif

if dbg_plot and cli11.found() and cairomm.found()
  deps = [
    cairomm,
    cli11,
    fmt,
    iptsd_common,
    iptsd_contacts,
    iptsd_container,
    iptsd_gfx,
    iptsd_hid,
    iptsd_ipts,
    iptsd_math,
    spdlog,
    stdcppfs,
  ]

  executable('ipts-proto-plot', 'proto-plot.cpp', dependencies: deps, include_directories: includes, install: true)
elif dbg_plot
  warning('Debug tool "plot" is enabled but has missing deps')
endif

if dbg_rt and cairomm.found() and gtkmm.found()
  deps = [
    fmt,
    gtkmm,
    cairomm,
    iptsd_common,
    iptsd_contacts,
    iptsd_container,
    iptsd_gfx,
    iptsd_ipts,
    iptsd_math,
    spdlog,
  ]

  executable('ipts-proto-rt', 'proto-rt.cpp', dependencies: deps, include_directories: includes, install: true)
elif dbg_rt
  warning('Debug tool "rt" is enabled but has missing deps')
endif
