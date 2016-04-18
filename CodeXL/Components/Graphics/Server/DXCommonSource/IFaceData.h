//=============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This is an interface for VTable class.
//=============================================================================

#ifndef IFACE_DATA
#define IFACE_DATA

class IFaceData;

/// map interface pointer to data about the object
typedef std::map< IUnknown*, IFaceData > IFacesMap;

/// \addtogroup Patcher

/// Stores information about an instances of an interface
class IFaceData
{
public:

    //-----------------------------------------------------------------------------
    /// Constructor
    /// \param strCallStack the callstack when the object was created
    /// \param bAppCreated true if the object was created by the app; false if created by perfstudio
    //-----------------------------------------------------------------------------
    IFaceData(std::string strCallStack, bool bAppCreated)
    {
        //AddCallStack(strCallStack);
        m_bAppCreated = bAppCreated;
    }

    //-----------------------------------------------------------------------------
    /// Default Constructor
    /// no callstack is added and it is assumbed to be created by perfstudio
    //-----------------------------------------------------------------------------
    IFaceData()
    {
        m_bAppCreated = false;
    }

    //-----------------------------------------------------------------------------
    /// Destructor
    //-----------------------------------------------------------------------------
    ~IFaceData()
    {
    }

    //-----------------------------------------------------------------------------
    /// Adds the callstack and increments the number of times
    /// \param strCallStack string containing the callstack to add
    //-----------------------------------------------------------------------------
    void AddCallStack(std::string strCallStack)
    {
        std::map< std::string, unsigned long >::iterator iCallStack = m_callStackMap.find(strCallStack);

        if (iCallStack == m_callStackMap.end())
        {
            // new call stack, add it and set value to 1
            m_callStackMap[ strCallStack ] = 1;
        }
        else
        {
            // increment count
            iCallStack->second++;
        }
    }

    //-----------------------------------------------------------------------------
    /// Generates a string of the callstacks
    /// \return a string containing all the callstacks
    //-----------------------------------------------------------------------------
    std::string GetCallStackString()
    {
        std::string strCS;

        unsigned long ulCount = 1;

        for (std::map< std::string, unsigned long >::iterator iCS = m_callStackMap.begin();
             iCS != m_callStackMap.end();
             iCS++)
        {
            strCS += FormatText("Call Stack %d: (Called %d times)\n%s\n\n", ulCount, iCS->second, iCS->first.c_str()).asCharArray();
            ulCount++;
        }

        return strCS;
    }

    /// stores unique callstacks when an object is created
    /// maps callstack to the number of times the object was created from that callstack
    std::map< std::string, unsigned long > m_callStackMap;

    /// indicates whether the app (true) or perfstudio (false) created the object
    bool m_bAppCreated;
};

#endif // IFACE_DATA