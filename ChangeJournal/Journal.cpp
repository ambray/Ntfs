#include "Journal.h"

Journal::Journal() : data(NULL), vol(INVALID_HANDLE_VALUE), error(0)
{}

Journal::Journal(std::wstring& vname) : data(NULL), vol(INVALID_HANDLE_VALUE), error(0)
{
	name = L"\\\\.\\" + vname;
	if (error = vopen())
		return;

	if (error = queryJournal())
		return;
}

Journal::~Journal()
{
	if (INVALID_HANDLE_VALUE != vol)
		CloseHandle(vol);

	if (NULL != data)
		HeapFree(GetProcessHeap(), 0, data);
}


/**
* Change the current handle, if possible. In this case we duplicate the provided handle,
* as we end up closing the current one in our destructor (which may cause problems if we're 
* sharing it with something else).
*/
int Journal::setHandle(HANDLE vh)
{
	int status = ERROR_SUCCESS;
	HANDLE tmp;

	if (INVALID_HANDLE_VALUE == vh || NULL == vh)
		return ERROR_INVALID_HANDLE;

	if (!DuplicateHandle(GetCurrentProcess(), vh, GetCurrentProcess(), &tmp, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
		error = status = GetLastError();
		return status;
	}

	vol = tmp;

	if ((status = queryJournal()))
		error = status;

	return status;
}


/**
* This will unpack and serialize each element of the USN_RECORD in a way that is
* consistent with its type.
*/
int Journal::marshallUsnRecord(IMarshaller* mr, PUSN_RECORD rec, std::wstring& out)
{
	int status = ERROR_SUCCESS;
	std::wstring tmp = L"{ ";
	LONGLONG fusn;
	DWORD storage;

	if (NULL == rec || NULL == mr)
		return ERROR_INVALID_PARAMETER;

	if (ERROR_SUCCESS != (status = mr->marshallULong(L"Version", rec->MajorVersion, tmp))) {
		return status;
	}

	tmp += L", ";

	if (2 == rec->MajorVersion) {
		if (ERROR_SUCCESS != (status = mr->marshallULongLong(L"FileReferenceNumber", ((PUSN_RECORD_V2)rec)->FileReferenceNumber, tmp))) {
			return status;
		}
		tmp += L", ";

		if (ERROR_SUCCESS != (status = mr->marshallULongLong(L"ParentFileReferenceNumber", ((PUSN_RECORD_V2)rec)->ParentFileReferenceNumber, tmp))) {
			return status;
		}
	
		tmp += L", ";
	}
	else if(3 == rec->MajorVersion || 4 == rec->MajorVersion) {
		if (ERROR_SUCCESS != (status = mr->marshallBytes(L"FileReferenceNumber", ((PUSN_RECORD_V3)rec)->FileReferenceNumber.Identifier, 16, tmp))) {
			return status;
		}

		tmp += L", ";
		if (ERROR_SUCCESS != (status = mr->marshallBytes(L"ParentFileReferenceNumber", ((PUSN_RECORD_V3)rec)->ParentFileReferenceNumber.Identifier, 16, tmp))) {
			return status;
		}
		tmp += L", ";
	}
	fusn = UNPACK_ELEMENT(rec, Usn);
	if (ERROR_SUCCESS != (status = mr->marshallLongLong(L"Usn", fusn, tmp))) {
		return status;
	}
	tmp += L", ";

	if (ERROR_SUCCESS != (status = mr->marshallULong(L"Reason", UNPACK_ELEMENT(rec, Reason), tmp))) {
		return status;
	}
	tmp += L", ";

	if (ERROR_SUCCESS != (status = mr->marshallULong(L"SourceInfo", UNPACK_ELEMENT(rec, SourceInfo), tmp))) {
		return status;
	}
	tmp += L", ";

	if (4 != rec->MajorVersion) {
		if (ERROR_SUCCESS != (status = mr->marshallULong(L"SecurityId", UNPACK_ELEMENT(rec, SecurityId), tmp))) {
			return status;
		}
		tmp += L", ";

		if (ERROR_SUCCESS != (status = mr->marshallULong(L"FileAttributes", UNPACK_ELEMENT(rec, FileAttributes), tmp))) {
			return status;
		}
		tmp += L", ";

		storage = UNPACK_ELEMENT(rec, FileNameLength);

		if (ERROR_SUCCESS != (status = mr->marshallWstr(L"FileName", UNPACK_ELEMENT(rec, FileName), storage, tmp))) {
			return status;
		}

	}

	tmp += L" }";

	out = tmp;

	return status;
}


/**
* Provides an alternative method to opening a handle to the volume,
* and setting the volume name.
*/
int Journal::setName(std::wstring& vn)
{
	int status = ERROR_SUCCESS;
	name = L"\\\\.\\" + vn;
	if (ERROR_SUCCESS != (status = vopen())) {
		data = NULL;
		return status;
	}

	return queryJournal();
}


/**
* Deletes the existing change journal, blocking until the operation is complete.
*/
int Journal::deleteJournal()
{
	int status = ERROR_SUCCESS;
	DWORD bytes;
	DELETE_USN_JOURNAL_DATA delJourn = { 0 };

	if (INVALID_HANDLE_VALUE == vol || NULL == data)
		return ERROR_INVALID_HANDLE;

	delJourn.UsnJournalID = data->UsnJournalID;
	delJourn.DeleteFlags = USN_DELETE_FLAG_DELETE | USN_DELETE_FLAG_NOTIFY;

	if (!DeviceIoControl(vol, FSCTL_DELETE_USN_JOURNAL, &delJourn, sizeof(delJourn), NULL, 0, &bytes, NULL)) {
		error = status = GetLastError();
		return status;
	}

	data = NULL;

	return status;
}

/**
* Creates a new change journal with the provided max size and allocation delta, then
* queries to refresh the tracked journal data.
*/
int Journal::createJournal(DWORDLONG& maxSize, DWORDLONG& allocDelta)
{
	int status = ERROR_SUCCESS;
	DWORD bytes;
	CREATE_USN_JOURNAL_DATA createData = { 0 };

	if (INVALID_HANDLE_VALUE == vol)
		return ERROR_INVALID_HANDLE;

	createData.AllocationDelta = allocDelta;
	createData.MaximumSize = maxSize;

	if (!DeviceIoControl(vol, FSCTL_CREATE_USN_JOURNAL, &createData, sizeof(createData), NULL, 0, &bytes, NULL)) {
		error = status = GetLastError();
		return status;
	}

	status = queryJournal();
	return status;
}

/**
* Does a combination of deleting and recreating the
* change journal, persisting the max size and allocation delta of the current change journal.
*/
int Journal::resetJournal()
{
	int status = ERROR_SUCCESS;
	DWORDLONG oldMax;
	DWORDLONG oldAlloc;

	if (INVALID_HANDLE_VALUE == vol || NULL == data)
		return ERROR_INVALID_HANDLE;

	oldMax = data->MaximumSize;
	oldAlloc = data->AllocationDelta;

	if (ERROR_SUCCESS != (status = deleteJournal())) {
		return status;
	}

	if (ERROR_SUCCESS != (createJournal(oldMax, oldAlloc))) {
		return status;
	}

	return status;
}


/**
* Invokes the provided function on each USN_RECORD in the set, passing an optional argument
* which is set to NULL if non is provided.
*/
int Journal::map(std::function<void(PUSN_RECORD, PVOID)>& func, PVOID optArg)
{
	int status = ERROR_SUCCESS;
	USN nextUsn = 0;
	PUSN_RECORD current = NULL;
	DWORD bytesToWalk;
	Buffer buf(QUERY_BUFFER_SIZE);

	const PBYTE buffer = buf.getBuffer();
	SIZE_T len = buf.size();

	if (NULL == buffer)
		return ERROR_OUTOFMEMORY;

	while (ERROR_SUCCESS == (status = getRecords(buf, nextUsn, &bytesToWalk))) {

		// get the next USN, which is prepended to the retrieved buffer
		nextUsn = *(USN*)buffer;
		// advance to the first actual record
		current = (PUSN_RECORD)((PBYTE)buffer + sizeof(USN));

		while ((PBYTE)current < (PBYTE)buffer + bytesToWalk) {
			try {
				func(current, optArg);
			}
			catch (...) {
				return GetLastError();
			}

			current = (PUSN_RECORD)((PBYTE)current + current->RecordLength);
		}
	}

	if (ERROR_NO_MORE_ITEMS == status)
		status = ERROR_SUCCESS;

	return status;
}


int Journal::getRecords(Buffer& buff, USN& next, PDWORD bytesRead)
{
	int status = ERROR_SUCCESS;
	DWORD bytes;
	READ_USN_JOURNAL_DATA_V0 readData = { 0 };

	const PBYTE pt = buff.getBuffer();
	SIZE_T len = buff.size();

	if (NULL == bytesRead || NULL == pt || 0 == len)
		return ERROR_INVALID_PARAMETER;

	if (INVALID_HANDLE_VALUE == vol || NULL == data)
		return ERROR_INVALID_HANDLE;


	// query all the things
	readData.ReasonMask = (DWORD)-1;
	readData.UsnJournalID = data->UsnJournalID;
	readData.StartUsn = next;

	if (!DeviceIoControl(vol, FSCTL_READ_USN_JOURNAL, &readData, sizeof(readData), pt, len, &bytes, NULL)) {
		status = GetLastError();
	}

	*bytesRead = bytes;
	if (bytes <= sizeof(USN))
		status = ERROR_NO_MORE_ITEMS;

	return status;
}

const HANDLE Journal::getHandle()
{
	return const_cast<const HANDLE>(vol);
}

const PUSN_JOURNAL_DATA Journal::getData()
{
	return const_cast<const PUSN_JOURNAL_DATA>(data);
}

int Journal::getError()
{
	return error;
}

int Journal::vopen()
{
	int status = ERROR_SUCCESS;
	DWORD shareMask = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
	DWORD accessMask = GENERIC_READ | GENERIC_WRITE;

	if (INVALID_HANDLE_VALUE == (vol = CreateFileW(name.c_str(), accessMask, shareMask, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_DEVICE, NULL))) {
		status = GetLastError();
	}

	return status;
}

int Journal::queryJournal()
{
	int status = ERROR_SUCCESS;
	DWORD bytesRet;

	if (INVALID_HANDLE_VALUE == vol)
		return ERROR_INVALID_HANDLE;

	if (NULL != data)
		HeapFree(GetProcessHeap(), 0, data);

	if (NULL == (data = (PUSN_JOURNAL_DATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(USN_JOURNAL_DATA)))) {
		return ERROR_OUTOFMEMORY;
	}

	if (!DeviceIoControl(vol, FSCTL_QUERY_USN_JOURNAL, NULL, 0, data, sizeof(USN_JOURNAL_DATA), &bytesRet, NULL)) {
		status = GetLastError();
	}

	return status;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Walk the given change journal buffer, recording the next record in the sequest in "next".
*/
int ChangeJournal::walkRecordBuffer(Buffer& buf, IJournal* journ, IMarshaller* marsh, std::vector<std::wstring>& output, USN* next, DWORD bytesToWalk)
{
	int status = ERROR_SUCCESS;
	std::wstring tmp;
	USN nextUsn = 0;
	PUSN_RECORD current;
	SIZE_T size = buf.size();
	const PBYTE buffer = buf.getBuffer();

	if (sizeof(USN) >= bytesToWalk)
		return ERROR_NO_MORE_ITEMS;

	if (NULL == buffer || bytesToWalk > size || NULL == next || NULL == marsh || NULL == journ)
		return ERROR_INVALID_PARAMETER;

	// get the next USN, which is prepended to the retrieved buffer
	nextUsn = *(USN*)buffer;
	// advance to the first actual record
	current = (PUSN_RECORD)((PBYTE)buffer + sizeof(USN));

	while ((PBYTE)current < (PBYTE)buffer + bytesToWalk) {
		// Marshall the current record, and append it to the list of current records.
		if (ERROR_SUCCESS != (status = journ->marshallUsnRecord(marsh, current, tmp))) {
			break;
		}

		output.push_back(tmp);
		current = (PUSN_RECORD)((PBYTE)current + current->RecordLength);
	}

	*next = nextUsn;
	return status;
}

/**
* Reads in a block of records from the current change journal, starting from the record indicated by "next".
* Please note that all reason codes are currently captured.
*/
int ChangeJournal::getRecords(IJournal* journal, Buffer& buf, USN& next, PDWORD bytesRead)
{
	int status = ERROR_SUCCESS;
	DWORD bytes;
	READ_USN_JOURNAL_DATA_V0 readData = { 0 };

	if (NULL == journal || NULL == bytesRead)
		return ERROR_INVALID_PARAMETER;

	const HANDLE vh = journal->getHandle();
	const PUSN_JOURNAL_DATA data = journal->getData();
	const PBYTE con = buf.getBuffer();
	SIZE_T len = buf.size();

	if (NULL == data || INVALID_HANDLE_VALUE == vh || NULL == con || 0 == len)
		return ERROR_INVALID_PARAMETER;

	// query all the things
	readData.ReasonMask = (DWORD)-1;
	readData.UsnJournalID = data->UsnJournalID;
	readData.StartUsn = next;

	if (!DeviceIoControl(vh, FSCTL_READ_USN_JOURNAL, &readData, sizeof(readData), con, len, &bytes, NULL)) {
		status = GetLastError();
	}

	*bytesRead = bytes;
	return status;
}

/**
* Reads the entire change journal, marshalling all of the records, and storing the resulting strings into the provided
* std::vector.
*/
int ChangeJournal::enumerateRecords(IJournal* journal, IMarshaller* marshaller, std::vector<std::wstring>& vec)
{
	int status = ERROR_SUCCESS;
	Buffer buf(QUERY_BUFFER_SIZE);
	DWORD bytes;
	USN next = 0;

	if (NULL == journal || NULL == marshaller)
		return ERROR_INVALID_PARAMETER;

	if (NULL == buf.getBuffer())
		return ERROR_OUTOFMEMORY;

	while (ERROR_SUCCESS == (status = getRecords(journal, buf, next, &bytes))) {
		if (ERROR_SUCCESS != walkRecordBuffer(buf, journal, marshaller, vec, &next, bytes))
			break;
	}

	if (ERROR_NO_MORE_ITEMS == status)
		status = ERROR_SUCCESS;

	return status;
}