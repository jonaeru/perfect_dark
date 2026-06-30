'use strict';

/**
 * Electron control panel for LLM Play — starts game + Gemma adapter and shows live status.
 */

const { app, BrowserWindow, ipcMain, shell } = require('electron');
const { spawn, spawnSync } = require('child_process');
const fs = require('fs');
const http = require('http');
const path = require('path');

const APP_TITLE = 'LLM Play';
const LOG_FILE = path.join(app.getPath('home'), 'Library/Logs/PerfectDarkLlmPlay.log');
const HEALTH_URL = 'http://127.0.0.1:8780/api/llm/health';
const GAME_MARKER = path.join('build', 'pd.arm64');

/** @type {BrowserWindow | null} */
let mainWindow = null;
/** @type {import('child_process').ChildProcess | null} */
let gameProcess = null;
/** @type {import('child_process').ChildProcess | null} */
let adapterProcess = null;
/** @type {NodeJS.Timeout | null} */
let pollTimer = null;
/** @type {boolean} */
let sessionStarting = false;

const session = {
  repoRoot: '',
  pythonBin: '',
  phase: 'idle',
  error: null,
  health: null,
  lastDecision: null,
  adapterStatus: null,
  ollamaHint: null,
};

function log(...args) {
  const line = `[${new Date().toISOString().slice(0, 19).replace('T', ' ')}] [llm-play] ${args.join(' ')}`;
  try {
    fs.mkdirSync(path.dirname(LOG_FILE), { recursive: true });
    fs.appendFileSync(LOG_FILE, `${line}\n`, 'utf8');
  } catch {
    // ignore
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
      if (cfgPath.endsWith('.json')) {
        const data = JSON.parse(fs.readFileSync(cfgPath, 'utf8'));
        return normalizeRepoRoot(data.repoRoot || '');
      }
      return normalizeRepoRoot(fs.readFileSync(cfgPath, 'utf8'));
    } catch {
      // try next
    }
  }
  return normalizeRepoRoot(process.env.PD_REPO_ROOT || '');
}

function discoverRepoRoot() {
  let candidate = app.isPackaged ? path.dirname(process.execPath) : path.resolve(__dirname, '../../..');
  for (let i = 0; i < 12; i += 1) {
    const gameBin = path.join(candidate, GAME_MARKER);
    if (fs.existsSync(gameBin)) {
      return normalizeRepoRoot(candidate);
    }
    const parent = path.dirname(candidate);
    if (parent === candidate) break;
    candidate = parent;
  }
  return '';
}

function resolveRepoRoot() {
  const baked = readBakedRepoRoot();
  if (baked && fs.existsSync(path.join(baked, GAME_MARKER))) {
    return baked;
  }
  return discoverRepoRoot();
}

function resolvePython() {
  const envPy = process.env.PD_PYTHON;
  const candidates = [];
  if (envPy) candidates.push(envPy);
  candidates.push(
    '/opt/homebrew/bin/python3',
    '/usr/local/bin/python3',
    '/Library/Frameworks/Python.framework/Versions/Current/bin/python3',
    path.join(app.getPath('home'), 'Library/Python/3.14/bin/python3'),
    path.join(app.getPath('home'), 'Library/Python/3.13/bin/python3'),
    path.join(app.getPath('home'), 'Library/Python/3.12/bin/python3'),
    path.join(app.getPath('home'), 'Library/Python/3.11/bin/python3'),
    path.join(app.getPath('home'), 'Library/Python/3.10/bin/python3'),
    '/usr/bin/python3',
  );

  const seen = new Set();
  for (const candidate of candidates) {
    if (!candidate || seen.has(candidate)) continue;
    seen.add(candidate);
    try {
      const result = spawnSync(candidate, ['-c', 'import sys; sys.exit(0 if sys.version_info >= (3, 10) else 1)'], {
        encoding: 'utf8',
        timeout: 5000,
      });
      if (result.status === 0) {
        return candidate;
      }
    } catch {
      // next
    }
  }
  return '';
}

