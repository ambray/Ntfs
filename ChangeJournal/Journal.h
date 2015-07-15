#pragma once

#include <Windows.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <functional>
#include <vector>
#include "..\Utils\Buffer.h"
#include "..\Utils\Marshaller.h"

#define HANDLE_OPEN_FAILURE		1
#define JOURNAL_QUERY_FAILURE	2
#define QUERY_BUFFER_SIZE		20000000		
#define UNPACK_ELEMENT(d, f) (((PUSN_RECORD)d)->MajorVersion == 2) ? ((PUSN_RECORD_V2)d)->f : ((PUSN_RECORD_V3)d)->f

class IJournal {
public:
	IJournal() {};
	virtual ~IJournal() {};
	virtual int setHandle(HANDLE) = 0;
	virtual const HANDLE getHandle() = 0;
	virtual const PUSN_JOURNAL_DATA getData() = 0;
	virtual int setName(std::wstring&) = 0;
	virtual int marshallUsnRecord(IMarshaller*, PUSN_RECORD, std::wstring&) = 0;
	virtual int deleteJournal() = 0;
	virtual int createJournal(DWORDLONG&, DWORDLONG&) = 0;
	virtual int resetJournal() = 0;
	virtual int getError() = 0;
	virtual int map(std::function<void(PUSN_RECORD, PVOID)>&, PVOID) = 0;
};

class Journal : IJournal {
public:
	Journal();
	Journal(std::wstring& vname);
	virtual ~Journal();
	virtual int setHandle(HANDLE vh);
	virtual int setName(std::wstring& vn);
	virtual const HANDLE getHandle();
	virtual const PUSN_JOURNAL_DATA getData();
	virtual int marshallUsnRecord(IMarshaller* mr, PUSN_RECORD rec, std::wstring& out);
	virtual int deleteJournal();
	virtual int createJournal(DWORDLONG& maxSize, DWORDLONG& allocDelta);
	virtual int resetJournal();
	virtual int getError();
	virtual int map(std::function<void(PUSN_RECORD, PVOID)>& func, PVOID optArg=NULL);
	virtual int getRecords(Buffer& buff, USN& next, PDWORD bytesRead);
private:
	HANDLE vol;
	PUSN_JOURNAL_DATA data;
	std::wstring name;
	int error;

	int vopen();
	int queryJournal();
};

namespace ChangeJournal {

	

	int walkRecordBuffer(Buffer& buf, IJournal* journ, IMarshaller* marsh, std::vector<std::wstring>& output, USN* next, DWORD bytesToWalk);
	int getRecords(IJournal* journal, Buffer& buf, USN& next, PDWORD bytesRead);
	int enumerateRecords(IJournal* journal, IMarshaller* marshaller, std::vector<std::wstring>& output);
}