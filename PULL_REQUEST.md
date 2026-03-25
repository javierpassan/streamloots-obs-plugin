# Pull Request: v3.0.0 — OBS 28+ Compatibility Rewrite

**Author:** SyerNide
**Target:** `develop` branch
**Tested on:** OBS Studio 32.1.0 (Windows 11), with StreamElements OBS.Live

---

## Why This Update

The current OBS plugin (v2.0.0) was last released in June 2023 and has not been updated since. OBS Studio has gone through significant changes in that time — Qt6 migration, built-in WebSocket support, stricter thread safety — and the plugin no longer works correctly on current versions.

I've been a Streamloots community member for 5 years and noticed the plugin issues firsthand while testing card integrations on OBS 32. Rather than just reporting the problems, I rebuilt the plugin to fix them.

## What's Fixed

**Invisible overlays** — The most impactful bug. The Streamloots backend sends `width: 1` and `height: 1` in the display-image metadata. The v2 plugin used those values for the browser source dimensions, creating a 1x1 pixel source that was invisible on screen. The v3 plugin reads the actual OBS canvas resolution and uses that instead.

**Crashes on OBS 28+** — OBS tightened thread affinity requirements in v28. The v2 plugin's timer-based cleanup (removing temporary sources after N seconds) ran directly on worker threads, which causes crashes. All deferred operations now use `obs_queue_task(OBS_TASK_UI, ...)`.

**Memory leaks** — Source references from `obs_source_create()` and `obs_frontend_get_current_scene()` weren't always released in cleanup paths. Fixed across all use cases.

**Qt6 compatibility** — Settings dialog rewritten for Qt6 (OBS dropped Qt5 in v28).

**Build system** — Updated CMakeLists.txt that works with current OBS build output and Visual Studio 2022/2026. No longer depends on the old `ObsPluginHelpers.cmake`.

## What's Improved

- **Settings dialog** now includes port configuration, auto-start toggle, and start/stop buttons
- **PressKey** uses the OBS hotkey API instead of Windows `SendInput()` (cross-platform, safer)
- **Video display** uses browser_source instead of ffmpeg_source (streams from CDN, preserves .webm transparency)
- **Error responses** are structured JSON with error codes for backend debugging
- **Settings persistence** via `obs_frontend_get_global_config()` API

## What's Unchanged (No Backend Changes Needed)

- WebSocket server on port 9006 using websocketpp + ASIO
- Same JSON protocol: `request-type`, `message-id`, `metadata`
- Same five commands: display-image, display-video, hide-camera, press-key, rotate-camera
- Same Tools → Streamloots menu entry

## Testing

| Test | Status |
|------|--------|
| Plugin loads on OBS 32.1.0 | ✅ |
| Settings dialog works | ✅ |
| WebSocket server starts | ✅ |
| display-image with Streamloots .webm URL | ✅ |
| Full-screen scaling (with width:1/height:1 from backend) | ✅ |
| Auto-cleanup after timeout | ✅ |
| Video controls hidden on .webm overlays | ✅ |
| Co-exists with StreamElements OBS.Live | ✅ |

## Files Changed

All source files rewritten — see `CHANGELOG.md` for the complete file-by-file comparison.
