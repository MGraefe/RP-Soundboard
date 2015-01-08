
#ifndef ConfigModel_H__
#define ConfigModel_H__

#include <vector>
#include <QtCore/QSettings>

class ConfigModel
{
public:
	void readConfig(QSettings *settings);
	void writeConfig(QSettings *settings);

	const char *getFileName(int itemId);
	void setFileName(int itemId, const char *fn);
	void playFile(int itemId);

private:
	std::vector<std::string> m_fns;
};

#endif // ConfigModel_H__