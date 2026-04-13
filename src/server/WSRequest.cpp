/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <obs-module.h>
#include "include/WSRequest.hpp"
#include "../plugin-macros.generated.h"
#include "../use-case/include/UseCaseManager.hpp"
#include "../responses/include/Response.hpp"
#include "../responses/include/ResponseError.hpp"

#include <obs-data.h>
#include <string>

/*
 * Expected JSON format from Streamloots backend:
 * {
 *   "request-type": "display-image" | "display-video" | "hide-camera"
 *                   | "press-key" | "rotate-camera",
 *   "message-id": "<correlation-id>",
 *   "metadata": { ... request-specific fields ... }
 * }
 */

std::string WSRequest::processMessage(const std::string &jsonPayload)
{
	obs_data_t *data = obs_data_create_from_json(jsonPayload.c_str());
	if (!data) {
		blog(LOG_WARNING, "Invalid JSON received");
		return ResponseError::make("", "invalid-json",
					   "Could not parse JSON payload");
	}

	const char *requestType = obs_data_get_string(data, "request-type");
	const char *messageId = obs_data_get_string(data, "message-id");
	obs_data_t *metadata = obs_data_get_obj(data, "metadata");

	std::string response;

	if (!requestType || strlen(requestType) == 0) {
		response = ResponseError::make(
			messageId ? messageId : "", "missing-request-type",
			"'request-type' field is required");
	} else {
		blog(LOG_INFO, "Processing request: type=%s id=%s",
		     requestType, messageId ? messageId : "(none)");

		bool ok = UseCaseManager::instance().execute(
			requestType, metadata);

		if (ok) {
			response = Response::make(
				messageId ? messageId : "", requestType);
		} else {
			response = ResponseError::make(
				messageId ? messageId : "", "execution-failed",
				"Use case execution failed");
		}
	}

	if (metadata)
		obs_data_release(metadata);
	obs_data_release(data);

	return response;
}
