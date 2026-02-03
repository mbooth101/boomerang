project('boomerang', 'c',
          version: '0.1.0',
    meson_version: '>= 1.0.0',
  default_options: [ 'warning_level=2', 'werror=true', 'c_std=gnu17', ],
)

i18n = import('i18n')
gnome = import('gnome')

cc = meson.get_compiler('c')

application_id = 'uk.co.matbooth.Boomerang'
extension_uuid = 'boomerang@matbooth.co.uk'
gettext_domain = meson.project_name()
package_name = meson.project_name()
package_version = meson.project_version()
package_website = 'https://github.com/mbooth101/boomerang'
package_localedir = get_option('prefix') / get_option('datadir') / 'locale'

config_h = configuration_data()
config_h.set_quoted('APPLICATION_ID', application_id)
config_h.set_quoted('GETTEXT_DOMAIN', gettext_domain)
config_h.set_quoted('PACKAGE_NAME', package_name)
config_h.set_quoted('PACKAGE_VERSION', package_version)
config_h.set_quoted('PACKAGE_WEBSITE', package_website)
config_h.set_quoted('PACKAGE_LOCALE_DIR',  package_localedir)

configure_file(output: 'config.h', configuration: config_h)
add_project_arguments(['-I' + meson.project_build_root()], language: 'c')


project_c_args = []
test_c_args = [
  '-Wpointer-to-int-cast',
  '-Wformat-nonliteral',
  '-Wignored-qualifiers',
  '-Winit-self',
  '-Wlogical-op',
  '-Wmissing-declarations',
  '-Wmissing-format-attribute',
  '-Wmissing-include-dirs',
  '-Wmissing-noreturn',
  '-Wnested-externs',
  '-Wno-cast-function-type',
  '-Wno-dangling-pointer',
  '-Wno-missing-field-initializers',
  '-Wno-sign-compare',
  '-Wno-unused-parameter',
  '-Wold-style-definition',
  '-Wpointer-arith',
  '-Wredundant-decls',
  '-Wstrict-prototypes',
  '-Wswitch-default',
  '-Wswitch-enum',
  '-Wundef',
  '-Wwrite-strings',
  '-fno-strict-aliasing',
  ['-Werror=format-security', '-Werror=format=2'],
]
if get_option('buildtype') != 'plain'
  test_c_args += '-fstack-protector-strong'
endif
foreach arg: test_c_args
  if cc.has_multi_arguments(arg)
    project_c_args += arg
  endif
endforeach
add_project_arguments(project_c_args, language: 'c')

subdir('po')
subdir('data')
subdir('src')
subdir('extension')

gnome.post_install(
     glib_compile_schemas: true,
    gtk_update_icon_cache: true,
  update_desktop_database: true,
)
