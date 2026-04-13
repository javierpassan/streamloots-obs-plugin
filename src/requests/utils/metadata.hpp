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

namespace MetadataUtils {

inline std::string getString(obs_data_t *meta, const char *key,
			     const char *fallback = "")
{
	if (!meta)
		return fallback;
	const char *val = obs_data_get_string(meta, key);
	return (val && strlen(val) > 0) ? val : fallback;
}

inline int getInt(obs_data_t *meta, const char *key, int fallback = 0)
{
	if (!meta)
		return fallback;
	return static_cast<int>(obs_data_get_int(meta, key));
}

inline double getDouble(obs_data_t *meta, const char *key,
			double fallback = 0.0)
{
	if (!meta)
		return fallback;
	return obs_data_get_double(meta, key);
}

inline bool getBool(obs_data_t *meta, const char *key, bool fallback = false)
{
	if (!meta)
		return fallback;
	return obs_data_get_bool(meta, key);
}

} // namespace MetadataUtils
