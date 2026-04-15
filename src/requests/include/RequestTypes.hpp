/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

namespace RequestTypes {
constexpr const char *DISPLAY_IMAGE = "display-image";
constexpr const char *DISPLAY_VIDEO = "display-video";
constexpr const char *HIDE_CAMERA = "hide-camera";
constexpr const char *PRESS_KEY = "press-key";
constexpr const char *ROTATE_CAMERA = "rotate-camera";
} // namespace RequestTypes
