//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Singleton base class
//==============================================================================

#ifndef _TSINGLETON_H_
#define _TSINGLETON_H_

#ifdef USE_POINTER_SINGLETON
#include <stdlib.h>        // for NULL

//=============================================================================
/// Singleton base class.
///
/// Implemented using a pointer to the instance. Requires calling DeleteInstance
/// to clean up.
//=============================================================================
template< class C >
class TSingleton
{
private:

    /// Static current instance
    static C* m_pInstance;

protected:

    //=============================================================================
    /// Hidden constructor
    //=============================================================================
    TSingleton()
    {
    }

    //=============================================================================
    /// Hidden destructor
    //=============================================================================
    virtual ~TSingleton()
    {
        DeleteInstance();
    }

public:

    //=============================================================================
    /// Current instance getter
    //=============================================================================
    static C* Instance()
    {
        if (m_pInstance == NULL)
        {
            m_pInstance = new C;
        }

        return m_pInstance;
    }

    /// Get a ref of the instance
    // \return Ref to instance object
    static C& Ref()
    {
        if (m_pInstance == NULL)
        {
            m_pInstance = new C;
        }

        return *m_pInstance;
    }

    //=============================================================================
    /// Delete current instance
    //=============================================================================
    static void DeleteInstance()
    {
        if (m_pInstance != NULL)
        {
            // ~TSingleton and DeleteInstance will loop
            // unless we set m_pInstance to NULL
            // before calling delete.
            C* copyOfPInstance = m_pInstance;
            m_pInstance = NULL;
            delete copyOfPInstance;
        }
    }
};

// Initialize the static member CurrentInstance
template< class C >
C* TSingleton<C>::m_pInstance = NULL;

#else

//=============================================================================
/// Singleton base class.
///
/// This singleton does not require deletion as it uses a static object.
//=============================================================================
template< typename C >
class TSingleton
{
private:

    /// The underlying instance of this singleton object
    static C m_Instance;

    /// A pointer to the singleton instance that gets set to Null when the object is destroyed.
    static C* m_pInstance;

protected:

    //=============================================================================
    /// Hidden constructor
    //=============================================================================
    TSingleton()
    {
        m_pInstance = &m_Instance;
    }

    //=============================================================================
    /// Hidden destructor
    //=============================================================================
    virtual ~TSingleton()
    {
        m_pInstance = 0;
    }

public:

    //=============================================================================
    /// Accessor to the instance.
    /// \return A pointer to the instance.
    //=============================================================================
    static C* Instance()
    {
        return m_pInstance;
    }

    //=============================================================================
    /// Accessor to a reference of the instance.
    /// \return A reference to the instance.
    //=============================================================================
    static C& Ref()
    {
        return m_Instance;
    }

    //=============================================================================
    /// Supplied for compatability with pointer based version ( see: USE_POINTER_SINGLETON ).
    //=============================================================================
    static void DeleteInstance()
    {
        /// Do nothing
    }
};

/// Initialize the instance.
template< typename C >
C TSingleton<C>::m_Instance;

/// Initialize the pointer to the instance
template< typename C >
C* TSingleton<C>::m_pInstance = &TSingleton<C>::m_Instance;

#endif

#endif // _TSINGLETON_H_
