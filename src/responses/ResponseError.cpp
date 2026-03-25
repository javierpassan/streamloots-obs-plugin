/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "include/ResponseError.hpp"
#include <obs-data.h>

std::string ResponseError::make(const std::string &messageId,
				const std::string &errorCode,
				const std::string &errorMessage)
{
	obs_data_t *resp = obs_data_create();
	obs_data_set_string(resp, "status", "error");
	obs_data_set_string(resp, "message-id", messageId.c_str());
	obs_data_set_string(resp, "error", errorCode.c_str());
	obs_data_set_string(resp, "error-message", errorMessage.c_str());

	const char *json = obs_data_get_json(resp);
	std::string result(json ? json : "{}");
	obs_data_release(resp);
	return result;
}
