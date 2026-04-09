/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Settings dialog (Tools > Streamloots). Extended from v2 which only
 * had volume and monitoring controls. Added port config, auto-start
 * toggle, and server start/stop buttons for easier troubleshooting.
 */

#include <obs-module.h>
#include "settings-dialog.h"
#include "../Config.hpp"
#include "../server/include/WSServer.h"
#include "../plugin-macros.generated.h"

#include <obs-frontend-api.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QMessageBox>

static SettingsDialog *dialogInstance = nullptr;

void SettingsDialog::showDialog()
{
	if (dialogInstance) {
		dialogInstance->raise();
		dialogInstance->activateWindow();
		return;
	}

	auto *mainWindow = static_cast<QWidget *>(
		obs_frontend_get_main_window());
	dialogInstance = new SettingsDialog(mainWindow);
	dialogInstance->setAttribute(Qt::WA_DeleteOnClose);
	QObject::connect(dialogInstance, &QDialog::destroyed, []() {
		dialogInstance = nullptr;
	});
	dialogInstance->show();
}

SettingsDialog::SettingsDialog(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle(QString("Streamloots Settings (v%1)")
			       .arg(PLUGIN_VERSION));
	setMinimumWidth(400);

	auto *mainLayout = new QVBoxLayout(this);

	auto *serverGroup = new QGroupBox("Server", this);
	auto *serverForm = new QFormLayout(serverGroup);

	portSpin_ = new QSpinBox(this);
	/* Clamped to 9006-9026: the Streamloots widget client only
	   scans this port range when trying to connect */
	portSpin_->setRange(Config::MIN_PORT, Config::MAX_PORT);
	serverForm->addRow("Port:", portSpin_);

	autoStartCheck_ = new QCheckBox("Auto-start when OBS loads", this);
	serverForm->addRow("", autoStartCheck_);

	statusLabel_ = new QLabel(this);
	serverForm->addRow("Status:", statusLabel_);

	auto *serverBtnLayout = new QHBoxLayout();
	auto *startBtn = new QPushButton("Start Server", this);
	auto *stopBtn = new QPushButton("Stop Server", this);
	serverBtnLayout->addWidget(startBtn);
	serverBtnLayout->addWidget(stopBtn);
	serverForm->addRow("", serverBtnLayout);

	mainLayout->addWidget(serverGroup);

	auto *audioGroup = new QGroupBox("Audio", this);
	auto *audioForm = new QFormLayout(audioGroup);

	auto *volLayout = new QHBoxLayout();
	volumeSlider_ = new QSlider(Qt::Horizontal, this);
	volumeSlider_->setRange(0, 100);
	volumeLabel_ = new QLabel("100%", this);
	volLayout->addWidget(volumeSlider_);
	volLayout->addWidget(volumeLabel_);
	audioForm->addRow("Volume:", volLayout);

	monitoringCombo_ = new QComboBox(this);
	monitoringCombo_->addItem("None", 0);
	monitoringCombo_->addItem("Monitor Only (mute on stream)", 1);
	monitoringCombo_->addItem("Monitor and Output", 2);
	audioForm->addRow("Monitoring:", monitoringCombo_);

	mainLayout->addWidget(audioGroup);

	auto *buttons = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	mainLayout->addWidget(buttons);

	connect(volumeSlider_, &QSlider::valueChanged, this,
		[this](int val) {
			volumeLabel_->setText(QString("%1%").arg(val));
		});

	connect(startBtn, &QPushButton::clicked, this, [this]() {
		saveSettings();
		WSServer::instance().start();
		statusLabel_->setText(
			WSServer::instance().isRunning()
				? "<span style='color:green'>Running</span>"
				: "<span style='color:red'>Failed</span>");
	});

	connect(stopBtn, &QPushButton::clicked, this, [this]() {
		WSServer::instance().stop();
		statusLabel_->setText(
			"<span style='color:gray'>Stopped</span>");
	});

	connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
		saveSettings();
		accept();
	});
	connect(buttons, &QDialogButtonBox::rejected, this,
		&QDialog::reject);

	loadSettings();
}

void SettingsDialog::loadSettings()
{
	const auto &cfg = Config::instance();
	portSpin_->setValue(cfg.port());
	volumeSlider_->setValue(cfg.volume());
	volumeLabel_->setText(QString("%1%").arg(cfg.volume()));
	monitoringCombo_->setCurrentIndex(
		static_cast<int>(cfg.monitoringMode()));
	autoStartCheck_->setChecked(cfg.autoStart());

	statusLabel_->setText(
		WSServer::instance().isRunning()
			? "<span style='color:green'>Running</span>"
			: "<span style='color:gray'>Stopped</span>");
}

void SettingsDialog::saveSettings()
{
	auto &cfg = Config::instance();
	cfg.setPort(static_cast<uint16_t>(portSpin_->value()));
	cfg.setVolume(volumeSlider_->value());
	cfg.setMonitoringMode(
		static_cast<MonitoringMode>(monitoringCombo_->currentIndex()));
	cfg.setAutoStart(autoStartCheck_->isChecked());
	cfg.save();
}
