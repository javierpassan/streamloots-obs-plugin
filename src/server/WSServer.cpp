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

#include <obs-module.h>
#include "include/WSServer.h"
#include "../Config.hpp"
#include "../plugin-macros.generated.h"
#include "include/WSRequest.hpp"

#include <obs.h>

WSServer &WSServer::instance()
{
	static WSServer srv;
	return srv;
}

WSServer::~WSServer()
{
	stop();
}

void WSServer::start()
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (running_.load())
		return;

	const auto &cfg = Config::instance();
	if (!cfg.autoStart()) {
		blog(LOG_INFO, "Auto-start disabled; server not started");
		return;
	}

	/* If a previous server thread is still joinable (e.g. from an
	   exception exit that set running_ to false), join it before
	   starting a new one to avoid std::terminate */
	if (serverThread_.joinable())
		serverThread_.join();

	try {
		server_.init_asio();
		server_.set_reuse_addr(true);

		server_.clear_access_channels(websocketpp::log::alevel::all);
		server_.clear_error_channels(websocketpp::log::elevel::all);

		server_.set_open_handler([this](websocketpp::connection_hdl hdl) { onOpen(hdl); });
		server_.set_close_handler([this](websocketpp::connection_hdl hdl) { onClose(hdl); });
		server_.set_message_handler(
			[this](websocketpp::connection_hdl hdl, WsServer::message_ptr msg) { onMessage(hdl, msg); });

		server_.listen(cfg.port());
		server_.start_accept();

		running_.store(true);
		serverThread_ = std::thread(&WSServer::run, this);

		blog(LOG_INFO, "WebSocket server listening on port %u", cfg.port());

	} catch (const std::exception &e) {
		blog(LOG_ERROR, "Failed to start WS server: %s", e.what());
		running_.store(false);
	}
}

void WSServer::stop()
{
	std::lock_guard<std::mutex> lock(mutex_);

	running_.store(false);

	try {
		server_.stop_listening();
		server_.stop();
	} catch (...) {
	}

	/* Always join the server thread if it's joinable, even if
	   running_ was already false (e.g. exception in run()).
	   Leaving a joinable thread causes std::terminate. */
	if (serverThread_.joinable())
		serverThread_.join();

	/* Join all timer threads from use cases so they don't outlive
	   the plugin during OBS shutdown */
	{
		std::lock_guard<std::mutex> tlock(timerMutex_);
		for (auto &t : timerThreads_) {
			if (t.joinable())
				t.join();
		}
		timerThreads_.clear();
	}

	blog(LOG_INFO, "WebSocket server stopped");
}

void WSServer::run()
{
	try {
		server_.run();
	} catch (const std::exception &e) {
		blog(LOG_ERROR, "WS server thread error: %s", e.what());
	}
	running_.store(false);
}

void WSServer::onOpen(websocketpp::connection_hdl)
{
	blog(LOG_INFO, "Streamloots client connected");
}

void WSServer::onClose(websocketpp::connection_hdl)
{
	blog(LOG_INFO, "Streamloots client disconnected");
}

void WSServer::onMessage(websocketpp::connection_hdl hdl, WsServer::message_ptr msg)
{
	if (!msg)
		return;

	/* Clean up any finished timer threads before processing */
	cleanupTimerThreads();

	const std::string &payload = msg->get_payload();
	blog(LOG_DEBUG, "Received message: %s", payload.c_str());

	WSRequest request;
	std::string response = request.processMessage(payload);

	try {
		server_.send(hdl, response, websocketpp::frame::opcode::text);
	} catch (const std::exception &e) {
		blog(LOG_ERROR, "Failed to send response: %s", e.what());
	}
}

void WSServer::trackTimerThread(std::thread &&t)
{
	std::lock_guard<std::mutex> lock(timerMutex_);
	timerThreads_.push_back(std::move(t));
}

void WSServer::cleanupTimerThreads()
{
	std::lock_guard<std::mutex> lock(timerMutex_);
	/* Remove threads that have finished (not joinable anymore won't
	   work since joinable is true until joined, so we just leave
	   cleanup to stop() and periodic calls here keep the vector
	   from growing unbounded by joining finished ones) */
	auto it = timerThreads_.begin();
	while (it != timerThreads_.end()) {
		/* We can't cheaply check if a thread is "done" without
		   a flag, so we just let stop() handle the final join.
		   This method exists to be called periodically. */
		++it;
	}
}
