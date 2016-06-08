//==============================================================================
// Copyright (c) 2010-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Device info utils class
//==============================================================================


#ifndef _DEVICE_INFO_UTILS_H_
#define _DEVICE_INFO_UTILS_H_

#include <string>
#include <cstring>
#include <map>
#include <vector>
#include "DeviceInfo.h"

//------------------------------------------------------------------------------------
/// Device info utils
//------------------------------------------------------------------------------------
class AMDTDeviceInfoUtils
{
public:

    /// Get AMDTDeviceInfoUtils instance
    /// \return the dynamically created AMDTDeviceInfoUtils instance
    static AMDTDeviceInfoUtils* Instance()
    {
        if (nullptr == ms_pInstance)
        {
            ms_pInstance = new AMDTDeviceInfoUtils();
        }

        return ms_pInstance;
    }

    /// Deletes the AMDTDeviceInfoUtils instance
    static void DeleteInstance()
    {
        delete ms_pInstance;
        ms_pInstance = nullptr;
    }

    /// Adds the specified device info
    /// \param asicType the asic type of the device info being added
    /// \param deviceInfo the device info being added
    void AddDeviceInfo(GDT_HW_ASIC_TYPE asicType, const GDT_DeviceInfo& deviceInfo);

    /// Adds the specified card info
    /// \param cardInfo the card being added
    void AddDevice(const GDT_GfxCardInfo& cardInfo);

    /// Function pointer type for a function that will translate device names
    typedef std::string(*DeviceNameTranslatorFunction)(const char* strDeviceName);

    /// Sets the Device name translator function
    /// \param deviceNametranslatorFunction the function to use to translate device names
    void SetDeviceNameTranslator(DeviceNameTranslatorFunction deviceNametranslatorFunction);

    /// Get device info from device ID
    /// \param[in] deviceID Device ID - usually queried from ADL
    /// \param[in] revisionID RevisionID - usually queried from ADL
    /// \param[out] deviceInfo Output device info if device id is found.
    /// \return True if device info is found
    bool GetDeviceInfo(size_t deviceID, size_t revisionID, GDT_DeviceInfo& deviceInfo) const;

    /// Get device info from CAL name string
    /// NOTE: this might not return the correct GDT_DeviceInfo instance, since some devices with the same CAL name might have different GDT_DeviceInfo instances
    /// \param[in] szCALDeviceName CAL device name string
    /// \param[out] deviceInfo Output device info if device id is found.
    /// \return True if device info is found
    bool GetDeviceInfo(const char* szCALDeviceName, GDT_DeviceInfo& deviceInfo) const;

    /// Get Graphics Card Info.
    /// \param[in] deviceID Device ID - usually queried from ADL
    /// \param[in] revisionID Revision ID - usually queried from ADL
    /// \param[out] cardInfo Output graphics card info if device id is found.
    /// \return True if info for deviceID is found
    bool GetDeviceInfo(size_t deviceID, size_t revisionID, GDT_GfxCardInfo& cardInfo) const;

    /// Get a vector of Graphics Card Info.
    /// \param[in] szCALDeviceName CAL device name string
    /// \param[out] cardList Output vector of graphics card info.
    /// \return True if any graphics card info is found for CAL device name.
    bool GetDeviceInfo(const char* szCALDeviceName, std::vector<GDT_GfxCardInfo>& cardList) const;

    /// Get a vector of Graphics Card Info.
    /// \param[in]  szMarketingDeviceName Marketing device name string
    /// \param[out] cardList Output vector of graphics card info.
    /// \return True if any graphics card info is found for Marketing device name.
    bool GetDeviceInfoMarketingName(const char* szMarketingDeviceName, std::vector<GDT_GfxCardInfo>& cardList) const;

    /// Query whether or not input device is APU or not
    /// \param[in] szCALDeviceName CAL device name string
    /// \param[out] bIsAPU flag indicating whether or not the specified device is an APU
    /// \return True if device info is found
    bool IsAPU(const char* szCALDeviceName, bool& bIsAPU) const;