function readLogTail(maxLines = 24) {
  try {
    if (!fs.existsSync(LOG_FILE)) return '';
    const lines = fs.readFileSync(LOG_FILE, 'utf8').split('\n');
    return lines.slice(-maxLines).join('\n').trim();
  } catch {
    return '';
  }
}

function readAdapterStatus(repoRoot) {
  if (!repoRoot) return null;
  const statusPath = path.join(repoRoot, 'journal', 'game_llm', 'perfect_dark', 'adapter_status.json');
  try {
    if (!fs.existsSync(statusPath)) return null;
    return JSON.parse(fs.readFileSync(statusPath, 'utf8'));
  } catch {
    return null;
  }
}

function killStaleAdapterProcesses(repoRoot) {
  if (!repoRoot) return;
  const patterns = [
    `${repoRoot}/tools/game_llm/run_adapter.py`,
    `${repoRoot}/tools/llm_adapter/run_adapter.py`,
  ];
  for (const pattern of patterns) {
    try {
      spawnSync('pkill', ['-f', pattern], { stdio: 'ignore' });
    } catch {
      // ignore
    }
  }
}

function killAllGameProcesses(repoRoot) {
  if (!repoRoot) return;
  try {
    spawnSync('pkill', ['-f', `${repoRoot}/build/pd.arm64`], { stdio: 'ignore' });
  } catch {
    // ignore
  }
  gameProcess = null;
}

/** Kill any process still bound to the LLM bridge port (stale zombies). */
function killProcessOnPort(port) {
  try {
    const result = spawnSync('lsof', ['-ti', `:${port}`], { encoding: 'utf8' });
    if (result.status !== 0 || !result.stdout?.trim()) return;
    for (const pid of result.stdout.trim().split('\n')) {
      if (!pid) continue;
      try {
        process.kill(Number(pid), 'SIGKILL');
      } catch {
        // ignore
      }
    }
  } catch {
    // ignore
  }
}

function checkOllamaModel(model = 'gemma3:4b') {
  return new Promise((resolve) => {
    const req = http.get('http://127.0.0.1:11434/api/tags', { timeout: 4000 }, (res) => {
      let body = '';
      res.on('data', (chunk) => { body += chunk; });
      res.on('end', () => {
        try {
          const payload = JSON.parse(body);
          const names = (payload.models || []).map((m) => m.name);
          const ok = names.includes(model) || names.some((n) => n.startsWith(`${model}:`));
          resolve({
            ok,
            message: ok
              ? `Ollama model ${model} ready`
              : `Ollama model '${model}' not installed. Click Pull Model or run: ollama pull ${model}`,
          });
        } catch {
          resolve({ ok: false, message: 'Could not read Ollama model list.' });
        }
      });
    });
    req.on('error', () => {
      resolve({ ok: false, message: 'Ollama is not running. Open the Ollama app, then pull gemma3:4b.' });
    });
    req.on('timeout', () => {
      req.destroy();
      resolve({ ok: false, message: 'Ollama timed out. Is it running?' });
    });
  });
}

function readLastDecision(repoRoot) {
  const logPath = path.join(repoRoot, 'journal', 'game_llm', 'perfect_dark', 'decisions.jsonl');
  try {
    if (!fs.existsSync(logPath)) return null;
    const lines = fs.readFileSync(logPath, 'utf8').trim().split('\n');
    const last = lines[lines.length - 1];
    if (!last) return null;
    const record = JSON.parse(last);
    return record.action || record;
  } catch {
    return null;
  }
}

function fetchHealth() {
  return new Promise((resolve) => {
    const req = http.get(HEALTH_URL, { timeout: 2000 }, (res) => {
      let body = '';
      res.on('data', (chunk) => { body += chunk; });
      res.on('end', () => {
        try {
          resolve(JSON.parse(body));
        } catch {
          resolve(null);
        }
      });
    });
    req.on('error', () => resolve(null));
    req.on('timeout', () => {
      req.destroy();
      resolve(null);
    });
  });
}

