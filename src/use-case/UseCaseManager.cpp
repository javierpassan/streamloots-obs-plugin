/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <obs-module.h>
#include "include/UseCaseManager.hpp"
#include "../plugin-macros.generated.h"
#include "../requests/include/RequestTypes.hpp"

#include "include/DisplayImage.hpp"
#include "include/DisplayVideo.hpp"
#include "include/HideCamera.hpp"
#include "include/PressKey.hpp"
#include "include/RotateCamera.hpp"

UseCaseManager &UseCaseManager::instance()
{
	static UseCaseManager mgr;
	return mgr;
}

bool UseCaseManager::execute(const std::string &requestType, obs_data_t *metadata)
{
	if (requestType == RequestTypes::DISPLAY_IMAGE) {
		return DisplayImage::execute(metadata);

	} else if (requestType == RequestTypes::DISPLAY_VIDEO) {
		return DisplayVideo::execute(metadata);

	} else if (requestType == RequestTypes::HIDE_CAMERA) {
		return HideCamera::execute(metadata);

	} else if (requestType == RequestTypes::PRESS_KEY) {
		return PressKey::execute(metadata);

	} else if (requestType == RequestTypes::ROTATE_CAMERA) {
		return RotateCamera::execute(metadata);

	} else {
		blog(LOG_WARNING, "Unknown request type: %s", requestType.c_str());
		return false;
	}
}
