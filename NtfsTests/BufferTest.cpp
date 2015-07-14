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