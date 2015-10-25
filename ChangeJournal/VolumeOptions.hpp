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
		* 
		*
		*/
		std::shared_ptr<void> getVolHandle();
		/// Volume name, filesys name, s/n, max comp. len
		/**
		* 
		*
		*/
		std::tuple<std::string, std::string, unsigned long> getVolInfo();

		/**
		* 
		*
		*/
		std::unique_ptr<NTFS_VOLUME_DATA_BUFFER> getVolData();

		/**
		* 
		*
		*/
		unsigned long getDriveType();

		/**
		* 
		*
		*/
		uint64_t getFileCount();

		/**
		* 
		*
		*/
		std::vector<uint8_t> getMftRecord(uint64_t recNum);

		/**
		* 
		*
		*/
		std::vector<uint8_t> processMftAttributes(uint64_t recNum, std::function<void(PNTFS_ATTRIBUTE)> func);

		/**
		* 
		*
		*/
		void processMftAttributes(std::vector<uint8_t>& record, std::function<void(PNTFS_ATTRIBUTE)> func);

	private:
		std::shared_ptr<void> vhandle;
	};


}