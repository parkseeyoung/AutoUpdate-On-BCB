#include <string>
#include <time.h>
#include <stdio.h>
using std::string;
class MyLog
{
public:
	MyLog();
	MyLog(string path):LogPath(path)
	{
	};
	void WriteLog(string mesg);
protected:
	string LogTime;
	string LogPath;
private:
};