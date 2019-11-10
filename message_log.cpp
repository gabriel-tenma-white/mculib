#include <mculib/message_log.hpp>
#include <mculib/printf.hpp>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#ifndef MESSAGELOG_SIZE
#define MESSAGELOG_SIZE 1024
#endif

using namespace std;

namespace mculib {
	MessageLog::MessageLog(char* buffer, uint32_t bufferSizeBytes)
		:buffer(buffer), bufferSizeBytes(bufferSizeBytes), _wpos(0) {
		bufferSizeMask = bufferSizeBytes - 1;
		if((bufferSizeMask + bufferSizeBytes) != (bufferSizeMask | bufferSizeBytes)) {
			// bufferSizeBytes is not a power of 2
			abort();
		}
		memset(buffer, 0, bufferSizeBytes);
	}
	void MessageLog::append(string_view message, bool newLine) {
		if(message.length() * 2 > bufferSizeBytes)
			return;
		
		uint32_t len = newLine ? (message.length() + 1) : message.length();
		
		// move the write pointer forward
		uint32_t wpos, new_wpos;
	retry:
		wpos = _wpos;
		new_wpos = wpos + len;

		// ensure no other thread will try to write to the same position
		if(!__sync_bool_compare_and_swap(&_wpos, wpos, new_wpos))
			goto retry;

		// copy message to buffer
		wpos &= bufferSizeMask;

		uint32_t part1Len = bufferSizeBytes - wpos;
		if(part1Len > message.length())
			part1Len = message.length();
		if(part1Len > 0)
			memcpy(buffer + wpos, message.data(), part1Len);

		uint32_t part2Len = message.length() - part1Len;
		if(part2Len > 0)
			memcpy(buffer, message.data() + part1Len, part2Len);

		if(newLine)
			buffer[(new_wpos - 1) & bufferSizeMask] = '\n';
	}
	
	void MessageLog::printf(const char* fmt, ...) {
		char buf[128];
		va_list ap;
		va_start(ap, fmt);
		int len = chvsnprintf(buf, sizeof(buf), fmt, ap);
		va_end(ap);
		if(len > sizeof(buf)) {
			len = sizeof(buf);
			buf[sizeof(buf) - 1] = 0;
		}
		if(len <= 0)
			return;
		append({buf, (size_t) len});
	}

	void MessageLog::rotate() {
		uint32_t rotation = (_wpos + 1) & bufferSizeMask;
		if(rotation == 0) return;
		std::rotate(buffer, buffer + rotation, buffer + bufferSizeBytes);
		_wpos = bufferSizeBytes - 1;
		buffer[_wpos] = 0;
	}

	char messageBuffer[MESSAGELOG_SIZE];
	MessageLog messageLog(messageBuffer, MESSAGELOG_SIZE);
}


extern "C" const char* dmesg() {
	mculib::messageLog.rotate();
	char* tmp = mculib::messageLog.buffer;
	char* end = tmp + mculib::messageLog.bufferSizeBytes - 1;
	while(tmp != end && (*tmp) == 0)
		tmp++;
	return tmp;
}
