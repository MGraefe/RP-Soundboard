
#ifndef ConfigModel_H__
#define ConfigModel_H__

#include <vector>
#include <QtCore/QSettings>

class ConfigModel
{
public:
	ConfigModel();
	void readConfig();
	void writeConfig();
	
	static QString GetConfigPath();
	static QString GetFullConfigPath();

	const char *getFileName(int itemId);
	void setFileName(int itemId, const char *fn);
	void playFile(int itemId);

	inline int getRows() const { return m_rows; }
	void setRows(int n);

	inline int getCols() const { return m_cols; }
	void setCols(int n);

	inline int getVolume() const { return m_volume; }
	void setVolume(int val);

	inline int getPlaybackLocal() const { return m_playbackLocal; }
	void setPlaybackLocal(int val);

private:
	std::vector<std::string> m_fns;
	int m_rows;
	int m_cols;
	int m_volume;
	int m_playbackLocal;
};

#endif // ConfigModel_H__