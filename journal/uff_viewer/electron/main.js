'use strict';

/**
 * Electron shell for the Perfect Dark uff map editor.
 * Spawns serve_editor.py and loads the editor in an embedded BrowserWindow
 * (no external browser dependency).
 */

const { app, BrowserWindow, dialog, ipcMain, Menu } = require('electron');
const { spawn } = require('child_process');
const fs = require('fs');
const http = require('http');
const path = require('path');

const APP_TITLE = 'Perfect Dark Map Editor';
const LOG_FILE = path.join(
  app.getPath('home'),
  'Library/Logs/PerfectDarkMapEditor.log'
);
const DEFAULT_HOST = '127.0.0.1';
const PORT_MIN = 8765;
const PORT_MAX = 8775;
const SERVE_MARKER = path.join('journal', 'uff_viewer', 'serve_editor.py');
const BUNDLED_EDITOR_SUBDIR = 'editor';

/** @type {import('child_process').ChildProcess | null} */
let serverProcess = null;
/** @type {BrowserWindow | null} */
let mainWindow = null;
/** @type {string} */
let activePort = '';
/** @type {boolean} */
let quitting = false;
/** @type {number} */
let restartAttempts = 0;
/** @type {{ pythonBin: string, serveScript: string, repoRoot: string, stateDir: string, bundleDir: string } | null} */
let serverContext = null;

const MAX_RESTART_ATTEMPTS = 3;

function log(...args) {
  const line = `[${new Date().toISOString().slice(0, 19).replace('T', ' ')}] [electron] ${args.join(' ')}`;
  try {
    fs.mkdirSync(path.dirname(LOG_FILE), { recursive: true });
    fs.appendFileSync(LOG_FILE, `${line}\n`, 'utf8');
  } catch {
    // ignore log write failures
  }
  console.log(line);
}

