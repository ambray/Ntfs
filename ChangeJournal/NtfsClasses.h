#pragma once

#include <Windows.h>
#include "..\Utils\Buffer.h"
#include "ntfs_defs.h"
#include <map>

typedef enum {
	UsnRecord,
	MftRecord,
} RecordType;

class IRecord {
public:
	IRecord() {};
	virtual ~IRecord() {};
	virtual const RecordType getType() = 0;
};

/**
*  Class needs to be able to store and retrieve at least
*  The following:
*
*	PNTFS_RECORD_HEADER			pRecordHead;
*	PNTFS_FILE_RECORD_HEADER	pFileRecord;
*	PSTANDARD_INFORMATION		pStdInfo;
*	PFILENAME_ATTRIBUTE			pFileName;
*	POBJECTID_ATTRIBUTE			pObjId;
*	PEA_INFORMATION				pEaInfo;
*	PEA_ATTRIBUTE				pEaAttr;
*/
class MftRecord : IRecord {
public:
	MftRecord(PVOID pr);
	virtual ~MftRecord();
	virtual const RecordType getType();
	int parseRecord(PNTFS_FILE_RECORD_HEADER fr);

private:
	Buffer* rec;
	std::map<NtfsAttributeType, SIZE_T> attribLoc;
};