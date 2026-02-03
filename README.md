# Boomerang

A description of this project.

## Building

Boomerang is built with the Meson build system:

    $ meson setup build
    $ meson compile -C build

Meson is also able to install Boomerang into your system:

    $ sudo meson install -C build

Gnome users should log out and back in order to activate the Gnome Shell extension.

## Translating

### Adding a New Translation

Re-generate the `po/boomerang.pot` file for the current code:

    $ meson compile -C build boomerang-pot

To add a new translation, add a new `<locale>.po` file where `<locale>` is a valid short locale code (e.g. "pt" for Portuguese) or sub-locale code (e.g. "pt_BR" for Brazilian Portuguese):

    $ cd po
    $ msginit --locale=<locale> --input=boomerang.pot

Open the `<locale>.po` file with your favourite editor and add your translation in the `msgstr` fields. Do not replace the original text in the `msgid` fields:

    #: hello:1
    msgid "Hello, world!"
    msgstr "Olá, mundo!"

Don't forget to add the new locale to the `LINGUAS` file.

### Updating an Existing Translation

Re-generate the `po/boomerang.pot` file for the current code:

    $ meson compile -C build boomerang-pot

To update a translation file with any new changes from the `boomerang.pot` file:

    $ cd po
    $ msgmerge --update <locale>.po boomerang.pot

This will add any new message IDs that need translating and attempt to match any changed message IDs with existing translations. Be sure to review translations marked as fuzzy and remove the fuzzy markers. You can check the progress of your translation by running:

    $ msgfmt --check --verbose <locale>.po

New translations and translation updates are welcomed as pull requests.

## License

Boomerang is made available under the [GNU General Public License Version 3](COPYING). The Boomerang logo and icons are based on the boomerang emoji character from the Google Noto Emoji font, and used here in accordance with the [SIL Open Font Licence](https://openfontlicense.org/open-font-license-official-text/).
