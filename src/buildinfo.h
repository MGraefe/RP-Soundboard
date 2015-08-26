
#ifndef buildinfo_H__
#define buildinfo_H__

#ifdef __cplusplus
extern "C" {
#endif

const char *buildinfo_getPluginVersion();
const char *buildinfo_getPluginName();
const char *buildinfo_getPluginAuthor();
const char *buildinfo_getPluginDescription();
const char *buildinfo_getBuildName();
const char *buildinfo_getPluginVersionShort();
const char *buildinfo_getBuildTarget();


//index: 0 = major, 1 = minor, 2 = revision, 3 = build
int buildinfo_getVersionNumber(int index);


#ifdef __cplusplus
} // extern "C"
#endif

#endif