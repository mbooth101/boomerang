extension_dir = get_option('datadir') / 'gnome-shell' / 'extensions' / 'boomerang@matbooth.co.uk'

extension_files = [
  'extension.js',
  'metadata.json',
]

install_data(
  extension_files,
  install_dir: extension_dir,
)

install_subdir(
  'icons',
  install_dir: extension_dir,
)
