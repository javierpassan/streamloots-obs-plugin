/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <string>

using WsServer = websocketpp::server<websocketpp::config::asio>;

class WSServer {
public:
	static WSServer &instance();

	void start();
	void stop();
	bool isRunning() const { return running_.load(); }

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

	WsServer server_;
	std::thread serverThread_;
	std::atomic<bool> running_{false};
	std::mutex mutex_;
};
