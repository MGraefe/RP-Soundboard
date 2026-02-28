
#include "common.h"
#include "TalkStateManager.h"
#include "ts3log.h"
#include "main.h"
#include <QMetaEnum>

#define RETURN_ENUM_CASE(val) case val: return #val
const char * TalkStateManager::toString(talk_state_e ts)
{
	switch (ts)
	{
		RETURN_ENUM_CASE(TS_INVALID);
		RETURN_ENUM_CASE(TS_PTT_WITHOUT_VA);
		RETURN_ENUM_CASE(TS_PTT_WITH_VA);
		RETURN_ENUM_CASE(TS_VOICE_ACTIVATION);
		RETURN_ENUM_CASE(TS_CONT_TRANS);
	default:
		throw std::logic_error("Fucked up toString method");
	}
}


TalkStateManager::TalkStateManager() :
	previousTalkState(TS_INVALID),
	defaultTalkState(TS_INVALID),
	currentTalkState(TS_INVALID),
	activeServerId(0),
	playingServerId(0)
{
	
}


TalkStateManager::~TalkStateManager()
{
	
}


void TalkStateManager::onStartPlaying(bool preview, QString filename)
{
	if (!preview)
	{
		playingServerId = activeServerId;
		setPlayTransMode();
	}
}


void TalkStateManager::onStopPlaying()
{
	setTalkTransMode();
}


void TalkStateManager::onPauseSound()
{
	setTalkTransMode();
}


void TalkStateManager::onUnpauseSound()
{
	setPlayTransMode();
}


void TalkStateManager::setTalkTransMode()
{
	if (previousTalkState == TS_INVALID)
		return;
	talk_state_e ts = previousTalkState;
	previousTalkState = TS_INVALID;
	setTalkState(activeServerId, ts);
}


void TalkStateManager::setPlayTransMode()
{
	talk_state_e s = getTalkState(activeServerId);
	if (defaultTalkState == TS_INVALID)
		defaultTalkState = s; // Set once at first played file
							  // Don't accept a sudden change to TS_CONT_TRANS except when defaultTalkState is also TS_CONT_TRANS
	if (s == TS_CONT_TRANS)
		s = defaultTalkState;

	// When s is invalid, use defaultTalkState (could also be invalid, care)
	if (s == TS_INVALID)
		s = defaultTalkState;

	// If state is still invalid it's bad luck :/
	if (s == TS_INVALID)
		return;

	previousTalkState = s;
	setContinuousTransmission(activeServerId);
}


void TalkStateManager::setActiveServerId(uint64 id)
{
	logDebug("TSMGR: Setting active server id: %i -> %i", (int)activeServerId, (int)id);
	if (id == activeServerId)
		return;
	talk_state_e oldCurrentTS = currentTalkState;
	if (activeServerId != 0 && previousTalkState != TS_INVALID)
		setTalkState(activeServerId, previousTalkState);
	activeServerId = id;
	if (oldCurrentTS == TS_CONT_TRANS)
	{
		previousTalkState = id != 0 ? getTalkState(id) : TS_INVALID;
		if (previousTalkState != TS_INVALID)
			setContinuousTransmission(id);
		else
			sb_stopPlayback();
	}
}


TalkStateManager::talk_state_e TalkStateManager::getTalkState(uint64 scHandlerID)
{
	char *vadStr;
	if (checkError(ts3Functions.getPreProcessorConfigValue(scHandlerID, "vad", &vadStr), "Error retrieving vad setting"))
		return TS_INVALID;
	bool vad = strcmp(vadStr, "true") == 0;
	ts3Functions.freeMemory(vadStr);

	int input;
	if (checkError(ts3Functions.getClientSelfVariableAsInt(scHandlerID, CLIENT_INPUT_DEACTIVATED, &input), "Error retrieving input setting"))
		return TS_INVALID;
	bool ptt = input == INPUT_DEACTIVATED;

	if (ptt)
		return vad ? TS_PTT_WITH_VA : TS_PTT_WITHOUT_VA;
	else
		return vad ? TS_VOICE_ACTIVATION : TS_CONT_TRANS;
}


bool TalkStateManager::setTalkState(uint64 scHandlerID, talk_state_e state)
{
	logDebug("TSMGR: Setting talk state of %ull to %s, previous was %s",
		(unsigned long long)scHandlerID, toString(state), toString(previousTalkState));
	
	if (scHandlerID == 0 || state == TS_INVALID)
		return false;

	bool va = state == TS_PTT_WITH_VA || state == TS_VOICE_ACTIVATION;
	bool in = state == TS_CONT_TRANS || state == TS_VOICE_ACTIVATION;

	if (checkError(ts3Functions.setPreProcessorConfigValue(
		scHandlerID, "vad", va ? "true" : "false"), "Error toggling vad"))
		return false;

	if (checkError(ts3Functions.setClientSelfVariableAsInt(scHandlerID, CLIENT_INPUT_DEACTIVATED,
		in ? INPUT_ACTIVE : INPUT_DEACTIVATED), "Error toggling input"))
		return false;

	ts3Functions.flushClientSelfUpdates(scHandlerID, nullptr);
	currentTalkState = state;
	return true;
}


bool TalkStateManager::setPushToTalk(uint64 scHandlerID, bool voiceActivation)
{
	return setTalkState(scHandlerID, voiceActivation ? TS_PTT_WITH_VA : TS_PTT_WITHOUT_VA);
}


bool TalkStateManager::setVoiceActivation(uint64 scHandlerID)
{
	return setTalkState(scHandlerID, TS_VOICE_ACTIVATION);
}


bool TalkStateManager::setContinuousTransmission(uint64 scHandlerID)
{
	return setTalkState(scHandlerID, TS_CONT_TRANS);
}


void TalkStateManager::onClientStopsTalking()
{
	// If we are in PTT mode and the client lets go of the PTT key while playing a sound, ptt state gets reset to not-talking.
	// This function checks for that case and sets it again to TS_CONTR_TRANS
	if (currentTalkState == TS_CONT_TRANS && (previousTalkState == TS_PTT_WITHOUT_VA || previousTalkState == TS_PTT_WITH_VA))
		setPlayTransMode();
}
