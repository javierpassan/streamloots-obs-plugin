/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <obs-data.h>
#include <string>

class UseCaseManager {
public:
	static UseCaseManager &instance();

	/**
	 * Execute a use case by request type.
	 * @param requestType  e.g. "display-image", "hide-camera"
	 * @param metadata     obs_data_t with request-specific fields
	 * @return true on success
	 */
	bool execute(const std::string &requestType, obs_data_t *metadata);

private:
	UseCaseManager() = default;
};
