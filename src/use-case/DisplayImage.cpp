/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * DisplayImage — creates a temporary browser source overlay that displays
 * a URL (GIF, image, or .webm video from Streamloots CDN) for a set
 * number of seconds, then automatically cleans it up.
 *
 * Key changes from v2:
 * - The Streamloots backend can send width/height as 1 in the metadata.
 *   The v2 plugin passed those values directly to the browser source,
 *   resulting in a 1x1 pixel source that was invisible on screen.
 *   Fixed by always using the OBS canvas resolution instead.
 * - v2 ran cleanup timers on worker threads and called OBS functions
 *   directly, which crashes on OBS 28+ due to thread affinity rules.
 *   Fixed by using obs_queue_task() to run cleanup on the UI thread.
 * - Timer threads are now tracked by WSServer and joined during shutdown
 *   so they don't outlive the plugin.
 */

#include <obs-module.h>
#include "include/DisplayImage.hpp"
#include "../plugin-macros.generated.h"
#include "../requests/utils/metadata.hpp"
#include "../Config.hpp"
#include "../server/include/WSServer.h"
#include "utils/getSceneItemInScene.hpp"

#include <obs.h>
#include <obs-frontend-api.h>
#include <thread>
#include <chrono>
#include <string>
#include <atomic>

static const char *STREAMLOOTS_SOURCE_PREFIX = "streamloots_img_";
static std::atomic<int> sourceCounter{0};

bool DisplayImage::execute(obs_data_t *metadata)
{
	if (!metadata)
		return false;

	std::string url = MetadataUtils::getString(metadata, "url");
	int seconds = MetadataUtils::getInt(metadata, "seconds", 5);

	if (url.empty()) {
		blog(LOG_WARNING, "DisplayImage: no URL provided");
		return false;
	}

	/* Grab the OBS canvas resolution up front. We need this in two places:
	   1) As the browser source internal resolution (so content renders
	      at full size, not 1x1 as the backend sometimes sends)
	   2) As the scene item bounds (so the source fills the canvas) */
	struct obs_video_info ovi;
	int canvasWidth = 1920;
	int canvasHeight = 1080;
	if (obs_get_video_info(&ovi)) {
		canvasWidth = static_cast<int>(ovi.base_width);
		canvasHeight = static_cast<int>(ovi.base_height);
	}

	blog(LOG_INFO, "DisplayImage: url=%s seconds=%d canvas=%dx%d",
	     url.c_str(), seconds, canvasWidth, canvasHeight);

	std::string sourceName =
		STREAMLOOTS_SOURCE_PREFIX + std::to_string(sourceCounter++);

	obs_data_t *settings = obs_data_create();
	obs_data_set_string(settings, "url", url.c_str());
	obs_data_set_int(settings, "width", canvasWidth);
	obs_data_set_int(settings, "height", canvasHeight);
	obs_data_set_bool(settings, "shutdown", true);
	obs_data_set_bool(settings, "restart_when_active", false);
	obs_data_set_bool(settings, "reroute_audio", true);

	/* CSS: transparent background for the browser page, plus force any
	   video or image element to fill the viewport. Streamloots serves
	   .webm files directly (not wrapped in a page), so the browser
	   renders them as bare <video> elements that need explicit sizing.
	   Also hide the Chromium video player controls. */
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
		blog(LOG_ERROR, "DisplayImage: failed to create browser source");
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
		blog(LOG_ERROR, "DisplayImage: no current scene");
		return false;
	}

	obs_scene_t *scene = obs_scene_from_source(sceneSrc);
	obs_sceneitem_t *item = obs_scene_add(scene, source);

	if (!item) {
		obs_source_release(source);
		obs_source_release(sceneSrc);
		blog(LOG_ERROR, "DisplayImage: failed to add to scene");
		return false;
	}

	/* Set bounds to fill the canvas — this is what the v2 plugin did via
	   obs_sceneitem_set_bounds(). Without this the source appears at its
	   native size which can be tiny or mispositioned. */
	struct vec2 bounds;
	bounds.x = static_cast<float>(canvasWidth);
	bounds.y = static_cast<float>(canvasHeight);
	obs_sceneitem_set_bounds_type(item, OBS_BOUNDS_SCALE_INNER);
	obs_sceneitem_set_bounds(item, &bounds);

	struct vec2 pos = {0.0f, 0.0f};
	obs_sceneitem_set_pos(item, &pos);

	blog(LOG_INFO, "DisplayImage: showing '%s' at %dx%d for %d seconds",
	     sourceName.c_str(), canvasWidth, canvasHeight, seconds);

	/* Schedule removal: sleep on a tracked thread, then queue the actual
	   scene manipulation back to the UI thread via obs_queue_task().
	   The thread is registered with WSServer so it gets joined during
	   shutdown instead of running after the plugin unloads. */
	std::string capturedName = sourceName;
	std::thread timerThread([capturedName, seconds, sceneSrc, source]() {
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

				/* Release the refs we held across the timer.
				   Every obs_source_create and
				   obs_frontend_get_current_scene adds a ref
				   that must be released to avoid leaks. */
				obs_source_release(c->source);
				obs_source_release(c->sceneSrc);
				delete c;
			},
			ctx, false);
	});

	WSServer::instance().trackTimerThread(std::move(timerThread));
	return true;
}
