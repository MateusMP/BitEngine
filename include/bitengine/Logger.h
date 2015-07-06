#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

namespace BitEngine{

class Logger
{
public:
    static void LogErrorToConsole(const char* format, ...){
        va_list argptr;
        va_start(argptr, format);
        vfprintf(stdout, format, argptr);
        va_end(argptr);
    }

    static void LogErrorToFile(FILE* f, const char* format, ...){
        va_list argptr;
        va_start(argptr, format);
        vfprintf(f, format, argptr);
        va_end(argptr);
    }

    static void ABORT(int e){
        exit(e);
    }

};


}
