#include "VolumeOptions.hpp"

ntfs::VolOps::VolOps(std::shared_ptr<void> volHandle) : vhandle(volHandle)
{
}

void ntfs::VolOps::setVolHandle(std::shared_ptr<void> vh)
{
	if(vhandle)
		vhandle.reset();

	vhandle = vh;
}

std::shared_ptr<void> ntfs::VolOps::getVolHandle()
{
	return vhandle;
}


std::tuple<std::string, std::string, unsigned long> ntfs::VolOps::getVolInfo()
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
	wchar_t			volNameBuf[MAX_PATH + 1] = { 0 };
	wchar_t			fsNameBuf[MAX_PATH + 1] = { 0 };
	std::string		volName;
	std::string		fsName;
	unsigned long	maxCompLen = 0;
	unsigned long	serial = 0;
	unsigned long	fsFlags = 0;

	if (!GetVolumeInformationByHandleW(vhandle.get(), volNameBuf, MAX_PATH, &serial, &maxCompLen, &fsFlags, fsNameBuf, MAX_PATH)) {
		throw VOL_API_INTERACTION_LASTERROR("Unable to query volume information!");
	}
	volName = conv.to_bytes(volNameBuf);
	fsName = conv.to_bytes(fsNameBuf);

	return std::make_tuple(volName, fsName, maxCompLen);
}

std::unique_ptr<NTFS_VOLUME_DATA_BUFFER> ntfs::VolOps::getVolData()
{
	unsigned long bytesRead = 0;
	auto tmp = std::unique_ptr<NTFS_VOLUME_DATA_BUFFER>(reinterpret_cast<PNTFS_VOLUME_DATA_BUFFER>(new unsigned char [vol_data_size]));
	
	if (!DeviceIoControl(vhandle.get(), FSCTL_GET_NTFS_VOLUME_DATA, nullptr, 0, tmp.get(), vol_data_size, &bytesRead, nullptr)) {
		throw VOL_API_INTERACTION_LASTERROR("Failed to get volume data!");
	}

	return tmp;
}

unsigned long ntfs::VolOps::getDriveType()
{
	std::string volname;

	std::tie(volname,
		std::ignore,
		std::ignore) = getVolInfo();

	return GetDriveTypeA(volname.c_str());
}

uint64_t ntfs::VolOps::getFileCount()
{
	uint64_t bytesPerSeg = 0;
	auto data = getVolData();
	if (!data)
		throw VOL_API_INTERACTION_ERROR("Bad data pointer returned!", ERROR_INVALID_PARAMETER);

	bytesPerSeg = data->BytesPerFileRecordSegment;

	if (0 == bytesPerSeg)
		throw VOL_API_INTERACTION_ERROR("Bad data returned... bytes per segment is 0!", ERROR_BUFFER_ALL_ZEROS);

	return uint64_t(data->MftValidDataLength.QuadPart / bytesPerSeg);
}

std::vector<uint8_t> ntfs::VolOps::getMftRecord(uint64_t recNum)
{
	NTFS_FILE_RECORD_INPUT_BUFFER	inBuf = { 0 };
	std::vector<uint8_t>			vec;
	size_t							recSize = sizeof(NTFS_FILE_RECORD_OUTPUT_BUFFER);
	unsigned long					bytesReturned = 0;

	auto data = getVolData();
	if (!data)
		throw VOL_API_INTERACTION_ERROR("Bad data pointer returned when querying MFT record!", ERROR_INVALID_PARAMETER);

	recSize += data->BytesPerFileRecordSegment;
	auto tmpbuf = std::unique_ptr<NTFS_FILE_RECORD_OUTPUT_BUFFER>(reinterpret_cast<PNTFS_FILE_RECORD_OUTPUT_BUFFER>(new unsigned char[recSize]));

	inBuf.FileReferenceNumber.QuadPart = recNum;
	if (!DeviceIoControl(vhandle.get(), FSCTL_GET_NTFS_FILE_RECORD, &inBuf, sizeof(inBuf), tmpbuf.get(), recSize, &bytesReturned, nullptr)) {
		throw VOL_API_INTERACTION_LASTERROR("Unable to retrieve file record!");
	}

	vec.resize(tmpbuf->FileRecordLength);
	memcpy(vec.data(), tmpbuf->FileRecordBuffer, tmpbuf->FileRecordLength);

	return vec;
}

std::vector<uint8_t> ntfs::VolOps::processMftAttributes(uint64_t recNum, std::function<void(NTFS_ATTRIBUTE*)> func)
{
	std::vector<uint8_t> vec;

	try {
		vec = getMftRecord(recNum);
		processMftAttributes(vec, func);
	}
	catch (const std::exception& e) {
		throw std::runtime_error(std::string("[VolOps] An exception occurred while processing the requested MFT record! Error: ") + e.what());
	}

	return vec;
}

void ntfs::VolOps::processMftAttributes(std::vector<uint8_t>& record, std::function<void(NTFS_ATTRIBUTE*)> func)
{
	NTFS_FILE_RECORD_HEADER*		header = reinterpret_cast<NTFS_FILE_RECORD_HEADER*>(record.data());
	NTFS_ATTRIBUTE*					current = nullptr;
	unsigned long					frecSize = 0;

	if (!record.size() || !func)
		throw VOL_API_INTERACTION_ERROR("Unable to process MFT record! Bad parameters provided.", ERROR_INVALID_PARAMETER);

	for (current = (NTFS_ATTRIBUTE*)((unsigned char*)header + header->AttributeOffset);
		current->AttributeType != NtfsAttributeType::AttributeEndOfRecord;
		current = (NTFS_ATTRIBUTE*)((unsigned char*)current + current->Length)) 
	{
		func(current);
	}

}
