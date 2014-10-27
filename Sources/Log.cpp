//
// Copyright (c) 2014, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the University of Illinois/NCSA Open
// Source License found in the LICENSE file in the root directory of this
// source tree. An additional grant of patent rights can be found in the
// PATENTS file in the same directory.
//

#include "DebugServer2/Log.h"
#include "DebugServer2/Host/Platform.h"

#include <sstream>

#include <cstdarg>
#include <cstdio>

namespace {

uint64_t sLogMask;
int sLogLevel;
bool sColorsEnabled;
FILE *sOutputStream = stderr;
}

namespace ds2 {

void SetLogMask(uint64_t mask) { sLogMask = mask; }

void SetLogLevel(uint32_t level) { sLogLevel = level; }

void SetLogColorsEnabled(bool enabled) { sColorsEnabled = enabled; }

void SetLogOutputStream(FILE *stream) { sOutputStream = stream; }

void vLog(int category, int level, char const *classname, char const *funcname,
          char const *format, va_list ap) {
  if (sLogLevel > level)
    return;

  if ((sLogMask & (1 << category)) == 0)
    return;

  std::stringstream ss;

  // Double the size until the whole log line fits in the buffer.
  std::vector<char> buffer;
  buffer.resize(128);
  size_t written_bytes;
  do {
    buffer.resize(buffer.size() * 2);
    written_bytes = vsnprintf(buffer.data(), buffer.size(), format, ap);
  } while (written_bytes >= buffer.size());

  ss << '[' << Host::Platform::GetCurrentProcessId() << ']';

  if (classname != nullptr) {
    ss << '[' << classname << "::" << funcname << ']';
  } else {
    ss << '[' << funcname << ']';
  }

  char const *color = nullptr;
  char const *label = nullptr;
  switch (level) {
  case kLogLevelError:
    color = "\x1b[1;31m";
    label = "ERROR  ";
    break;
  case kLogLevelWarning:
    color = "\x1b[1;33m";
    label = "WARNING";
    break;
  case kLogLevelInfo:
    color = "\x1b[1;32m";
    label = "INFO   ";
    break;
  case kLogLevelDebug:
    color = "\x1b[1;36m";
    label = "DEBUG  ";
    break;
  default:
    break;
  }

  ss << ' ';

  if (color != nullptr && sColorsEnabled) {
    ss << color << label << "\x1b[m" << ':';
  } else if (label != nullptr) {
    ss << label << ':';
  }

  ss << ' ' << buffer.data() << std::endl;

  fputs(ss.str().c_str(), sOutputStream);
  fflush(sOutputStream);
}

void Log(int category, int level, char const *classname, char const *funcname,
         char const *format, ...) {
  va_list ap;

  va_start(ap, format);
  vLog(category, level, classname, funcname, format, ap);
  va_end(ap);
}
}