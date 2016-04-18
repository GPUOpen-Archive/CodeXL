//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This is a local version of TSingleton.h.  It defines a Google Mock class
///        and the includes the TSingleton.s from Common/Src/TSingleton
//==============================================================================

#ifndef _LOCAL_SINGLETON_H_
#define _LOCAL_SINGLETON_H_

#ifdef CL_UNITTEST_MOCK
#include "../../Tests/TestCLMock/TestCLAPITraceMock.h"
#define TSingletonMockGen(ClassName,MockClassName)  class ClassName; \
    template<> \
    class TSingleton<ClassName>{ \
    public: \
        static MockClassName *obj;\
    protected: \
        TSingleton(){} \
        virtual ~TSingleton(){} \
    public : \
        inline static MockClassName* Instance(void){ \
            if (obj == NULL) { obj = new MockClassName; } \
            return obj;} \
        inline static void DeleteInstance() { \
            if (obj != NULL) { \
                MockClassName* copyOfPInstance = obj; \
                obj = NULL; \
                delete copyOfPInstance;}}};

#else
#define TSingletonMockGen(ClassName,MockClassName)
#endif

#include <TSingleton.h>

#endif // _LOCAL_SINGLETON_H_
