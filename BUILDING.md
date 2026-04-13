# Getting Started — obs-streamloots v3.0.0

This guide walks you through building the Streamloots OBS plugin from source on Windows, macOS, and Linux. The plugin is compatible with **OBS Studio 28.0.0 and later** (tested up to 31.x / 32.x).

---

## Prerequisites

| Tool | Version | Notes |
|------|---------|-------|
| CMake | 3.16+ | [cmake.org](https://cmake.org/download/) |
| Git | any | |
| C++ compiler | C++17 | MSVC 2022, Xcode 15+, or GCC 11+ |
| OBS Studio | 28+ | Must be installed (used to locate libobs) |
| Qt6 | 6.x | Bundled with OBS; point CMake at your OBS Qt |
| ASIO | latest | Header-only: [github.com/chriskohlhoff/asio](https://github.com/chriskohlhoff/asio) |
| WebSocket++ | latest | Header-only: [github.com/zaphoyd/websocketpp](https://github.com/zaphoyd/websocketpp) |

### Installing dependencies

**Ubuntu / Debian:**
```bash
sudo apt install build-essential cmake git obs-studio libobs-dev \
  libqt6-dev libasio-dev libwebsocketpp-dev
```

**macOS (Homebrew):**
```bash
brew install cmake obs-studio asio websocketpp
```

**Windows:**
Download and unpack ASIO and WebSocket++ to a known directory. You will pass the paths to CMake.

---

## Clone the repository

```bash
git clone https://github.com/streamloots/streamloots-obs-plugin.git
cd streamloots-obs-plugin
```

---

## Build

### Linux

```bash
mkdir build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build . --parallel
```

### macOS

```bash
mkdir build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build . --parallel
```

### Windows (Visual Studio 2022)

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 `
  -DASIO_DIR="C:\deps\asio" `
  -DCPPWS_DIR="C:\deps\websocketpp"
cmake --build . --config RelWithDebInfo
```

---

## Install

Copy the built plugin binary to your OBS plugins directory:

| Platform | Plugin directory |
|----------|-----------------|
| Windows | `C:\Program Files\obs-studio\obs-plugins\64bit\` |
| macOS | `~/Library/Application Support/obs-studio/plugins/obs-streamloots/bin/` |
| Linux | `~/.config/obs-studio/plugins/obs-streamloots/bin/64bit/` |

Then restart OBS Studio. You should see **Streamloots** under the **Tools** menu.

---

## VSCode setup

Install the **CMake Tools** extension, then create `.vscode/settings.json`:

```json
{
  "cmake.configureArgs": [
    "-DASIO_DIR=${workspaceFolder}/deps/asio",
    "-DCPPWS_DIR=${workspaceFolder}/deps/websocketpp"
  ],
  "cmake.buildDirectory": "${workspaceFolder}/build"
}
```

---

## Testing

1. Build and install the plugin (see above).
2. Open OBS Studio.
3. Go to **Tools → Streamloots** to verify the settings dialog opens.
4. Check the log (**Help → Log Files → View Current Log**) for:
   ```
   [obs-streamloots] obs-streamloots v3.0.0 loaded
   [obs-streamloots] WebSocket server listening on port 9006
   ```
5. Send a test WebSocket message to `ws://localhost:9006`:

```json
{
  "request-type": "display-image",
  "message-id": "test-001",
  "metadata": {
    "url": "https://media.giphy.com/media/v1.Y2lkPTc5MGI3NjExaHRoN2k/giphy.gif",
    "seconds": 5,
    "width": 400,
    "height": 400
  }
}
```

You can use a tool like **Postman** (WebSocket tab), **wscat**, or the browser console:

```javascript
const ws = new WebSocket('ws://localhost:9006');
ws.onopen = () => ws.send(JSON.stringify({
  "request-type": "display-image",
  "message-id": "test-002",
  "metadata": { "url": "https://example.com/image.gif", "seconds": 5 }
}));
ws.onmessage = (e) => console.log(e.data);
```

---

## Supported request types

| Type | Description | Metadata fields |
|------|-------------|-----------------|
| `display-image` | Shows a browser source with URL for N seconds | `url`, `seconds`, `width`, `height` |
| `display-video` | Shows a video browser source for N seconds | `url`, `seconds`, `width`, `height` |
| `hide-camera` | Hides a named source for N seconds | `source_name`, `seconds` |
| `press-key` | Triggers an OBS hotkey by name | `hotkey_name` |
| `rotate-camera` | Rotates a named source by degrees for N seconds | `source_name`, `degrees`, `seconds` |

---

## Changelog (v2.0.0 → v3.0.0)

- **OBS 28+ / Qt6 compatibility** — fully compatible with OBS Studio 28–32+
- **Thread safety** — all OBS object manipulation now uses `obs_queue_task()` for correct thread affinity
- **Improved cleanup** — temporary sources are properly released, preventing memory leaks
- **Modern CMake** — auto-detects system ASIO/WebSocket++ if not manually specified
- **Settings persistence** — port, volume, monitoring mode, auto-start saved via OBS config API
- **Safer PressKey** — uses OBS hotkey API instead of raw platform key simulation
- **Improved error handling** — JSON parse errors and missing sources return structured error responses
