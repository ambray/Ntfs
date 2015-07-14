#pragma once

#include <Windows.h>
#include <string>
#include <iostream>
#include <regex>
#include <map>
#include <vector>


// Contains all valid help request strings.
static WCHAR* helpOpts[] = {
	L"-h",
	L"--help",
	L"/?",
	NULL,
};

// Regex containing valid prefixes for command line arguments.
static WCHAR* delimExp = L"^(--|-|/)";

class ArgParser {
public:
	ArgParser(WCHAR** argv, int argc);
	~ArgParser();
	bool helpRequested();
	
	bool getAttribute(std::wstring& attrib, std::wstring& out);
	bool getAttribute(PWSTR attrib, std::wstring& out);
	bool getAttribute(std::wstring& attrib);
	bool getAttribute(PWSTR attrib);


	bool stripDelims(std::wstring& arg);
	int parseArgs(WCHAR** argv, int argc);
	const std::map<std::wstring, std::wstring>& getArgs();

private:
	std::map<std::wstring, std::wstring> args;
	bool help;

	bool hasDelim(std::wstring& arg, std::wstring* storage=NULL);
	bool isHelpOpt(std::wstring& arg);
	bool locate(std::wstring& needle, std::wstring* out = NULL);
};