function normalizeRepoRoot(raw) {
  if (!raw) return '';
  let p = raw.trim();
  if (p.includes('/Library/MobileDocuments/')) {
    p = p.replace(/\/Library\/MobileDocuments\//g, '/Library/Mobile Documents/');
  }
  return p;
}

function fileReadable(filePath) {
  try {
    fs.accessSync(filePath, fs.constants.R_OK);
    const fd = fs.openSync(filePath, 'r');
    const buf = Buffer.alloc(1);
    fs.readSync(fd, buf, 0, 1, 0);
    fs.closeSync(fd);
    return true;
  } catch {
    return false;
  }
}

async function ensureLocalFile(filePath, maxAttempts = 12) {
  for (let attempt = 0; attempt < maxAttempts; attempt += 1) {
    if (fileReadable(filePath)) {
      return true;
    }
    // eslint-disable-next-line no-await-in-loop
    await new Promise((r) => setTimeout(r, 350));
  }
  return false;
}

function resolveBundledEditorDir() {
  const resourceDir = process.resourcesPath || __dirname;
  const candidates = [
    path.join(resourceDir, BUNDLED_EDITOR_SUBDIR),
    path.join(__dirname, BUNDLED_EDITOR_SUBDIR),
  ];
  for (const bundleDir of candidates) {
    const serveScript = path.join(bundleDir, 'serve_editor.py');
    const html = path.join(bundleDir, 'uff_map.html');
    if (fileReadable(serveScript) && fileReadable(html)) {
      return bundleDir;
    }
  }
  return '';
}

function repoRootLooksValid(root) {
  if (!root) return false;
  // Bundled editor does not need journal/uff_viewer on disk (iCloud-safe).
  if (resolveBundledEditorDir()) {
    return fs.existsSync(root);
  }
  return fileReadable(path.join(root, SERVE_MARKER));
}

function readBakedRepoRoot() {
  const resourceDir = process.resourcesPath || __dirname;
  const candidates = [
    path.join(__dirname, 'repo-config.json'),
    path.join(resourceDir, 'repo-config.json'),
    path.join(resourceDir, 'repo_root.txt'),
  ];
  for (const cfgPath of candidates) {
    try {
      if (!fileReadable(cfgPath)) continue;
      let root = '';
      if (cfgPath.endsWith('.json')) {
        const cfg = JSON.parse(fs.readFileSync(cfgPath, 'utf8'));
        root = normalizeRepoRoot(cfg.repoRoot || '');
      } else {
        root = normalizeRepoRoot(fs.readFileSync(cfgPath, 'utf8'));
      }
      if (repoRootLooksValid(root)) {
        return root;
      }
    } catch {
      // try next
    }
  }
  return '';
}

function discoverRepoRoot(startDir) {
  let candidate = path.resolve(startDir);
  for (let i = 0; i < 12; i += 1) {
    const marker = path.join(candidate, SERVE_MARKER);
    if (fileReadable(marker)) {
      return candidate;
    }
    const parent = path.dirname(candidate);
    if (parent === candidate) break;
    candidate = parent;
  }
  return '';
}

function resolveRepoRoot() {
  const fromEnv = normalizeRepoRoot(process.env.PD_REPO_ROOT || '');
  if (fromEnv && repoRootLooksValid(fromEnv)) {
    log(`REPO_ROOT from PD_REPO_ROOT=${fromEnv}`);
    return fromEnv;
  }

  const baked = readBakedRepoRoot();
  if (baked) {
    log(`REPO_ROOT from baked config=${baked}`);
    return baked;
  }

  const exeDir = path.dirname(app.getPath('exe'));
  const fromExe = discoverRepoRoot(exeDir);
  if (fromExe) {
    log(`REPO_ROOT discovered from exe=${fromExe}`);
    return fromExe;
  }

  const fromCwd = discoverRepoRoot(process.cwd());
  if (fromCwd) {
    log(`REPO_ROOT discovered from cwd=${fromCwd}`);
    return fromCwd;
  }

  return '';
}

function resolveEditorPaths(repoRoot) {
  const bundleDir = resolveBundledEditorDir();
  if (bundleDir) {
    return {
      bundleDir,
      serveScript: path.join(bundleDir, 'serve_editor.py'),
      source: 'bundle',
    };
  }
  const repoViewer = path.join(repoRoot, 'journal', 'uff_viewer');
  const repoServe = path.join(repoViewer, 'serve_editor.py');
  if (fileReadable(repoServe)) {
    return {
      bundleDir: '',
      serveScript: repoServe,
      source: 'repo',
    };
  }
  return null;
}

function resolveStateDir(repoRoot, editorSource) {
  const envState = (process.env.PD_EDITOR_STATE_DIR || '').trim();
  if (envState) {
    fs.mkdirSync(envState, { recursive: true });
    return envState;
  }
  const repoViewer = path.join(repoRoot, 'journal', 'uff_viewer');
  if (editorSource === 'repo') {
    try {
      const probe = path.join(repoViewer, '.pd_editor_write_probe');
      fs.writeFileSync(probe, 'ok', 'utf8');
      fs.unlinkSync(probe);
      return repoViewer;
    } catch {
      // fall through
    }
  }
  const fallback = path.join(
    app.getPath('home'),
    'Library/Application Support/PerfectDarkMapEditor'
  );
  fs.mkdirSync(fallback, { recursive: true });
  return fallback;
}

function resolvePython() {
  const candidates = [];
  const seen = new Set();

  const add = (p) => {
    if (!p || seen.has(p)) return;
    seen.add(p);
    candidates.push(p);
  };

  if (process.env.PD_PYTHON) add(process.env.PD_PYTHON);
  add('/opt/homebrew/bin/python3');
  add('/usr/local/bin/python3');
  add('/Library/Frameworks/Python.framework/Versions/Current/bin/python3');
  const home = app.getPath('home');
  for (const ver of ['3.14', '3.13', '3.12', '3.11', '3.10']) {
    add(path.join(home, 'Library/Python', ver, 'bin/python3'));
  }
  add('/usr/bin/python3');

  for (const py of candidates) {
    try {
      if (!fs.existsSync(py)) continue;
      const { status } = require('child_process').spawnSync(py, [
        '-c',
        'import sys; sys.exit(0 if sys.version_info >= (3, 10) else 1)',
      ]);
      if (status === 0) {
        return py;
      }
    } catch {
      // try next
    }
  }
  return '';
}

function fetchHealth(port) {
  return new Promise((resolve) => {
    const req = http.get(
      {
        hostname: DEFAULT_HOST,
        port,
        path: '/api/health',
        timeout: 2000,
      },
      (res) => {
        let body = '';
        res.on('data', (chunk) => {
          body += chunk;
        });
        res.on('end', () => {
          try {
            resolve(JSON.parse(body));
          } catch {
            resolve(null);
          }
        });
      }
    );
    req.on('error', () => resolve(null));
    req.on('timeout', () => {
      req.destroy();
      resolve(null);
    });
  });
}

function healthOk(port) {
  return new Promise((resolve) => {
    const req = http.get(
      {
        hostname: DEFAULT_HOST,
        port,
        path: '/api/health',
        timeout: 2000,
      },
      (res) => {
        res.resume();
        resolve(res.statusCode === 200);
      }
    );
    req.on('error', () => resolve(false));
    req.on('timeout', () => {
      req.destroy();
      resolve(false);
    });
  });
}

async function findHealthyPort(minPort, maxPort, expectedBundleDir = '') {
  for (let port = minPort; port <= maxPort; port += 1) {
    // eslint-disable-next-line no-await-in-loop
    const health = await fetchHealth(port);
    if (!health || health.ok !== true) continue;
    if (expectedBundleDir && health.bundleDir !== expectedBundleDir) {
      continue;
    }
    return String(port);
  }
  return '';
}

async function waitForServerReady(expectedPid, stateDir) {
  const portFile = path.join(stateDir, '.editor_server.port');
  for (let attempt = 0; attempt < 80; attempt += 1) {
    let portFromFile = '';
    try {
      if (fs.existsSync(portFile)) {
        portFromFile = fs.readFileSync(portFile, 'utf8').trim();
      }
    } catch {
      // ignore
    }

    if (portFromFile && (await healthOk(Number(portFromFile)))) {
      return portFromFile;
    }

    const scanned = await findHealthyPort(PORT_MIN, PORT_MAX, serverContext?.bundleDir || '');
    if (scanned && serverProcess && !serverProcess.killed) {
      return scanned;
    }

    if (serverProcess && serverProcess.exitCode !== null) {
      throw new Error(`serve_editor.py exited with code ${serverProcess.exitCode}`);
    }

    await new Promise((r) => setTimeout(r, 150));
  }
  throw new Error('Timed out waiting for /api/health on ports 8765–8775');
}

function buildSpawnEnv(repoRoot, stateDir, bundleDir) {
  const pythonPath = [
    repoRoot,
    bundleDir,
    path.join(repoRoot, 'journal', 'uff_viewer'),
    process.env.PYTHONPATH || '',
  ]
    .filter(Boolean)
    .join(':');
  const env = {
    ...process.env,
    PD_REPO_ROOT: repoRoot,
    PD_EDITOR_STATE_DIR: stateDir,
    PYTHONPATH: pythonPath,
    PATH: `/opt/homebrew/bin:/usr/local/bin:${process.env.PATH || ''}`,
  };
  if (bundleDir) {
    env.PD_EDITOR_BUNDLE_DIR = bundleDir;
  }
  return env;
}

function formatSpawnCommand(pythonBin, args, env) {
  const argStr = args.map((a) => JSON.stringify(a)).join(' ');
  return `${JSON.stringify(pythonBin)} ${argStr} (PD_REPO_ROOT=${env.PD_REPO_ROOT}, PD_EDITOR_STATE_DIR=${env.PD_EDITOR_STATE_DIR}, PYTHONPATH=${env.PYTHONPATH})`;
}

function spawnServer(pythonBin, serveScript, repoRoot, stateDir, bundleDir) {
  const spawnArgs = [
    serveScript,
    '--host',
    DEFAULT_HOST,
    '--port',
    String(PORT_MIN),
  ];
  const spawnEnv = buildSpawnEnv(repoRoot, stateDir, bundleDir);
  log(`Spawn command: ${formatSpawnCommand(pythonBin, spawnArgs, spawnEnv)}`);
  log(
    `Starting serve_editor.py from ${serveScript} (bundle=${bundleDir || 'repo'}, state=${stateDir})`
  );
  const child = spawn(pythonBin, spawnArgs, {
    cwd: repoRoot,
    env: spawnEnv,
    stdio: ['ignore', 'pipe', 'pipe'],
  });

  child.stdout.on('data', (chunk) => {
    const text = chunk.toString().trim();
    if (text) log(`[serve_editor] ${text}`);
  });
  child.stderr.on('data', (chunk) => {
    const text = chunk.toString().trim();
    if (text) log(`[serve_editor:err] ${text}`);
  });
  child.on('exit', (code, signal) => {
    handleServerExit(code, signal);
  });

  return child;
}

async function handleServerExit(code, signal) {
  const exitedPid = serverProcess ? serverProcess.pid : 'unknown';
  log(
    `serve_editor.py exited pid=${exitedPid} code=${code} signal=${signal || ''}`
  );
  serverProcess = null;

  if (quitting) {
    return;
  }

  const stillHealthy = await findHealthyPort(PORT_MIN, PORT_MAX, serverContext?.bundleDir || '');
  if (stillHealthy) {
    log(
      `Editor still healthy on port ${stillHealthy} after child exit; adopting existing server`
    );
    activePort = stillHealthy;
    restartAttempts = 0;
    if (mainWindow && !mainWindow.isDestroyed()) {
      mainWindow.loadURL(editorUrl(stillHealthy));
    }
    return;
  }

  if (!serverContext) {
    log('No server context for restart; cannot recover');
    return;
  }

  if (restartAttempts < MAX_RESTART_ATTEMPTS) {
    restartAttempts += 1;
    log(
      `Restarting editor server (attempt ${restartAttempts}/${MAX_RESTART_ATTEMPTS})`
    );
    try {
      serverProcess = spawnServer(
        serverContext.pythonBin,
        serverContext.serveScript,
        serverContext.repoRoot,
        serverContext.stateDir,
        serverContext.bundleDir
      );
      log(`Restarted server PID ${serverProcess.pid}`);
      activePort = await waitForServerReady(serverProcess.pid, serverContext.stateDir);
      restartAttempts = 0;
      log(`Restart healthy on port ${activePort}`);
      if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.loadURL(editorUrl(activePort));
      }
    } catch (err) {
      log(`Restart failed: ${err.message}`);
    }
    return;
  }

  if (mainWindow) {
    const detail =
      signal === 'SIGTERM' && code === null
        ? 'The server process was stopped (SIGTERM).'
        : `Editor server stopped unexpectedly (code ${code}, signal ${signal || 'none'}).`;
    dialog.showErrorBox(APP_TITLE, `${detail}\n\nLog: ${LOG_FILE}`);
    app.quit();
  }
}

