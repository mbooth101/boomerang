boomerang_sources = [
  'main.c',
  'boomerang-application.c',
  'boomerang-canvas.c',
  'boomerang-screenshot.c',
]

boomerang_sources += gnome.compile_resources('boomerang-resources',
  'gresource.xml',
  c_name: 'boomerang'
)

boomerang_deps = [
  dependency('gtk4'),
  dependency('epoxy'),
]

executable('boomerang', boomerang_sources,
  dependencies: boomerang_deps,
  install: true,
)

