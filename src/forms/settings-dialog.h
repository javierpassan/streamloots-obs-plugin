/*
 * obs-streamloots — Streamloots integration plugin for OBS Studio
 * Copyright (C) 2023 Streamloots <engineering@streamloots.com>
 * v3.0.0 update by SyerNide (2026) — compatibility rewrite for OBS 28+
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QDialog>
#include <QSpinBox>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

class SettingsDialog : public QDialog {
	Q_OBJECT

public:
	static void showDialog();

private:
	explicit SettingsDialog(QWidget *parent = nullptr);

	void loadSettings();
	void saveSettings();

	QSpinBox *portSpin_ = nullptr;
	QSlider *volumeSlider_ = nullptr;
	QLabel *volumeLabel_ = nullptr;
	QComboBox *monitoringCombo_ = nullptr;
	QCheckBox *autoStartCheck_ = nullptr;
	QLabel *statusLabel_ = nullptr;
};