function showStartupError(message) {
  log(`ERROR: ${message}`);
  let tail = '';
  try {
    if (fileReadable(LOG_FILE)) {
      const lines = fs.readFileSync(LOG_FILE, 'utf8').split('\n').filter(Boolean);
      tail = lines.slice(-20).join('\n');
    }
  } catch {
    // ignore
  }
  dialog.showErrorBox(
    APP_TITLE,
    `${message}\n\nLog: ${LOG_FILE}${tail ? `\n\nRecent log:\n${tail}` : ''}`
  );
}

function editorUrl(port) {
  // One-shot cache bust on load; server sends no-store for HTML anyway.
  return `http://${DEFAULT_HOST}:${port}/?v=${Date.now()}`;
}

function sendMenuAction(win, action) {
  const w = win || mainWindow;
  if (w && !w.isDestroyed()) {
    w.webContents.send('editor:menu-action', action);
  }
}

/** Forward single-key shortcuts to the renderer (menu accelerators miss keyDown in focused webviews on macOS). */
function bindEditorShortcuts(contents) {
  const KEY_ACTIONS = {
    KeyE: 'toggle-edit',
    KeyF: 'toggle-fly',
    KeyG: 'toggle-snap-grid',
    KeyM: 'toggle-minimap',
    KeyT: 'test-map',
    KeyX: 'export-assets',
  };
  contents.on('before-input-event', (event, input) => {
    if (input.type !== 'keyDown' || input.isAutoRepeat) return;
    if (input.control || input.meta || input.alt || input.shift) return;
    const action = KEY_ACTIONS[input.code];
    if (!action) return;
    event.preventDefault();
    sendMenuAction(mainWindow, action);
  });
}

