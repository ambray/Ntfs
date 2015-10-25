#pragma once

#include <Windows.h>
#include <memory>
#include <string>
#include <stdint.h>
#include <vector>
#include <tuple>
#include <codecvt>

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

		void setVolHandle(std::shared_ptr<void> vh);
		std::shared_ptr<void> getVolHandle();
		/// Volume name, filesys name, s/n, max comp. len
		std::tuple<std::string, std::string, unsigned long> getVolInfo();
		std::unique_ptr<NTFS_VOLUME_DATA_BUFFER> getVolData();
		unsigned long getDriveType();
		uint64_t getFileCount();
		std::vector<uint8_t> getMftRecord(uint64_t recNum);

	private:
		std::shared_ptr<void> vhandle;
	};
}