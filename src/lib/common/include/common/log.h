#pragma once

#include <ostream>

std::ostream &GetStream();

#define LOG(strm) GetStream() << __FILE__ << "," << __FUNCTION__ << "," << __LINE__ << "," << strm << std::endl;