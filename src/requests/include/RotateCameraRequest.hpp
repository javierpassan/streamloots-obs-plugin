/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "RequestBase.hpp"

class RotateCameraRequest : public RequestBase {
public:
    bool parse(obs_data_t *metadata) override;
};
