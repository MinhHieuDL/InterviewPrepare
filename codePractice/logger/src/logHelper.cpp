#include "logHelper.h"

// Generate defination for toStringHelper api
#define ENTRY(value) case value: return #value;

#define DEFINE_TO_STR(TABLE, type_name)                 \
    const char* toStringHelper(type_name eValue) \
    {                                                   \
        switch (eValue)                                 \
        {                                               \
            TABLE(ENTRY)                                \
            default: return "UNKNOWN";                  \
        }                                               \
    }

DEFINE_TO_STR(LOG_LVL_TABLE, enum LOG_LVL)

#undef ENTRY 
#undef DEFINE_TO_STR