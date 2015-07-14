#include "ArgParser.h"

ArgParser::ArgParser(WCHAR** argv, int argc) : help(false)
{
	parseArgs(argv, argc);
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
int ArgParser::parseArgs(WCHAR** argv, int argc)
{
	int status = ERROR_SUCCESS;
	
	if (NULL == argv || 0 == argc)
		return ERROR_INVALID_PARAMETER;

	if (1 == argc)
		return ERROR_NO_MORE_ITEMS;

	for (auto i = 0; i < argc; ++i) {
		std::wstring tmp(argv[i]);
		std::wstring cont;

		if (isHelpOpt(tmp))
			continue;

		if (hasDelim(tmp)) {
			stripDelims(tmp);
			if ((i + 1) < argc && !hasDelim(std::wstring(argv[i+1]), &cont)) {
				args.insert(std::pair<std::wstring, std::wstring>(tmp, cont));
				++i;
			}
			else {
				args.insert(std::pair<std::wstring, std::wstring>(tmp, std::wstring(L"enabled")));
			}
		}

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

bool ArgParser::getAttribute(std::wstring& attrib, std::wstring& out)
{

	return locate(attrib, &out);

}

bool ArgParser::getAttribute(PWCHAR attrib, std::wstring& out)
{
	bool found = false;
	std::wstring cont;

	if (NULL == attrib)
		return found;

	cont = attrib;

	found = locate(cont, &out);

	return found;
}


bool ArgParser::getAttribute(std::wstring& attrib)
{
	return locate(attrib);
}

bool ArgParser::getAttribute(PWCHAR attrib)
{
	bool found = false;
	std::wstring tmp;

	if (NULL == attrib)
		return found;

	tmp = attrib;

	found = locate(tmp);

	return found;
}