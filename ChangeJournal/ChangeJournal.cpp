#include "ChangeJournal.hpp"

namespace {
	constexpr bool boolify(BOOL f) { return !!f; }
}

namespace ntfs {
	ChangeJournal::ChangeJournal(std::shared_ptr<void> vol) : vhandle(vol)
	{
	}

	void ChangeJournal::setCurrentVolume(std::shared_ptr<void> vh)
	{
		if (!vh)
			throw std::runtime_error("[ChangeJournal] Invalid volume handle provided!");
		vhandle.reset();
		vhandle = vh;
	}

	std::shared_ptr<void> ChangeJournal::getCurrentVolume()
	{
		return vhandle;
	}

	std::vector<uint8_t> ChangeJournal::getRecords(USN& next)
	{
		READ_USN_JOURNAL_DATA_V0	rData = { 0 };
		unsigned long				bytesRead = 0;
		std::vector<uint8_t>		vec;

		auto jInfo = getJournalData();

		vec.resize(default_buffer_size);
		rData.ReasonMask = (uint32_t)-1;
		rData.UsnJournalID = jInfo->UsnJournalID;
		rData.StartUsn = next;

		if (!DeviceIoControl(vhandle.get(), FSCTL_READ_USN_JOURNAL, &rData, sizeof(rData), vec.data(), vec.size(), &bytesRead, nullptr)) {

			unsigned long error = GetLastError();
			// Return an empty vector if the query failed because
			// no more records exist past the current point
			if (ERROR_NO_MORE_ITEMS == error) {
				if (sizeof(USN) <= bytesRead)
					next = *(reinterpret_cast<USN*>(vec.data()));
				else
					next = 0;

				vec.resize(0);
				return vec;
			}
			else {
				throw CG_API_INTERACTION_ERROR("An error occurred while reading the change journal!", error);
			}
		}
		// If we don't do this, we would need to pack along bytesRead somehow.
		// This could be changed for performance reasons (extra copies being bad and all)
		// down the road, but this is currently done for 1.) convenience, and 2.) due to this
		// probably not being our biggest bottleneck currently (since we have to go to disk for
		// more records).
		vec.resize(bytesRead);
		if(vec.size() >= sizeof(USN))
			next = *(reinterpret_cast<USN*>(vec.data()));

		return vec;
	}

	bool ChangeJournal::mapBuffer(std::vector<uint8_t>& buf, std::function<void(PUSN_RECORD)> func)
	{
		bool success = true;
		unsigned char* begin = buf.data();
		unsigned char* current = nullptr;
		size_t size = buf.size();
		
		if (size <= (sizeof(USN) + sizeof(USN_RECORD)))
			return false;

		current = (begin + sizeof(USN));

		while (current < (begin + size)) {
			func(reinterpret_cast<PUSN_RECORD>(current));
			current = (current + (reinterpret_cast<PUSN_RECORD>(current)->RecordLength));
		} 		
		
		return success;
	}

	void ChangeJournal::mapRecords(std::function<void(PUSN_RECORD)> func)
	{
		bool success = true;
		try {
			auto data = getJournalData();
			auto recordNum = data->FirstUsn;

			while (success) {
				auto vec = getRecords(recordNum);
				if (!vec.size())
					success = false;

				success = mapBuffer(vec, func);
			}
		}
		catch (const std::exception& e) {
			throw std::runtime_error((std::string("[ChangeJournal] An exception occurred! Failed to map change journal records! Caught: ") + e.what()));
		}

	}

	std::unique_ptr<USN_JOURNAL_DATA> ChangeJournal::getJournalData()
	{
		unsigned long	bytesRead = 0;
		auto			jData = std::make_unique<USN_JOURNAL_DATA>();

		if (!DeviceIoControl(vhandle.get(), FSCTL_QUERY_USN_JOURNAL, nullptr, 0, jData.get(), sizeof(USN_JOURNAL_DATA), &bytesRead, nullptr)) {
			throw CG_API_INTERACTION_LASTERROR("An error occurred while querying the journal data!");
		}

		return jData;
	}

	bool ChangeJournal::createUsnJournal(uint64_t maxSize, uint64_t allocationDelta)
	{
		CREATE_USN_JOURNAL_DATA		udata = { 0 };
		unsigned long				bytesRead = 0;
		bool						success = true;

		udata.AllocationDelta = allocationDelta;
		udata.MaximumSize = maxSize;

		if (!DeviceIoControl(vhandle.get(), FSCTL_CREATE_USN_JOURNAL, &udata, sizeof(udata), nullptr, 0, &bytesRead, nullptr)) {
			success = false;
		}

		return success;
	}

