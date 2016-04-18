//=====================================================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief Class thta catches un-handled exceptions that will cause the profiled application
/// to crash. Reports the crash including its call-stack into our log file
//=====================================================================================

#ifndef UNHANDLEDEXCEPTIONHANDLER_H
#define UNHANDLEDEXCEPTIONHANDLER_H

// Infra:
#include <AMDTOSWrappers/Include/osUnhandledExceptionHandler.h>


//=============================================================================
/// Catches un-handled exceptions that will cause the profiled application
/// to crash. Reports the crash including its call-stack into our log file
//=============================================================================
class UnhandledExceptionHandler : public osUnhandledExceptionHandler
{
public:
    virtual ~UnhandledExceptionHandler();
    static bool init();

protected:
    virtual void onUnhandledException(osExceptionCode exceptionCode, void* pExceptionContext);

private:
    static bool hookSetUnhandledExceptionFilter();

private:
    // Don't allow the creation of this class (only init should create this class)
    UnhandledExceptionHandler();
};

#endif // UNHANDLEDEXCEPTIONHANDLER_H

