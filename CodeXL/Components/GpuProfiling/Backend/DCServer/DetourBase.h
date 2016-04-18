#ifndef _DC_DETOUR_BASE_H_
#define _DC_DETOUR_BASE_H_

#include <windows.h>
#include <string>
#include <vector>

#include <AMDTBaseTools/Include/gtString.h>

/// \addtogroup Common
// @{

//------------------------------------------------------------------------------------
/// This class provides simple interface for detouring
//------------------------------------------------------------------------------------
class DetourBase
{
    //------------------------------------------------------------------------------------
    /// This struct encapsulates two properties of a dll
    //------------------------------------------------------------------------------------
    struct DLLPair
    {
        gtString    m_strDll;      ///< DLL name string
        bool        m_bAttached;   ///< DLL attached or not

        /// Constructor
        DLLPair()
        {
            m_bAttached = false;
        }
    };

public:
    /// Default Constructor
    DetourBase();

    /// Constructor for dlls that always have same name regardless of the version
    /// \param[in] strDll name string
    DetourBase(const gtString& strDll);

    /// Destructor
    virtual ~DetourBase();

    /// Called in UpdateHooks, try to call OnAttach when dll is loaded
    void Attach();

    /// Detach, restore to real function pointer
    /// \return true if successful, false otherwise
    virtual bool Detach();

    /// Add different version of dlls e.g. D3DX11_42.dll D3DX11_43.dll
    /// \param[in] strDll dll name string
    void AddDLL(const gtString& strDll);

    /// Get dll module
    /// \return dll module
    HMODULE GetModule()
    {
        return m_hMod;
    };

protected:
    /// virtual function that every derived class should override
    virtual bool OnAttach();

    std::vector<DLLPair> m_vecDllPair;  ///< a list of dlls of different versions e.g. D3DX11_42.dll D3DX11_43.dll
    bool        m_bAttached;            ///< Indicates whether the library has been attached or not regardless of version
    HMODULE     m_hMod;                 ///< Current loaded dll module handle
};


// @}

#endif