/** Native macOS application menu — document, edit, view, play surfaces. */
function buildApplicationMenu(win) {
  const isMac = process.platform === 'darwin';
  const template = [
    ...(isMac
      ? [{
          label: app.name,
          submenu: [
            { role: 'about' },
            { type: 'separator' },
            { role: 'services' },
            { type: 'separator' },
            { role: 'hide' },
            { role: 'hideOthers' },
            { role: 'unhide' },
            { type: 'separator' },
            { role: 'quit' },
          ],
        }]
      : []),
    {
      label: 'File',
      submenu: [
        { label: 'New Map', accelerator: 'CmdOrCtrl+N', click: () => sendMenuAction(win, 'new') },
        { label: 'Open…', accelerator: 'CmdOrCtrl+O', click: () => sendMenuAction(win, 'open') },
        { label: 'Open Saved…', click: () => sendMenuAction(win, 'open-saved') },
        { type: 'separator' },
        { label: 'Save', accelerator: 'CmdOrCtrl+S', click: () => sendMenuAction(win, 'save') },
        { label: 'Save As…', accelerator: 'CmdOrCtrl+Shift+S', click: () => sendMenuAction(win, 'save-as') },
        { type: 'separator' },
        { label: 'Export JSON…', click: () => sendMenuAction(win, 'export') },
        { label: 'Revert to Cached', click: () => sendMenuAction(win, 'revert-cached') },
        { type: 'separator' },
        { label: 'Delete Map…', click: () => sendMenuAction(win, 'delete') },
        ...(!isMac ? [{ type: 'separator' }, { role: 'quit' }] : []),
      ],
    },
    {
      label: 'Edit',
      submenu: [
        { label: 'Undo', accelerator: 'CmdOrCtrl+Z', click: () => sendMenuAction(win, 'undo') },
        { label: 'Redo', accelerator: 'CmdOrCtrl+Shift+Z', click: () => sendMenuAction(win, 'redo') },
      ],
    },
    {
      label: 'View',
      submenu: [
        { label: 'Toggle Fly Mode', click: () => sendMenuAction(win, 'toggle-fly') },
        { label: 'Toggle Edit Mode', click: () => sendMenuAction(win, 'toggle-edit') },
        { label: 'Toggle Minimap', click: () => sendMenuAction(win, 'toggle-minimap') },
        { type: 'separator' },
        { label: 'Isometric', click: () => sendMenuAction(win, 'view-iso') },
        { label: 'Top', click: () => sendMenuAction(win, 'view-top') },
        { label: 'Front', click: () => sendMenuAction(win, 'view-front') },
        { label: 'Side', click: () => sendMenuAction(win, 'view-side') },
        { type: 'separator' },
        { label: 'Toggle Help', click: () => sendMenuAction(win, 'toggle-help') },
      ],
    },
    {
      label: 'Play',
      submenu: [
        { label: 'Test Map', click: () => sendMenuAction(win, 'test-map') },
        { label: 'Export Assets', click: () => sendMenuAction(win, 'export-assets') },
        { type: 'separator' },
        { label: 'Build Settings…', click: () => sendMenuAction(win, 'build-settings') },
      ],
    },
  ];
  Menu.setApplicationMenu(Menu.buildFromTemplate(template));
}

