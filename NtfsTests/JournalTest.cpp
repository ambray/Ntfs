#include <gmock\gmock.h>
#include "..\ChangeJournal\Journal.h"
#include <string>
#include "JournalMock.h"

#define ERROR_CANT_FIND		2
typedef struct {
	USN_RECORD_V2 record;
	WCHAR moreStuff[10];
} USN_TEST_RECORD_V2, *PUSN_TEST_RECORD_V2;

typedef struct {
	USN_RECORD_V3 record;
	WCHAR moreStuff[10];
} USN_TEST_RECORD_V3, *PUSN_TEST_RECORD_V3;

TEST(JournalTest, BadVolname)
{
	int bad = 0;
	Journal j(std::wstring(L"asjdfkj"));

	bad = j.getError();
	ASSERT_EQ(bad, ERROR_CANT_FIND);
	ASSERT_EQ(INVALID_HANDLE_VALUE, j.getHandle());
	ASSERT_EQ(NULL, j.getData());
}

// NOTE: this test requires admin privs to run as intended (you need to be administrator to open a handle to \\.\C:)
TEST(JournalTest, CheckVolData)
{
	HANDLE tmp;
	DWORD bytes;
	USN_JOURNAL_DATA data = { 0 };

	tmp = CreateFileA("\\\\.\\C:", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_DEVICE, NULL);

	ASSERT_NE(INVALID_HANDLE_VALUE, tmp);

	ASSERT_TRUE(DeviceIoControl(tmp, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &data, sizeof(data), &bytes, NULL));

	Journal j(std::wstring(L"C:"));

	ASSERT_EQ(0, j.getError());
	ASSERT_NE(INVALID_HANDLE_VALUE, j.getHandle());

	const PUSN_JOURNAL_DATA tmpData = j.getData();
	ASSERT_NE(NULL, (DWORD)tmpData); 

	ASSERT_EQ(data.UsnJournalID, tmpData->UsnJournalID);

}


TEST(JournalTest, SetHandle)
{
	HANDLE tmp = INVALID_HANDLE_VALUE;
	Journal j;

	tmp = CreateFileA("\\\\.\\C:", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_DEVICE, NULL);

	ASSERT_NE(INVALID_HANDLE_VALUE, tmp);
	ASSERT_EQ(ERROR_SUCCESS, j.setHandle(tmp));
	ASSERT_NE(INVALID_HANDLE_VALUE, j.getHandle());
	ASSERT_EQ(ERROR_INVALID_HANDLE, j.setHandle(((HANDLE)((DWORD)-1))));
}


TEST(JournalTest, MarshallRecordTest)
{
	PWCHAR fname = L"AAAAAAAA";
	BYTE buf[16] = { 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4f, 0x50, 0x51 };
	USN_TEST_RECORD_V2 rv2 = { 0 };
	USN_TEST_RECORD_V3 rv3 = { 0 };
	std::wstring out, out2 = L"";

	JsonMarshaller j;
	Journal c;
	rv3.record.MajorVersion = 3;
	rv3.record.FileAttributes = rv2.record.FileAttributes = 1111;
	rv2.record.FileReferenceNumber = 222222;
	rv2.record.MajorVersion = 2;
	rv3.record.Usn = rv2.record.Usn = 123456;
	rv3.record.FileNameLength = rv2.record.FileNameLength = wcslen(fname) + 1;

	rv3.record.FileName[0] = rv2.record.FileName[0] = L'A';
	rv3.record.FileName[1] = rv2.record.FileName[1] = L'A';
	wcscpy(rv2.moreStuff, fname);
	wcscpy(rv3.moreStuff, fname);

	memcpy((PVOID)rv3.record.FileReferenceNumber.Identifier, buf, 16);
	memcpy(((PVOID)rv3.record.ParentFileReferenceNumber.Identifier), buf, 16);

	ASSERT_EQ(ERROR_INVALID_PARAMETER, c.marshallUsnRecord(NULL, (PUSN_RECORD)&rv2, out));
	ASSERT_EQ(ERROR_INVALID_PARAMETER, c.marshallUsnRecord((IMarshaller*)&j, NULL, out));
	ASSERT_EQ(ERROR_SUCCESS, c.marshallUsnRecord((IMarshaller*)&j, (PUSN_RECORD)&rv2, out));
	ASSERT_EQ(ERROR_SUCCESS, c.marshallUsnRecord((IMarshaller*)&j, (PUSN_RECORD)&rv3, out2));

	std::wcout << L"Result from first step: " << out << std::endl << L"Results from second: " << out2 << std::endl;
}


TEST(JournalTest, SetNameTest)
{
	std::wstring vn = L"C:";
	Journal j;

	ASSERT_EQ(ERROR_SUCCESS, j.setName(vn));
	ASSERT_NE(INVALID_HANDLE_VALUE, j.getHandle());
	ASSERT_NE(NULL, (SIZE_T)j.getData());

	vn = L"Derp";
	ASSERT_NE(ERROR_SUCCESS, j.setName(vn));
	ASSERT_EQ(INVALID_HANDLE_VALUE, j.getHandle());
	ASSERT_EQ(NULL, (SIZE_T)j.getData());
}

TEST(JournalTest, TestBufferEnum)
{
	JsonMarshaller jm;
	Journal j(std::wstring(L"C:"));
	std::vector<std::wstring> list;

	ASSERT_EQ(ERROR_SUCCESS, ChangeJournal::enumerateRecords((IJournal*)&j, (IMarshaller*)&jm, list));
	ASSERT_NE(0, list.size());
}