#pragma once

#include <Windows.h>
#include <memory>
#include <vector>
#include <sstream>
#include <iomanip>
#include <functional>
#include <codecvt>
#include <stdint.h>
#include <iostream>
#include <string>

#define USN_FIELD_BY_VERSION(rec, field)\
	((rec->MajorVersion == 2) ? ((PUSN_RECORD_V2)rec)->field : ((PUSN_RECORD_V3)rec)->field)

#define CG_API_INTERACTION_ERROR(msg, err)\
	std::runtime_error(("[ChangeJournal] "  msg + std::to_string(__LINE__) + " " + std::to_string(err)))

#define CG_API_INTERACTION_LASTERROR(msg)\
	CG_API_INTERACTION_ERROR(msg, GetLastError())

namespace ntfs {

	constexpr uint32_t default_buffer_size = 8196;
	constexpr uint32_t vol_share_mask = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
	constexpr uint32_t vol_access_mask = GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE;

	class ChangeJournal {
	public:
		ChangeJournal(std::shared_ptr<void> vol);
		ChangeJournal() = default;
		~ChangeJournal() = default;
		ChangeJournal(const ChangeJournal&) = default;
		ChangeJournal(ChangeJournal&&) = default;
		ChangeJournal& operator=(const ChangeJournal&) = default;
		ChangeJournal& operator=(ChangeJournal&&) = default;

		void setCurrentVolume(std::shared_ptr<void> vh);
		std::shared_ptr<void> getCurrentVolume();

		std::vector<uint8_t> getRecords(USN& next);
		bool mapBuffer(std::vector<uint8_t>& buf, std::function<void(PUSN_RECORD)> func);

		// This continues from the first USN Change Journal Record
		bool mapRecords(std::function<void(PUSN_RECORD)> func);

		std::unique_ptr<USN_JOURNAL_DATA> getJournalData();
		bool createUsnJournal(uint64_t maxSize, uint64_t allocationDelta);
		bool deleteUsnJournal(uint64_t journalId);
		bool resetJournal();

	private:
		std::shared_ptr<void> vhandle;

	};

	template <typename T>
	std::string bytes_to_string(T start, T stop)
	{
		std::ostringstream oss;
		oss << std::hex << std::setfill('0');
		while (start != stop)
			oss << " 0x" << std::setw(2) << static_cast<int>(*start++);

		return oss.str();
	}


	std::string usn_stringify_to_json(PUSN_RECORD rec);

}