#include <gmock\gmock.h>
#include "..\Utils\Marshaller.h"


TEST(JsonMarshallerTest, LongMarshall)
{
	WCHAR buf[1025] = { 0 };
	std::wstring base = L"\"Key\" : 1024";
	std::wstring out;
	DWORD sample = 1024;
	JsonMarshaller j;

	memset(buf, 0x41, sizeof(WCHAR) * 1024);

	ASSERT_EQ(ERROR_INVALID_PARAMETER, j.marshallULong(buf, sample, out));
	ASSERT_EQ(ERROR_INVALID_PARAMETER, j.marshallULong(NULL, sample, out));
	ASSERT_EQ(ERROR_SUCCESS, j.marshallULong(L"Key", sample, out));
	ASSERT_EQ(ERROR_SUCCESS, base.compare(out.c_str()));
}


TEST(JsonMarshallerTest, LongLongMarshall)
{
	WCHAR buf[1025] = { 0 };
	std::wstring base = L"\"Key\" : \"123456789000\"";
	ULONGLONG sample = 123456789000;
	std::wstring out;
	JsonMarshaller j;

	memset(buf, 0x41, sizeof(WCHAR) * 1024);

	ASSERT_EQ(ERROR_INVALID_PARAMETER, j.marshallULongLong(buf, sample, out));
	ASSERT_EQ(ERROR_INVALID_PARAMETER, j.marshallULongLong(NULL, sample, out));
	ASSERT_EQ(ERROR_SUCCESS, j.marshallULongLong(L"Key", sample, out));
	ASSERT_EQ(ERROR_SUCCESS, base.compare(out.c_str()));
}


TEST(JsonMarshallerTest, SignedLongLongMarshall)
{
	WCHAR buf[1025] = { 0 };
	std::wstring base = L"\"Key\" : \"-123456789000\"";
	LONGLONG sample = -123456789000;
	std::wstring out;
	JsonMarshaller j;

	memset(buf, 0x41, sizeof(WCHAR) * 1024);

	ASSERT_EQ(ERROR_INVALID_PARAMETER, j.marshallLongLong(buf, sample, out));
	ASSERT_EQ(ERROR_INVALID_PARAMETER, j.marshallLongLong(NULL, sample, out));
	ASSERT_EQ(ERROR_SUCCESS, j.marshallLongLong(L"Key", sample, out));
	std::wcout << L"Actual: " << out << L" vs " << base << std::endl;
	ASSERT_EQ(ERROR_SUCCESS, base.compare(out.c_str()));
}

TEST(JsonMarshallerTest, SignedLongMarshall)
{
	WCHAR buf[1025] = { 0 };
	std::wstring base = L"\"Key\" : -1024";
	std::wstring out;
	LONG sample = -1024;
	JsonMarshaller j;

	memset(buf, 0x41, sizeof(WCHAR) * 1024);

	ASSERT_EQ(ERROR_INVALID_PARAMETER, j.marshallLong(buf, sample, out));
	ASSERT_EQ(ERROR_INVALID_PARAMETER, j.marshallLong(NULL, sample, out));
	ASSERT_EQ(ERROR_SUCCESS, j.marshallLong(L"Key", sample, out));
	std::wcout << L"Actual: " << out << L" vs " << base << std::endl;
	ASSERT_EQ(ERROR_SUCCESS, base.compare(out.c_str()));
}

TEST(JsonMarshallerTest, ByteMarshall)
{
	std::wstring base = L"\"Key\" : \"AAAAAAAAAAAAAAAA\"";
	BYTE buf[17] = { 0 };
	BYTE badbuf[1024] = { 0 };
	std::wstring out;
	JsonMarshaller j;
	
	memset(buf, 0x41, 16);

	ASSERT_EQ(ERROR_INVALID_PARAMETER, j.marshallBytes(L"Key", badbuf, 1024, out));
	ASSERT_EQ(ERROR_INVALID_PARAMETER, j.marshallBytes((PWCHAR)NULL, buf, 16, out));
	ASSERT_EQ(ERROR_INVALID_PARAMETER, j.marshallBytes(L"Key", (PBYTE)NULL, 16, out));

	out = L"";
	memset(buf, 0x41, 16);
	ASSERT_EQ(ERROR_SUCCESS, j.marshallBytes(L"Key", buf, 16, out));
	ASSERT_EQ(ERROR_SUCCESS, base.compare(out.c_str()));
}


TEST(JsonMarshallerTest, WstrMarshall)
{
	PWCHAR stuff = L"Value. And stuff.";
	WCHAR buf_too_big[1025];
	std::wstring base = L"\"Key\" : \"Value. And stuff.\"";
	std::wstring out;
	DWORD len = wcslen(stuff);
	JsonMarshaller j;

	memset(buf_too_big, 0x41, sizeof(WCHAR) * 1024);

	ASSERT_EQ(ERROR_INVALID_PARAMETER, j.marshallWstr(NULL, stuff, len, out));
	ASSERT_EQ(ERROR_INVALID_PARAMETER, j.marshallWstr(L"Key", NULL, len, out));
	ASSERT_EQ(ERROR_INVALID_PARAMETER, j.marshallWstr(L"Key", buf_too_big, 1024, out));
	ASSERT_EQ(ERROR_SUCCESS, j.marshallWstr(L"Key", stuff, len, out));
	ASSERT_EQ(ERROR_SUCCESS, base.compare(out.c_str()));
}