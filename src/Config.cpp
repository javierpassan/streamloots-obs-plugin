/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <obs-module.h>
#include "Config.hpp"
#include "plugin-macros.generated.h"

#include <obs-frontend-api.h>
#include <util/config-file.h>

static constexpr const char *CONFIG_SECTION = "obs-streamloots";

Config &Config::instance()
{
	static Config cfg;
	return cfg;
}

void Config::load()
{
	config_t *global = obs_frontend_get_app_config();
	if (!global)
		return;

	config_set_default_uint(global, CONFIG_SECTION, "port", DEFAULT_PORT);
	config_set_default_int(global, CONFIG_SECTION, "volume", DEFAULT_VOLUME);
	config_set_default_int(global, CONFIG_SECTION, "monitoring_mode", 0);
	config_set_default_bool(global, CONFIG_SECTION, "auto_start", true);

	port_ = static_cast<uint16_t>(config_get_uint(global, CONFIG_SECTION, "port"));
	volume_ = static_cast<int>(config_get_int(global, CONFIG_SECTION, "volume"));
	monitoringMode_ = static_cast<MonitoringMode>(config_get_int(global, CONFIG_SECTION, "monitoring_mode"));
	autoStart_ = config_get_bool(global, CONFIG_SECTION, "auto_start");

	blog(LOG_INFO, "Config loaded: port=%u volume=%d monitoring=%d autoStart=%d", port_, volume_,
	     static_cast<int>(monitoringMode_), autoStart_);
}

void Config::save() const
{
	config_t *global = obs_frontend_get_app_config();
	if (!global)
		return;

	config_set_uint(global, CONFIG_SECTION, "port", port_);
	config_set_int(global, CONFIG_SECTION, "volume", volume_);
	config_set_int(global, CONFIG_SECTION, "monitoring_mode", static_cast<int>(monitoringMode_));
	config_set_bool(global, CONFIG_SECTION, "auto_start", autoStart_);

	config_save(global);
	blog(LOG_INFO, "Config saved");
}
