#pragma once

#include <string>

#define LOG_LVL_TABLE(X) \
    X(DEBUG)             \
    X(WARN)              \
    X(ERROR)             \
    X(INFO) 

// Genarate LOG_LVL enum
enum LOG_LVL{ 
    #define ENTRY(evalue) evalue,
        LOG_LVL_TABLE(ENTRY)
    #undef ENTRY
};

// Generate toString api here
const char* toStringHelper(LOG_LVL eValue);
