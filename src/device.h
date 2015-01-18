

#ifndef DEVICE_H__
#define DEVICE_H__

CAPI void sb_handleCaptureData(uint64 serverConnectionHandlerID, short* samples,
	int sampleCount, int channels, int* edited);
CAPI void sb_handlePlaybackData(uint64 serverConnectionHandlerID, short* samples, int sampleCount,
	int channels, const unsigned int *channelSpeakerArray, unsigned int *channelFillMask);
CAPI int sb_playFile(const char *filename);
CAPI void sb_stopPlayback();
//CAPI void sb_setVolume(int vol);
//CAPI void sb_setLocalPlayback(int enabled);
CAPI void sb_init();
CAPI void sb_kill();
CAPI void sb_onServerChange(uint64 serverID);
CAPI void sb_readConfig();
CAPI void sb_saveConfig();
CAPI void sb_openDialog();
CAPI void sb_playButton(int btn);
CAPI void sb_openAbout();


#endif