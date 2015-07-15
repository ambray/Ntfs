#pragma once

#include <Windows.h>

typedef enum _MftRecordNumber : LONGLONG {
	Mft = 0,
	MftMirr,
	MftLogFile,
	MftVolume,
	MftAttrDef,
	MftRootFileIndex,
	MftBitmap,
	MftBoot,
	MftBadClus,
	MftSecure,
	MftUpcase,
	MftExtend,
	MftQuota = 24,
	MftObjId,
	MftReparse
} MftRecordNumber, *PMftRecordNumber;

typedef enum _FileRecordFlags : USHORT {
	RecordInUse = 0x0001,
	RecordDirectory
} FileRecordFlags, *PFileRecordFlags;

typedef enum _NtfsAttributeType {
	AttributeStandardInformation = 0x10,
	AttributeAttributeList = 0x20,
	AttributeFileName = 0x30,
	AttributeObjectId = 0x40,
	AttributeSecurityDescriptor = 0x50,
	AttributeVolumeName = 0x60,
	AttributeVolumeInformation = 0x70,
	AttributeData = 0x80,
	AttributeIndexRoot = 0x90,
	AttributeIndexAllocation = 0xA0,
	AttributeBitmap = 0xB0,
	AttributeReparsePoint = 0xC0,
	AttributeEAInformation = 0xD0,
	AttributeEA = 0xE0,
	AttributePropertySet = 0xF0,
	AttributeLoggedUtilityStream = 0x100
} NtfsAttributeType, *PNtfsAttributeType;

typedef struct _NTFS_RECORD_HEADER {
	ULONG	Type;
	USHORT	UsaOffset;
	USHORT	UsaCount;
	USN		Usn;
} NTFS_RECORD_HEADER, *PNTFS_RECORD_HEADER;

typedef struct _NTFS_FILE_RECORD_HEADER {
	NTFS_RECORD_HEADER	RecordHeader;
	USHORT				SequenceCount;
	USHORT				LinkCount;
	USHORT				AttributeOffset;
	FileRecordFlags		Flags;
	ULONG				BytesInUse;
	ULONG				BytesAllocated;
	ULONGLONG			BaseFileRecord;
	USHORT				NextAttributeNumber;
	USHORT				Padding;
	ULONG				MftRecordNumber;
	USHORT				UpdateSequenceNumber;
} NTFS_FILE_RECORD_HEADER, *PNTFS_FILE_RECORD_HEADER;

typedef struct _NTFS_ATTRIBUTE {
	NtfsAttributeType AttributeType;
	ULONG			  Length;
	BOOLEAN			  NonResident;
	UCHAR			  NameLen;
	USHORT			  NameOffset;
	USHORT			  Flags; // 0x0001 -> Compressed
	USHORT			  AttributeNumber;
} NTFS_ATTRIBUTE, *PNTFS_ATTRIBUTE;


typedef struct _NTFS_RESIDENT_ATTRIBUTE {
	NTFS_ATTRIBUTE		Attribute;
	ULONG				ValueLength;
	USHORT				Offset;
	USHORT				Flags; // 1 -> indexed
} NTFS_RESIDENT_ATTRIBUTE, *PNTFS_RESIDENT_ATTRIBUTE;

typedef struct _NTFS_NONRESIDENT_ATTRIBUTE {
	NTFS_ATTRIBUTE		Attribute;
	ULONGLONG			LowVcn;
	ULONGLONG			HighVcn;
	USHORT				RunArrayOffset;
	UCHAR				CompressionUnit;
	UCHAR				Reserved[5];
	ULONGLONG			AllocSize;
	ULONGLONG			DataSize;
	ULONGLONG			InitializedSize;
	ULONGLONG			CompressedSize;
} NTFS_NONRESIDENT_ATTRIBUTE, *PNTFS_NONRESIDENT_ATTRIBUTE;


typedef struct _STANDARD_INFORMATION {
	ULONGLONG			CreationTime;
	ULONGLONG			ChangeTime;
	ULONGLONG			LastWriteTime;
	ULONGLONG			LastAccessTime;
	ULONG				FileAttributes;
	ULONG				Reserved[3];
	/* NTFS 3.0 only */
	ULONG				QuotaId;
	ULONGLONG			QuotaCharge;
	USN					Usn;
} STANDARD_INFORMATION, *PSTANDARD_INFORMATION;

typedef struct _NTFS_ATTRIBUTE_LIST {
	NtfsAttributeType	AttributeType;
	USHORT				Length;
	UCHAR				NameLen;
	UCHAR				NameOffset;
	ULONGLONG			LowVcn;
	ULONGLONG			FileReferenceNumber;
	USHORT				AttributeNumber;
	USHORT				Reserved[3];
} NTFS_ATTRIBUTE_LIST, *PNTFS_ATTRIBUTE_LIST;

