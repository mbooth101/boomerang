/* Copyright 2026 Mat Booth
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import Clutter from 'gi://Clutter';
import GObject from 'gi://GObject';
import Gio from 'gi://Gio';
import Shell from 'gi://Shell';
import St from 'gi://St';
import Meta from 'gi://Meta';

import { Extension, gettext as _ } from 'resource:///org/gnome/shell/extensions/extension.js';
import * as PanelMenu from 'resource:///org/gnome/shell/ui/panelMenu.js';

import * as Main from 'resource:///org/gnome/shell/ui/main.js';

async function executeBoomerang(screenshot, callback, cancellable = null) {
  const proc = new Gio.Subprocess({
    argv: ['boomerang', "-s", screenshot],
    flags: Gio.SubprocessFlags.NONE,
  });

  let cancelId = 0;
  try {
    proc.init(cancellable);

    if (cancellable instanceof Gio.Cancellable) {
      cancelId = cancellable.connect(() => proc.force_exit());
    }

    await new Promise((resolve, reject) => {
      proc.wait_async(cancellable, (p, res) => {
        try {
          p.wait_finish(res);
          resolve();
        } catch (e) {
          console.error(e);
          reject(e);
        }
      })
    });
  } catch (e) {
    // Only report an error if it wasn't the user just hitting cancel
    if (!e.matches(Gio.IOErrorEnum, Gio.IOErrorEnum.CANCELLED)) {
      Main.notifyError(_('Boomerang failed to launch'), e.message);
      console.error(e);
    }
  } finally {
    if (cancelId > 0) {
      cancellable.disconnect(cancelId);
    }
    callback();
  }
}

const Indicator = GObject.registerClass(
  class Indicator extends PanelMenu.Button {
    _init(iconUri) {
      super._init(0.5, _('Boomerang'), true);

      this._cancellable = null;

      const icon = new Gio.FileIcon({
        file: Gio.File.new_for_uri(iconUri),
      });

      this._boomerangIcon = new St.Icon({
        gicon: icon,
      });
      this._stopIcon = new St.Icon({
        icon_name: 'screencast-stop-symbolic',
      });

      this._boomerangStatusIcon = new St.Icon({
        gicon: icon,
        style_class: 'system-status-icon',
      });

      this._box = new St.BoxLayout();
      this._box.add_child(this._boomerangIcon);
      this._box.add_child(this._stopIcon);

      this.add_child(this._boomerangStatusIcon);

      this._clickGesture = new Clutter.ClickGesture();
      this._clickGesture.set_recognize_on_press(true);
      this._clickGesture.connect('recognize', () => {
        this.do_boomerang();
      });
      this.add_action(this._clickGesture);
    }

    async do_boomerang() {
      if (this._cancellable == null) {
        this._show_active();

        let [tempFile, stream] = Gio.File.new_tmp("boomerang-XXXXXX");
        this._filename = tempFile.get_path();
        const screenshot = new Shell.Screenshot();
        await screenshot.screenshot(false, stream.get_output_stream());
        stream.close(null);

        this._cancellable = new Gio.Cancellable();
        executeBoomerang(this._filename,
          () => {
            this._show_deactive();
            this._cancellable = null;
          }, this._cancellable);
      } else {
        this._cancellable.cancel();
      }
    }

    _show_active() {
      this.remove_child(this._boomerangStatusIcon);
      this.add_style_class_name('screen-recording-indicator');
      this.add_child(this._box);
    }

    _show_deactive() {
      this.remove_child(this._box);
      this.remove_style_class_name('screen-recording-indicator');
      this.add_child(this._boomerangStatusIcon);
    }
  });

export default class BoomerangExtension extends Extension {

  enable() {
    this._settings = this.getSettings();

    const iconUri = '%s/icons/hicolor/scalable/boomerang-status-symbolic.svg'.format(this.metadata.dir.get_uri());
    this._indicator = new Indicator(iconUri);
    Main.panel.addToStatusArea(this.uuid, this._indicator);

    Main.wm.addKeybinding('activate-boomerang-key', this._settings,
      Meta.KeyBindingFlags.IGNORE_AUTOREPEAT, Shell.ActionMode.NORMAL, () => {
        this._indicator.do_boomerang();
      });
  }

  disable() {
    Main.wm.removeKeybinding('activate-boomerang-key');
    this._indicator.destroy();
    this._indicator = null;
  }
}
