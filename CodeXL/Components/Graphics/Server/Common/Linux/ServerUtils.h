//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  An interface containing shared functionality needed by the individual
///         server plugins
//==============================================================================

#ifndef SERVERUTILS_H_
#define SERVERUTILS_H_

class ServerUtils
{
public:

    /// Constructor
    ServerUtils() {}

    /// Destructor
    ~ServerUtils() {}

    /// CanBind
    /// Is this shared library allowed to bind with the executable it's currently preloaded into
    /// \return true if it is allowed to bind to this exe, false if not
    static bool CanBind(const char* programName);

    /// Put up a dialog box to give the user time to attach a debugger to the application rather than
    /// the perf studio app.
    /// \param serverName the name of the server plugin loaded into the target application
    /// \param initialized has the server plugin been initialized
    /// \return 0 if successful, non-zero on error
    static int CheckForDebuggerAttach(const char* serverName, bool initialized);
};

#endif // SERVERUTILS_H_
