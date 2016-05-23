
#pragma once

#include <cstdint>
#include "ui_HotkeySetDialog.h"

namespace Ui {
	class HotkeySetDialog;
}

class ConfigQt;

class HotkeySetDialog : public QDialog
{
	Q_OBJECT

public:
	explicit HotkeySetDialog(ConfigQt *parent, uint32_t *modifiersRes, uint32_t *keyCodeRes);

	bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

public slots:
		void quitGood();

private:
	Ui::HotkeySetDialog *ui;
	uint32_t modifiers;
	uint32_t keyCode;
	uint32_t *modifiersRes;
	uint32_t *keyCodeRes;
	bool inputBlocked;
};