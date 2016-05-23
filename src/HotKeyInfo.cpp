
#include "HotkeyInfo.h"
#include "device.h"
#include <map>
#include <cassert>
#include "ts3log.h"
#include <set>

#ifdef _WIN32
#include "Windows.h"
#endif

#define NAME_BUTTON "button"
#define NAME_MODIFIERS "modifiers"
#define NAME_KEYCODE "keyCode"

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
HotkeyInfo::HotkeyInfo(int buttonId, uint32_t modifiers, uint32_t keyCode) :
	buttonId(buttonId),
	modifiers(modifiers),
	keyCode(keyCode),
	id(0),
	window(nullptr)
{

}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
HotkeyInfo::HotkeyInfo(const QSettings &settings) :
	id(0),
	window(nullptr)
{
	buttonId = settings.value(NAME_BUTTON).toInt();
	modifiers = (uint32_t)settings.value(NAME_MODIFIERS).toInt();
	keyCode = (uint32_t)settings.value(NAME_KEYCODE).toInt();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void HotkeyInfo::saveToConfig(QSettings &settings) const
{
	settings.setValue(NAME_BUTTON, buttonId);
	settings.setValue(NAME_MODIFIERS, modifiers);
	settings.setValue(NAME_KEYCODE, keyCode);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
QString HotkeyInfo::toString() const
{
	return getModifiersString() + getKeyString();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
QString HotkeyInfo::getModifiersString() const
{
	QString mods;
#ifdef _WIN32
	if (modifiers & MOD_WIN)
		mods += "WIN + ";
	if (modifiers & MOD_CONTROL)
		mods += "CTRL + ";
	if (modifiers & MOD_SHIFT)
		mods += "SHIFT + ";
	if (modifiers & MOD_ALT)
		mods += "ALT + ";
#endif
	return mods;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
QString HotkeyInfo::getKeyString() const
{
	QString key;
#ifdef _WIN32
	static_assert(sizeof(ushort) == sizeof(WCHAR), "WCHAR is weird size");
	WCHAR buf[128];
	UINT scanCode = MapVirtualKeyW(keyCode, MAPVK_VK_TO_VSC);
	BYTE keyboardState[256];
	GetKeyboardState(keyboardState);
	//int res = ToUnicode(keyCode, scanCode, keyboardState, buf, ARRAYSIZE(buf), 0);
	int res = GetKeyNameTextW(scanCode << 16, buf, ARRAYSIZE(buf));
	if (res != 0)
		key = QString::fromUtf16((ushort*)buf);
	else
		key = "?";
#endif
	return key;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
uint32_t HotkeyInfo::getNextHotkeyId()
{
	static std::set<uint32_t> usedIdSet;
	uint32_t id = 1;
	while (usedIdSet.find(id) != usedIdSet.end())
		id++;
	usedIdSet.insert(id);
	return id;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void HotkeyInfo::registerHotkey(const QWidget *window)
{
	this->window = window;
	id = getNextHotkeyId();

#ifdef _WIN32
	logInfo("Registering key %i to id %i with window id: %i", buttonId, id, (int)window->winId());
	BOOL res = RegisterHotKey((HWND)window->winId(), id, modifiers, keyCode);
	assert(res);
#endif
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void HotkeyInfo::unregisterHotkey()
{
#ifdef _WIN32
	if (id != 0)
	{
		logInfo("Window id: %i", (int)window->winId());
		BOOL res = UnregisterHotKey((HWND)window->winId(), id);
		assert(res);
		id = 0;
	}
#endif
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void HotkeyInfo::performAction()
{
	logInfo("Hotkey button: %i, id: %i", buttonId, id);
	if (buttonId == -1)
		sb_stopPlayback();
	else
		sb_playButton(buttonId);
}

