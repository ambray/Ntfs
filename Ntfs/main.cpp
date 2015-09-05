#include <Windows.h>
#include "..\ChangeJournal\Journal.h"
#include "..\Utils\Buffer.h"
#include "..\Utils\Marshaller.h"
#include "..\Utils\ArgParser.h"
#include <vector>
#include <deque>
#include <string>
#include <iostream>

typedef enum {
	QueryJournal = 1,
	DeleteJournal,
	ResetJournal = 4,

} ActionList;

static WCHAR* argDescriptions[] = {
	L"Sets the current volume to operate on; default is C:",
	L"Specifies the file to ouput results into. Default\n\t\t is out.json",
	L"Queries the current change journal, dumping all records.",
	L"Deletes the current change journal.",
	L"Resets the change journal.",
	NULL,
};

static WCHAR* supportedArgs[] = {
	L"-v",
	L"/v",
	L"--volume",
	L"-o",
	L"--output",
	L"/o",
	L"-q",
	L"/q",
	L"--query",
	L"-d",
	L"/d",
	L"--delete",
	L"-r",
	L"/r",
	L"--reset",
	NULL,
};


int queryChangeJournal(std::wstring& volume, std::wstring& outfile)
{
	int status = ERROR_SUCCESS;
	Journal journ(volume);
	JsonMarshaller jmarsh;
	std::deque<std::wstring> vec;

	if (ERROR_SUCCESS != (status = ChangeJournal::enumerateRecords((IJournal*)&journ, (IMarshaller*)&jmarsh, vec))) {
		std::wcout << std::endl << L"[x] Failed to open change journal!";
		return status;
	}

	status = jmarsh.recordsToFile(outfile, vec);

	return status;
}

int resetChangeJournal(std::wstring& volume)
{
	int status = ERROR_SUCCESS;
	Journal journ(volume);

	status = journ.resetJournal();

	return status;
}

int deleteChangeJournal(std::wstring& volume)
{
	int status = ERROR_SUCCESS;
	Journal journ(volume);

	status = journ.deleteJournal();

	return status;
}

static inline VOID printHelp()
{
	DWORD msgIndex = 0;

	std::wcout << L"[*] The following USN Change Journal Operations are available: " << std::endl;

	for (auto i = 0; NULL != supportedArgs[i]; ++i) {
		// There are three separate entries for each option
		if (3 > i)
			msgIndex = 0;
		else
			msgIndex = i / 3;

		std::wcout << L"\t -> " << supportedArgs[i] << L" : " << ((NULL != argDescriptions[msgIndex]) ? argDescriptions[msgIndex] : L"") << std::endl;
	}
}

static inline DWORD getActions(ArgParser& ap)
{
	DWORD tmp = 0;

	if (ap.getAttribute(L"q") || ap.getAttribute(L"query"))
		tmp |= ActionList::QueryJournal;

	if (ap.getAttribute(L"r") || ap.getAttribute(L"reset"))
		tmp |= ActionList::ResetJournal;

	if (ap.getAttribute(L"d") || ap.getAttribute(L"delete"))
		tmp |= ActionList::DeleteJournal;

	return tmp;
}



int wmain(int argc, WCHAR** argv, WCHAR** envp)
{
	int status = ERROR_SUCCESS;
	std::wstring volume = L"C:";
	std::wstring outfile = L"out.json";
	std::wstring outattr;
	std::wstring currentOp;
	DWORD actionMask = 0;
	ArgParser ap(argv, argc);

	if (ap.helpRequested()) {
		printHelp();
		return status;
	}

	if (ap.getAttribute(L"v", volume) || ap.getAttribute(L"volume", volume)) {
		std::wcout << L"[*] Volume change requested" << std::endl;
	}

	if (ap.getAttribute(L"o", outfile) || ap.getAttribute(L"output", outfile)) {
		std::wcout << L"[*] Output file change requested" << std::endl;
	}

	actionMask = getActions(ap);
	if (0 == actionMask) {
		printHelp();
		return status;
	}

	std::wcout << L"[*] Preparing to perform requested operations on Volume: " << volume << L", storing results in " << outfile << std::endl;

	if (actionMask & ActionList::QueryJournal) {
		std::wcout << L"[*] Preparing to enumerate change journal...";
		if (ERROR_SUCCESS != (status = queryChangeJournal(volume, outfile))) {
			std::wcout << std::endl << L"[x] Failed to query the journal! Exited with status: " << status << std::endl;
			return status;
		}

		std::wcout << L" Done." << std::endl;
	}

	if (actionMask & ActionList::ResetJournal) {
		std::wcout << L"[*] Preparing to reset the change journal...";
		if (ERROR_SUCCESS != (status = resetChangeJournal(volume))) {
			std::wcout << std::endl << L"[x] Failed to reset the journal! Exited with status: " << status << std::endl;
			return status;
		}

		std::wcout << L" Done." << std::endl;
	}

	if (actionMask & ActionList::DeleteJournal) {
		std::wcout << L"[*] Preparing to delete the change journal...";
		if (ERROR_SUCCESS != (status = deleteChangeJournal(volume))) {
			std::wcout << std::endl << L"[x] Failed to delete the journal! Exited with status: " << status << std::endl;
			return status;
		}

		std::wcout << L" Done." << std::endl;
	}
	
	return status;
}