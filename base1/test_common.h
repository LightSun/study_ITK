#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <string>
#include <vector>
#include <map>
#include <iostream>

#define String std::string
#define CString const String&

#define FUNC_START(tag) printf("start call %s\n", tag);
#define FUNC_END(tag) printf("end call %s\n", tag);

#ifndef MED_ASSERT
#define MED_ASSERT(condition)                                                   \
    do                                                                      \
    {                                                                       \
        if (!(condition))                                                   \
        {                                                                   \
            std::cout << "Assertion failure: " << __FILE__ << "::" << __FUNCTION__  \
                                     << __LINE__ \
                                     << " >> " << #condition << std::endl;  \
            abort();                                                        \
        }                                                                   \
    } while (0)
#endif

#endif // TEST_COMMON_H