function waitForHealth(timeoutMs = 90000) {
  const deadline = Date.now() + timeoutMs;
  return new Promise((resolve) => {
    const tick = async () => {
      const health = await fetchHealth();
      if (health && health.ok) {
        resolve(health);
        return;
      }
      if (Date.now() >= deadline) {
        resolve(null);
        return;
      }
      setTimeout(tick, 1000);
    };
    tick();
  });
}

function appendProcessOutput(child, label) {
  const stream = fs.createWriteStream(LOG_FILE, { flags: 'a' });
  child.stdout?.on('data', (chunk) => {
    stream.write(`[${label}] ${chunk}`);
  });
  child.stderr?.on('data', (chunk) => {
    stream.write(`[${label}] ${chunk}`);
  });
  child.on('close', (code) => {
    stream.write(`[${label}] exited ${code}\n`);
    stream.end();
  });
}

function killProcess(proc) {
  if (!proc || proc.killed) return;
  try {
    proc.kill('SIGTERM');
    setTimeout(() => {
      try {
        if (!proc.killed) proc.kill('SIGKILL');
      } catch {
        // ignore
      }
    }, 2000);
  } catch {
    // ignore
  }
}

function computePhase(health) {
  if (session.error) return 'error';
  if (sessionStarting) return 'starting';
  if (!health) {
    if (gameProcess || adapterProcess) return 'starting';
    return session.phase === 'stopped' ? 'stopped' : 'idle';
  }
  if (!health.enabled) return 'bridge';
  if (health.adapter_connected) return 'ai_active';
  if (adapterProcess || session.adapterStatus?.error) return 'waiting_ai';
  return 'bridge';
}

function windowTitleForPhase(phase) {
  switch (phase) {
    case 'ai_active':
      return `${APP_TITLE} — AI active`;
    case 'waiting_ai':
      return `${APP_TITLE} — Waiting`;
    case 'bridge':
    case 'starting':
      return `${APP_TITLE} — ${phase === 'starting' ? 'Starting' : 'Bridge'}`;
    case 'error':
      return `${APP_TITLE} — Error`;
    default:
      return APP_TITLE;
  }
}

function buildStatusPayload() {
  const health = session.health;
  const phase = computePhase(health);
  session.phase = phase;
  return {
    phase,
    repoRoot: session.repoRoot,
    error: session.error || session.adapterStatus?.error || null,
    ollamaHint: session.ollamaHint,
    adapterStatus: session.adapterStatus,
    health,
    gameRunning: Boolean(gameProcess && !gameProcess.killed),
    adapterRunning: Boolean(adapterProcess && !adapterProcess.killed),
    lastDecision: session.lastDecision,
    logTail: readLogTail(),
  };
}

function pushStatus() {
  const payload = buildStatusPayload();
  if (mainWindow && !mainWindow.isDestroyed()) {
    mainWindow.setTitle(windowTitleForPhase(payload.phase));
    mainWindow.webContents.send('llm:status', payload);
  }
}

async function pollLoop() {
  session.health = await fetchHealth();
  if (session.repoRoot) {
    session.lastDecision = readLastDecision(session.repoRoot);
    session.adapterStatus = readAdapterStatus(session.repoRoot);
    if (session.adapterStatus?.error && !session.health?.adapter_connected) {
      session.ollamaHint = session.adapterStatus.error;
    }
  }

  if (!session.health && !gameProcess && !adapterProcess && session.phase !== 'starting') {
    session.phase = 'stopped';
  }

  /* Game window closed while we expected a session — auto-recover once. */
  if (
    session.health?.enabled
    && !gameProcess
    && !adapterProcess
    && !sessionStarting
    && session.phase !== 'stopped'
    && session.phase !== 'idle'
  ) {
    session.health = null;
    log('Game exited unexpectedly — restarting session');
    startSession().catch((err) => log('auto-restart failed', err.message));
  }

  if (session.health && !session.health.enabled && !gameProcess) {
    // game closed
    killProcess(adapterProcess);
    adapterProcess = null;
  }

  pushStatus();
}

