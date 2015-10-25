#pragma once

#include <Windows.h>
namespace ntfs {
	enum class MftRecordNumber : LONGLONG {
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
	};

	enum class FileRecordFlags : USHORT {
		RecordInUse = 0x0001,
		RecordDirectory
	};

	enum class NtfsAttributeType {
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
	};

	struct NTFS_RECORD_HEADER {
		ULONG	Type;
		USHORT	UsaOffset;
		USHORT	UsaCount;
		USN		Usn;
	};

	struct NTFS_FILE_RECORD_HEADER {
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
	};

	struct NTFS_ATTRIBUTE {
		NtfsAttributeType AttributeType;
		ULONG			  Length;
		BOOLEAN			  NonResident;
		UCHAR			  NameLen;
		USHORT			  NameOffset;
		USHORT			  Flags; // 0x0001 -> Compressed
		USHORT			  AttributeNumber;
	};


	struct NTFS_RESIDENT_ATTRIBUTE {
		NTFS_ATTRIBUTE		Attribute;
		ULONG				ValueLength;
		USHORT				Offset;
		USHORT				Flags; // 1 -> indexed
	};

	struct NTFS_NONRESIDENT_ATTRIBUTE {
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
	};


	struct STANDARD_INFORMATION {
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
	};

	struct NTFS_ATTRIBUTE_LIST {
		NtfsAttributeType	AttributeType;
		USHORT				Length;
		UCHAR				NameLen;
		UCHAR				NameOffset;
		ULONGLONG			LowVcn;
		ULONGLONG			FileReferenceNumber;
		USHORT				AttributeNumber;
		USHORT				Reserved[3];
	};

	struct FILENAME_ATTRIBUTE {
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
	};

	struct OBJECTID_ATTRIBUTE {
		GUID				ObjectId;
		union {

			struct {
				GUID		OrigVolumeId;
				GUID		OrigObjectId;
				GUID		DomainId;
			};

			UCHAR			ExtendedInfo[48];
		};

	};

	struct VOLUME_INFORMATION {
		ULONG				Unknown[2];
		UCHAR				MajorVersion;
		UCHAR				MinorVersion;
		USHORT				Flags;
	};

	struct DIRECTORY_INDEX {
		ULONG				EntriesOffset;
		ULONG				IndexBlockLenght;
		ULONG				AllocSize;
		ULONG				Flags; // 0 -> small directory | 1 -> large directory
	};

	struct DIRECTORY_ENTRY {
		ULONGLONG			FileReferenceNumber;
		USHORT				Length;
		USHORT				AttributeLength;
		ULONG				Flags;
		/* possibly - FILENAME_ATTRIBUTE and ULONGLONG? */
	};

	struct INDEX_ROOT {
		NtfsAttributeType	AttributeType;
		ULONG				CollationRule;
		ULONG				BytesPerIndexBlock;
		DIRECTORY_INDEX		DirectoryIndex;
	};

	struct INDEX_BLOCK_HEADER {
		NTFS_RECORD_HEADER	RecordHeader;
		ULONGLONG			IndexBlockVcn;
		DIRECTORY_INDEX		DirectoryIndex;
	};


	struct REPARSE_POINT {
		ULONG				ReparseTag;
		USHORT				ReparseDataLen;
		USHORT				Reserved;
		UCHAR				ReparseData[1];
	};

	struct EA_INFORMATION {
		ULONG				EaLength;
		ULONG				EaQueryLength;
	};

	struct EA_ATTRIBUTE {
		ULONG				NextEntryOffset;
		UCHAR				Flags;
		UCHAR				EaNameLength;
		USHORT				EaValueLength;
		CHAR				EaName[1];
	};

	struct ATTRIBUTE_DEFINITION {
		WCHAR				AttributeName[64];
		ULONG				AttributeNumber;
		ULONG				Reserved[2];
		ULONG				Flags;
		ULONGLONG			MinSize;
		ULONGLONG			MaxSize;
	};

#pragma pack(push, 1)

	struct BOOT_BLOCK {
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
	};

#pragma pack(pop)
}