function createWindow(url) {
  mainWindow = new BrowserWindow({
    title: APP_TITLE,
    width: 1440,
    height: 900,
    minWidth: 1024,
    minHeight: 700,
    backgroundColor: '#0a0a12',
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: true,
    },
  });

  buildApplicationMenu(mainWindow);
  bindEditorShortcuts(mainWindow.webContents);
  mainWindow.loadURL(url);
  mainWindow.on('closed', () => {
    mainWindow = null;
  });
}

function killServer() {
  if (!serverProcess || serverProcess.killed) return;
  log(`Stopping server PID ${serverProcess.pid}`);
  try {
    serverProcess.kill('SIGTERM');
  } catch {
    // ignore
  }
  serverProcess = null;
}

async function bootstrap() {
  log(`=== electron map-editor start (PATH=${process.env.PATH || '<empty>'}) ===`);

  const repoRoot = resolveRepoRoot();
  if (!repoRoot) {
    showStartupError(
      'Could not locate the Perfect Dark repository (journal/uff_viewer/serve_editor.py).\n\n' +
        'Set PD_REPO_ROOT to your repo path, or rebuild with ./scripts/build-map-editor-electron.sh'
    );
    app.quit();
    return;
  }

  const editorPaths = resolveEditorPaths(repoRoot);
  if (!editorPaths) {
    showStartupError(
      `Map editor not found (bundled Resources/editor/ or ${path.join(repoRoot, 'journal/uff_viewer')}).\n\n` +
        'Rebuild: ./scripts/build-map-editor-electron.sh\n' +
        'If the repo is in iCloud Drive: Finder → right-click repo → Download Now.'
    );
    app.quit();
    return;
  }

  const { serveScript, bundleDir, source } = editorPaths;
  const stateDir = resolveStateDir(repoRoot, source);
  log(`Using editor source=${source}, bundle=${bundleDir || 'n/a'}, state=${stateDir}`);

  if (!(await ensureLocalFile(serveScript))) {
    showStartupError(
      `Map editor server script is not readable (iCloud may still be downloading):\n${serveScript}\n\n` +
        'Finder → right-click repo folder → Download Now, then retry.'
    );
    app.quit();
    return;
  }

  const existing = await findHealthyPort(PORT_MIN, PORT_MAX, bundleDir);
  if (existing) {
    activePort = existing;
    log(`Reusing healthy server on port ${activePort}`);
    createWindow(editorUrl(activePort));
    return;
  }

  const pythonBin = resolvePython();
  if (!pythonBin) {
    showStartupError(
      'Python 3.10+ is required but was not found.\n\nInstall: brew install python\nOr set PD_PYTHON to a 3.10+ interpreter.'
    );
    app.quit();
    return;
  }
  log(`Using python: ${pythonBin}`);

  serverContext = { pythonBin, serveScript, repoRoot, stateDir, bundleDir };
  serverProcess = spawnServer(pythonBin, serveScript, repoRoot, stateDir, bundleDir);
  log(`Server PID ${serverProcess.pid}`);

  try {
    activePort = await waitForServerReady(serverProcess.pid, stateDir);
  } catch (err) {
    showStartupError(`Editor server failed to start: ${err.message}`);
    quitting = true;
    killServer();
    app.quit();
    return;
  }

  const url = editorUrl(activePort);
  log(`Server healthy on port ${activePort}; loading ${url}`);
  createWindow(url);
}

app.whenReady().then(() => {
  ipcMain.handle('editor:open-json-file', async (event) => {
    const win = BrowserWindow.fromWebContents(event.sender);
    const result = await dialog.showOpenDialog(win || mainWindow, {
      title: 'Open map JSON',
      filters: [{ name: 'Map JSON', extensions: ['json'] }],
      properties: ['openFile'],
    });
    if (result.canceled || !result.filePaths.length) return null;
    const filePath = result.filePaths[0];
    try {
      const content = fs.readFileSync(filePath, 'utf8');
      return { path: filePath, name: path.basename(filePath), content };
    } catch (err) {
      throw new Error('Could not read file: ' + err.message);
    }
  });
  return bootstrap();
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('before-quit', () => {
  quitting = true;
  killServer();
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0 && activePort) {
    createWindow(editorUrl(activePort));
  }
});

process.on('SIGINT', () => {
  quitting = true;
  killServer();
  app.quit();
});

process.on('SIGTERM', () => {
  quitting = true;
  killServer();
  app.quit();
});
