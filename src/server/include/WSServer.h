/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * WebSocket server using websocketpp + standalone ASIO (same stack as v2).
 * Runs on port 9006 by default. The Streamloots backend connects here
 * to send card redemption commands. This is independent of the
 * obs-websocket plugin that ships with OBS 28+ (which runs on 4455).
 */

#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <string>

using WsServer = websocketpp::server<websocketpp::config::asio>;

class WSServer {
public:
	static WSServer &instance();

	void start();
	void stop();
	bool isRunning() const { return running_.load(); }

	/* Register a timer thread so we can join it during shutdown.
	   Called by use cases that schedule deferred cleanup. */
	void trackTimerThread(std::thread &&t);

private:
	WSServer() = default;
	~WSServer();

	WSServer(const WSServer &) = delete;
	WSServer &operator=(const WSServer &) = delete;

	void run();
	void onMessage(websocketpp::connection_hdl hdl,
		       WsServer::message_ptr msg);
	void onOpen(websocketpp::connection_hdl hdl);
	void onClose(websocketpp::connection_hdl hdl);

	/* Joins and removes any finished timer threads */
	void cleanupTimerThreads();

	WsServer server_;
	std::thread serverThread_;
	std::atomic<bool> running_{false};
	std::mutex mutex_;

	/* Timer threads from use cases (display-image, hide-camera, etc.)
	   Tracked here so we can join them during shutdown instead of
	   letting detached threads outlive the plugin. */
	std::mutex timerMutex_;
	std::vector<std::thread> timerThreads_;
};
