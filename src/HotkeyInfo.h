

#pragma once

#include <stdint.h>
#include <QString>
#include <QWidget>
#include <QSettings>
#include <mutex>

class HotkeyInfo
{
public:
	HotkeyInfo(int buttonId, uint32_t modifiers, uint32_t keyCode);
	HotkeyInfo(const QSettings &settings);

	QString toString() const;
	
	QString getModifiersString() const;
	QString getKeyString() const;
	void registerHotkey(const QWidget *window);
	void unregisterHotkey();

	void saveToConfig(QSettings &settings) const;
	void performAction();

	uint32_t getModifiers() const { return modifiers; }
	uint32_t getKeyCode() const { return keyCode; }
	int getButtonId() const { return buttonId; }
	uint32_t getId() const { return id; }

	static uint32_t getNextHotkeyId();


private:
	int buttonId;
	uint32_t modifiers;
	uint32_t keyCode;
	uint32_t id;
	const QWidget *window;
};

