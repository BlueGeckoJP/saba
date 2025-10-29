project('saba', 'c', default_options: ['c_std=c23'])

notcurses_dep = dependency('notcurses-core')
dbus_dep = dependency('dbus-1')

executable(
    'saba',
    ['src/main.c', 'src/metadata_utils.c'],
    dependencies: [notcurses_dep, dbus_dep]
)
