#pragma once

void OutputLog(const char* format, ...);

#ifndef DLOG
#if defined(DEBUG) || defined(_DEBUG)
#define DLOG(x, ...) OutputLog( x "\n", ##__VA__ARGS__);
#else
#define DLOG(x, ...)
#endif
#endif //DLOG

#ifndef ELOG
#define ELOG(x, ...) OutputLog("%s(%d): " x "\n", __FILE__, __LINE__, ##__VA_ARGS__);
#endif // ELOG
