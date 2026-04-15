/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * HideCamera — temporarily hides a named source for N seconds.
 * Uses obs_queue_task() for the restore callback to avoid the thread
 * safety crashes that affected the v2 plugin on OBS 28+.
 * Timer thread is tracked by WSServer for clean shutdown.
 */

#include <obs-module.h>
#include "include/HideCamera.hpp"
#include "../plugin-macros.generated.h"
#include "../requests/utils/metadata.hpp"
#include "../server/include/WSServer.h"
#include "utils/getSceneItemInScene.hpp"

#include <obs.h>
#include <obs-frontend-api.h>
#include <thread>
#include <chrono>
#include <string>

bool HideCamera::execute(obs_data_t *metadata)
{
	if (!metadata)
		return false;

	std::string sourceName = MetadataUtils::getString(metadata, "source_name");
	int seconds = MetadataUtils::getInt(metadata, "seconds", 5);

	if (sourceName.empty()) {
		blog(LOG_WARNING, "HideCamera: no source_name provided");
		return false;
	}

	obs_source_t *sceneSrc = obs_frontend_get_current_scene();
	if (!sceneSrc) {
		blog(LOG_ERROR, "HideCamera: no current scene");
		return false;
	}

	obs_scene_t *scene = obs_scene_from_source(sceneSrc);
	obs_sceneitem_t *item = SceneUtils::getSceneItemByName(scene, sourceName.c_str());

	if (!item) {
		obs_source_release(sceneSrc);
		blog(LOG_WARNING, "HideCamera: source '%s' not found in scene", sourceName.c_str());
		return false;
	}

	obs_sceneitem_set_visible(item, false);
	blog(LOG_INFO, "HideCamera: hiding '%s' for %d seconds", sourceName.c_str(), seconds);

	obs_source_get_ref(sceneSrc);

	std::thread timerThread([sourceName, seconds, sceneSrc]() {
		std::this_thread::sleep_for(std::chrono::seconds(seconds));

		struct RestoreCtx {
			std::string name;
			obs_source_t *sceneSrc;
		};

		auto *ctx = new RestoreCtx{sourceName, sceneSrc};

		obs_queue_task(
			OBS_TASK_UI,
			[](void *param) {
				auto *c = static_cast<RestoreCtx *>(param);
				obs_scene_t *sc = obs_scene_from_source(c->sceneSrc);
				if (sc) {
					obs_sceneitem_t *si = SceneUtils::getSceneItemByName(sc, c->name.c_str());
					if (si)
						obs_sceneitem_set_visible(si, true);
				}
				obs_source_release(c->sceneSrc);
				delete c;
			},
			ctx, false);
	});

	WSServer::instance().trackTimerThread(std::move(timerThread));

	obs_source_release(sceneSrc);
	return true;
}
