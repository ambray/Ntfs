#pragma once

#include <Windows.h>
#include <string>
#include <deque>
#include <iostream>
#include <fstream>
#include <regex>


enum DataFormat {
	JsonData,
};

class IMarshaller {
public:
	IMarshaller() {};
	virtual ~IMarshaller() {};
	virtual int marshallULong(PWCHAR, ULONG, std::wstring&) = 0;
	virtual int marshallLong(PWCHAR, LONG, std::wstring&) = 0;
	virtual int marshallULongLong(PWCHAR, ULONGLONG&, std::wstring&) = 0;
	virtual int marshallLongLong(PWCHAR, LONGLONG&, std::wstring&) = 0;
	virtual int marshallBytes(PWCHAR, PBYTE, SIZE_T, std::wstring&) = 0;
	virtual int marshallWstr(PWCHAR, PWCHAR, SIZE_T, std::wstring&) = 0;
	virtual int recordsToFile(std::wstring&, std::deque<std::wstring>&, bool) = 0;
	virtual const DataFormat marshallerType() = 0;
};


class JsonMarshaller : IMarshaller {
public:
	JsonMarshaller();
	virtual ~JsonMarshaller();
	virtual int marshallULong(PWCHAR key, ULONG in, std::wstring& output);
	virtual int marshallLong(PWCHAR key, LONG in, std::wstring& output);
	virtual int marshallULongLong(PWCHAR key, ULONGLONG& in, std::wstring& output);
	virtual int marshallLongLong(PWCHAR key, LONGLONG& in, std::wstring& output);
	virtual int marshallBytes(PWCHAR key, PBYTE in, SIZE_T len, std::wstring& output);
	virtual int marshallWstr(PWCHAR key, PWCHAR in, SIZE_T len, std::wstring& output);
	virtual int recordsToFile(std::wstring& outfile, std::deque<std::wstring>& strbuf, bool truncate=true);
	virtual const DataFormat marshallerType();
};