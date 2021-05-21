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

#define _POSIX_C_SOURCE 200809L
#define _ISOC11_SOURCE

#include "logger.h"
#include "unidef.h"

#include <pthread.h>

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static FILE *log_file;
static LOGGER_LEVEL log_level;
static LOGGER_TARGET log_target;
static pthread_mutex_t log_mutex;

static const char *const LOGGER_LEVEL_name[] = { "INFO", "DEBUG", "WARNING",
						 "ERROR", "NONE" };

/**
 * 初始化logger。调用logger前必须调用该函数进行初始化
 * @param path
 * @param level
 */
void logger_init(const char *path, const LOGGER_LEVEL level,
		 const LOGGER_TARGET target)
{
	log_file = fopen(path, "a");
	if (log_file == NULL) {
		time_t current_time;
		time(&current_time);

		struct tm *timeinfo;
		timeinfo = localtime(&current_time);

		fprintf(stderr,
			"%4d-%02d-%02d %02d:%02d:%02d logger_init() FATAL: Failed to open log file "
			"%s. Abort.\n",
			timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
			timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
			timeinfo->tm_sec, path);
		exit(1);
	}

	pthread_mutex_init(&log_mutex, NULL);
	log_level = level;
	log_target = target;
}

void logger_write(const LOGGER_LEVEL level, const char *format, ...)
{
	time_t current_time;
	time(&current_time);

	struct tm *timeinfo;
	timeinfo = localtime(&current_time);

	if (log_file == NULL) {
		fprintf(stderr,
			"%4d-%02d-%02d %02d:%02d:%02d logger_write() FATAL: Logger not initialized. Abort."
			"Abort.\n",
			timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
			timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
			timeinfo->tm_sec);
		exit(1);
	}

	if (level >= log_level) {
		pthread_mutex_lock(&log_mutex);
		va_list arg_ptr;

		if (log_target & LOGGER_TARGET_FILE) {
			fprintf(log_file, "%4d-%02d-%02d %02d:%02d:%02d [%s] ",
				timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
				timeinfo->tm_mday, timeinfo->tm_hour,
				timeinfo->tm_min, timeinfo->tm_sec,
				LOGGER_LEVEL_name[(int)level]);
			va_start(arg_ptr, format);
			vfprintf(log_file, format, arg_ptr);
			va_end(arg_ptr);
			fprintf(log_file, "\n");
			fflush(log_file);
		}

		if (log_target & LOGGER_TARGET_CONSOLE) {
			fprintf(stdout, "%4d-%02d-%02d %02d:%02d:%02d [%s] ",
				timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
				timeinfo->tm_mday, timeinfo->tm_hour,
				timeinfo->tm_min, timeinfo->tm_sec,
				LOGGER_LEVEL_name[(int)level]);
			va_start(arg_ptr, format);
			vfprintf(stdout, format, arg_ptr);
			va_end(arg_ptr);
			fprintf(stdout, "\n");
			fflush(stdout);
		}

		pthread_mutex_unlock(&log_mutex);
	}
}

void logger_write_raw(const LOGGER_LEVEL level, const char *addtional,
		      const void *data, size_t data_size)
{
	time_t current_time;
	time(&current_time);

	struct tm *timeinfo;
	timeinfo = localtime(&current_time);

	if (log_file == NULL) {
		fprintf(stderr,
			"%4d-%02d-%02d %02d:%02d:%02d logger_write() FATAL: Logger not initialized. Abort."
			"Abort.\n",
			timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
			timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
			timeinfo->tm_sec);
		exit(1);
	}

	if (level >= log_level) {
		pthread_mutex_lock(&log_mutex);
		if (log_target & LOGGER_TARGET_FILE) {
			fprintf(log_file, "%4d-%02d-%02d %02d:%02d:%02d [%s] %s",
				timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
				timeinfo->tm_mday, timeinfo->tm_hour,
				timeinfo->tm_min, timeinfo->tm_sec,
				LOGGER_LEVEL_name[(int)level],
				addtional);

			for (size_t i = 0; i < data_size; i++) {
				if (i % 16 == 0)
					fprintf(log_file, "\n");
				if (i % 16 == 0)
					fprintf(log_file, "%02x", ((const uint8_t *)data)[i]);
				else
					fprintf(log_file, " %02x", ((const uint8_t *)data)[i]);
			}

			fflush(log_file);
		}

		if (log_target & LOGGER_TARGET_CONSOLE) {
			fprintf(stdout, "%4d-%02d-%02d %02d:%02d:%02d [%s] %s",
				timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
				timeinfo->tm_mday, timeinfo->tm_hour,
				timeinfo->tm_min, timeinfo->tm_sec,
				LOGGER_LEVEL_name[(int)level],
				addtional);

			for (size_t i = 0; i < data_size; i++) {
				if (i % 16 == 0)
					fprintf(stdout, "\n");
				if (i % 16 == 0)
					fprintf(stdout, "%02x", ((const uint8_t *)data)[i]);
				else
					fprintf(stdout, " %02x", ((const uint8_t *)data)[i]);
			}
			fflush(stdout);
		}

		pthread_mutex_unlock(&log_mutex);
	}
}
