#include "Volume.h"

Volume::Volume() : hVol(INVALID_HANDLE_VALUE), volData(VOL_DATA_SIZE)
{

}

Volume::Volume(std::wstring& volName) : hVol(INVALID_HANDLE_VALUE), volData(VOL_DATA_SIZE)
{
	std::wstring path = L"\\\\.\\";
	path += volName;

	vopen(path);
}

Volume::Volume(HANDLE volHandle) : hVol(INVALID_HANDLE_VALUE), volData(VOL_DATA_SIZE)
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
	BYTE tmpData[VOL_DATA_SIZE] = { 0 };
	int status = ERROR_SUCCESS;
	DWORD tmp;
	DWORD alloc = VOL_DATA_SIZE;

	if (INVALID_HANDLE_VALUE == hVol)
		return ERROR_INVALID_HANDLE;

	if (!db.isValid() || alloc > db.size())
		status = db.resize(alloc);

	if (ERROR_SUCCESS != status)
		return status;


	if (!DeviceIoControl(hVol, FSCTL_GET_NTFS_VOLUME_DATA, NULL, 0, tmpData, VOL_DATA_SIZE, &tmp, NULL)) {
		return GetLastError();
	}

	db.copyTo(tmpData, VOL_DATA_SIZE);

	return status;
}


/**
* Will get a record from the MFT based on its record number.
*/
int Volume::getMftRecordByNumber(LONGLONG recordNum, Buffer& buffer) 
{
	NTFS_FILE_RECORD_INPUT_BUFFER	ntInBuff = { 0 };
	PNTFS_FILE_RECORD_OUTPUT_BUFFER	ntOutBuff = NULL;
	ULONG							recordSize = 0;
	ULONG							bytesRet;
	int								status = ERROR_SUCCESS;
	
	if (INVALID_HANDLE_VALUE == hVol)
		return ERROR_INVALID_HANDLE;

	status = getVolData(volData);

	if (ERROR_SUCCESS != status)
		return status;

	const PNTFS_VOLUME_DATA_BUFFER data = (PNTFS_VOLUME_DATA_BUFFER)volData.getBuffer();
	if (NULL == data)
		return ERROR_OUTOFMEMORY;

	recordSize = data->BytesPerFileRecordSegment;
	recordSize += sizeof(NTFS_FILE_RECORD_OUTPUT_BUFFER);
	if (!buffer.isValid() || recordSize > buffer.size())
		status = buffer.resize(recordSize);

	if (ERROR_SUCCESS != status)
		return status;

	ntInBuff.FileReferenceNumber.QuadPart = recordNum;
	if (!DeviceIoControl(hVol, FSCTL_GET_NTFS_FILE_RECORD, &ntInBuff, sizeof(ntInBuff), buffer.getBuffer(), buffer.size(), &bytesRet, NULL)) {
		status = GetLastError();
	}

	return status;
}

int Volume::getFileCount(LONGLONG& count) 
{
	int status = ERROR_SUCCESS;
	LARGE_INTEGER num = { 0 };

	if (INVALID_HANDLE_VALUE == hVol)
		return ERROR_INVALID_HANDLE;

	status = getVolData(volData);

	if (ERROR_SUCCESS != status)
		return status;

	const PNTFS_VOLUME_DATA_BUFFER buf = (const PNTFS_VOLUME_DATA_BUFFER)volData.getBuffer();
	if (NULL == buf)
		return ERROR_INVALID_STATE;
	
	num.QuadPart = buf->BytesPerFileRecordSegment;

	if (0 == num.QuadPart)
		return ERROR_BAD_ARGUMENTS;

	count = buf->MftValidDataLength.QuadPart / num.QuadPart;

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

	status = getVolData(volData);

	return status;
}