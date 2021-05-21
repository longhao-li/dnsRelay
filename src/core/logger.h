/**
 * MIT License
 *
 * Copyright (c) 2021 qwqllh
 * 
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
 */

#ifndef CORE_LOGGER_H_
#define CORE_LOGGER_H_

#include <stddef.h>

typedef enum {
	LOGGER_INFO = 0,
	LOGGER_DEBUG = 1,
	LOGGER_WARNING = 2,
	LOGGER_ERROR = 3,
	LOGGER_NONE = 4
} LOGGER_LEVEL;

typedef enum {
	LOGGER_TARGET_NONE = 0x00,
	LOGGER_TARGET_CONSOLE = 0x01,
	LOGGER_TARGET_FILE = 0x02
} LOGGER_TARGET;

extern void logger_init(const char *path, const LOGGER_LEVEL level,
			const LOGGER_TARGET target);
extern void logger_write(const LOGGER_LEVEL level, const char *format, ...);

extern void logger_write_raw(const LOGGER_LEVEL level, const char *addtional,
			     const void *data, size_t data_size);

#endif /* CORE_LOGGER_H_ */