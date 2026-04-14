/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "include/Response.hpp"
#include <obs-data.h>

std::string Response::make(const std::string &messageId, const std::string &requestType)
{
	obs_data_t *resp = obs_data_create();
	obs_data_set_string(resp, "status", "ok");
	obs_data_set_string(resp, "message-id", messageId.c_str());
	obs_data_set_string(resp, "request-type", requestType.c_str());

	const char *json = obs_data_get_json(resp);
	std::string result(json ? json : "{}");
	obs_data_release(resp);
	return result;
}
