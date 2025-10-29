project('saba', 'c', default_options: ['c_std=c23'])

notcurses_dep = dependency('notcurses-core')

executable(
    'saba',
    'main.c',
    dependencies: notcurses_dep
)