typedef struct _FILENAME_ATTRIBUTE {
	ULONGLONG			DirectoryFileRefNumber;
	ULONGLONG			CreationTime;
	ULONGLONG			ChangeTime;
	ULONGLONG			LastWriteTime;
	ULONGLONG			LastAccessTime;
	ULONGLONG			AllocSize;
	ULONGLONG			DataSize;
	ULONG				FileAttributes;
	ULONG				Reserved;
	UCHAR				NameLen;
	UCHAR				NameType; // 0x01 -> Long, 0x02 -> short
	WCHAR				Name[1];
} FILENAME_ATTRIBUTE, *PFILENAME_ATTRIBUTE;

typedef struct _OBJECTID_ATTRIBUTE {
	GUID				ObjectId;
	union {

		struct {
			GUID		OrigVolumeId;
			GUID		OrigObjectId;
			GUID		DomainId;
		};

		UCHAR			ExtendedInfo[48];
	};

} OBJECTID_ATTRIBUTE, *POBJECTID_ATTRIBUTE;

typedef struct _VOLUME_INFORMATION {
	ULONG				Unknown[2];
	UCHAR				MajorVersion;
	UCHAR				MinorVersion;
	USHORT				Flags;
} VOLUME_INFORMATION, *PVOLUME_INFORMATION;

typedef struct _DIRECTORY_INDEX {
	ULONG				EntriesOffset;
	ULONG				IndexBlockLenght;
	ULONG				AllocSize;
	ULONG				Flags; // 0 -> small directory | 1 -> large directory
} DIRECTORY_INDEX, *PDIRECTORY_INDEX;

typedef struct _DIRECTORY_ENTRY {
	ULONGLONG			FileReferenceNumber;
	USHORT				Length;
	USHORT				AttributeLength;
	ULONG				Flags;
	/* possibly - FILENAME_ATTRIBUTE and ULONGLONG? */
} DIRECTORY_ENTRY, *PDIRECTORY_ENTRY;

typedef struct _INDEX_ROOT {
	NtfsAttributeType	AttributeType;
	ULONG				CollationRule;
	ULONG				BytesPerIndexBlock;
	DIRECTORY_INDEX		DirectoryIndex;
} INDEX_ROOT, *PINDEX_ROOT;

typedef struct _INDEX_BLOCK_HEADER {
	NTFS_RECORD_HEADER	RecordHeader;
	ULONGLONG			IndexBlockVcn;
	DIRECTORY_INDEX		DirectoryIndex;
} INDEX_BLOCK_HEADER, *PINDEX_BLOCK_HEADER;


typedef struct _REPARSE_POINT {
	ULONG				ReparseTag;
	USHORT				ReparseDataLen;
	USHORT				Reserved;
	UCHAR				ReparseData[1];
} REPARSE_POINT, *PREPARSE_POINT;

typedef struct _EA_INFORMATION {
	ULONG				EaLength;
	ULONG				EaQueryLength;
} EA_INFORMATION, *PEA_INFORMATION;

typedef struct _EA_ATTRIBUTE {
	ULONG				NextEntryOffset;
	UCHAR				Flags;
	UCHAR				EaNameLength;
	USHORT				EaValueLength;
	CHAR				EaName[1];
} EA_ATTRIBUTE, *PEA_ATTRIBUTE;

typedef struct _ATTRIBUTE_DEFINITION {
	WCHAR				AttributeName[64];
	ULONG				AttributeNumber;
	ULONG				Reserved[2];
	ULONG				Flags;
	ULONGLONG			MinSize;
	ULONGLONG			MaxSize;
} ATTRIBUTE_DEFINITION, *PATTRIBUTE_DEFINITION;

#pragma pack(push, 1)

typedef struct _BOOT_BLOCK {
	UCHAR				Jump[3];
	UCHAR				Format[8];
	USHORT				BytesPerSector;
	UCHAR				SectorsPerCluster;
	USHORT				BootSectors;
	UCHAR				Mbz1;
	USHORT				Mbz2;
	USHORT				Reserved1;
	UCHAR				MediaType;
	USHORT				Mbz3;
	USHORT				SectorsPerTrack;
	USHORT				NumberOfHeads;
	ULONG				PartitionOffset;
	ULONG				Reserved2[2];
	ULONGLONG			TotalSectors;
	ULONGLONG			MftStartLcn;
	ULONGLONG			Mft2StartLcn;
	ULONG				ClustersPerFileRecord;
	ULONG				ClustersPerIndexBlock;
	LARGE_INTEGER		VolumeSerialNo;
	UCHAR				Code[0x1AE];
	USHORT				BootSig;
} BOOT_BLOCK, *PBOOT_BLOCK;

#pragma pack(pop)