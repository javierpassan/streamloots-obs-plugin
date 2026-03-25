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

namespace SceneUtils {

/*
 * Finds a scene item by source name within a given scene.
 * Returns nullptr if not found.  Caller must release the returned
 * scene item via obs_sceneitem_release() if addref is true.
 */
struct FindData {
	const char *name;
	obs_sceneitem_t *item;
};

inline bool findItemCallback(obs_scene_t * /*scene*/, obs_sceneitem_t *item,
			     void *param)
{
	auto *fd = static_cast<FindData *>(param);
	obs_source_t *src = obs_sceneitem_get_source(item);
	if (src) {
		const char *srcName = obs_source_get_name(src);
		if (srcName && strcmp(srcName, fd->name) == 0) {
			fd->item = item;
			return false; // stop enumeration
		}
	}
	return true; // continue
}

inline obs_sceneitem_t *getSceneItemByName(obs_scene_t *scene,
					   const char *name)
{
	if (!scene || !name)
		return nullptr;
	FindData fd{name, nullptr};
	obs_scene_enum_items(scene, findItemCallback, &fd);
	return fd.item;
}

/*
 * Gets the current scene as an obs_scene_t*.
 * Caller must release the returned source via obs_source_release().
 */
inline obs_scene_t *getCurrentScene(obs_source_t **outSource = nullptr)
{
	obs_source_t *sceneSrc = obs_frontend_get_current_scene();
	if (!sceneSrc)
		return nullptr;

	obs_scene_t *scene = obs_scene_from_source(sceneSrc);
	if (outSource)
		*outSource = sceneSrc;
	else
		obs_source_release(sceneSrc);

	return scene;
}

} // namespace SceneUtils
