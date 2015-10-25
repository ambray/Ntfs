#pragma once
/********************************************************************************
* The MIT License (MIT)
*
* Copyright (c) 2015, Aaron M. Bray, aaron.m.bray@gmail.com

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*********************************************************************************/

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

/// Helper macro to obtain a field from the correct offset of a given PUSN_RECORD.
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

		/**
		* Setter method for the HANDLE the class is currently operating on
		*
		* @throws std::runtime_error if a bad pointer is provided.
		* @param a shared_ptr containing the HANDLE (e.g., std::shared_ptr<void>(HANDLE, CloseHandle))
		*/
		void setCurrentVolume(std::shared_ptr<void> vh);

		/**
		* Gets a shared pointer containing the HANDLE the class instance is currently operating on
		*
		* @return a shared_ptr containing the HANDLE (e.g., std::shared_ptr<void>(HANDLE, CloseHandle))
		*/
		std::shared_ptr<void> getCurrentVolume();

		/**
		* Returns a vector of default_buffer_size or less, containing from "next" onward. The value in next is
		* replaced with the next USN value returned.
		*
		* @throws std::runtime_error if operation fails fatally (e.g., volume handle is bad)
		* @param next As input, this contains the next update sequence number (USN) to start with; value will be replaced
		*        upon successful completion with the next USN past the end of the buffer returned.
		* @return A vector of default_buffer_size or less containing the records obtained. A vector of size 0 indicates
		*         that no more items were available in the change journal.
		*/
		std::vector<uint8_t> getRecords(USN& next);

		/**
		* Walks the buffer of USN_RECORDs contained in vector buf, and applies callable func to each or them.
		*
		* @param buf Vector containing a buffer of USN_RECORDs
		* @param func A std::function that will be called with a pointer to each record in the buffer.
		* @return Will return true unless the buffer is <= sizeof(USN) + sizeof(USN_RECORD); used to indicate
		*         termination of operation.
		*/
		bool mapBuffer(std::vector<uint8_t>& buf, std::function<void(PUSN_RECORD)> func);

		/**
		* Walks the change journal, starting from the first record, and maps func over all records.
		*
		* @throws std::runtime_error if an exception occurs during processing.
		* @param func a std::function that will be called with a pointer to each record in the change journal
		*/
		void mapRecords(std::function<void(PUSN_RECORD)> func);

		/**
		* Retrieve the current USN Change Journal's data.
		*
		* @throws std::runtime_error if the operation fails to complete successfully, or std::bad_alloc if the unique_ptr
		*		  allocation fails.
		* @return a unique_ptr containing the current change journal's USN_JOURNAL_DATA
		*/
		std::unique_ptr<USN_JOURNAL_DATA> getJournalData();

		/**
		* Attempts to create a new USN Change Journal
		*
		* @param maxSize Indicates the max size of the new change journal to be created.
		* @param allocationDelta Indicates the allocation delta of the new change journal to be created.
		* @return will return true if the operation succeeds, otherwise false will be returned.
		*/
		bool createUsnJournal(uint64_t maxSize, uint64_t allocationDelta);

		/**
		* Attempts to delete the current USN change journal, blocking until the operation completes.
		*
		* @param journalId Indicates the journalId to delete.
		* @return will return true if the operation completes successfully, otherwise false.
		*/
		bool deleteUsnJournal(uint64_t journalId);

		/**
		* Attempts to delete the current active change journal, and create a new one (preserving attributes)
		*
		* @throws std::runtime_error if an exception occurs during processing.
		* @return true if the operation completes successfully, false if deletion/recreation fails.
		*/
		bool resetJournal();

	private:
		std::shared_ptr<void> vhandle;

	};

	/**
	* Converts a congiuous range of bytes to a hex string
	*
	* @param start This should be a pointer to the beginning of the range
	* @param stop This should be a pointer to the end of the range of operation
	* @return a std::string containing the resulting hex string will be returned.
	*/
	template <typename T>
	std::string bytes_to_string(T start, T stop)
	{
		std::ostringstream oss;
		oss << std::hex << std::setfill('0');
		while (start != stop)
			oss << " 0x" << std::setw(2) << static_cast<int>(*start++);

		return oss.str();
	}

	
	/**
	* Will generate a JSON string out of the provided USN_RECORD.
	*
	* @param rec A pointer to the USN_RECORD to serialize.
	* @return a std::string containing the serialized record, or an empty string if a NULL value was provided.
	*/
	std::string usn_stringify_to_json(PUSN_RECORD rec);

}