function startPollLoop() {
  if (pollTimer) return;
  pollTimer = setInterval(() => {
    pollLoop().catch((err) => log('poll error', err.message));
  }, 2000);
  pollLoop().catch(() => {});
}

function stopPollLoop() {
  if (pollTimer) {
    clearInterval(pollTimer);
    pollTimer = null;
  }
}

function ensurePdIni(repoRoot, pythonBin) {
  const script = path.join(repoRoot, 'scripts', 'ensure-llm-pd-ini.py');
  const result = spawnSync(pythonBin, [script, path.join(repoRoot, 'pd.ini')], {
    encoding: 'utf8',
    cwd: repoRoot,
  });
  if (result.status !== 0) {
    throw new Error(result.stderr || result.stdout || 'ensure-llm-pd-ini.py failed');
  }
  log('pd.ini', (result.stdout || '').trim());
}

/** Redeploy uff with PDMAP_SEG_MODE=empty so in-box FPS has no phantom wall sheet. */
function ensureMapAssets(repoRoot, pythonBin) {
  const script = path.join(repoRoot, 'scripts', 'ensure-llm-map-assets.py');
  const result = spawnSync(pythonBin, [script], {
    encoding: 'utf8',
    cwd: repoRoot,
    env: { ...process.env, PDMAP_SEG_MODE: 'empty' },
  });
  if (result.status !== 0) {
    throw new Error(
      result.stderr || result.stdout || 'ensure-llm-map-assets.py failed (phantom wall seg?)',
    );
  }
  log('map', (result.stdout || '').trim());
}

function launchAdapter() {
  if (!session.repoRoot || !session.pythonBin || adapterProcess) return;
  const adapterScript = path.join(session.repoRoot, 'tools', 'game_llm', 'run_adapter.py');
  log('Launching adapter', session.pythonBin, adapterScript);
  adapterProcess = spawn(
    session.pythonBin,
    [adapterScript, '--game', 'perfect_dark', '--backend', 'auto'],
    {
      cwd: session.repoRoot,
      env: {
        ...process.env,
        PYTHONUNBUFFERED: '1',
        PATH: `/opt/homebrew/bin:/usr/local/bin:${process.env.PATH || ''}`,
      },
    },
  );
  appendProcessOutput(adapterProcess, 'adapter');
  adapterProcess.on('close', (code) => {
    adapterProcess = null;
    if (code !== 0 && !session.health?.adapter_connected) {
      const st = readAdapterStatus(session.repoRoot);
      if (st?.error) session.error = st.error;
    }
    pushStatus();
  });
}

async function startSession() {
  if (sessionStarting) {
    return buildStatusPayload();
  }
  if (gameProcess && adapterProcess) {
    return buildStatusPayload();
  }

  sessionStarting = true;
  session.error = null;
  session.repoRoot = resolveRepoRoot();
  pushStatus();

  try {
    if (!session.repoRoot) {
      throw new Error('Could not find the Perfect Dark repo (build/pd.arm64). Rebuild LLM Play.app from the repo.');
    }

    const gameBin = path.join(session.repoRoot, 'build', 'pd.arm64');
    if (!fs.existsSync(gameBin)) {
      throw new Error('Game binary missing. Run: cd build && cmake .. && cmake --build . -j8');
    }

    session.pythonBin = resolvePython();
    if (!session.pythonBin) {
      throw new Error('Python 3.10+ required. Install via brew install python or set PD_PYTHON.');
    }

    ensurePdIni(session.repoRoot, session.pythonBin);
    ensureMapAssets(session.repoRoot, session.pythonBin);

    const existingHealth = await fetchHealth();
    if (existingHealth?.ok && existingHealth.enabled && existingHealth.adapter_connected) {
      session.health = existingHealth;
      log('Reusing healthy session');
      startPollLoop();
      return buildStatusPayload();
    }

    killStaleAdapterProcesses(session.repoRoot);
    killAllGameProcesses(session.repoRoot);
    killProcessOnPort(8780);
    await new Promise((r) => setTimeout(r, 2000));

    log('Launching game');
    gameProcess = spawn(gameBin, ['--test-map', '--moddir', 'mods/mod_allinone'], {
      cwd: session.repoRoot,
      env: { ...process.env, PATH: `/opt/homebrew/bin:/usr/local/bin:${process.env.PATH || ''}` },
      detached: false,
    });
    appendProcessOutput(gameProcess, 'game');
    gameProcess.on('close', () => {
      gameProcess = null;
      killProcess(adapterProcess);
      adapterProcess = null;
      pushStatus();
    });
    const health = await waitForHealth(120000);

    if (!health) {
      throw new Error('Game bridge did not start within 2 minutes. Check ~/Library/Logs/PerfectDarkLlmPlay.log');
    }
    if (!health.enabled) {
      throw new Error('LLM bridge disabled in pd.ini. Run ensure-llm-pd-ini or enable [LLM] Enabled=1.');
    }
    session.health = health;

    killStaleAdapterProcesses(session.repoRoot);

    const ollama = await checkOllamaModel('gemma3:4b');
    session.ollamaHint = ollama.message;
    if (!ollama.ok) {
      session.error = ollama.message;
      log('Ollama preflight failed', ollama.message);
    } else if (!adapterProcess) {
      launchAdapter();
    }

    startPollLoop();
    log('Session started');
  } catch (err) {
    session.error = err.message || String(err);
    session.phase = 'error';
    log('start failed', session.error);
  } finally {
    sessionStarting = false;
    pushStatus();
  }

  return buildStatusPayload();
}

