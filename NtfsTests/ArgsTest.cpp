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

TEST(ArgParserTest, GetAttribsTest)
{
	WCHAR* stuff[] = {
		L"-s",
		L"abcd",
		L"-t",
		L"/l",
	};

	int count = 4;

	ArgParser ap(stuff, count);

	const std::map<std::wstring, std::wstring>& ats = ap.getArgs();

	auto tmp = ats.find(std::wstring(L"s"));
	ASSERT_NE(tmp, ats.end());

	ASSERT_EQ(ERROR_SUCCESS, tmp->second.compare(std::wstring(L"abcd")));
	tmp = ats.find(std::wstring(L"t"));
	ASSERT_NE(tmp, ats.end());

	ASSERT_EQ(ERROR_SUCCESS, tmp->second.compare(std::wstring(L"enabled")));

}


TEST(ArgParserTest, AltGetAttribTest)
{
	std::wstring tmp;
	WCHAR* stuff[] = {
		L"-s",
		L"abcd",
		L"-t",
		L"/l",
	};

	int count = 4;

	ArgParser ap(stuff, count);

	ASSERT_FALSE(ap.getAttribute(NULL));
	ASSERT_TRUE(ap.getAttribute(L"s"));
	ASSERT_TRUE(ap.getAttribute(L"/l"));
	ASSERT_FALSE(ap.getAttribute(L"-/l"));
	ASSERT_FALSE(ap.getAttribute(NULL, tmp));
	ASSERT_TRUE(ap.getAttribute(L"-t", tmp));
	ASSERT_EQ(ERROR_SUCCESS, tmp.compare(L"enabled"));
	
}


TEST(ArgParserTest, TestArgParsing)
{
	ArgParser ap(NULL, 5);
	PWCHAR s = L"-This";


	ASSERT_FALSE(ap.getAttribute(NULL));
	ASSERT_FALSE(ap.getAttribute(L"-s"));
	ASSERT_EQ(ERROR_INVALID_PARAMETER, ap.parseArgs(&s, 0));
	ASSERT_EQ(ERROR_NO_MORE_ITEMS, ap.parseArgs(&s, 1));
}