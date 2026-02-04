data_conf = configuration_data()
data_conf.set('APPLICATION_ID', application_id)
data_conf.set('EXTENSION_UUID', extension_uuid)
data_conf.set('GETTEXT_DOMAIN', gettext_domain)
data_conf.set('PACKAGE_NAME', package_name)
data_conf.set('PACKAGE_VERSION', package_version)

# RPM Spec file
rpm_file_configured = configure_file(
          input: '@0@.spec.in'.format(package_name),
         output: '@0@.spec'.format(package_name),
  configuration: data_conf
)

# Desktop file
desktop_file_configured = configure_file(
          input: '@0@.desktop.in'.format(application_id),
         output: '@0@.desktop.in'.format(application_id),
  configuration: data_conf
)

desktop_file = i18n.merge_file(
        input: desktop_file_configured,
       output: '@0@.desktop'.format(application_id),
         type: 'desktop',
       po_dir: '../po',
      install: true,
  install_dir: get_option('datadir') / 'applications'
)

desktop_file_validate = find_program('desktop-file-validate', required:false)
if desktop_file_validate.found()
  test('Validate .desktop', desktop_file_validate, args: [desktop_file.full_path()])
endif

# Appdata file
appdata_file_configured = configure_file(
          input: '@0@.metainfo.xml.in'.format(application_id),
         output: '@0@.metainfo.xml.in'.format(application_id),
  configuration: data_conf
)

appdata_file = i18n.merge_file(
        input: appdata_file_configured,
       output: '@0@.metainfo.xml'.format(application_id),
       po_dir: '../po',
      install: true,
  install_dir: get_option('datadir') / 'metainfo'
)

appstream_util = find_program('appstream-util', required: false)
if appstream_util.found()
  test('Validate .metainfo.xml', appstream_util, args: ['validate-relax', '--nonet', appdata_file.full_path()])
endif

# Icon file
scalable_dir = 'icons' / 'hicolor' / 'scalable' / 'apps'
install_data(
  scalable_dir / ('@0@.svg').format(application_id),
  install_dir: get_option('datadir') / scalable_dir
)
