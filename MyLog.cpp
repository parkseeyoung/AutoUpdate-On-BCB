#include "MyLog.h"

void MyLog::WriteLog(string mesg)
{
	//…Ë÷√ ±º‰
	time_t dtime;
	time(&dtime);
	char lgtime[20];
	strftime(lgtime,20,"%Y-%m-%d %X",localtime(&dtime));
	LogTime = lgtime;
	if(LogTime==""||LogPath=="")
		return;
	mesg = LogTime+"  "+mesg;
	FILE*fp = fopen(LogPath.c_str(),"a");
	fputs(mesg.c_str(),fp);
	fclose(fp);
}