/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "include/RotateCameraRequest.hpp"
#include "utils/metadata.hpp"

bool RotateCameraRequest::parse(obs_data_t *metadata)
{
	if (!metadata)
		return false;
	url_ = MetadataUtils::getString(metadata, "url");
	seconds_ = MetadataUtils::getInt(metadata, "seconds", 5);
	return true;
}
