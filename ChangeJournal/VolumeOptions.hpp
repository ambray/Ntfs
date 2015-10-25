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
#include <string>
#include <stdint.h>
#include <vector>
#include <tuple>
#include <codecvt>
#include <functional>
#include "ntfs_defs.h"

#define EXTRACT_ATTRIBUTE(base, type)\
	((base->NonResident) ? (type*)((unsigned char*)base + ((ntfs::NTFS_NONRESIDENT_ATTRIBUTE*)base)->RunArrayOffset) :\
						   (type*)((unsigned char*)base + ((ntfs::NTFS_RESIDENT_ATTRIBUTE*)base)->Offset))

#define VOL_API_INTERACTION_ERROR(msg, err)\
	std::runtime_error(("[VolOps] "  msg + std::to_string(__LINE__) + " " + std::to_string(err)))

#define VOL_API_INTERACTION_LASTERROR(msg)\
	VOL_API_INTERACTION_ERROR(msg, GetLastError())

namespace ntfs {

	constexpr uint32_t vol_data_size = sizeof(NTFS_VOLUME_DATA_BUFFER) + sizeof(NTFS_EXTENDED_VOLUME_DATA);

	class VolOps {
	public:
		VolOps(std::shared_ptr<void> volHandle);
		VolOps() = default;
		~VolOps() = default;
		VolOps(const VolOps&) = default;
		VolOps(VolOps&&) = default;
		VolOps& operator=(const VolOps&) = default;
		VolOps& operator=(VolOps&&) = default;

		/**
		* Setter method for the HANDLE the class is currently operating on
		*
		* @param a shared_ptr containing the HANDLE (e.g., std::shared_ptr<void>(HANDLE, CloseHandle))
		*/
		void setVolHandle(std::shared_ptr<void> vh);

		/**
		* Gets a shared pointer containing the HANDLE the class instance is currently operating on
		*
		* @return a shared_ptr containing the HANDLE (e.g., std::shared_ptr<void>(HANDLE, CloseHandle))
		*/
		std::shared_ptr<void> getVolHandle();

		/**
		* Returns useful information about the current volume.
		*
		* @throws std::runtime_error if the operation fails fatally (e.g., bad handle)
		* @return std::tuple containing (in order): the volume name, the filesystem name, 
		*         and the max component length.
		*/
		std::tuple<std::string, std::string, unsigned long> getVolInfo();

		/**
		* Gets the volume data for the current volume.
		*
		* @throws std::runtime_error if the operation fails to complete
		* @return a unique_ptr containing the NTFS_VOLUME_DATA_BUFFER, and is vol_data_size bytes in size.
		*/
		std::unique_ptr<NTFS_VOLUME_DATA_BUFFER> getVolData();

		/**
		* Returns the drive type of the current volume (See: MSDN documentation for GetDriveType())
		* 
		* @throws std::runtime_error if getVolInfo fails
		* @return unsigned long indicating the drive type
		*/
		unsigned long getDriveType();

		/**
		* Returns the file count on the current volume.
		*
		* @throws std::runtime_error if the operation is unable to complete.
		* @return a uint64_t containing the total number of files on the volume.
		*/
		uint64_t getFileCount();

		/**
		* Gets a Master File Table record given its number
		*
		* @throws std::runtime_error if the operation is unable to complete
		* @param recNum The file being requested
		* @return A vector containing the MFT record.
		*/
		std::vector<uint8_t> getMftRecord(uint64_t recNum);

		/**
		* Retrieves an MFT record by number, maps callable func across all of its attributes, and returns the
		* retrieved record back in a std::vector.
		*
		* @param recNum The record to retrieve
		* @param func The function which will be provided each attribute
		* @return the retrieved MFT record, after processing is complete (including changes made during by func)
		*/
		std::vector<uint8_t> processMftAttributes(uint64_t recNum, std::function<void(NTFS_ATTRIBUTE*)> func);

		/**
		* Maps func across the attributes contained within record.
		*
		* @param record A retrieved MFT record, stored within a vector.
		* @param func The callable that will be mapped against all attributes contained in record.
		* @return None
		*/
		void processMftAttributes(std::vector<uint8_t>& record, std::function<void(NTFS_ATTRIBUTE*)> func);

	private:
		std::shared_ptr<void> vhandle;
	};


}