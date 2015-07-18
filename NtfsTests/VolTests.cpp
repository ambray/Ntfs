#include <Windows.h>
#include <gmock\gmock.h>
#include "..\ChangeJournal\Volume.h"
#include "..\Utils\Buffer.h"
#include <iostream>
#include <string>
#include <vector>
#include"..\ChangeJournal\ntfs_defs.h"

TEST(VolumeTest, TestVolInfo)
{
	VOL_INFO v1;
	VOL_INFO v2;
	WCHAR fn[MAX_PATH + 1] = { 0 };
	WCHAR fs[MAX_PATH + 1] = { 0 };

	Volume v;
	HANDLE tmp = INVALID_HANDLE_VALUE;

	ASSERT_EQ(ERROR_INVALID_HANDLE, v.setHandle(tmp));

	ASSERT_EQ(ERROR_INVALID_HANDLE, v.getVolInfo(v1));

	tmp = CreateFileA("\\\\.\\C:", GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_DEVICE, NULL);
	std::cout << "Last Error: " << GetLastError() << std::endl;
	ASSERT_NE(INVALID_HANDLE_VALUE, tmp);

	ASSERT_EQ(ERROR_SUCCESS, v.setHandle(tmp));

	ASSERT_TRUE(GetVolumeInformationByHandleW(tmp, fn, MAX_PATH, &v1.serial, &v1.maxComponentLength, &v1.fsFlags, fs, MAX_PATH));

	v1.fsName = fs;
	v1.volName = fn;
	std::wcout << v1.fsName << std::endl;
	v1.driveType = GetDriveTypeA("\\\\.\\C:");
	ASSERT_EQ(ERROR_SUCCESS, v.getVolInfo(v2));
	ASSERT_EQ(ERROR_SUCCESS, v1.fsName.compare(v2.fsName.c_str()));
	ASSERT_EQ(v1.driveType, v2.driveType);
	ASSERT_EQ(v1.fsFlags, v2.fsFlags);
	ASSERT_EQ(v1.serial, v2.serial);
	ASSERT_EQ(v1.maxComponentLength, v2.maxComponentLength);

}


TEST(VolumeTest, TestVolData)
{
	PNTFS_VOLUME_DATA_BUFFER volData = NULL;
	std::wstring volname = L"C:";
	Volume v(volname);
	Volume s;
	DWORD bytes;
	DWORD size = sizeof(NTFS_VOLUME_DATA_BUFFER) + sizeof(NTFS_EXTENDED_VOLUME_DATA);
	Buffer b(size);
	Buffer n(2);
	Buffer x((DWORD)-1);

	ASSERT_EQ(ERROR_INVALID_HANDLE, s.getVolData(n));

	const HANDLE tmp = v.getHandle();
	ASSERT_NE(INVALID_HANDLE_VALUE, tmp);
	volData = (PNTFS_VOLUME_DATA_BUFFER)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
	ASSERT_NE(NULL, (SIZE_T)volData);

	ASSERT_TRUE(DeviceIoControl(tmp, FSCTL_GET_NTFS_VOLUME_DATA, NULL, 0, volData, size, &bytes, NULL));

	ASSERT_EQ(ERROR_SUCCESS, v.getVolData(b));
	const PBYTE buf = b.getBuffer();
	ASSERT_EQ(ERROR_SUCCESS, memcmp(volData, (PNTFS_VOLUME_DATA_BUFFER)buf, size));
	HeapFree(GetProcessHeap(), 0, volData);

	ASSERT_EQ(ERROR_SUCCESS, v.getVolData(x));
	ASSERT_NE(NULL, (SIZE_T)x.getBuffer());


}


TEST(VolumeTest, TestGetMftRecord)
{
	Volume v(std::wstring(L"C:"));
	Buffer mft(1024);
	Volume s;


	ASSERT_NE(INVALID_HANDLE_VALUE, v.getHandle());
	ASSERT_EQ(ERROR_SUCCESS, v.getMftRecordByNumber(0, mft));

	const PNTFS_RECORD_HEADER rec = (const PNTFS_RECORD_HEADER)mft.getBuffer();
	ASSERT_NE(NULL, (SIZE_T)rec);
	mft.resize(100);
	ASSERT_EQ(ERROR_SUCCESS, v.getMftRecordByNumber(100, mft));
	ASSERT_EQ(ERROR_INVALID_HANDLE, s.getMftRecordByNumber(0, mft));



}

TEST(VolumeTest, TestGetFileCound)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	LONGLONG fCount;
	Volume s;

	hFile = CreateFileA("\\\\.\\C:", GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_DEVICE, NULL);
	ASSERT_NE(INVALID_HANDLE_VALUE, hFile);

	Volume v(hFile);
	ASSERT_NE(INVALID_HANDLE_VALUE, v.getHandle());

	ASSERT_EQ(ERROR_SUCCESS, v.getFileCount(fCount));
	ASSERT_NE(0, fCount);

	ASSERT_EQ(ERROR_INVALID_HANDLE, s.getFileCount(fCount));

}

TEST(VolumeTest, TestBadData)
{
	Volume v;

	ASSERT_EQ(ERROR_INVALID_HANDLE, v.setHandle((HANDLE)12345));
	ASSERT_EQ(ERROR_FILE_NOT_FOUND, v.vopen(std::wstring(L"asdfasdfjl")));

}