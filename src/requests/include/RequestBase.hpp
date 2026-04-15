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

class RequestBase {
public:
	virtual ~RequestBase() = default;
	virtual bool parse(obs_data_t *metadata) = 0;

	const std::string &url() const { return url_; }
	int seconds() const { return seconds_; }

protected:
	std::string url_;
	int seconds_ = 5;
};
