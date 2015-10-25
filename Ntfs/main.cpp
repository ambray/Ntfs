#include <Windows.h>
#include "..\ChangeJournal\ChangeJournal.hpp"
#include "..\Utils\Buffer.h"
#include "..\Utils\Marshaller.h"
#include "..\Utils\ArgParser.h"
#include <vector>
#include <codecvt>
#include <sstream>
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


int queryChangeJournal(std::shared_ptr<void> volume, std::string& outfile)
{
	int status = ERROR_SUCCESS;
	try {
		ntfs::ChangeJournal journal(volume);
		journal.mapRecords([&](auto p) {
			DWORD bytes;
			auto tmp = ntfs::usn_stringify_to_json(p);
			std::cout << "Record: " << tmp << std::endl;
		});
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
		status = ERROR_FAIL_FAST_EXCEPTION;
	}


	return status;
}

int resetChangeJournal(std::shared_ptr<void> vol)
{
	int status = ERROR_SUCCESS;

	try {
		ntfs::ChangeJournal journal(vol);
		if (!journal.resetJournal()) {
			std::cout << "Unable to reset the journal!" << std::endl;
			return ERROR_FAIL_NOACTION_REBOOT;
		}
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
		status = ERROR_EXCEPTION_IN_RESOURCE_CALL;
	}
	
	if (ERROR_SUCCESS == status)
		std::cout << "The operation completed successfully." << std::endl;

	return status;
}

int deleteChangeJournal(std::shared_ptr<void> vol)
{
	int status = ERROR_SUCCESS;

	try {

	}
	catch (const std::exception& e) {

	}

	return status;
}

static VOID printHelp()
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

static DWORD getActions(ArgParser& ap)
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



int main(int argc, char** argv, char** envp)
{
	int status = ERROR_SUCCESS;
	std::string volume = "\\\\.\\C:";
	std::string outfile = "out.json";
	std::string outattr;
	std::string currentOp;
	DWORD actionMask = 0;

	ArgParser ap(argv, argc);

	if (ap.helpRequested()) {
		printHelp();
		return status;
	}

	if (ap.getAttribute("v", volume) || ap.getAttribute("volume", volume)) {
		std::wcout << L"[*] Volume change requested" << std::endl;
	}

	if (ap.getAttribute("o", outfile) || ap.getAttribute("output", outfile)) {
		std::wcout << L"[*] Output file change requested" << std::endl;
	}

	actionMask = getActions(ap);
	if (0 == actionMask) {
		printHelp();
		return status;
	}

	std::cout << "[*] Preparing to perform requested operations on Volume: " << volume << ", storing results in " << outfile << std::endl;
	auto vh = CreateFileA(volume.c_str(), ntfs::vol_access_mask, ntfs::vol_share_mask, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_DEVICE, nullptr);
	if (INVALID_HANDLE_VALUE == vh)
		return GetLastError();

	std::shared_ptr<void> vhandle(vh, CloseHandle);
	if (actionMask & ActionList::QueryJournal) {
		std::wcout << L"[*] Preparing to enumerate change journal...";
		if (ERROR_SUCCESS != (status = queryChangeJournal(vhandle, outfile))) {
			std::wcout << std::endl << L"[x] Failed to query the journal! Exited with status: " << status << std::endl;
			return status;
		}

		std::wcout << L" Done." << std::endl;
	}

	if (actionMask & ActionList::ResetJournal) {
		std::wcout << L"[*] Preparing to reset the change journal...";
		if (ERROR_SUCCESS != (status = resetChangeJournal(vhandle))) {
			std::wcout << std::endl << L"[x] Failed to reset the journal! Exited with status: " << status << std::endl;
			return status;
		}

		std::wcout << L" Done." << std::endl;
	}

	if (actionMask & ActionList::DeleteJournal) {
		std::wcout << L"[*] Preparing to delete the change journal...";
		if (ERROR_SUCCESS != (status = deleteChangeJournal(vhandle))) {
			std::wcout << std::endl << L"[x] Failed to delete the journal! Exited with status: " << status << std::endl;
			return status;
		}

		std::wcout << L" Done." << std::endl;
	}
	
	return status;
}