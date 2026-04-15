/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * PressKey — triggers an OBS hotkey by name. The v2 plugin used Windows
 * SendInput() to simulate raw keyboard presses, which was platform-specific
 * and could trigger anti-cheat false positives. Replaced with the OBS
 * hotkey API which is cross-platform and only affects OBS itself.
 */

#include <obs-module.h>
#include "include/PressKey.hpp"
#include "../plugin-macros.generated.h"
#include "../requests/utils/metadata.hpp"

#include <obs.h>
#include <string>

/*
 * OBS hotkey trigger callback — finds a named hotkey and triggers it.
 */
struct HotkeyFindData {
	const char *name;
	obs_hotkey_id id;
	bool found;
};

static bool findHotkeyCallback(void *param, obs_hotkey_id id, obs_hotkey_t *hotkey)
{
	auto *fd = static_cast<HotkeyFindData *>(param);
	const char *hkName = obs_hotkey_get_name(hotkey);
	if (hkName && strcmp(hkName, fd->name) == 0) {
		fd->id = id;
		fd->found = true;
		return false; // stop
	}
	return true; // continue
}

bool PressKey::execute(obs_data_t *metadata)
{
	if (!metadata)
		return false;

	std::string hotkeyName = MetadataUtils::getString(metadata, "hotkey_name");

	if (hotkeyName.empty()) {
		blog(LOG_WARNING, "PressKey: no hotkey_name provided");
		return false;
	}

	HotkeyFindData fd{hotkeyName.c_str(), OBS_INVALID_HOTKEY_ID, false};
	obs_enum_hotkeys(findHotkeyCallback, &fd);

	if (!fd.found) {
		blog(LOG_WARNING, "PressKey: hotkey '%s' not found", hotkeyName.c_str());
		return false;
	}

	// Trigger the hotkey (press + release)
	obs_hotkey_trigger_routed_callback(fd.id, true);
	obs_hotkey_trigger_routed_callback(fd.id, false);

	blog(LOG_INFO, "PressKey: triggered hotkey '%s'", hotkeyName.c_str());
	return true;
}
