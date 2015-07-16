#include <Windows.h>
#include <gmock\gmock.h>
#include "..\ChangeJournal\Volume.h"
#include "..\Utils\Buffer.h"
#include <iostream>
#include <string>
#include <vector>

TEST(VolumeTest, TestVolInfo)
{
	VOL_INFO v1;
	VOL_INFO v2;
	WCHAR fn[MAX_PATH + 1] = { 0 };
	WCHAR fs[MAX_PATH + 1] = { 0 };

	Volume v;
	HANDLE tmp = INVALID_HANDLE_VALUE;

	ASSERT_EQ(ERROR_INVALID_HANDLE, v.setHandle(tmp));

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
	DWORD bytes;
	DWORD size = sizeof(NTFS_VOLUME_DATA_BUFFER) + sizeof(NTFS_EXTENDED_VOLUME_DATA);
	Buffer b(size);

	const HANDLE tmp = v.getHandle();
	ASSERT_NE(INVALID_HANDLE_VALUE, tmp);
	volData = (PNTFS_VOLUME_DATA_BUFFER)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
	ASSERT_NE(NULL, (SIZE_T)volData);

	ASSERT_TRUE(DeviceIoControl(tmp, FSCTL_GET_NTFS_VOLUME_DATA, NULL, 0, volData, size, &bytes, NULL));

	ASSERT_EQ(ERROR_SUCCESS, v.getVolData(b));
	const PBYTE buf = b.getBuffer();
	ASSERT_EQ(ERROR_SUCCESS, memcmp(volData, (PNTFS_VOLUME_DATA_BUFFER)buf, size));
	HeapFree(GetProcessHeap(), 0, volData);

}
