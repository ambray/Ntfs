#include "Volume.h"

Volume::Volume() : hVol(INVALID_HANDLE_VALUE)
{

}

Volume::Volume(std::wstring& volName) : hVol(INVALID_HANDLE_VALUE)
{
	std::wstring path = L"\\\\.\\";
	path += volName;

	vopen(path);
}

Volume::Volume(HANDLE volHandle) : hVol(INVALID_HANDLE_VALUE)
{
	setHandle(volHandle);
}

Volume::~Volume() 
{
	if (INVALID_HANDLE_VALUE != hVol)
		CloseHandle(hVol);
}

/**
* Duplicate the handle for our object, ensuring we'll be safe on shutdown.
*/
int Volume::setHandle(HANDLE nHandle) 
{
	int status = ERROR_SUCCESS;
	HANDLE tmp;

	if (INVALID_HANDLE_VALUE == nHandle || NULL == nHandle)
		return ERROR_INVALID_HANDLE;

	if (!DuplicateHandle(GetCurrentProcess(), nHandle, GetCurrentProcess(), &tmp, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
		status = GetLastError();
		return status;
	}

	hVol = tmp;

	return status;
}

const HANDLE Volume::getHandle() 
{
	return const_cast<const HANDLE>(hVol);
}

int Volume::getVolInfo(VOL_INFO& vi) 
{
	WCHAR volName[MAX_PATH + 1] = { 0 };
	WCHAR fsName[MAX_PATH + 1] = { 0 };

	int status = ERROR_SUCCESS;

	if (INVALID_HANDLE_VALUE == hVol)
		return ERROR_INVALID_HANDLE;

	if (!GetVolumeInformationByHandleW(hVol, volName, MAX_PATH, &vi.serial, &vi.maxComponentLength, &vi.fsFlags, fsName, MAX_PATH)) {
		return GetLastError();
	}

	vi.volName = volName;
	vi.fsName = fsName;
	vi.driveType = GetDriveTypeW(vi.volName.c_str());

	return status;
}

int Volume::getVolData(Buffer& db) 
{
	int status = ERROR_SUCCESS;
	DWORD tmp;
	DWORD alloc = sizeof(NTFS_VOLUME_DATA_BUFFER) + sizeof(NTFS_EXTENDED_VOLUME_DATA);

	if (INVALID_HANDLE_VALUE == hVol)
		return ERROR_INVALID_HANDLE;

	if (alloc > db.size())
		status = db.resize(alloc);

	if (ERROR_SUCCESS != status)
		return status;


	if (!DeviceIoControl(hVol, FSCTL_GET_NTFS_VOLUME_DATA, NULL, 0, db.getBuffer(), db.size(), &tmp, NULL)) {
		return GetLastError();
	}

	return status;
}

int Volume::getMftRecordByNumber(LONGLONG, Buffer& buf) 
{
	int status = ERROR_SUCCESS;

	return status;
}

int Volume::getFileCount(LONGLONG& count) 
{
	int status = ERROR_SUCCESS;

	return status;
}


int Volume::vopen(std::wstring& fname)
{
	int status = ERROR_SUCCESS;
	DWORD accessMask = GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE;
	DWORD share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

	if (INVALID_HANDLE_VALUE == (hVol = CreateFileW(fname.c_str(), accessMask, share, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_DEVICE, NULL))) {
		return GetLastError();
	}


	return status;
}