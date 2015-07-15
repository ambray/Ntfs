#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include "..\Utils\Buffer.h"
#include "ntfs_defs.h"

typedef struct {
	std::wstring volName;
	std::wstring fsName;
	DWORD		 serial;
	DWORD		 fsFlags;
	DWORD		 maxComponentLength;
	DWORD	     driveType;
} VOL_INFO, *PVOL_INFO;

class IVolume {
public:
	IVolume() {};
	virtual ~IVolume() {};
	virtual int setHandle(HANDLE) = 0;
	virtual const HANDLE getHandle() = 0;
	virtual int getVolInfo(VOL_INFO&) = 0;
	virtual int getVolData(Buffer&) = 0;
	virtual int getMftRecordByNumber(LONGLONG, Buffer&) = 0;
	virtual int getFileCount(LONGLONG& count) = 0;
};


class Volume : IVolume {
public:
	Volume();
	Volume(std::wstring& volName);
	Volume(HANDLE volHandle);
	virtual ~Volume();
	virtual int setHandle(HANDLE nHandle);
	virtual const HANDLE getHandle();
	virtual int getVolInfo(VOL_INFO& vi);
	virtual int getVolData(Buffer& db);
	virtual int getMftRecordByNumber(LONGLONG, Buffer& buf);
	virtual int getFileCount(LONGLONG& count);
private:
	HANDLE hVol;

	int vopen(std::wstring& fname);
};
