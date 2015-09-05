#include "Marshaller.h"

JsonMarshaller::JsonMarshaller()
{

}

JsonMarshaller::~JsonMarshaller()
{

}

/**
* Marshalls a single unsigned long, and appends it (along with its corresponding key) to the std::wstring
* provided via output.
* Returns either ERROR_SUCCESS, or ERROR_INVALID_PARAMETER on failure.
*/
int JsonMarshaller::marshallULong(PWCHAR key, ULONG in, std::wstring& output)
{
	WCHAR buf[MAX_PATH + 1] = { 0 };
	int status = ERROR_SUCCESS;
	PWCHAR fmt = L"\"%s\" : %lu";

	if (NULL == key || (4 + wcslen(key) + wcslen(fmt)) > MAX_PATH)
		return ERROR_INVALID_PARAMETER;

	_snwprintf(buf, MAX_PATH, fmt, key, in);

	output += buf;

	return status;
}


int JsonMarshaller::marshallLong(PWCHAR key, LONG in, std::wstring& output)
{
	WCHAR buf[MAX_PATH + 1] = { 0 };
	int status = ERROR_SUCCESS;
	PWCHAR fmt = L"\"%s\" : %ld";

	if (NULL == key || (4 + wcslen(key) + wcslen(fmt)) > MAX_PATH)
		return ERROR_INVALID_PARAMETER;

	_snwprintf(buf, MAX_PATH, fmt, key, in);

	output += buf;

	return status;
}


/**
* Marshalls a signed long long, along with the key provided, and appends all to the std::wstring.
* As above, returns either ERROR_SUCCESS, or ERROR_INVALID_PARAMETER.
*/
int JsonMarshaller::marshallULongLong(PWCHAR key, ULONGLONG& in, std::wstring& output)
{
	WCHAR buf[MAX_PATH + 1] = { 0 };
	int status = ERROR_SUCCESS;
	PWCHAR fmt = L"\"%s\" : \"%llu\"";

	if (NULL == key || (8 + wcslen(key) + wcslen(fmt)) > MAX_PATH)
		return ERROR_INVALID_PARAMETER;

	_snwprintf(buf, MAX_PATH, fmt, key, in);

	output += buf;

	return status;
}

int JsonMarshaller::marshallLongLong(PWCHAR key, LONGLONG& in, std::wstring& output)
{
	WCHAR buf[MAX_PATH + 1] = { 0 };
	int status = ERROR_SUCCESS;
	PWCHAR fmt = L"\"%s\" : \"%lld\"";

	if (NULL == key || (8 + wcslen(key) + wcslen(fmt)) > MAX_PATH)
		return ERROR_INVALID_PARAMETER;

	_snwprintf(buf, MAX_PATH, fmt, key, in);

	output += buf;
	return status;
}

/**
* Marshalls a counted byte string of length len.
*/
int JsonMarshaller::marshallBytes(PWCHAR key, PBYTE in, SIZE_T len, std::wstring& output)
{
	WCHAR buf[MAX_PATH + 1] = { 0 };
	int status = ERROR_SUCCESS;
	PWCHAR finish = L"S\"";
	std::wstring fmt = L"\"%s\" : \"%.";

	if (NULL == key || NULL == in || (12 + fmt.size() + wcslen(key) + wcslen(finish) + len) > MAX_PATH)
		return ERROR_INVALID_PARAMETER;

	// Need to generate the correct format string to
	// handle the byte buffer.
	_snwprintf(buf, 4, L"%d", len);
	fmt += buf;
	fmt += finish;

	memset(buf, 0, sizeof(WCHAR) * 4);
	_snwprintf(buf, MAX_PATH, fmt.c_str(), key, in);

	output += buf;

	return status;
}


/**
* Marshalls a counted wide character string, appending it to the std::wstring provided by output, with key 'key'.
* It is necessary in this case for it to be counted, as lots of places in Windows provide counted strings that aren't guarenteed to be
* NULL terminated, and thus are dangerous to feed directly into a std::wstring contstructor.
*/
int JsonMarshaller::marshallWstr(PWCHAR key, PWCHAR in, SIZE_T len, std::wstring& output)
{
	int status = ERROR_SUCCESS;
	WCHAR buf[MAX_PATH + 1] = { 0 };
	std::wstring fmt = L"\"%s\" : \"%.";
	PWCHAR fin = L"s\"";

	if (NULL == key || NULL == in || (10 + len + fmt.size()) > MAX_PATH)
		return ERROR_INVALID_PARAMETER;

	// Need to generate the rest of the format string,
	// Since we have to accomodate counted strings that may (potentially)
	// not be NULL terminated.
	_snwprintf(buf, 4, L"%d", len);
	fmt += buf;
	fmt += fin;
	
	memset(buf, 0, sizeof(WCHAR) * 4);
	_snwprintf(buf, MAX_PATH, fmt.c_str(), key, in);
	output += buf;

	return status;
}


/**
* Writes a std::deque of marshalled records out to file, truncating if "truncate" is set to true,
* appending otherwise.
*/
int JsonMarshaller::recordsToFile(std::wstring& outfile, std::deque<std::wstring>& recs, bool truncate)
{
	int status = ERROR_SUCCESS;
	std::wfstream f;
	std::wregex rx(L"\n");

	if (recs.empty() || outfile.empty())
		return ERROR_EMPTY;

	if (truncate)
		f.open(outfile, std::ios::out | std::ios::trunc);
	else
		f.open(outfile, std::ios::out | std::ios::app);

	if (!f.is_open())
		return GetLastError();

	f << L"{ \"Records\" : [ ";

	auto endRec = recs.end();
	--endRec;

	for (auto i : recs) {
		f << i;

		if (i.compare(endRec->c_str()))
			f << L", ";
	}

	f << L" ] }";

	f.close();
	return status;
}

/**
* Indicates that this is a JSON marshaller.
*/
const DataFormat JsonMarshaller::marshallerType()
{
	return JsonData;
}