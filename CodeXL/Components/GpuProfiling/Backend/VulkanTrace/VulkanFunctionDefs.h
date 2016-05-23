#ifndef _VK_FUNCTION_ENUM_DEFS_H_
#define _VK_FUNCTION_ENUM_DEFS_H_

#include <string>
#include <map>

using namespace std;

#define CODEXL_INCLUDE
#include <Server/VulkanServer/VKT/vktEnumerations.h>
#undef CODEXL_INCLUDE

// ----------------------------------------------------------------------------------
// Class Name: vulkanFunctionDefs Utility class handling the types and strings
//             for vulkan API functions
// ----------------------------------------------------------------------------------
class vulkanFunctionDefs
{
public:

    /// Convert vulkan API name string to enum
    /// \param strName API name string
    /// \return enum representation of vulkan API
    static VkFuncId ToVKFuncType(const std::string& strName);

    /// Extract the function type from the function name
    /// \param strAPIName the API name
    /// \return vkAPIType enumeration describing the API function type
    static vkAPIType TovkAPIType(const std::string& strAPIName);

    /// Extract the function type from the function name
    /// \param apiType the API type
    /// \param strAPIName[output] the API function name
    /// \return true for success
    static bool vkAPITypeToString(VkFuncId apiType, std::string& strAPIName);

    /// Get the function type from the function ID:
    /// \param inAPIFuncId vulkan function id
    /// \return the API type
    static vkAPIType GetAPIGroupFromAPI(VkFuncId inAPIFuncId);

private:

    /// Initializes the maps
    static void Initialize();

    /// Utility function. Adds an interface function to the maps
    /// \param apiStr the function name
    /// \param the enumeration for the API function
    /// \param apiType the enumerated type of the API function
    static void AddInterfaceFunction(const std::string& apiStr, VkFuncId functionType, vkAPIType apiType);

    static void InitCreateAPI();
    static void InitDestroyAPI();
    static void InitGetAPI();
    static void InitMemAPI();
    static void InitDescriptorAPI();
    static void InitQueueSubmissionAPI();
    static void InitCmdBufProfiledAPI();
    static void InitCmdBufNonProfiledAPI();
    static void InitSyncAPI();
    static void InitKHRAPI();

    /// True iff the static members are already initialized
    static bool m_sIsInitialized;

    // Map from string to vulkan function type
    static map<string, VkFuncId> m_sVKAPIMap;

    // Map from vulkan function type to string
    static map<VkFuncId, std::string> m_sVKAPIStringsMap;

    // Map from string to vulkan API type
    static map<string, vkAPIType> m_sVKAPITypeMap;
    static string m_sMissingInterfaces;
};


#endif //_VK_FUNCTION_ENUM_DEFS_H_


