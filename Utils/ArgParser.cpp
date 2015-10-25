#include "ArgParser.h"

ArgParser::ArgParser(WCHAR** argv, int argc) : help(false)
{
	std::vector<std::wstring> vec;

	if (nullptr == argv || 0 == argc || nullptr == *argv)
		throw std::runtime_error("Invalid arguments provided on command line!");

	for (auto i = 0; i < argc; ++i)
		vec.push_back(argv[i]);

	parseArgs(vec);
}

ArgParser::ArgParser(CHAR** argv, int argc)
{
	std::vector<std::wstring> vec;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;

	if(nullptr == argv || 0 == argc || nullptr == *argv)
		throw std::runtime_error("Invalid arguments provided on command line!");

	for (auto i = 0; i < argc; ++i) {
		auto tmp = conv.from_bytes(argv[i]);
		vec.push_back(tmp);
	}

	parseArgs(vec);
}


ArgParser::~ArgParser()
{

}

bool ArgParser::helpRequested()
{
	return help;
}

/**
* Checks to see if the option provided is in the list of "help request" options.
*/
bool ArgParser::isHelpOpt(std::wstring& arg)
{
	bool isHelp = false;

	for (auto i = 0; NULL != helpOpts[i]; ++i) {
		if (!arg.compare(helpOpts[i])) {
			isHelp = help = true;
			break;
		}
	}

	return isHelp;
}

/**
* Processes the provided string, stripping away beginning delimiters identified
* in the regex string "delimExp".
*/
bool ArgParser::stripDelims(std::wstring& arg)
{
	bool fixed = false;
	std::wregex list(delimExp);
	std::wstring tmp;

	tmp = std::regex_replace(arg, list, L"");

	arg = tmp;

	fixed = true;
	return fixed;
}


/**
* Checks the wstring provided in arg to see if it conforms to 
* the command line argument syntax, and will store it into the
* optional storage pointer (normalized), if it is non-NULL. 
*/
bool ArgParser::hasDelim(std::wstring& arg, std::wstring* storage)
{
	bool delim = false;
	std::wstring tmp = delimExp;
	tmp += L".+";
	std::wregex delimE(tmp);

	if (std::regex_match(arg, delimE)) {
			
		if (NULL != storage) {
			stripDelims(arg);
			*storage = arg;
		}

		return true;
	}

	if (NULL != storage)
		*storage = arg;

	return delim;
}


/**
* Initializes the map containing all parsed, normalized action/argument pairs.
*/
int ArgParser::parseArgs(std::vector<std::wstring>& argv)
{
	int status = ERROR_SUCCESS;
	int i = 0;

	if (1 == argv.size())
		return ERROR_NO_MORE_ITEMS;

	for (auto tmp : argv) {
		std::wstring cont;

		if (isHelpOpt(tmp))
			continue;

		if (hasDelim(tmp)) {
			stripDelims(tmp);
			if ((i + 1) < argv.size() && !hasDelim(std::wstring(argv[i+1]), &cont)) {
				args.insert(std::pair<std::wstring, std::wstring>(tmp, cont));
				++i;
			}
			else {
				args.insert(std::pair<std::wstring, std::wstring>(tmp, std::wstring(L"enabled")));
			}
		}
		++i;
	}

	return status;
}

/**
* Underlying private method all of the getAttribute methods feed into.
* Returns a boolean indicating whether or not the argument is found, and optionally populates a wstring pointer
* with the result.
*/
bool ArgParser::locate(std::wstring& attrib, std::wstring* out)
{
	bool found = false;
	// All the entries are normalized prior to insertion,
	// need to strip the delimiters off prior to searching
	stripDelims(attrib);

	auto needle = args.find(attrib);

	if (needle != args.end()) {
		found = true;
		if (NULL != out)
			*out = needle->second;
	}


	return found;
}

const std::map<std::wstring, std::wstring>& ArgParser::getArgs()
{
	return args;
}

bool ArgParser::getAttribute(std::wstring attrib, std::wstring& out)
{

	return locate(attrib, &out);

}


bool ArgParser::getAttribute(std::wstring attrib)
{
	return locate(attrib);
}

bool ArgParser::getAttribute(std::string attrib, std::string& out)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
	std::wstring outVal = conv.from_bytes(out);
	auto inVal = conv.from_bytes(attrib);

	auto tmp = locate(inVal, &outVal);
	out = conv.to_bytes(outVal);

	return tmp;
}

bool ArgParser::getAttribute(std::string attrib)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
	
	auto inv = conv.from_bytes(attrib);
	return locate(inv);
}

