/**
 * Preload — safe bridges for native file picker and application menu actions.
 */
'use strict';

const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electronAPI', {
  /** Native JSON open dialog; returns { path, name, content } or null if cancelled. */
  openJsonFile: () => ipcRenderer.invoke('editor:open-json-file'),
  /** Subscribe to File/Edit/View/Play menu actions from the native menu bar. */
  onMenuAction: (callback) => {
    if (typeof callback !== 'function') return;
    ipcRenderer.on('editor:menu-action', (_event, action) => callback(action));
  },
});
