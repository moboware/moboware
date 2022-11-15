#pragma once

#include "common/log_stream.h"
#include <filesystem>
#include <iomanip>
#include <ostream>
#include <thread>

std::ostream& GetStream();

inline std::ostream& logstream(const std::filesystem::path& fileName, const std::string& functionName, const std::size_t line)
{
  GetStream() << std::hex << std::this_thread::get_id() << "," << std::dec << fileName.filename() << "," << functionName << "," << line << ",";
  return GetStream();
}

#define LOG(strm) LOG_STREAM(LogStream::DEBUG, __FILE__, __FUNCTION__, __LINE__, strm);
//#define LOG(strm) logstream(__FILE__, __FUNCTION__, __LINE__) << strm << std::endl