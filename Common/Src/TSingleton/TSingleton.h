//==============================================================================
// Copyright (c) 2012-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A simple template based singleton class wrapper.
//==============================================================================

#ifndef _TSINGLETON_H_
#define _TSINGLETON_H_

// The main use for lazy allocation version here is to aid in debugging problems.
// Ctors for globally allocated things can be a pain to debug.

#if defined(USE_POINTER_SINGLETON)

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

    // Static current instance
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
    // Current instance getter
    //=============================================================================
    static C* Instance()
    {
        if (m_pInstance == nullptr)
        {
            m_pInstance = new C;
        }

        return m_pInstance;
    }

    //=============================================================================
    // Delete current instance
    //=============================================================================
    static void DeleteInstance()
    {
        if (m_pInstance != nullptr)
        {
            // ~TSingleton and DeleteInstance will loop
            // unless we set m_pInstance to nullptr
            // before calling delete.
            C* copyOfPInstance = m_pInstance;
            m_pInstance = nullptr;
            delete copyOfPInstance;
        }
    }
};

// Initialize the static member CurrentInstance
template< class C >
C* TSingleton<C>::m_pInstance = nullptr;

#else

//=============================================================================
/// Singleton base class.
///
/// This singleton does not require deletion as it uses a static object.
//=============================================================================
template< class C >
class TSingleton
{
private:

    /// The underlying instance of this singleton object
    static C m_Instance;

    /// A pointer to the singleton instance that gets set to nullptr when the object is destroyed.
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
template< class C >
C TSingleton<C>::m_Instance;

/// Initialize the pointer to the instance
template< class C >
C* TSingleton<C>::m_pInstance = &TSingleton<C>::m_Instance;

#endif

#endif // _TSINGLETON_H_
