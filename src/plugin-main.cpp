/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Entry point for the plugin. The v2 plugin started its WebSocket server
 * directly in obs_module_load(), but that fires before scenes are fully
 * initialized. Moved server startup to OBS_FRONTEND_EVENT_FINISHED_LOADING
 * so that incoming requests can safely reference scene items.
 */

#include <obs-module.h>
#include <obs-frontend-api.h>

#include "plugin-macros.generated.h"
#include "Config.hpp"
#include "server/include/WSServer.h"
#include "forms/settings-dialog.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-streamloots", "en-US")

MODULE_EXPORT const char *obs_module_description(void)
{
	return "Streamloots OBS integration – receives card-redemption "
	       "events and applies visual effects to your stream.";
}

static void on_frontend_event(enum obs_frontend_event event, void * /*data*/)
{
	switch (event) {
	case OBS_FRONTEND_EVENT_FINISHED_LOADING:
		blog(LOG_INFO, "OBS finished loading – starting Streamloots WS server v%s",
		     PLUGIN_VERSION);
		WSServer::instance().start();
		break;

	/* Handle both shutdown events — SCRIPTING_SHUTDOWN fires first during
	   normal exit, but if a forced quit happens we need EXIT as a fallback */
	case OBS_FRONTEND_EVENT_SCRIPTING_SHUTDOWN:
	case OBS_FRONTEND_EVENT_EXIT:
		blog(LOG_INFO, "OBS exiting – stopping Streamloots WS server");
		WSServer::instance().stop();
		break;

	default:
		break;
	}
}

bool obs_module_load(void)
{
	blog(LOG_INFO, "obs-streamloots v%s loaded", PLUGIN_VERSION);

	Config::instance().load();

	obs_frontend_add_tools_menu_item(
		"Streamloots", [](void *) { SettingsDialog::showDialog(); },
		nullptr);

	/* Don't start the server here — wait for FINISHED_LOADING so all
	   scenes and sources exist before we try to manipulate them */
	obs_frontend_add_event_callback(on_frontend_event, nullptr);

	return true;
}

void obs_module_unload(void)
{
	obs_frontend_remove_event_callback(on_frontend_event, nullptr);
	WSServer::instance().stop();
	Config::instance().save();
	blog(LOG_INFO, "obs-streamloots unloaded");
}