	bool ChangeJournal::deleteUsnJournal(uint64_t journalId)
	{
		DELETE_USN_JOURNAL_DATA delData = { 0 };
		unsigned long			bytesRead = 0;
		bool					success = true;

		delData.UsnJournalID = journalId;
		delData.DeleteFlags = USN_DELETE_FLAG_DELETE | USN_DELETE_FLAG_NOTIFY;

		if (!DeviceIoControl(vhandle.get(), FSCTL_DELETE_USN_JOURNAL, &delData, sizeof(delData), nullptr, 0, &bytesRead, nullptr)) {
			success = false;
		}

		return success;
	}

	bool ChangeJournal::resetJournal()
	{
		bool success = true;
		try {
			auto journalData = getJournalData();
			if (!(success = deleteUsnJournal(journalData->UsnJournalID))) {
				return success;
			}
			else {
				success = createUsnJournal(journalData->MaximumSize, journalData->AllocationDelta);
			}
		}
		catch (const std::exception& e) {
			throw std::runtime_error(std::string("[ChangeJournal] Failed to reset the change journal! Caught: ") + e.what());
		}

		return success;
	}

	std::string usn_stringify_to_json(PUSN_RECORD rec)
	{
		wchar_t buf[MAX_PATH + 1] = { 0 };
		std::stringstream ss;
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;

		if (nullptr == rec)
			return "";

		auto size = ((sizeof(wchar_t) * MAX_PATH) < USN_FIELD_BY_VERSION(rec, FileNameLength) ? sizeof(wchar_t) * MAX_PATH : USN_FIELD_BY_VERSION(rec, FileNameLength));

		// This may seem like an odd choice, but we find ourselves in the strange situation of having
		// a buffer of wchar_t's in the record that is not guarenteed to be NULL terminated, and a 
		// byte (rather than character) count, which causes bad behavior if std::wstring(buf, count) is used,
		// or if wstring tmp = buf is used. It's probably unlikely that FileNameLength will be bigger than 
		// MAX_PATH * sizeof(wchar_t), but in the case that it is, we truncate (since that's an exceptional case,
		// and our fixed-sized buffer is "MAX_PATH" in length).
		_snwprintf_s(buf, size, L"%s", USN_FIELD_BY_VERSION(rec, FileName));

		ss << "{ \"Filename\" : \"" << conv.to_bytes(buf) << "\", " << "\"MajorVersion\" : " << USN_FIELD_BY_VERSION(rec, MajorVersion) << ", \"Usn\" : "
			<< USN_FIELD_BY_VERSION(rec, Usn) << ", \"TimeStamp\" : " << USN_FIELD_BY_VERSION(rec, TimeStamp.QuadPart) << ", \"Reason\" : " << USN_FIELD_BY_VERSION(rec, Reason)
			<< ", \"SourceInfo\" : " << USN_FIELD_BY_VERSION(rec, SourceInfo) << ", \"SecurityId\" : " << USN_FIELD_BY_VERSION(rec, SecurityId) << ", \"FileAttributes\" : "
			<< USN_FIELD_BY_VERSION(rec, FileAttributes) << ", \"FileReferenceNumber\" : ";
		
		if (rec->MajorVersion == 2) {
			ss << reinterpret_cast<PUSN_RECORD_V2>(rec)->FileReferenceNumber << ", \"ParentFileReferenceNumber\" : "
				<< reinterpret_cast<PUSN_RECORD_V2>(rec)->ParentFileReferenceNumber;
		}
		else {
			auto tmp = reinterpret_cast<PUSN_RECORD_V3>(rec);
			ss << "\"" << bytes_to_string(std::begin(tmp->FileReferenceNumber.Identifier), std::end(tmp->FileReferenceNumber.Identifier)) << "\", "
			 << "\"ParentFileReferenceNumber\" : "
			 << "\"" << bytes_to_string(std::begin(tmp->ParentFileReferenceNumber.Identifier), std::end(tmp->ParentFileReferenceNumber.Identifier)) << "\"";
		}

		ss << " }";

		return ss.str();
	}
}