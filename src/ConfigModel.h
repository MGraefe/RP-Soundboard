
#ifndef ConfigModel_H__
#define ConfigModel_H__

#include <vector>
#include <QtCore/QString>

class ConfigModel
{
public:
	enum notifications_e
	{
		NOTIFY_SET_FILENAME,
		NOTIFY_SET_ROWS,
		NOTIFY_SET_COLS,
		NOTIFY_SET_VOLUME,
		NOTIFY_SET_PLAYBACK_LOCAL,
		NOTIFY_SET_MUTE_MYSELF_DURING_PB,
		NOTIFY_SET_WINDOW_SIZE,
	};

	class Observer
	{
	public:
		virtual void notify(ConfigModel &model, notifications_e what, int data) = 0;
	};

public:
	ConfigModel();
	void readConfig();
	void writeConfig();
	
	static QString GetConfigPath();
	static QString GetFullConfigPath();

	QString getFileName(int itemId) const;
	void setFileName(int itemId, const QString &fn);

	inline int getRows() const { return m_rows; }
	void setRows(int n);

	inline int getCols() const { return m_cols; }
	void setCols(int n);

	inline int getVolume() const { return m_volume; }
	void setVolume(int val);

	inline bool getPlaybackLocal() const { return m_playbackLocal; }
	void setPlaybackLocal(bool val);
	
	inline bool getMuteMyselfDuringPb() const { return m_muteMyselfDuringPb; }
	void setMuteMyselfDuringPb(bool val);

	void getWindowSize(int *width, int *height) const;
	void setWindowSize(int width, int height);

	void addObserver(Observer *obs);
	void remObserver(Observer *obs);

private:
	void notify(notifications_e what, int data);

	std::vector<Observer*> m_obs;
	std::vector<QString> m_fns;
	int m_rows;
	int m_cols;
	int m_volume;
	bool m_playbackLocal;
	bool m_muteMyselfDuringPb;
	int m_windowWidth;
	int m_windowHeight;
};

#endif // ConfigModel_H__