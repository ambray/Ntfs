#include <gmock\gmock.h>
#include "..\Utils\ArgParser.h"

TEST(ArgParserTest, TestHelp)
{
	WCHAR* helps[] = {
		L"-h",
		L"blah",
		L"xyz",
	};

	WCHAR* help2[] = {
		L"abcd",
		L"efg",
		L"hij",
	};

	int argc = 3;

	ArgParser ap(helps, argc);
	ArgParser ap2(help2, argc);

	ASSERT_TRUE(ap.helpRequested());
	ASSERT_FALSE(ap2.helpRequested());
}

TEST(ArgParserTest, TestDelims)
{
	WCHAR* dels[] = {
		L"Asfd",
		L"-s",
		L"mfg",
		L"rlm",
		L"--herp",
		L"derp",
		L"-f",
	};

	int argc = 7;
	std::wstring thing1;
	std::wstring thing2;
	std::wstring thing3;

	ArgParser ap(dels, argc);
	ASSERT_TRUE(ap.getAttribute(std::wstring(L"-s"), thing1));
	ASSERT_EQ(ERROR_SUCCESS, thing1.compare(L"mfg"));

	ASSERT_TRUE(ap.getAttribute(std::wstring(L"--herp"), thing2));
	ASSERT_EQ(ERROR_SUCCESS, thing2.compare(L"derp"));
	
	ASSERT_FALSE(ap.getAttribute(std::wstring(L"rlm"), thing3));
	ASSERT_TRUE(thing3.empty());

	ASSERT_TRUE(ap.getAttribute(std::wstring(L"-f"), thing1));
	ASSERT_EQ(ERROR_SUCCESS, thing1.compare(L"enabled"));
}


TEST(ArgParserTest, TestStripDelims)
{
	WCHAR* stuff[] = {
		L"--h",
		L"/h",
		L"-h",
		L"h",
	};

	int scount = 4;

	ArgParser ap(stuff, scount);

	std::wstring tmp = stuff[0];
	std::wstring base = L"h";

	for (auto i = 0; i < scount; ++i) {
		tmp = stuff[i];
		ap.stripDelims(tmp);
		ASSERT_EQ(ERROR_SUCCESS, base.compare(tmp.c_str()));
	}

	ASSERT_TRUE(ap.getAttribute(base));

}
