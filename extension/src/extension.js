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

async function executeBoomerang(indicator, cancellable = null) {
  const proc = new Gio.Subprocess({
    argv: ['boomerang'],
    flags: Gio.SubprocessFlags.NONE,
  });

  let cancelId = 0;
  try {
    indicator.add_style_class_name('screen-recording-indicator');

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
    Main.notifyError(_('Boomerang failed to launch'), e.message);
    console.error(e);
  } finally {
    indicator.remove_style_class_name('screen-recording-indicator');
    if (cancelId > 0) {
      cancellable.disconnect(cancelId);
    }
  }
}

const Indicator = GObject.registerClass(
  class Indicator extends PanelMenu.Button {
    _init(statusIconUri) {
      super._init(0.5, _('Boomerang'), true);

      const statusIcon = new Gio.FileIcon({
        file: Gio.File.new_for_uri(statusIconUri),
      });
      this.add_child(new St.Icon({
        gicon: statusIcon,
        style_class: 'system-status-icon',
      }));

      this.connect('button-press-event', (actor, event) => {
        return this._handleAction(actor, event);
      });
    }

    _handleAction(_, event) {
      if (event.get_button() === Clutter.BUTTON_PRIMARY) {
        executeBoomerang(this);
      }
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
        executeBoomerang(this._indicator);
      });
  }

  disable() {
    Main.wm.removeKeybinding('activate-boomerang-key');
    this._indicator.destroy();
    this._indicator = null;
  }
}
