tools = get_option('debug_tools')

dbg_plot = tools.contains('plot')
dbg_show = tools.contains('show')

if dbg_plot and cairomm.found()
  deps = dependencies + [
    cairomm,
    cli11,
    iptsd_gfx,
    iptsd_ipts,
  ]

  executable('iptsd-plot', 'plot.cpp', dependencies: deps, include_directories: includes, install: true)
elif dbg_plot
  warning('Debug tool "plot" is enabled but has missing deps')
endif

