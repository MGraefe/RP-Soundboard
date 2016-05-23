
#include "HotkeySetDialog.h"
#include "config_qt.h"
#include "ui_HotkeySetDialog.h"

#include <QTimer>

#ifdef _WIN32
#include "Windows.h"
#endif


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
HotkeySetDialog::HotkeySetDialog(ConfigQt *parent, uint32_t *modifiersRes, uint32_t *keyCodeRes) :
	QDialog(parent, Qt::Window),
	ui(new Ui::HotkeySetDialog),
	modifiers(0),
	keyCode(0),
	modifiersRes(modifiersRes),
	keyCodeRes(keyCodeRes),
	inputBlocked(false)
{
	ui->setupUi(this);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
bool HotkeySetDialog::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#ifdef _WIN32
	MSG *msg = static_cast<MSG*>(message);
	if ((msg->message == WM_KEYDOWN || msg->message == WM_KEYUP ||
		 msg->message == WM_SYSKEYDOWN || msg->message == WM_SYSKEYUP)
		 && !inputBlocked)
	{
		uint32_t modifier = 0;
		uint32_t code = 0;
		switch(msg->wParam)
		{
		case VK_ESCAPE:
			done(0);
			break;
		case VK_CONTROL:
			modifier = MOD_CONTROL;
			break;
		case VK_MENU:
			modifier = MOD_ALT;
			break;
		case VK_SHIFT:
			modifier = MOD_SHIFT;
			break;
		case VK_LBUTTON:
		case VK_RBUTTON:
		case VK_MBUTTON:
			break;
		default:
			code = msg->wParam;
			break;
		}

		if (modifier != 0)
		{
			if (msg->message == WM_KEYDOWN)
				modifiers |= modifier; // set
			else if (modifiers != 0)
				modifiers &= (~modifier); // reset
		}

		if (code != 0)
		{
			if (msg->message == WM_KEYDOWN)
				keyCode = code; // set key
		}

		// Update display
		HotkeyInfo hk(0, modifiers, keyCode);
		ui->hotkeyEdit->setText(hk.toString());

		// are we done? Wait a little to show the user what he has selected and then
		// quit the dialog
		if (msg->message == WM_KEYUP && modifier == 0 && keyCode != 0)
		{
			ui->hotkeyEdit->setEnabled(false);
			inputBlocked = true;
			if (modifiersRes)
				*modifiersRes = modifiers;
			if (keyCodeRes)
				*keyCodeRes = keyCode;
			QTimer::singleShot(500, this, SLOT(quitGood()));
		}

		return true;
	}
#endif

	return QDialog::nativeEvent(eventType, message, result);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void HotkeySetDialog::quitGood()
{
	done(1);
}
