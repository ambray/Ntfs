#include <gmock\gmock.h>
#include "..\Utils\Buffer.h"


TEST(BufferTest, TestStandardAlloc)
{
	DWORD cSize = 1024;
	DWORD bSize = 20000000;
	Buffer b(cSize);
	Buffer b2(bSize);

	ASSERT_EQ(ERROR_SUCCESS, b.getError());
	ASSERT_EQ(cSize, b.size());
	ASSERT_NE(NULL, (SIZE_T)b.getBuffer());

	ASSERT_EQ(ERROR_SUCCESS, b2.getError());
	ASSERT_EQ(bSize, b2.size());
	ASSERT_NE(NULL, (SIZE_T)b2.getBuffer());

	b.clear();
}


TEST(BufferTest, TestGiantAlloc)
{
	BYTE buf[1024];
	Buffer b((SIZE_T)-1);

	memset(buf, 0xcc, 1024);

	ASSERT_EQ(NULL, (SIZE_T)b.getBuffer());
	ASSERT_NE(ERROR_SUCCESS, b.getError());
	ASSERT_NE(ERROR_SUCCESS, b.copyTo(buf, 1024));
	ASSERT_NE(ERROR_SUCCESS, b.copyFrom(buf, 1024));
	ASSERT_NE(ERROR_SUCCESS, b.compare(buf, 1024));
	b.clear();
}


TEST(BufferTest, TestResize)
{
	Buffer b(1024);
	ASSERT_NE(NULL, (SIZE_T)b.getBuffer());
	ASSERT_EQ(1024, b.size());
	b.clear();

	b.resize(2048);
	ASSERT_EQ(ERROR_SUCCESS, b.getError());
	ASSERT_NE(NULL, (SIZE_T)b.getBuffer());
	ASSERT_EQ(2048, b.size());
	b.clear();
}

TEST(BufferTest, TestCopyTo)
{
	BYTE buf[2048] = { 0 };
	PBYTE badbuf = NULL;
	Buffer b(1024);
	Buffer c(2048);


	memset(buf, 0xcc, 2048);

	ASSERT_EQ(ERROR_BUFFER_OVERFLOW, b.copyTo(buf, 2048));
	ASSERT_EQ(ERROR_INVALID_PARAMETER, b.copyTo(badbuf, 1024));
	ASSERT_EQ(ERROR_SUCCESS, c.copyTo(buf, 2048));
	ASSERT_EQ(ERROR_SUCCESS, memcmp(buf, c.getBuffer(), 2048));

}


TEST(BufferTest, TestCopyFrom)
{
	BYTE buf1[1024] = { 0 };
	BYTE buf2[1024];
	PBYTE badbuf = NULL;
	Buffer b(1024);

	memset(buf2, 0xcc, 1024);
	ASSERT_EQ(ERROR_BUFFER_OVERFLOW, b.copyFrom(buf1, 512));
	ASSERT_EQ(ERROR_INVALID_PARAMETER, b.copyFrom(badbuf, 1024));

	ASSERT_EQ(ERROR_SUCCESS, b.copyTo(buf2, 1024));
	ASSERT_EQ(ERROR_SUCCESS, b.copyFrom(buf1, 1024));
	ASSERT_EQ(ERROR_SUCCESS, memcmp(buf1, buf2, 1024));

}


TEST(BufferTest, TestCompare)
{
	BYTE buf[2048];
	PBYTE badbuf = NULL;
	Buffer b(1024);

	memset(buf, 0xcc, 2048);

	ASSERT_EQ(ERROR_SUCCESS, b.copyTo(buf, 1024));
	ASSERT_EQ(ERROR_SUCCESS, b.compare(buf, 1024));
	// buffer is too large
	ASSERT_NE(ERROR_SUCCESS, b.compare(buf, 2048));

	memset(buf, 0x41, 1024);
	ASSERT_NE(ERROR_SUCCESS, b.compare(buf, 1024));
}


TEST(BufferTest, TestChangeType)
{
	BYTE buf[1024];
	Buffer b(1024, BufferType::TypeHeap);

	memset(buf, 0x41, 1024);
	
	ASSERT_EQ(ERROR_SUCCESS, b.copyTo(buf, 1024));
	const PBYTE b2 = b.getBuffer();

	ASSERT_EQ(ERROR_SUCCESS, memcmp(buf, b2, 1024));

	ASSERT_EQ(ERROR_SUCCESS, b.setType(BufferType::TypeVirtual));
	ASSERT_EQ(ERROR_SUCCESS, b.copyTo(buf, 1024));

	const PBYTE b3 = b.getBuffer();

	ASSERT_EQ(ERROR_SUCCESS, memcmp(buf, b3, 1024));

}

TEST(BufferTest, TestCopyOffsets)
{
	BYTE bufTest[1024] = { 0 };
	BYTE buf1[1024];
	BYTE buf2[1024];

	Buffer b(2048);

	memset(buf1, 0x41, 1024);
	memset(buf2, 0x42, 1024);

	ASSERT_NE(NULL, (SIZE_T)b.getBuffer());
	ASSERT_EQ(ERROR_SUCCESS, b.copyToOffset(buf1, 0, 1024));
	ASSERT_EQ(ERROR_SUCCESS, b.copyToOffset(buf2, 1024, 1024));

	ASSERT_EQ(ERROR_SUCCESS, b.copyFromOffset(bufTest, 0, 1024));
	ASSERT_EQ(ERROR_SUCCESS, memcmp(buf1, bufTest, 1024));
	
	SecureZeroMemory(bufTest, 1024);

	ASSERT_EQ(ERROR_SUCCESS, b.copyFromOffset(bufTest, 1024, 1024));
	ASSERT_EQ(ERROR_SUCCESS, memcmp(buf2, bufTest, 1024));

	ASSERT_EQ(ERROR_INVALID_PARAMETER, b.copyFromOffset(NULL, 1024, 1024));
	ASSERT_EQ(ERROR_INVALID_PARAMETER, b.copyToOffset(NULL, 1024, 1024));

	ASSERT_EQ(ERROR_BUFFER_OVERFLOW, b.copyToOffset(buf1, 1026, 1024));
	ASSERT_EQ(ERROR_BUFFER_OVERFLOW, b.copyFromOffset(buf1, 2000, 1024));
}


TEST(BufferTest, TestChangeAttribs)
{
	BUFFER_ATTRIBS attrs;
	MEMORY_BASIC_INFORMATION binf = { 0 };

	Buffer b(1024);


	memset(&attrs, 0, sizeof(attrs));

	attrs.type = BufferType::TypeHeap;
	attrs.u.hHeap = GetProcessHeap();

	ASSERT_EQ(ERROR_SUCCESS, b.setAttribs(&attrs));
	ASSERT_EQ(1024, b.size());
	ASSERT_NE(NULL, (SIZE_T)b.getBuffer());


	attrs.type = BufferType::TypeVirtual;
	attrs.u.memProtect = PAGE_EXECUTE_READWRITE;

	ASSERT_EQ(ERROR_SUCCESS, b.setAttribs(&attrs));

	const PBYTE buf = b.getBuffer();
	ASSERT_NE(NULL, (SIZE_T)buf);
	VirtualQuery(buf, &binf, b.size());

	ASSERT_EQ(PAGE_EXECUTE_READWRITE, binf.AllocationProtect);
}