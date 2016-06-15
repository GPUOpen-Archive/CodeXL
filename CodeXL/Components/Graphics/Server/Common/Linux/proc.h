//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Handles the setting up and retrieval of information for all processes
///         running in the system
//==============================================================================

class ProcImpl;      // forward declaration

/// Proc class.
/// Handles the setting up and retrieval of information for all processes
/// running in the system.
/// First, a Proc Object is Opened. Read() is then called to fetch the
/// relevent process information needed. The accessor methods can then be
/// called to retrieve the process information for external use.
class Proc
{
public:
    /// Constructor
    Proc();

    /// Destructor
    ~Proc();

    /// Open a Proc object. Needs to be called before reading.
    /// \return true if opened successfully, false if error
    bool Open();

    /// Read from a Proc object. Sets the data for the next process
    /// in the list
    /// \return true if the data for the next process has been set up
    /// correctly, false if there is an error or there are no more
    /// processes to retrieve.
    bool Read();

    /// Close a Proc object.
    /// \return true if closed successfully, false if error
    bool Close();

    /// Accessor method to get the name of the current process
    /// \return full name of current process
    const char* GetProcName();

    /// Accessor method to get the ID of the current process
    /// \return ID of the current process
    int GetProcessId();

private:
    /// Internal implementation object
    ProcImpl* m_pProcImpl;
};
