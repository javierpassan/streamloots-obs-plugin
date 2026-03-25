/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * DisplayVideo — same pattern as DisplayImage but originally used
 * ffmpeg_source in v2. Switched to browser_source because Streamloots
 * serves .webm files over HTTPS — browser_source handles these natively
 * with transparency support, and avoids having to download the file first.
 *
 * Same canvas resolution fix as DisplayImage (see that file for details).
 */

#include <obs-module.h>
#include "include/DisplayVideo.hpp"
#include "../plugin-macros.generated.h"
#include "../requests/utils/metadata.hpp"
#include "../Config.hpp"
#include "utils/getSceneItemInScene.hpp"

#include <obs.h>
#include <obs-frontend-api.h>
#include <thread>
#include <chrono>
#include <string>
#include <atomic>

static const char *STREAMLOOTS_VIDEO_PREFIX = "streamloots_vid_";
static std::atomic<int> videoCounter{0};

bool DisplayVideo::execute(obs_data_t *metadata)
{
	if (!metadata)
		return false;

	std::string url = MetadataUtils::getString(metadata, "url");
	int seconds = MetadataUtils::getInt(metadata, "seconds", 10);

	if (url.empty()) {
		blog(LOG_WARNING, "DisplayVideo: no URL provided");
		return false;
	}

	struct obs_video_info ovi;
	int canvasWidth = 1920;
	int canvasHeight = 1080;
	if (obs_get_video_info(&ovi)) {
		canvasWidth = static_cast<int>(ovi.base_width);
		canvasHeight = static_cast<int>(ovi.base_height);
	}

	blog(LOG_INFO, "DisplayVideo: url=%s seconds=%d canvas=%dx%d",
	     url.c_str(), seconds, canvasWidth, canvasHeight);

	std::string sourceName =
		STREAMLOOTS_VIDEO_PREFIX + std::to_string(videoCounter++);

	obs_data_t *settings = obs_data_create();
	obs_data_set_string(settings, "url", url.c_str());
	obs_data_set_int(settings, "width", canvasWidth);
	obs_data_set_int(settings, "height", canvasHeight);
	obs_data_set_bool(settings, "shutdown", true);
	obs_data_set_bool(settings, "restart_when_active", false);
	obs_data_set_bool(settings, "reroute_audio", true);
	obs_data_set_string(settings, "css",
			    "body { background-color: rgba(0,0,0,0); "
			    "margin: 0; overflow: hidden; }"
			    "video, img { width: 100vw; height: 100vh; "
			    "object-fit: contain; }"
			    "video::-webkit-media-controls { display: none !important; }");

	obs_source_t *source = obs_source_create(
		"browser_source", sourceName.c_str(), settings, nullptr);
	obs_data_release(settings);

	if (!source) {
		blog(LOG_ERROR, "DisplayVideo: failed to create browser source");
		return false;
	}

	float volume =
		static_cast<float>(Config::instance().volume()) / 100.0f;
	obs_source_set_volume(source, volume);

	auto mode = Config::instance().monitoringMode();
	obs_source_set_monitoring_type(
		source,
		static_cast<obs_monitoring_type>(static_cast<int>(mode)));

	obs_source_t *sceneSrc = obs_frontend_get_current_scene();
	if (!sceneSrc) {
		obs_source_release(source);
		blog(LOG_ERROR, "DisplayVideo: no current scene");
		return false;
	}

	obs_scene_t *scene = obs_scene_from_source(sceneSrc);
	obs_sceneitem_t *item = obs_scene_add(scene, source);

	if (!item) {
		obs_source_release(source);
		obs_source_release(sceneSrc);
		blog(LOG_ERROR, "DisplayVideo: failed to add to scene");
		return false;
	}

	struct vec2 bounds;
	bounds.x = static_cast<float>(canvasWidth);
	bounds.y = static_cast<float>(canvasHeight);
	obs_sceneitem_set_bounds_type(item, OBS_BOUNDS_SCALE_INNER);
	obs_sceneitem_set_bounds(item, &bounds);

	struct vec2 pos = {0.0f, 0.0f};
	obs_sceneitem_set_pos(item, &pos);

	blog(LOG_INFO, "DisplayVideo: showing '%s' at %dx%d for %d seconds",
	     sourceName.c_str(), canvasWidth, canvasHeight, seconds);

	std::string capturedName = sourceName;
	std::thread([capturedName, seconds, sceneSrc, source]() {
		std::this_thread::sleep_for(std::chrono::seconds(seconds));

		struct RemoveCtx {
			std::string name;
			obs_source_t *sceneSrc;
			obs_source_t *source;
		};

		auto *ctx = new RemoveCtx{capturedName, sceneSrc, source};

		obs_queue_task(
			OBS_TASK_UI,
			[](void *param) {
				auto *c = static_cast<RemoveCtx *>(param);
				obs_scene_t *sc =
					obs_scene_from_source(c->sceneSrc);
				if (sc) {
					obs_sceneitem_t *si =
						SceneUtils::getSceneItemByName(
							sc, c->name.c_str());
					if (si)
						obs_sceneitem_remove(si);
				}
				obs_source_release(c->source);
				obs_source_release(c->sceneSrc);
				delete c;
			},
			ctx, false);
	}).detach();

	return true;
}
