#pragma once

#include <windows.h>
#include <fstream>
#include <time.h>
#include <string>
#include <sstream>


#define LOG_FILE "C:\\Astromount\\out.log"
//#define CDR_FILE "C:\\cdr.txt"
#define CDR_DIR "C:\\Astromount"

inline std::string cdrFileName() {
	static char buf[80];
	SYSTEMTIME t;
	GetLocalTime(&t);
	sprintf_s(buf, CDR_DIR "\\%04d_%02d_%02d.log", t.wYear, t.wMonth, t.wDay);
	return buf;
}

inline std::string currentDateTime() {
	//time_t     now = time(0);
	//struct tm  tstruct;
	//tstruct = *localtime(&now);

	static char buf[80];
	//strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	SYSTEMTIME t;
	GetLocalTime(&t);
	sprintf_s(buf, "%02d:%02d:%02d.%03d", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
	return buf;
}

#define LOG(args) //\
{ \
	static std::ofstream fout; \
	fout.open(LOG_FILE, std::ios_base::app); \
	fout << "[" << currentDateTime() << "]\t" << GetCurrentThreadId() << "\t" << args << std::endl; \
	fout.close(); \
}

#define CDR(args) \
{ \
	static std::ofstream fout; \
	fout.open(cdrFileName(), std::ios_base::app); \
	fout << currentDateTime() << ";" << args << std::endl; \
	fout.close(); \
}