    /// Get hardware generation from device ID
    /// \param[in] deviceID Device id
    /// \param[out] gen Hardware generation
    /// \return True if device info is found
    bool GetHardwareGeneration(size_t deviceID, GDT_HW_GENERATION& gen) const;

    /// Get hardware generation from device name
    /// \param[in] szCALDeviceName Device name
    /// \param[out] gen Hardware generation
    /// \return True if device info is found
    bool GetHardwareGeneration(const char* szCALDeviceName, GDT_HW_GENERATION& gen) const;

    /// Get all cards from the specified hardware generation
    /// \param[in] gen Hardware generation
    /// \param[out] cardList Output vector of graphics card info.
    /// \return true if successful, false otherwise
    bool GetAllCardsInHardwareGeneration(GDT_HW_GENERATION gen, std::vector<GDT_GfxCardInfo>& cardList) const;

    /// Get all cards with the specified device id
    /// \param[in] deviceId DeviceId
    /// \param[out] cardList Output vector of graphics card info.
    /// \return true if successful, false otherwise
    bool GetAllCardsWithDeviceId(size_t deviceID, std::vector<GDT_GfxCardInfo>& cardList) const;

    /// Get hardware generation display name
    /// \param[in] gen Hardware generation
    /// \param[out] strGenerationDisplayName the display name for the specified hardware generation
    /// \return true if successful, false otherwise
    bool GetHardwareGenerationDisplayName(GDT_HW_GENERATION gen, std::string& strGenerationDisplayName) const;

    /// Determine if the specified device is a member of the specified family
    /// \param[in] szCALDeviceName CAL device name
    /// \param[in] generation Generation enum
    /// \param[out] bRes Set to true if input device name is a specified family card
    /// \return false if device name is not found
    bool IsXFamily(const char* szCALDeviceName, GDT_HW_GENERATION generation, bool& bRes) const
    {
        GDT_HW_GENERATION gen = GDT_HW_GENERATION_NONE;

        if (GetHardwareGeneration(szCALDeviceName, gen))
        {
            bRes = gen == generation;
            return true;
        }
        else
        {
            return false;
        }
    }

    /// Determine if the specified device is a member of the VI family
    /// \param[in] szCALDeviceName CAL device name
    /// \param[out] bIsVI Set to true if input device name is a member of the VI family
    /// \return false if device name is not found
    bool IsVIFamily(const char* szCALDeviceName, bool& bIsVI) const
    {
        return IsXFamily(szCALDeviceName, GDT_HW_GENERATION_VOLCANICISLAND, bIsVI);
    }

    /// Determine if the specified device is a member of the CI family
    /// \param[in] szCALDeviceName CAL device name
    /// \param[out] bIsCI Set to true if input device name is a member of the CI family
    /// \return false if device name is not found
    bool IsCIFamily(const char* szCALDeviceName, bool& bIsCI) const
    {
        return IsXFamily(szCALDeviceName, GDT_HW_GENERATION_SEAISLAND, bIsCI);
    }

    /// Determine if the specified device is a member of the SI family
    /// \param[in] szCALDeviceName CAL device name
    /// \param[out] bIsSI Set to true if input device name is a member of the SI family
    /// \return false if device name is not found
    bool IsSIFamily(const char* szCALDeviceName, bool& bIsSI) const
    {
        return IsXFamily(szCALDeviceName, GDT_HW_GENERATION_SOUTHERNISLAND, bIsSI);
    }

    /// Determine if the specified device is based on GCN architecture.
    /// \param[in] szCALDeviceName CAL device name
    /// \param[out] bIsGCN Set to true if input device name is based on GCN architecture.
    /// \return false if device name is not found
    bool IsGCN(const char* szCALDeviceName, bool& bIsGCN) const
    {
        bIsGCN = false;
        bool bRet = IsVIFamily(szCALDeviceName, bIsGCN);

        if (!bIsGCN)
        {
            bRet = IsCIFamily(szCALDeviceName, bIsGCN);
        }

        if (!bIsGCN)
        {
            bRet = IsSIFamily(szCALDeviceName, bIsGCN);
        }

        return bRet;
    }

