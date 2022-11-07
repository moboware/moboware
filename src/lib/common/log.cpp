#include "common/log.h"

#include <iostream>

std::ostream& _logStream = std::cout;

std::ostream& GetStream()
{
  return _logStream;
}