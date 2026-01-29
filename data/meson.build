# Desktop file
desktop_conf = configuration_data()
desktop_conf.set('APPLICATION_ID', application_id)
desktop_conf.set('APPLICATION_NAME', application_name)
desktop_conf.set('APPLICATION_DESCRIPTION', application_description)
desktop_conf.set('PACKAGE_NAME', package_name)

desktop_file_configured = configure_file(
          input: '@0@.desktop.in'.format(application_id),
         output: '@0@.desktop.in'.format(application_id),
  configuration: desktop_conf
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
