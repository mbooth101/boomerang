# Main extension files
extension_dir = get_option('datadir') / 'gnome-shell' / 'extensions' / 'boomerang@matbooth.co.uk'

extension_files = [
  'extension.js',
]

install_data(
  extension_files,
  install_dir: extension_dir,
)

install_subdir(
  'icons',
  install_dir: extension_dir,
)

# Extension metadata file
# Note that extension metadata is not currently translatable, see bug: https://gitlab.gnome.org/GNOME/gnome-shell/-/issues/2288
metadata_conf = configuration_data()
metadata_conf.set('EXTENSION_UUID', extension_uuid)
metadata_conf.set('GETTEXT_DOMAIN', gettext_domain)
metadata_conf.set('PACKAGE_NAME', package_name)
metadata_conf.set('PACKAGE_VERSION', package_version)
metadata_conf.set('PACKAGE_WEBSITE', package_website)

metadata_file_configured = configure_file(
          input: 'metadata.json.in',
         output: 'metadata.json',
  configuration: metadata_conf,
    install_dir: extension_dir,
)
