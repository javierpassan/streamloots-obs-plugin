# Streamloots - OBS Plugin

An OBS Studio plugin that receives real-time commands from [Streamloots](https://www.streamloots.com) to apply interactive card effects to your stream. Display images, play videos, hide your camera, rotate sources, and trigger hotkeys, all powered by viewer card redemptions.

**v3.0.0 update by [SyerNide](https://github.com/SyerNide)** - compatibility rewrite for OBS Studio 28+

## Compatibility

| OBS Studio | Status |
|------------|--------|
| 28.x - 32.x | Supported |
| 27.x and below | Use v2.0.0 |

## What Changed in v3.0.0

The v2 plugin was last updated in June 2023. Since then OBS moved to Qt6, bundled obs-websocket 5.x, and tightened thread safety rules. The v3 rewrite fixes all known compatibility issues while keeping the protocol identical (no backend changes needed).

- Overlays were invisible because the backend sends width:1/height:1 in metadata and v2 used those values directly. Now reads the actual OBS canvas resolution.
- Crashes on OBS 28+ from manipulating scene items on worker threads. Fixed with obs_queue_task().
- Memory leaks from source references not being released after cleanup.
- Qt6 compatibility for the settings dialog.
- PressKey switched from Windows SendInput() to the cross-platform OBS hotkey API.
- Settings (port, volume, monitoring mode) now persist via the OBS config API.
- Video player controls hidden on .webm overlays via CSS.

Full changelog: [CHANGELOG.md](CHANGELOG.md)

## How It Works

```
Streamloots Backend --ws://localhost:9006--> obs-streamloots plugin --OBS API--> Your Scene
```

A viewer redeems a card, Streamloots sends a WebSocket command, the plugin creates/modifies OBS sources, and the effect auto-cleans up after the timeout.

Same protocol, same port, same JSON format as v2.

## Supported Commands

| Command | Description |
|---------|-------------|
| display-image | Full-screen overlay (GIF, image, .webm) for N seconds |
| display-video | Same as display-image (both use browser source in v3) |
| hide-camera | Hides a named source for N seconds |
| rotate-camera | Rotates a named source by N degrees for N seconds |
| press-key | Triggers an OBS hotkey by name |

## Source Folder Structure

```
src/
  plugin-main.cpp       # OBS module entry point, server lifecycle
  Config.cpp/hpp        # Persistent settings (port, volume, monitoring)
  server/               # WebSocket server (websocketpp + ASIO)
  use-case/             # Request handlers (display, hide, rotate, etc.)
  requests/             # JSON request parsing and types
  responses/            # JSON response builders
  forms/                # Qt6 settings dialog (Tools > Streamloots)
```

## OBS Notes

### Important Types
- obs_scene_t: Represents a scene in OBS
- obs_source_t: Represents a source (attached or not to OBS), cannot apply transform
- obs_sceneitem_t: Represents a source attached to a scene, can apply transform operations
- obs_data_t: Similar to JSON, can be used for source properties or to pass data

More information: https://obsproject.com/docs/reference-core-objects.html

### Reference Repos
- https://github.com/obsproject/obs-studio
- https://github.com/obsproject/obs-websocket

### How to Find Property Names for a Source

OBS API documentation is unclear on property names. The methods to configure properties receive a `propId` which is a plain string with no documented values.

Workaround: inspect the properties directly from OBS.

1. Configure a source manually in OBS with the settings you want
2. Name the source with a unique identifier like `sample`
3. Use this code in any of the plugin's use cases:
```cpp
obs_source_t *sampleSource = obs_get_source_by_name("sample");
obs_data_t *sampleProperties = obs_source_get_settings(sampleSource);
const char *json = obs_data_get_json(sampleProperties);
blog(LOG_INFO, "sourceProperties: %s", json);
```
4. Execute the use case by sending a request from Postman
5. Check the log for the JSON serialization of the source settings

## Build from Source

See [BUILDING.md](BUILDING.md) for detailed step-by-step instructions. Quick version for Windows:

```powershell
mkdir build && cd build
cmake .. -G "Visual Studio 18 2026" -A x64 ^
  -DOBS_SOURCE_DIR="C:/obs-studio" ^
  -DOBS_BUILD_DIR="C:/obs-studio/build_x64" ^
  -DOBS_DEPS_DIR="C:/obs-studio/.deps/obs-deps-2025-08-23-x64" ^
  -DCMAKE_PREFIX_PATH="C:/obs-studio/.deps/obs-deps-qt6-2025-08-23-x64"
cmake --build . --config RelWithDebInfo --parallel
```

## GitHub Actions & CI

The scripts contained in `.github/scripts` can be used to build and package the plugin and take care of setting up obs-studio as well as its own dependencies. A default workflow for GitHub Actions is also provided and will use these scripts.

### Building a Release

Create and push a tag and GitHub Actions will run the pipeline in Release Mode. This mode uses the tag as its version number instead of the git ref in normal mode.

### Signing and Notarizing on macOS

On macOS, Release Mode builds can be signed and sent to Apple for notarization if the necessary codesigning credentials are added as secrets to your repository. You'll need a paid Apple Developer Account for this.

- On your Apple Developer dashboard, go to "Certificates, IDs & Profiles" and create two signing certificates:
    - One of the "Developer ID Application" type (signs the plugin binaries)
    - One of the "Developer ID Installer" type (signs the installer)
- Using the Keychain app on macOS, export these two certificates and keys into a .p12 file protected with a strong password
- Encode the .p12 file into its base64 representation by running `base64 YOUR_P12_FILE`
- Add the following secrets in your GitHub repository settings:
    - `MACOS_SIGNING_APPLICATION_IDENTITY`: Name of the "Developer ID Application" signing certificate
    - `MACOS_SIGNING_INSTALLER_IDENTITY`: Name of the "Developer ID Installer" signing certificate
    - `MACOS_SIGNING_CERT`: Base64-encoded string generated above
    - `MACOS_SIGNING_CERT_PASSWORD`: Password used to generate the .p12 certificate
    - `MACOS_NOTARIZATION_USERNAME`: Your Apple Developer account's username
    - `MACOS_NOTARIZATION_PASSWORD`: Your Apple Developer account's password (use a generated "app password" for this)

## License

GPL-2.0-or-later
