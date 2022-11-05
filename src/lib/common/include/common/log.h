#pragma once

#include <ostream>
#include <filesystem>
#include <iomanip>
#include <thread>

std::ostream& GetStream();

inline std::ostream& logstream(const std::filesystem::path& fileName, const std::string& functionName, const std::size_t line)
{
  GetStream()
    << "0x" << std::hex << std::this_thread::get_id()
    << "," << std::dec << fileName.filename()
    << "," << functionName
    << "," << line
    << ",";
  return GetStream();
}

#define LOG(strm) logstream(__FILE__, __FUNCTION__, __LINE__) << strm << std::endl