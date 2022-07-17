project('hidrd', 'c', meson_version: '>=0.56.0')

# Import autotools
autotools = import('unstable_external_project')

# Bootstrap configure
run_command('bootstrap', check: true)

configure_options = [
  '--prefix=@PREFIX@',
  '--libdir=@PREFIX@/@LIBDIR@',
  '--includedir=@PREFIX@/@INCLUDEDIR@',
]

library = get_option('default_library')

if library == 'shared'
  configure_options += '--enable-shared'
  configure_options += '--disable-static'
elif library == 'static'
  configure_options += '--disable-shared'
  configure_options += '--enable-static'
endif

hidrd = autotools.add_project('configure', configure_options : configure_options)

hidrd_item = hidrd.dependency('hidrd_item').as_system()
hidrd_util = hidrd.dependency('hidrd_util').as_system()
hidrd_usage = hidrd.dependency('hidrd_usage').as_system()

hidrd_util_dep = declare_dependency(dependencies: [hidrd_util])
hidrd_usage_dep = declare_dependency(dependencies: [hidrd_usage, hidrd_util])
hidrd_item_dep = declare_dependency(dependencies: [hidrd_item, hidrd_util, hidrd_usage])