async function stopSession() {
  killProcess(adapterProcess);
  killProcess(gameProcess);
  adapterProcess = null;
  gameProcess = null;
  if (session.repoRoot) {
    killStaleAdapterProcesses(session.repoRoot);
    killAllGameProcesses(session.repoRoot);
  }
  session.health = null;
  session.error = null;
  session.phase = 'stopped';
  pushStatus();
  return buildStatusPayload();
}

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 420,
    height: 380,
    minWidth: 360,
    minHeight: 320,
    title: APP_TITLE,
    backgroundColor: '#0d1117',
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: true,
    },
  });

  mainWindow.loadFile(path.join(__dirname, 'ui', 'index.html'));

  mainWindow.on('closed', () => {
    mainWindow = null;
  });
}

ipcMain.handle('llm:get-status', () => buildStatusPayload());
ipcMain.handle('llm:start-session', () => startSession());
ipcMain.handle('llm:stop-session', () => stopSession());
ipcMain.handle('llm:open-log', () => {
  shell.openPath(LOG_FILE);
});

/** Pull default Gemma model via Ollama (can take several minutes). */
ipcMain.handle('llm:pull-model', async () => {
  session.error = null;
  session.ollamaHint = 'Pulling gemma3:4b… (this may take a few minutes)';
  pushStatus();
  return new Promise((resolve) => {
    const pull = spawn('ollama', ['pull', 'gemma3:4b'], {
      env: { ...process.env, PATH: `/opt/homebrew/bin:/usr/local/bin:${process.env.PATH || ''}` },
    });
    appendProcessOutput(pull, 'ollama-pull');
    pull.on('close', async (code) => {
      if (code === 0) {
        session.ollamaHint = 'Model installed. Restarting adapter…';
        session.error = null;
        killStaleAdapterProcesses(session.repoRoot);
        adapterProcess = null;
        pushStatus();
        launchAdapter();
        resolve({ ok: true });
      } else {
        session.error = 'ollama pull gemma3:4b failed. Check Terminal and Ollama app.';
        pushStatus();
        resolve({ ok: false });
      }
    });
  });
});

app.whenReady().then(async () => {
  createWindow();
  startPollLoop();
  // One-tap behavior: auto-start when the panel opens.
  await startSession();
});

app.on('window-all-closed', () => {
  stopPollLoop();
  killProcess(adapterProcess);
  killProcess(gameProcess);
  app.quit();
});

app.on('before-quit', () => {
  stopPollLoop();
  killProcess(adapterProcess);
  killProcess(gameProcess);
});
