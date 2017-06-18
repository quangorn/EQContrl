#pragma once

#include <windows.h>
#include <fstream>
#include <time.h>
#include <string>
#include <sstream>


#define LOG_FILE "C:\\out.log"
#define ENC_FILE "C:\\enc.txt"
#define CDR_FILE "C:\\cdr.txt"

inline std::string currentDateTime() {
	//time_t     now = time(0);
	//struct tm  tstruct;
	//tstruct = *localtime(&now);

	static char buf[80];
	//strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	SYSTEMTIME t;
	GetLocalTime(&t);
	sprintf_s(buf, "[%02d:%02d:%02d.%03d]\t", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
	return buf;
}
#ifdef _LOG
#define LOG(args) \
{ \
	static std::ofstream fout; \
	fout.open(LOG_FILE, std::ios_base::app); \
	fout << currentDateTime() << args << std::endl; \
	fout.close(); \
}
#else
#define LOG(args)
#endif

#define ENC(args)// \
//{ \
//	static std::ofstream fout; \
//	fout.open(ENC_FILE, std::ios_base::app); \
//	fout << currentDateTime() << args << std::endl; \
//	fout.close(); \
//}

#define CDR(args)// \
//{ \
//	static std::ofstream fout; \
//	fout.open(CDR_FILE, std::ios_base::app); \
//	fout << currentDateTime() << args << std::endl; \
//	fout.close(); \
//}
