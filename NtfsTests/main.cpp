#include <Windows.h>
#include <gmock\gmock.h>



int wmain(int argc, WCHAR** argv, WCHAR** envp)
{
	::testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}