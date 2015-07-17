#include "NtfsClasses.h"

MftRecord::MftRecord(PVOID) : rec(NULL)
{}

MftRecord::~MftRecord()
{}

const RecordType MftRecord::getType()
{
	return RecordType::MftRecord;
}


int MftRecord::parseRecord(PNTFS_FILE_RECORD_HEADER fr)
{
	int status = ERROR_SUCCESS;

	return status;
}
