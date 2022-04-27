/** Copyright (C) 2013 Ultimaker - Released under terms of the AGPLv3 License */
#include <stdio.h>
#include <stdarg.h>

#ifdef _OPENMP
    #include <omp.h>
#endif // _OPENMP
#include "logoutput.h"


namespace cura {

std::function<void(std::shared_ptr<std::string>)> errorHandler = nullptr;
std::function<bool()> isKeepingHandler = nullptr;

static int verbose_level;
static bool progressLogging;

void increaseVerboseLevel()
{
    verbose_level++;
}

void enableProgressLogging()
{
    progressLogging = true;
}

int std_string_format(std::shared_ptr<std::string> str, const char* format, ...) {
    std::string tmp;

    va_list marker;
    va_start(marker, format);

    size_t num = _vscprintf(format, marker);

    if (num >= tmp.capacity())
        tmp.reserve(num + 1);

    vsprintf_s((char*)tmp.data(), tmp.capacity(), format, marker);

    va_end(marker);

    *str = tmp.c_str();
    return str->size();
}

void logError(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    #pragma omp critical
    {
        fprintf(stderr, "[ERROR] ");
        vfprintf(stderr, fmt, args);
        fflush(stderr);

        if (errorHandler) {
            std::shared_ptr<std::string> desc = std::make_shared<std::string>();
            std_string_format(desc, fmt, args);
            errorHandler(desc);
        }
    }
    va_end(args);
}

void logWarning(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    #pragma omp critical
    {
        fprintf(stderr, "[WARNING] ");
        vfprintf(stderr, fmt, args);
        fflush(stderr);
    }
    va_end(args);
}

void logAlways(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    #pragma omp critical
    {
        vfprintf(stderr, fmt, args);
        fflush(stderr);
    }
    va_end(args);
}

void log(const char* fmt, ...)
{
    va_list args;
    if (verbose_level < 1)
        return;

    va_start(args, fmt);
    #pragma omp critical
    {
        vfprintf(stderr, fmt, args);
        fflush(stderr);
    }
    va_end(args);
}

void logDebug(const char* fmt, ...)
{
    va_list args;
    if (verbose_level < 2)
    {
        return;
    }
    va_start(args, fmt);
    #pragma omp critical
    {
        fprintf(stderr, "[DEBUG] ");
        vfprintf(stderr, fmt, args);
        fflush(stderr);
    }
    va_end(args);
}

void logProgress(const char* type, int value, int maxValue, float percent)
{
    if (!progressLogging)
        return;

    #pragma omp critical
    {
        fprintf(stderr, "Progress:%s:%i:%i \t%f%%\n", type, value, maxValue, percent);
        fflush(stderr);
    }
}

}//namespace cura