    /// Translates the reported device name to the true device name exposed in the DeviceInfo table.
    /// \param strDeviceName the device name reported by the runtime.
    /// \return the true device name as exposed by the device info table.
    std::string TranslateDeviceName(const char* strDeviceName) const;

private:
    /// private constructor
    AMDTDeviceInfoUtils() : m_pDeviceNameTranslatorFunction(nullptr) {}

    /// private destructor
    virtual ~AMDTDeviceInfoUtils() {}

    //------------------------------------------------------------------------------------
    /// const char* comparer used in the DeviceNameMap below
    //------------------------------------------------------------------------------------
    struct cmp_str
    {
        /// Operator () overload function
        /// \param a left operand
        /// \param b right operand
        /// \return true if a is less than b
        bool operator()(const char* a, const char* b) const
        {
            return std::strcmp(a, b) < 0;
        }
    };

    typedef std::multimap<size_t, GDT_GfxCardInfo> DeviceIDMap;                        ///< typedef for map from device id to card info
    typedef std::pair<size_t, GDT_GfxCardInfo> DeviceIDMapPair;                        ///< typedef for device id / card info pair

    typedef std::multimap<const char*, GDT_GfxCardInfo, cmp_str> DeviceNameMap;        ///< typedef for map from CAL device name to card info (with custom comparer)
    typedef std::pair<const char*, GDT_GfxCardInfo> DeviceNameMapPair;                 ///< typedef for device name / card info pair

    typedef std::multimap<GDT_HW_GENERATION, GDT_GfxCardInfo> DeviceHWGenerationMap;   ///< typedef for map from hardware generation to card info
    typedef std::pair<GDT_HW_GENERATION, GDT_GfxCardInfo> DeviceHWGenerationMapPair;   ///< typedef for hardware generation / card info pair

    typedef std::map<GDT_HW_ASIC_TYPE, GDT_DeviceInfo> ASICTypeDeviceInfoMap;          ///< typedef for map from asic type to device info
    typedef std::pair<GDT_HW_ASIC_TYPE, GDT_DeviceInfo> ASICTypeDeviceInfoMapPair;     ///< typedef for asic type / device info pair

    DeviceIDMap           m_deviceIDMap;            ///< device ID to card info map.
    DeviceNameMap         m_deviceNameMap;          ///< cal device name to card info map.
    DeviceNameMap         m_deviceMarketingNameMap; ///< marketing device name to card info map.
    DeviceHWGenerationMap m_deviceHwGenerationMap;  ///< hardware generation to card info map.
    ASICTypeDeviceInfoMap m_asicTypeDeviceInfoMap;  ///< ASIC type to device info map.

    DeviceNameTranslatorFunction m_pDeviceNameTranslatorFunction; /// the function to call to translate device names

    static AMDTDeviceInfoUtils* ms_pInstance;
};

//------------------------------------------------------------------------------------
/// Device info manager
//------------------------------------------------------------------------------------
class AMDTDeviceInfoManager
{
    /// Get singleton AMDTDeviceInfoManager instance
    /// \return the singleton AMDTDeviceInfoManager instance
    static AMDTDeviceInfoManager* Instance()
    {
        return &ms_instance;
    }

private:

    /// Constructor
    AMDTDeviceInfoManager();

    /// Destructor
    ~AMDTDeviceInfoManager();

    /// Locates and calls the device info utils function to initialize internal device info.
    /// This allows internal versions of the tools to expose hardware not exposed in the public
    /// versions of the tools (i.e. unreleased hardware)
    void CallInitInternalDeviceInfo() const;

    static AMDTDeviceInfoManager ms_instance; ///< the singleton AMDTDeviceInfoManager instance
};

#endif // _DEVICE_INFO_UTILS_H_
