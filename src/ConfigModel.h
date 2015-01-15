
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
	inline int getCols() const { return m_cols; }

	inline void setRows(int n) { m_rows = n; }
	inline void setCols(int n) { m_cols = n; }

private:
	std::vector<std::string> m_fns;
	int m_rows;
	int m_cols;
};

#endif // ConfigModel_H__