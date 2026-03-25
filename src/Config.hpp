/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <obs.h>
#include <string>

enum class MonitoringMode { None = 0, MonitorOnly = 1, MonitorAndOutput = 2 };

class Config {
public:
	static Config &instance();

	void load();
	void save() const;

	// Accessors
	uint16_t port() const { return port_; }
	void setPort(uint16_t p) { port_ = p; }

	int volume() const { return volume_; }
	void setVolume(int v) { volume_ = v; }

	MonitoringMode monitoringMode() const { return monitoringMode_; }
	void setMonitoringMode(MonitoringMode m) { monitoringMode_ = m; }

	bool autoStart() const { return autoStart_; }
	void setAutoStart(bool a) { autoStart_ = a; }

private:
	Config() = default;

	static constexpr uint16_t DEFAULT_PORT = 9006;
	static constexpr int DEFAULT_VOLUME = 100;

	uint16_t port_ = DEFAULT_PORT;
	int volume_ = DEFAULT_VOLUME;
	MonitoringMode monitoringMode_ = MonitoringMode::None;
	bool autoStart_ = true;
};
