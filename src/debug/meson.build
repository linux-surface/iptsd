deps = [
	cli11,
	fmt,
	gsl,
	iptsd_common,
	iptsd_ipts,
	spdlog,
]

executable('ipts-dbg', 'debug.cpp', dependencies: deps,
	   include_directories: includes, install: true)

deps = [
	cairomm,
	cairommpng,
	cli11,
	fmt,
	iptsd_common,
	iptsd_contacts,
	iptsd_container,
	iptsd_gfx,
	iptsd_ipts,
	iptsd_math,
	spdlog,
]

executable('ipts-proto-plot', 'proto-plot.cpp', dependencies: deps,
	   include_directories: includes, install: true)

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

executable('ipts-proto-rt', 'proto-rt.cpp', dependencies: deps,
	   include_directories: includes, install: true)
