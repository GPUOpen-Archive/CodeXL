//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file defines all the OpenCL API objects.
//==============================================================================

#ifndef _CL_API_DEFS_H_
#define _CL_API_DEFS_H_

/// \defgroup CLAPIDefs CLAPIDefs
/// This module define all OpenCL APIs object
///
/// \ingroup CLTraceAgent
// @{

#include <sstream>
#include <cstdlib>
#include <cstring> //memcpy
#ifdef _WIN32
    #include <CL\cl_d3d11.h>
    #include <CL\cl_d3d10.h>
#endif
#include "CLStringUtils.h"
#include "CLAPIDefBase.h"
#include "../Common/StringUtils.h"
#include "../Common/Defs.h"
#ifdef CL_TRACE_TEST
    #include "../../Tests/CLAPITraceTest/CLAPITraceTest.h"
    #define FRIENDTESTCASE(classname) friend class CLAPITraceTest::classname##Test
#else
    #define FRIENDTESTCASE(classname)
#endif

#define RETVALMIN(x,y) x <= y ? x : y
#define REPLACEDNULLVAL(x,y) x ? NULL : y

//------------------------------------------------------------------------------------
/// clGetPlatformIDs
//------------------------------------------------------------------------------------
class CLAPI_clGetPlatformIDs : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetPlatformIDs()
    {
        m_platform_list = NULL;
    }

    /// Destructor
    ~CLAPI_clGetPlatformIDs()
    {
        if (m_platform_list != NULL)
        {
            FreeArray(m_platform_list);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << m_num_entries << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_platform_list, RETVALMIN(m_num_platformsVal, m_num_entries)) << s_strParamSeparator
           << CLStringUtils::GetIntString(REPLACEDNULLVAL(m_replaced_null_param, m_num_platforms), m_num_platformsVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetPlatformIDs
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param num_entries Parameter for CLAPI_clGetPlatformIDs
    /// \param platform_list Parameter for CLAPI_clGetPlatformIDs
    /// \param num_platforms Parameter for CLAPI_clGetPlatformIDs
    /// \param replaced_null_param flag indicating if the user app passed null to num_platforms
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_uint  num_entries,
        cl_platform_id*   platform_list,
        cl_uint* num_platforms,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetPlatformIDs;
        m_num_entries = num_entries;
        m_num_platforms = num_platforms;
        m_replaced_null_param = replaced_null_param;

        if (retVal == CL_SUCCESS)
        {
            m_num_platformsVal = *num_platforms;
            DeepCopyArray(&m_platform_list, platform_list, RETVALMIN(m_num_platformsVal, m_num_entries));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetPlatformIDs(const CLAPI_clGetPlatformIDs& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetPlatformIDs& operator = (const CLAPI_clGetPlatformIDs& obj);

private:
    cl_uint         m_num_entries;         ///< parameter for clGetPlatformIDs
    cl_platform_id* m_platform_list;       ///< parameter for clGetPlatformIDs
    cl_uint*        m_num_platforms;       ///< parameter for clGetPlatformIDs
    cl_uint         m_num_platformsVal;    ///< parameter for clGetPlatformIDs
    bool            m_replaced_null_param; ///< flag indicating that we've provided a replacement for a NULL m_num_platforms value
    cl_int          m_retVal;              ///< return value
};

//------------------------------------------------------------------------------------
/// clGetPlatformInfo
//------------------------------------------------------------------------------------
class CLAPI_clGetPlatformInfo : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetPlatformInfo()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clGetPlatformInfo()
    {
        if (m_param_value != NULL)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_platform) << s_strParamSeparator
           << CLStringUtils::GetPlatformInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size << s_strParamSeparator
           << CLStringUtils::GetPlatformInfoValueString(m_param_value, m_retVal) << s_strParamSeparator
           << CLStringUtils::GetSizeString(REPLACEDNULLVAL(m_replaced_null_param, m_param_value_size_ret), m_param_value_size_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetPlatformInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param platform Parameter for CLAPI_clGetPlatformInfo
    /// \param param_name Parameter for CLAPI_clGetPlatformInfo
    /// \param param_value_size Parameter for CLAPI_clGetPlatformInfo
    /// \param param_value Parameter for CLAPI_clGetPlatformInfo
    /// \param param_value_size_ret Parameter for CLAPI_clGetPlatformInfo
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_platform_id platform,
        cl_platform_info  param_name,
        size_t   param_value_size,
        void* param_value,
        size_t*  param_value_size_ret,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetPlatformInfo;
        m_platform = platform;
        m_param_name = param_name;
        m_param_value_size = param_value_size;
        m_param_value_size_ret = param_value_size_ret;
        m_replaced_null_param = replaced_null_param;
        m_param_value_size_retVal = *param_value_size_ret;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, RETVALMIN(m_param_value_size_retVal, m_param_value_size));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetPlatformInfo(const CLAPI_clGetPlatformInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetPlatformInfo& operator = (const CLAPI_clGetPlatformInfo& obj);

private:
    cl_platform_id   m_platform;                ///< parameter for clGetPlatformInfo
    cl_platform_info m_param_name;              ///< parameter for clGetPlatformInfo
    size_t           m_param_value_size;        ///< parameter for clGetPlatformInfo
    void*            m_param_value;             ///< parameter for clGetPlatformInfo
    size_t*          m_param_value_size_ret;    ///< parameter for clGetPlatformInfo
    size_t           m_param_value_size_retVal; ///< dereferenced value of m_param_value_size_ret
    bool             m_replaced_null_param;     ///< flag indicating that we've provided a replacement for a NULL m_param_value_size_ret value
    cl_int           m_retVal;                  ///< return value
};

//------------------------------------------------------------------------------------
/// clGetDeviceIDs
//------------------------------------------------------------------------------------
class CLAPI_clGetDeviceIDs : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetDeviceIDs()
    {
        m_device_list = NULL;
    }

    /// Destructor
    ~CLAPI_clGetDeviceIDs()
    {
        if (m_device_list != NULL)
        {
            FreeArray(m_device_list);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_platform) << s_strParamSeparator
           << CLStringUtils::GetDeviceTypeString(m_device_type) << s_strParamSeparator
           << m_num_entries << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_device_list, RETVALMIN(m_num_devicesVal, m_num_entries)) << s_strParamSeparator
           << CLStringUtils::GetIntString(REPLACEDNULLVAL(m_replaced_null_param, m_num_devices), m_num_devicesVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetDeviceIDs
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param platform Parameter for CLAPI_clGetDeviceIDs
    /// \param device_type Parameter for CLAPI_clGetDeviceIDs
    /// \param num_entries Parameter for CLAPI_clGetDeviceIDs
    /// \param device_list Parameter for CLAPI_clGetDeviceIDs
    /// \param num_devices Parameter for CLAPI_clGetDeviceIDs
    /// \param replaced_null_param flag indicating if the user app passed null to num_devices
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_platform_id platform,
        cl_device_type device_type,
        cl_uint  num_entries,
        cl_device_id*  device_list,
        cl_uint* num_devices,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetDeviceIDs;
        m_platform = platform;
        m_device_type = device_type;
        m_num_entries = num_entries;
        m_num_devices = num_devices;
        m_replaced_null_param = replaced_null_param;

        if (retVal == CL_SUCCESS)
        {
            m_num_devicesVal = *num_devices;
            DeepCopyArray(&m_device_list, device_list, RETVALMIN(m_num_devicesVal, m_num_entries));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetDeviceIDs(const CLAPI_clGetDeviceIDs& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetDeviceIDs& operator = (const CLAPI_clGetDeviceIDs& obj);

private:
    cl_platform_id m_platform;            ///< parameter for clGetDeviceIDs
    cl_device_type m_device_type;         ///< parameter for clGetDeviceIDs
    cl_uint        m_num_entries;         ///< parameter for clGetDeviceIDs
    cl_device_id*  m_device_list;         ///< parameter for clGetDeviceIDs
    cl_uint*       m_num_devices;         ///< parameter for clGetDeviceIDs
    cl_uint        m_num_devicesVal;      ///< parameter for clGetDeviceIDs
    bool           m_replaced_null_param; ///< flag indicating that we've provided a replacement for a NULL m_num_devices value
    cl_int         m_retVal;              ///< return value
};

//------------------------------------------------------------------------------------
/// clGetDeviceInfo
//------------------------------------------------------------------------------------
class CLAPI_clGetDeviceInfo : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetDeviceInfo()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clGetDeviceInfo()
    {
        if (m_param_value != NULL)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_device) << s_strParamSeparator
           << CLStringUtils::GetDeviceInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size << s_strParamSeparator
           << CLStringUtils::GetDeviceInfoValueString(m_param_name, RETVALMIN(m_param_value_size_retVal, m_param_value_size), m_param_value, m_retVal) << s_strParamSeparator
           << CLStringUtils::GetSizeString(REPLACEDNULLVAL(m_replaced_null_param, m_param_value_size_ret), m_param_value_size_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetDeviceInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param device Parameter for CLAPI_clGetDeviceInfo
    /// \param param_name Parameter for CLAPI_clGetDeviceInfo
    /// \param param_value_size Parameter for CLAPI_clGetDeviceInfo
    /// \param param_value Parameter for CLAPI_clGetDeviceInfo
    /// \param param_value_size_ret Parameter for CLAPI_clGetDeviceInfo
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_device_id   device,
        cl_device_info param_name,
        size_t   param_value_size,
        void* param_value,
        size_t*  param_value_size_ret,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetDeviceInfo;
        m_device = device;
        m_param_name = param_name;
        m_param_value_size = param_value_size;
        m_param_value_size_ret = param_value_size_ret;
        m_replaced_null_param = replaced_null_param;
        m_param_value_size_retVal = *param_value_size_ret;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, RETVALMIN(m_param_value_size_retVal, m_param_value_size));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetDeviceInfo(const CLAPI_clGetDeviceInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetDeviceInfo& operator = (const CLAPI_clGetDeviceInfo& obj);

private:
    cl_device_id   m_device;                  ///< parameter for clGetDeviceInfo
    cl_device_info m_param_name;              ///< parameter for clGetDeviceInfo
    size_t         m_param_value_size;        ///< parameter for clGetDeviceInfo
    void*          m_param_value;             ///< parameter for clGetDeviceInfo
    size_t*        m_param_value_size_ret;    ///< parameter for clGetDeviceInfo
    size_t         m_param_value_size_retVal; ///< dereferenced value of m_param_value_size_ret
    bool           m_replaced_null_param;     ///< flag indicating that we've provided a replacement for a NULL m_param_value_size_ret value
    cl_int         m_retVal;                  ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateContextBase
//------------------------------------------------------------------------------------
class CLAPI_clCreateContextBase : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateContextBase()
    {
        m_uiContextID = ms_NumInstance;
        ms_mtx.Lock();
        ms_NumInstance++;
        ms_mtx.Unlock();
    }

    /// Virtual destructor
    virtual ~CLAPI_clCreateContextBase() {}

    /// Get Instance num
    /// \return get num of instance created
    cl_uint GetInstanceNum() const
    {
        return ms_NumInstance;
    }

    /// Get instance id
    /// \return instance id
    cl_uint GetInstanceID() const
    {
        return m_uiContextID;
    }

    /// Get context pointer
    /// \return context
    cl_context GetContext() const
    {
        return m_retVal;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

protected:
    /// Add cl context and CreateContextAPIObject to info manager
    void AddToInfoManager(cl_context context);

    cl_uint    m_uiContextID; ///< context id
    cl_context m_retVal;      ///< return value
private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateContextBase(const CLAPI_clCreateContextBase& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateContextBase& operator = (const CLAPI_clCreateContextBase& obj);

    static cl_uint   ms_NumInstance; ///< number of instances created
    static AMDTMutex ms_mtx;         ///< mutex to protect ms_NumInstance
};

//------------------------------------------------------------------------------------
/// clCreateContext
//------------------------------------------------------------------------------------
class CLAPI_clCreateContext : public CLAPI_clCreateContextBase
{
    FRIENDTESTCASE(clCreateContext);
public:
    /// Constructor
    CLAPI_clCreateContext()
    {
        m_device_list = NULL;
    }

    /// Destructor
    ~CLAPI_clCreateContext()
    {
        if (m_device_list != NULL)
        {
            FreeArray(m_device_list);
        }
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetContextPropertiesString(m_properties, m_vecProperties) << s_strParamSeparator
           << m_num_devices << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_device_list, m_num_devices) << s_strParamSeparator
           << StringUtils::ToHexString(m_pfn_notify) << s_strParamSeparator
           << StringUtils::ToHexString(m_user_data) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateContext
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param properties Parameter for CLAPI_clCreateContext
    /// \param num_devices Parameter for CLAPI_clCreateContext
    /// \param device_list Parameter for CLAPI_clCreateContext
    /// \param pfn_notify Parameter for CLAPI_clCreateContext
    /// \param user_data Parameter for CLAPI_clCreateContext
    /// \param errcode_ret Parameter for CLAPI_clCreateContext
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        const cl_context_properties*  properties,
        cl_uint  num_devices,
        const cl_device_id*  device_list,
        void (CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*),
        void* user_data,
        cl_int*  errcode_ret,
        cl_context retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateContext;
        m_properties = properties;

        int num_properties = 0;

        if (properties != NULL)
        {
            // properties is 0 terminated
            while (properties[0] != 0 && num_properties < SP_MAX_NUM_CONTEXT_PROPERTIES)
            {
                m_vecProperties.push_back(properties[0]);
                properties++;
                num_properties++;
            }
        }

        if (num_properties == SP_MAX_NUM_CONTEXT_PROPERTIES)
        {
            //add a dummy value (zero) that tells GetContextPropertiesString that the list has been truncated
            m_vecProperties.push_back(0);
        }

        m_num_devices = num_devices;
        DeepCopyArray(&m_device_list, device_list, num_devices);
        this->m_pfn_notify = pfn_notify;
        m_user_data = user_data;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
        AddToInfoManager(m_retVal);
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateContext(const CLAPI_clCreateContext& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateContext& operator = (const CLAPI_clCreateContext& obj);

private:
    const cl_context_properties*       m_properties;     ///< parameter for clCreateContext
    std::vector<cl_context_properties> m_vecProperties;  ///< vector containing items defined in properties parameter
    cl_uint                            m_num_devices;    ///< parameter for clCreateContext
    cl_device_id*                      m_device_list;    ///< parameter for clCreateContext
    void (CL_CALLBACK* m_pfn_notify)(const char*, const void*, size_t, void*); ///< parameter for clCreateContext
    void*                              m_user_data;      ///< parameter for clCreateContext
    cl_int*                            m_errcode_ret;    ///< parameter for clCreateContext
    cl_int                             m_errcode_retVal; ///< parameter for clCreateContext
};

const int MAX_PRE_ALLOC_PROPS = 11;

//------------------------------------------------------------------------------------
/// clCreateContextFromType
//------------------------------------------------------------------------------------
class CLAPI_clCreateContextFromType : public CLAPI_clCreateContextBase
{
    FRIENDTESTCASE(clCreateContextFromType);
public:
    /// Constructor
    CLAPI_clCreateContextFromType()
    {
        m_vecProperties.reserve(MAX_PRE_ALLOC_PROPS);
    }

    /// Destructor
    ~CLAPI_clCreateContextFromType() {}

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetContextPropertiesString(m_properties, m_vecProperties) << s_strParamSeparator
           << CLStringUtils::GetDeviceTypeString(m_device_type) << s_strParamSeparator
           << StringUtils::ToHexString(pfn_notify) << s_strParamSeparator
           << StringUtils::ToHexString(m_user_data) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateContextFromType
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param properties Parameter for CLAPI_clCreateContextFromType
    /// \param device_type Parameter for CLAPI_clCreateContextFromType
    /// \param pfn_notify Parameter for CLAPI_clCreateContextFromType
    /// \param user_data Parameter for CLAPI_clCreateContextFromType
    /// \param errcode_ret Parameter for CLAPI_clCreateContextFromType
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        const cl_context_properties*  properties,
        cl_device_type device_type,
        void (CL_CALLBACK* pfn_notify1)(const char*, const void*, size_t, void*),
        void* user_data,
        cl_int*  errcode_ret,
        cl_context retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateContextFromType;
        m_properties = properties;

        int num_properties = 0;

        if (properties != NULL)
        {
            // properties is 0 terminated
            while (properties[0] != 0 && num_properties < SP_MAX_NUM_CONTEXT_PROPERTIES)
            {
                m_vecProperties.push_back(properties[0]);
                properties++;
                num_properties++;
            }
        }

        if (num_properties == SP_MAX_NUM_CONTEXT_PROPERTIES)
        {
            //add a dummy value (zero) that tells GetContextPropertiesString that the list has been truncated
            m_vecProperties.push_back(0);
        }

        m_device_type = device_type;
        this->pfn_notify = pfn_notify1;
        m_user_data = user_data;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
        AddToInfoManager(m_retVal);
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateContextFromType(const CLAPI_clCreateContextFromType& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateContextFromType& operator = (const CLAPI_clCreateContextFromType& obj);

private:
    const cl_context_properties*       m_properties;     ///< parameter for clCreateContextFromType
    cl_device_type                     m_device_type;    ///< parameter for clCreateContextFromType
    void (CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*); ///< parameter for clCreateContextFromType
    void*                              m_user_data;      ///< parameter for clCreateContextFromType
    cl_int*                            m_errcode_ret;    ///< parameter for clCreateContextFromType
    cl_int                             m_errcode_retVal; ///< parameter for clCreateContextFromType
    std::vector<cl_context_properties> m_vecProperties;  ///< vector containing items defined in properties parameter
};

//------------------------------------------------------------------------------------
/// clRetainContext
//------------------------------------------------------------------------------------
class CLAPI_clRetainContext : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clRetainContext() {}

    /// Destructor
    ~CLAPI_clRetainContext() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clRetainContext
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clRetainContext
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context  context,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clRetainContext;
        m_context = context;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clRetainContext(const CLAPI_clRetainContext& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clRetainContext& operator = (const CLAPI_clRetainContext& obj);

private:
    cl_context  m_context; ///< parameter for clRetainContext
    cl_int      m_retVal;  ///< return value
};

//------------------------------------------------------------------------------------
/// clReleaseContext
//------------------------------------------------------------------------------------
class CLAPI_clReleaseContext : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clReleaseContext() {}

    /// Destructor
    ~CLAPI_clReleaseContext() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clReleaseContext
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clReleaseContext
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context  context,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clReleaseContext;
        m_context = context;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clReleaseContext(const CLAPI_clReleaseContext& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clReleaseContext& operator = (const CLAPI_clReleaseContext& obj);

private:
    cl_context m_context; ///< parameter for clReleaseContext
    cl_int     m_retVal;  ///< return value
};

//------------------------------------------------------------------------------------
/// clGetContextInfo
//------------------------------------------------------------------------------------
class CLAPI_clGetContextInfo : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetContextInfo()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clGetContextInfo()
    {
        if (m_param_value != NULL)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetContextInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size << s_strParamSeparator
           << CLStringUtils::GetContextInfoValueString(m_param_name, RETVALMIN(m_param_value_size_retVal, m_param_value_size), m_param_value, m_retVal) << s_strParamSeparator
           << CLStringUtils::GetSizeString(REPLACEDNULLVAL(m_replaced_null_param, m_param_value_size_ret), m_param_value_size_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetContextInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clGetContextInfo
    /// \param param_name Parameter for CLAPI_clGetContextInfo
    /// \param param_value_size Parameter for CLAPI_clGetContextInfo
    /// \param param_value Parameter for CLAPI_clGetContextInfo
    /// \param param_value_size_ret Parameter for CLAPI_clGetContextInfo
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context  context,
        cl_context_info   param_name,
        size_t   param_value_size,
        void* param_value,
        size_t*  param_value_size_ret,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetContextInfo;
        m_context = context;
        m_param_name = param_name;
        m_param_value_size = param_value_size;
        m_param_value_size_ret = param_value_size_ret;
        m_replaced_null_param = replaced_null_param;
        m_param_value_size_retVal = *param_value_size_ret;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, RETVALMIN(m_param_value_size_retVal, m_param_value_size));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetContextInfo(const CLAPI_clGetContextInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetContextInfo& operator = (const CLAPI_clGetContextInfo& obj);

private:
    cl_context      m_context;                 ///< parameter for clGetContextInfo
    cl_context_info m_param_name;              ///< parameter for clGetContextInfo
    size_t          m_param_value_size;        ///< parameter for clGetContextInfo
    void*           m_param_value;             ///< parameter for clGetContextInfo
    size_t*         m_param_value_size_ret;    ///< parameter for clGetContextInfo
    size_t          m_param_value_size_retVal; ///< parameter for clGetContextInfo
    bool            m_replaced_null_param;     ///< flag indicating that we've provided a replacement for a NULL m_param_value_size_ret value
    cl_int          m_retVal;                  ///< return value
};

#define MAX_DEVICE_NAME_STR 256

//------------------------------------------------------------------------------------
/// clCreateCommandQueueBase -- base class for clCreateCommandQueue and clCreateCommandQueueWithProperties
//------------------------------------------------------------------------------------
class CLAPI_clCreateCommandQueueBase : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateCommandQueueBase()
    {
        m_uiQueueID = ms_NumInstance;
        ms_mtx.Lock();
        ms_NumInstance++;
        ms_mtx.Unlock();
        m_bUserSetProfileFlag = false;
    }

    /// Destructor
    ~CLAPI_clCreateCommandQueueBase() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// Get Instance num
    /// \return get num of instance created
    cl_uint GetInstanceNum() const
    {
        return ms_NumInstance;
    }

    /// Get instance id
    /// \return instance id
    cl_uint GetInstanceID() const
    {
        return m_uiQueueID;
    }

    /// m_bUserSetProfileFlag Accessor
    /// \return m_bUserSetProfileFlag
    bool UserSetProfileFlag() const
    {
        return m_bUserSetProfileFlag;
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << StringUtils::ToHexString(m_device) << s_strParamSeparator
           << GetPropertiesString() << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Get context of current queue
    /// \return cl_context
    const CLAPI_clCreateContextBase* GetCreateContextAPIObject() const
    {
        return m_createContextAPIObj;
    }

    /// Get the device name
    /// \return the device name
    const char* GetDeviceName() const
    {
        return m_szDevice;
    }

    /// Get the device type
    /// \return the device type
    cl_device_type GetDeviceType() const
    {
        return m_dtype;
    }

protected:
    /// Save the parameter values, return value and time stamps of CLAPI_clCreateCommandQueueBase
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param type the function type (clCreateCommandQueue/clCreateCommandQueueWithProperties)
    /// \param context Parameter for CLAPI_clCreateCommandQueueBase
    /// \param device Parameter for CLAPI_clCreateCommandQueueBase
    /// \param errcode_ret Parameter for CLAPI_clCreateCommandQueueBase
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context context,
        cl_device_id device,
        cl_int* errcode_ret,
        cl_command_queue retVal);

    /// Pure virtual function to get the string representation of the properties parameter
    /// \return the string representation of the properties parameter
    virtual std::string GetPropertiesString() = 0;

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateCommandQueueBase(const CLAPI_clCreateCommandQueueBase& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateCommandQueueBase& operator = (const CLAPI_clCreateCommandQueueBase& obj);

protected:
    cl_context                  m_context;                         ///< parameter for clCreateCommandQueue/clCreateCommandQueueWithProperties
    cl_device_id                m_device;                          ///< parameter for clCreateCommandQueue/clCreateCommandQueueWithProperties
    cl_int*                     m_errcode_ret;                     ///< parameter for clCreateCommandQueue/clCreateCommandQueueWithProperties
    cl_int                      m_errcode_retVal;                  ///< parameter for clCreateCommandQueue/clCreateCommandQueueWithProperties
    cl_command_queue            m_retVal;                          ///< return value
    cl_device_type              m_dtype;                           ///< device type
    char                        m_szDevice[MAX_DEVICE_NAME_STR];   ///< device name
    static cl_uint              ms_NumInstance;                    ///< number of instance created
    static AMDTMutex            ms_mtx;                            ///< mutex to protect m_sNumInstance
    cl_uint                     m_uiQueueID;                       ///< queue id
    const CLAPI_clCreateContextBase* m_createContextAPIObj;        ///< CreateContextAPIObject pointer
    bool                        m_bUserSetProfileFlag;             ///< A flag indicating whether or not user set CL_QUEUE_PROFILING_ENABLE when creating cmd queue
};

//------------------------------------------------------------------------------------
/// clCreateCommandQueue
//------------------------------------------------------------------------------------
class CLAPI_clCreateCommandQueue : public CLAPI_clCreateCommandQueueBase
{
    FRIENDTESTCASE(clCreateCommandQueue);
public:
    /// Save the parameter values, return value and time stamps of CLAPI_clCreateCommandQueue
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateCommandQueue
    /// \param device Parameter for CLAPI_clCreateCommandQueue
    /// \param properties the properties parameter for this API
    /// \param errcode_ret Parameter for CLAPI_clCreateCommandQueue
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context context,
        cl_device_id device,
        cl_command_queue_properties properties,
        cl_int* errcode_ret,
        cl_command_queue retVal);

protected:
    /// Gets the string representation of the properties parameter
    /// \return the string representation of the properties parameter
    std::string GetPropertiesString()
    {
        return CLStringUtils::GetCommandQueuePropertyString(m_properties);
    }

private:
    cl_command_queue_properties m_properties; ///< parameter for clCreateCommandQueue
};

//------------------------------------------------------------------------------------
/// clCreateCommandQueueWithProperties
//------------------------------------------------------------------------------------
class CLAPI_clCreateCommandQueueWithProperties : public CLAPI_clCreateCommandQueueBase
{
public:
    /// Save the parameter values, return value and time stamps of CLAPI_clCreateCommandQueue
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateCommandQueue
    /// \param device Parameter for CLAPI_clCreateCommandQueue
    /// \param pProperties the properties parameter for this API
    /// \param bUserSetProfileFlag flag indicating whether the CL_QUEUE_PROFILING_ENABLE property was set by the original API call
    /// \param properties the properties parameter for this API
    /// \param errcode_ret Parameter for CLAPI_clCreateCommandQueue
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context context,
        cl_device_id device,
        const cl_queue_properties* pProperties,
        bool bUserSetProfileFlag,
        cl_int* errcode_ret,
        cl_command_queue retVal);

protected:
    /// Gets the string representation of the properties parameter
    /// \return the string representation of the properties parameter
    std::string GetPropertiesString()
    {
        return CLStringUtils::GetCommandQueuePropertiesString(m_pProperties, m_vecProperties);
    }

private:
    const cl_queue_properties* m_pProperties;         ///< parameter for clCreateCommandQueue
    std::vector<cl_queue_properties> m_vecProperties; ///< vector containing items defined in properties parameter
};

//------------------------------------------------------------------------------------
/// clRetainCommandQueue
//------------------------------------------------------------------------------------
class CLAPI_clRetainCommandQueue : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clRetainCommandQueue() {}

    /// Destructor
    ~CLAPI_clRetainCommandQueue() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clRetainCommandQueue
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param command_queue Parameter for CLAPI_clRetainCommandQueue
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_command_queue  command_queue,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clRetainCommandQueue;
        m_command_queue = command_queue;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clRetainCommandQueue(const CLAPI_clRetainCommandQueue& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clRetainCommandQueue& operator = (const CLAPI_clRetainCommandQueue& obj);

private:
    cl_command_queue m_command_queue; ///< parameter for clRetainCommandQueue
    cl_int           m_retVal;        ///< return value
};

//------------------------------------------------------------------------------------
/// clReleaseCommandQueue
//------------------------------------------------------------------------------------
class CLAPI_clReleaseCommandQueue : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clReleaseCommandQueue() {}

    /// Destructor
    ~CLAPI_clReleaseCommandQueue() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clReleaseCommandQueue
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param command_queue Parameter for CLAPI_clReleaseCommandQueue
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_command_queue  command_queue,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clReleaseCommandQueue;
        m_command_queue = command_queue;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clReleaseCommandQueue(const CLAPI_clReleaseCommandQueue& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clReleaseCommandQueue& operator = (const CLAPI_clReleaseCommandQueue& obj);

private:
    cl_command_queue m_command_queue; ///< parameter for clReleaseCommandQueue
    cl_int           m_retVal;        ///< return value
};

//------------------------------------------------------------------------------------
/// clGetCommandQueueInfo
//------------------------------------------------------------------------------------
class CLAPI_clGetCommandQueueInfo : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetCommandQueueInfo()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clGetCommandQueueInfo()
    {
        if (m_param_value != NULL)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << CLStringUtils::GetCommandQueueInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size << s_strParamSeparator
           << CLStringUtils::GetCommandQueueInfoValueString(m_param_name, m_param_value, m_retVal) << s_strParamSeparator
           << CLStringUtils::GetSizeString(REPLACEDNULLVAL(m_replaced_null_param, m_param_value_size_ret), m_param_value_size_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetCommandQueueInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param command_queue Parameter for CLAPI_clGetCommandQueueInfo
    /// \param param_name Parameter for CLAPI_clGetCommandQueueInfo
    /// \param param_value_size Parameter for CLAPI_clGetCommandQueueInfo
    /// \param param_value Parameter for CLAPI_clGetCommandQueueInfo
    /// \param param_value_size_ret Parameter for CLAPI_clGetCommandQueueInfo
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_command_queue  command_queue,
        cl_command_queue_info   param_name,
        size_t   param_value_size,
        void* param_value,
        size_t*  param_value_size_ret,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetCommandQueueInfo;
        m_command_queue = command_queue;
        m_param_name = param_name;
        m_param_value_size = param_value_size;
        m_param_value_size_ret = param_value_size_ret;
        m_replaced_null_param = replaced_null_param;
        m_param_value_size_retVal = *param_value_size_ret;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, RETVALMIN(m_param_value_size_retVal, m_param_value_size));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetCommandQueueInfo(const CLAPI_clGetCommandQueueInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetCommandQueueInfo& operator = (const CLAPI_clGetCommandQueueInfo& obj);

private:
    cl_command_queue      m_command_queue;           ///< parameter for clGetCommandQueueInfo
    cl_command_queue_info m_param_name;              ///< parameter for clGetCommandQueueInfo
    size_t                m_param_value_size;        ///< parameter for clGetCommandQueueInfo
    void*                 m_param_value;             ///< parameter for clGetCommandQueueInfo
    size_t*               m_param_value_size_ret;    ///< parameter for clGetCommandQueueInfo
    size_t                m_param_value_size_retVal; ///< parameter for clGetCommandQueueInfo
    bool                  m_replaced_null_param;     ///< flag indicating that we've provided a replacement for a NULL m_param_value_size_ret value
    cl_int                m_retVal;                  ///< return value
};


//------------------------------------------------------------------------------------
/// clSetCommandQueueProperty
//------------------------------------------------------------------------------------
class CLAPI_clSetCommandQueueProperty : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clSetCommandQueueProperty() {}

    /// Destructor
    ~CLAPI_clSetCommandQueueProperty() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << CLStringUtils::GetCommandQueuePropertyString(m_properties) << s_strParamSeparator
           << CLStringUtils::GetBoolString(m_enable) << s_strParamSeparator
           << CLStringUtils::GetCommandQueuePropertiesString(m_old_properties, m_old_properties_val);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clSetCommandQueueProperty
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param command_queue Parameter for CLAPI_clSetCommandQueueProperty
    /// \param properties Parameter for CLAPI_clSetCommandQueueProperty
    /// \param enable Parameter for CLAPI_clSetCommandQueueProperty
    /// \param old_properties Parameter for CLAPI_clSetCommandQueueProperty
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_command_queue command_queue,
        cl_command_queue_properties properties,
        cl_bool enable,
        cl_command_queue_properties* old_properties,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clSetCommandQueueProperty;
        m_command_queue = command_queue;
        m_properties = properties;
        m_enable = enable;
        m_old_properties = old_properties;

        if (old_properties != NULL)
        {
            m_old_properties_val = *old_properties;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clSetCommandQueueProperty(const CLAPI_clSetCommandQueueProperty& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clSetCommandQueueProperty& operator = (const CLAPI_clSetCommandQueueProperty& obj);

private:
    cl_command_queue             m_command_queue;      ///< parameter for clSetCommandQueueProperty
    cl_command_queue_properties  m_properties;         ///< parameter for clSetCommandQueueProperty
    cl_bool                      m_enable;             ///< parameter for clSetCommandQueueProperty
    cl_command_queue_properties* m_old_properties;     ///< parameter for clSetCommandQueueProperty
    cl_command_queue_properties  m_old_properties_val; ///< parameter for clSetCommandQueueProperty
    cl_int                       m_retVal;             ///< return value
};


//------------------------------------------------------------------------------------
/// clCreateBuffer
//------------------------------------------------------------------------------------
class CLAPI_clCreateBuffer : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateBuffer() {}

    /// Destructor
    ~CLAPI_clCreateBuffer() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetMemFlagsString(m_flags) << s_strParamSeparator
           << m_size << s_strParamSeparator
           << StringUtils::ToHexString(m_host_ptr) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateBuffer
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateBuffer
    /// \param flags Parameter for CLAPI_clCreateBuffer
    /// \param size Parameter for CLAPI_clCreateBuffer
    /// \param host_ptr Parameter for CLAPI_clCreateBuffer
    /// \param errcode_ret Parameter for CLAPI_clCreateBuffer
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context  context,
        cl_mem_flags   flags,
        size_t   size,
        void* host_ptr,
        cl_int*  errcode_ret,
        cl_mem retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateBuffer;
        m_context = context;
        m_flags = flags;
        m_size = size;
        m_host_ptr = host_ptr;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateBuffer(const CLAPI_clCreateBuffer& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateBuffer& operator = (const CLAPI_clCreateBuffer& obj);

private:
    cl_context   m_context;        ///< parameter for clCreateBuffer
    cl_mem_flags m_flags;          ///< parameter for clCreateBuffer
    size_t       m_size;           ///< parameter for clCreateBuffer
    void*        m_host_ptr;       ///< parameter for clCreateBuffer
    cl_int*      m_errcode_ret;    ///< parameter for clCreateBuffer
    cl_int       m_errcode_retVal; ///< parameter for clCreateBuffer
    cl_mem       m_retVal;         ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateSubBuffer
//------------------------------------------------------------------------------------
class CLAPI_clCreateSubBuffer : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateSubBuffer()
    {
        m_buffer_create_info = NULL;
    }

    /// Destructor
    ~CLAPI_clCreateSubBuffer()
    {
        if (m_buffer_create_type == CL_BUFFER_CREATE_TYPE_REGION && m_buffer_create_info != NULL)
        {
            FreeBuffer(m_buffer_create_info);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_buffer) << s_strParamSeparator
           << CLStringUtils::GetMemFlagsString(m_flags) << s_strParamSeparator
           << CLStringUtils::GetBufferCreateString(m_buffer_create_type) << s_strParamSeparator
           << CLStringUtils::GetBufferInfoString(m_buffer_create_type, m_buffer_create_info) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateSubBuffer
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param buffer Parameter for CLAPI_clCreateSubBuffer
    /// \param flags Parameter for CLAPI_clCreateSubBuffer
    /// \param buffer_create_type Parameter for CLAPI_clCreateSubBuffer
    /// \param buffer_create_info Parameter for CLAPI_clCreateSubBuffer
    /// \param errcode_ret Parameter for CLAPI_clCreateSubBuffer
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_mem   buffer,
        cl_mem_flags   flags,
        cl_buffer_create_type   buffer_create_type,
        const void* buffer_create_info,
        cl_int*  errcode_ret,
        cl_mem retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateSubBuffer;
        m_buffer = buffer;
        m_flags = flags;
        m_buffer_create_type = buffer_create_type;

        if (m_buffer_create_type == CL_BUFFER_CREATE_TYPE_REGION)
        {
            DeepCopyBuffer(&m_buffer_create_info, buffer_create_info, sizeof(cl_buffer_region));
        }
        else
        {
            m_buffer_create_info = const_cast<void*>(buffer_create_info);
        }

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateSubBuffer(const CLAPI_clCreateSubBuffer& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateSubBuffer& operator = (const CLAPI_clCreateSubBuffer& obj);

private:
    cl_mem                m_buffer;             ///< parameter for clCreateSubBuffer
    cl_mem_flags          m_flags;              ///< parameter for clCreateSubBuffer
    cl_buffer_create_type m_buffer_create_type; ///< parameter for clCreateSubBuffer
    void*                 m_buffer_create_info; ///< parameter for clCreateSubBuffer
    cl_int*               m_errcode_ret;        ///< parameter for clCreateSubBuffer
    cl_int                m_errcode_retVal;     ///< parameter for clCreateSubBuffer
    cl_mem                m_retVal;             ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateImage2D
//------------------------------------------------------------------------------------
class CLAPI_clCreateImage2D : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateImage2D()
    {
        m_image_format = NULL;
    }

    /// Destructor
    ~CLAPI_clCreateImage2D()
    {
        if (m_image_format != NULL)
        {
            delete m_image_format;
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetMemFlagsString(m_flags) << s_strParamSeparator
           << CLStringUtils::GetImageFormatsString(m_image_format, 1) << s_strParamSeparator
           << m_image_width << s_strParamSeparator
           << m_image_height << s_strParamSeparator
           << m_image_row_pitch << s_strParamSeparator
           << StringUtils::ToHexString(m_host_ptr) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateImage2D
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateImage2D
    /// \param flags Parameter for CLAPI_clCreateImage2D
    /// \param image_format Parameter for CLAPI_clCreateImage2D
    /// \param image_width Parameter for CLAPI_clCreateImage2D
    /// \param image_height Parameter for CLAPI_clCreateImage2D
    /// \param image_row_pitch Parameter for CLAPI_clCreateImage2D
    /// \param host_ptr Parameter for CLAPI_clCreateImage2D
    /// \param errcode_ret Parameter for CLAPI_clCreateImage2D
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context  context,
        cl_mem_flags   flags,
        const cl_image_format*  image_format,
        size_t   image_width,
        size_t   image_height,
        size_t   image_row_pitch,
        void* host_ptr,
        cl_int*  errcode_ret,
        cl_mem retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateImage2D;
        m_context = context;
        m_flags = flags;
        //m_image_format = image_format;
        DeepCopyArray(&m_image_format, image_format, 1);
        m_image_width = image_width;
        m_image_height = image_height;
        m_image_row_pitch = image_row_pitch;
        m_host_ptr = host_ptr;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateImage2D(const CLAPI_clCreateImage2D& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateImage2D& operator = (const CLAPI_clCreateImage2D& obj);

private:
    cl_context       m_context;         ///< parameter for clCreateImage2D
    cl_mem_flags     m_flags;           ///< parameter for clCreateImage2D
    cl_image_format* m_image_format;    ///< parameter for clCreateImage2D
    size_t           m_image_width;     ///< parameter for clCreateImage2D
    size_t           m_image_height;    ///< parameter for clCreateImage2D
    size_t           m_image_row_pitch; ///< parameter for clCreateImage2D
    void*            m_host_ptr;        ///< parameter for clCreateImage2D
    cl_int*          m_errcode_ret;     ///< parameter for clCreateImage2D
    cl_int           m_errcode_retVal;  ///< parameter for clCreateImage2D
    cl_mem           m_retVal;          ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateImage3D
//------------------------------------------------------------------------------------
class CLAPI_clCreateImage3D : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateImage3D()
    {
        m_image_format = NULL;
    }

    /// Destructor
    ~CLAPI_clCreateImage3D()
    {
        if (m_image_format != NULL)
        {
            delete m_image_format;
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetMemFlagsString(m_flags) << s_strParamSeparator
           << CLStringUtils::GetImageFormatsString(m_image_format, 1) << s_strParamSeparator
           << m_image_width << s_strParamSeparator
           << m_image_height << s_strParamSeparator
           << m_image_depth << s_strParamSeparator
           << m_image_row_pitch << s_strParamSeparator
           << m_image_slice_pitch << s_strParamSeparator
           << StringUtils::ToHexString(m_host_ptr) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateImage3D
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateImage3D
    /// \param flags Parameter for CLAPI_clCreateImage3D
    /// \param image_format Parameter for CLAPI_clCreateImage3D
    /// \param image_width Parameter for CLAPI_clCreateImage3D
    /// \param image_height Parameter for CLAPI_clCreateImage3D
    /// \param image_depth Parameter for CLAPI_clCreateImage3D
    /// \param image_row_pitch Parameter for CLAPI_clCreateImage3D
    /// \param image_slice_pitch Parameter for CLAPI_clCreateImage3D
    /// \param host_ptr Parameter for CLAPI_clCreateImage3D
    /// \param errcode_ret Parameter for CLAPI_clCreateImage3D
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context  context,
        cl_mem_flags   flags,
        const cl_image_format*  image_format,
        size_t   image_width,
        size_t   image_height,
        size_t   image_depth,
        size_t   image_row_pitch,
        size_t   image_slice_pitch,
        void* host_ptr,
        cl_int*  errcode_ret,
        cl_mem retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateImage3D;
        m_context = context;
        m_flags = flags;
        //m_image_format = image_format;
        DeepCopyArray(&m_image_format, image_format, 1);
        m_image_width = image_width;
        m_image_height = image_height;
        m_image_depth = image_depth;
        m_image_row_pitch = image_row_pitch;
        m_image_slice_pitch = image_slice_pitch;
        m_host_ptr = host_ptr;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateImage3D(const CLAPI_clCreateImage3D& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateImage3D& operator = (const CLAPI_clCreateImage3D& obj);

private:
    cl_context       m_context;           ///< parameter for clCreateImage3D
    cl_mem_flags     m_flags;             ///< parameter for clCreateImage3D
    cl_image_format* m_image_format;      ///< parameter for clCreateImage3D
    size_t           m_image_width;       ///< parameter for clCreateImage3D
    size_t           m_image_height;      ///< parameter for clCreateImage3D
    size_t           m_image_depth;       ///< parameter for clCreateImage3D
    size_t           m_image_row_pitch;   ///< parameter for clCreateImage3D
    size_t           m_image_slice_pitch; ///< parameter for clCreateImage3D
    void*            m_host_ptr;          ///< parameter for clCreateImage3D
    cl_int*          m_errcode_ret;       ///< parameter for clCreateImage3D
    cl_int           m_errcode_retVal;    ///< parameter for clCreateImage3D
    cl_mem           m_retVal;            ///< return value
};

//------------------------------------------------------------------------------------
/// clRetainMemObject
//------------------------------------------------------------------------------------
class CLAPI_clRetainMemObject : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clRetainMemObject() {}

    /// Destructor
    ~CLAPI_clRetainMemObject() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_memobj);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clRetainMemObject
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param memobj Parameter for CLAPI_clRetainMemObject
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_mem   memobj,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clRetainMemObject;
        m_memobj = memobj;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clRetainMemObject(const CLAPI_clRetainMemObject& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clRetainMemObject& operator = (const CLAPI_clRetainMemObject& obj);

private:
    cl_mem   m_memobj; ///< parameter for clRetainMemObject
    cl_int   m_retVal; ///< return value
};

//------------------------------------------------------------------------------------
/// clReleaseMemObject
//------------------------------------------------------------------------------------
class CLAPI_clReleaseMemObject : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clReleaseMemObject() {}

    /// Destructor
    ~CLAPI_clReleaseMemObject() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_memobj);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clReleaseMemObject
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param memobj Parameter for CLAPI_clReleaseMemObject
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_mem   memobj,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clReleaseMemObject;
        m_memobj = memobj;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clReleaseMemObject(const CLAPI_clReleaseMemObject& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clReleaseMemObject& operator = (const CLAPI_clReleaseMemObject& obj);

private:
    cl_mem  m_memobj; ///< parameter for clReleaseMemObject
    cl_int  m_retVal; ///< return value
};

//------------------------------------------------------------------------------------
/// clGetSupportedImageFormats
//------------------------------------------------------------------------------------
class CLAPI_clGetSupportedImageFormats : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetSupportedImageFormats()
    {
        m_image_formats = NULL;
    }

    /// Destructor
    ~CLAPI_clGetSupportedImageFormats()
    {
        if (m_image_formats != NULL)
        {
            FreeArray(m_image_formats);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetMemFlagsString(m_flags) << s_strParamSeparator
           << CLStringUtils::GetMemObjectTypeString(m_image_type) << s_strParamSeparator
           << m_num_entries << s_strParamSeparator
           << CLStringUtils::GetImageFormatsString(m_image_formats, RETVALMIN(m_num_image_formatsVal, m_num_entries)) << s_strParamSeparator
           << CLStringUtils::GetIntString(REPLACEDNULLVAL(m_replaced_null_param, m_num_image_formats), m_num_image_formatsVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetSupportedImageFormats
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clGetSupportedImageFormats
    /// \param flags Parameter for CLAPI_clGetSupportedImageFormats
    /// \param image_type Parameter for CLAPI_clGetSupportedImageFormats
    /// \param num_entries Parameter for CLAPI_clGetSupportedImageFormats
    /// \param image_formats Parameter for CLAPI_clGetSupportedImageFormats
    /// \param num_image_formats Parameter for CLAPI_clGetSupportedImageFormats
    /// \param replaced_null_param flag indicating if the user app passed null to num_image_formats
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context  context,
        cl_mem_flags   flags,
        cl_mem_object_type   image_type,
        cl_uint  num_entries,
        cl_image_format*  image_formats,
        cl_uint* num_image_formats,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetSupportedImageFormats;
        m_context = context;
        m_flags = flags;
        m_image_type = image_type;
        m_num_entries = num_entries;
        m_num_image_formats = num_image_formats;
        m_replaced_null_param = replaced_null_param;

        if (retVal == CL_SUCCESS)
        {
            m_num_image_formatsVal = *num_image_formats;
            DeepCopyArray(&m_image_formats, image_formats, RETVALMIN(m_num_image_formatsVal, m_num_entries));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetSupportedImageFormats(const CLAPI_clGetSupportedImageFormats& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetSupportedImageFormats& operator = (const CLAPI_clGetSupportedImageFormats& obj);

private:
    cl_context         m_context;              ///< parameter for clGetSupportedImageFormats
    cl_mem_flags       m_flags;                ///< parameter for clGetSupportedImageFormats
    cl_mem_object_type m_image_type;           ///< parameter for clGetSupportedImageFormats
    cl_uint            m_num_entries;          ///< parameter for clGetSupportedImageFormats
    cl_image_format*   m_image_formats;        ///< parameter for clGetSupportedImageFormats
    cl_uint*           m_num_image_formats;    ///< parameter for clGetSupportedImageFormats
    cl_uint            m_num_image_formatsVal; ///< parameter for clGetSupportedImageFormats
    bool               m_replaced_null_param;  ///< flag indicating that we've provided a replacement for a NULL m_num_image_formats value
    cl_int             m_retVal;               ///< return value
};

//------------------------------------------------------------------------------------
/// clGetMemObjectInfo
//------------------------------------------------------------------------------------
class CLAPI_clGetMemObjectInfo : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetMemObjectInfo()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clGetMemObjectInfo()
    {
        if (m_param_value != NULL)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_memobj) << s_strParamSeparator
           << CLStringUtils::GetMemInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size << s_strParamSeparator
           << CLStringUtils::GetMemInfoValueString(m_param_name, m_param_value, m_retVal) << s_strParamSeparator
           << CLStringUtils::GetSizeString(REPLACEDNULLVAL(m_replaced_null_param, m_param_value_size_ret), m_param_value_size_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetMemObjectInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param memobj Parameter for CLAPI_clGetMemObjectInfo
    /// \param param_name Parameter for CLAPI_clGetMemObjectInfo
    /// \param param_value_size Parameter for CLAPI_clGetMemObjectInfo
    /// \param param_value Parameter for CLAPI_clGetMemObjectInfo
    /// \param param_value_size_ret Parameter for CLAPI_clGetMemObjectInfo
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_mem   memobj,
        cl_mem_info param_name,
        size_t   param_value_size,
        void* param_value,
        size_t*  param_value_size_ret,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetMemObjectInfo;
        m_memobj = memobj;
        m_param_name = param_name;
        m_param_value_size = param_value_size;
        m_param_value_size_ret = param_value_size_ret;
        m_replaced_null_param = replaced_null_param;
        m_param_value_size_retVal = *param_value_size_ret;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, RETVALMIN(m_param_value_size_retVal, m_param_value_size));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetMemObjectInfo(const CLAPI_clGetMemObjectInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetMemObjectInfo& operator = (const CLAPI_clGetMemObjectInfo& obj);

private:
    cl_mem      m_memobj;                  ///< parameter for clGetMemObjectInfo
    cl_mem_info m_param_name;              ///< parameter for clGetMemObjectInfo
    size_t      m_param_value_size;        ///< parameter for clGetMemObjectInfo
    void*       m_param_value;             ///< parameter for clGetMemObjectInfo
    size_t*     m_param_value_size_ret;    ///< parameter for clGetMemObjectInfo
    size_t      m_param_value_size_retVal; ///< parameter for clGetMemObjectInfo
    bool        m_replaced_null_param;     ///< flag indicating that we've provided a replacement for a NULL m_param_value_size_ret value
    cl_int      m_retVal;                  ///< return value
};

//------------------------------------------------------------------------------------
/// clGetImageInfo
//------------------------------------------------------------------------------------
class CLAPI_clGetImageInfo : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetImageInfo()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clGetImageInfo()
    {
        if (m_param_value != NULL)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_image) << s_strParamSeparator
           << CLStringUtils::GetImageInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size << s_strParamSeparator
           << CLStringUtils::GetImageInfoValueString(m_param_name, m_param_value, m_retVal) << s_strParamSeparator
           << CLStringUtils::GetSizeString(REPLACEDNULLVAL(m_replaced_null_param, m_param_value_size_ret), m_param_value_size_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetImageInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param image Parameter for CLAPI_clGetImageInfo
    /// \param param_name Parameter for CLAPI_clGetImageInfo
    /// \param param_value_size Parameter for CLAPI_clGetImageInfo
    /// \param param_value Parameter for CLAPI_clGetImageInfo
    /// \param param_value_size_ret Parameter for CLAPI_clGetImageInfo
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_mem   image,
        cl_image_info  param_name,
        size_t   param_value_size,
        void* param_value,
        size_t*  param_value_size_ret,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetImageInfo;
        m_image = image;
        m_param_name = param_name;
        m_param_value_size = param_value_size;
        m_param_value_size_ret = param_value_size_ret;
        m_replaced_null_param = replaced_null_param;
        m_param_value_size_retVal = *param_value_size_ret;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, RETVALMIN(m_param_value_size_retVal, m_param_value_size));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetImageInfo(const CLAPI_clGetImageInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetImageInfo& operator = (const CLAPI_clGetImageInfo& obj);

private:
    cl_mem        m_image;                   ///< parameter for clGetImageInfo
    cl_image_info m_param_name;              ///< parameter for clGetImageInfo
    size_t        m_param_value_size;        ///< parameter for clGetImageInfo
    void*         m_param_value;             ///< parameter for clGetImageInfo
    size_t*       m_param_value_size_ret;    ///< parameter for clGetImageInfo
    size_t        m_param_value_size_retVal; ///< parameter for clGetImageInfo
    bool          m_replaced_null_param;     ///< flag indicating that we've provided a replacement for a NULL m_param_value_size_ret value
    cl_int        m_retVal;                  ///< return value
};

//------------------------------------------------------------------------------------
/// clSetMemObjectDestructorCallback
//------------------------------------------------------------------------------------
class CLAPI_clSetMemObjectDestructorCallback : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clSetMemObjectDestructorCallback() {}

    /// Destructor
    ~CLAPI_clSetMemObjectDestructorCallback() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_memobj) << s_strParamSeparator
           << StringUtils::ToHexString(pfn_notify) << s_strParamSeparator
           << StringUtils::ToHexString(m_user_data);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clSetMemObjectDestructorCallback
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param memobj Parameter for CLAPI_clSetMemObjectDestructorCallback
    /// \param pfn_notify Parameter for CLAPI_clSetMemObjectDestructorCallback
    /// \param user_data Parameter for CLAPI_clSetMemObjectDestructorCallback
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_mem   memobj,
        void (CL_CALLBACK* pfn_notify1)(cl_mem memobj , void* user_data),
        void* user_data,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clSetMemObjectDestructorCallback;
        m_memobj = memobj;
        this->pfn_notify = pfn_notify1;
        m_user_data = user_data;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clSetMemObjectDestructorCallback(const CLAPI_clSetMemObjectDestructorCallback& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clSetMemObjectDestructorCallback& operator = (const CLAPI_clSetMemObjectDestructorCallback& obj);

private:
    cl_mem   m_memobj;    ///< parameter for clSetMemObjectDestructorCallback
    void (CL_CALLBACK* pfn_notify)(cl_mem memobj , void* user_data); ///< parameter for clSetMemObjectDestructorCallback
    void*    m_user_data; ///< parameter for clSetMemObjectDestructorCallback
    cl_int   m_retVal;    ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateSampler
//------------------------------------------------------------------------------------
class CLAPI_clCreateSampler : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateSampler() {}

    /// Destructor
    ~CLAPI_clCreateSampler() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetBoolString(m_normalized_coords) << s_strParamSeparator
           << CLStringUtils::GetAddressingModeString(m_addressing_mode) << s_strParamSeparator
           << CLStringUtils::GetFilterModeString(m_filter_mode) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateSampler
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateSampler
    /// \param normalized_coords Parameter for CLAPI_clCreateSampler
    /// \param addressing_mode Parameter for CLAPI_clCreateSampler
    /// \param filter_mode Parameter for CLAPI_clCreateSampler
    /// \param errcode_ret Parameter for CLAPI_clCreateSampler
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context  context,
        cl_bool  normalized_coords,
        cl_addressing_mode   addressing_mode,
        cl_filter_mode filter_mode,
        cl_int*  errcode_ret,
        cl_sampler retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateSampler;
        m_context = context;
        m_normalized_coords = normalized_coords;
        m_addressing_mode = addressing_mode;
        m_filter_mode = filter_mode;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateSampler(const CLAPI_clCreateSampler& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateSampler& operator = (const CLAPI_clCreateSampler& obj);

private:
    cl_context         m_context;           ///< parameter for clCreateSampler
    cl_bool            m_normalized_coords; ///< parameter for clCreateSampler
    cl_addressing_mode m_addressing_mode;   ///< parameter for clCreateSampler
    cl_filter_mode     m_filter_mode;       ///< parameter for clCreateSampler
    cl_int*            m_errcode_ret;       ///< parameter for clCreateSampler
    cl_int             m_errcode_retVal;    ///< parameter for clCreateSampler
    cl_sampler         m_retVal;            ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateSamplerWithProperties
//------------------------------------------------------------------------------------
class CLAPI_clCreateSamplerWithProperties : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateSamplerWithProperties() {}

    /// Destructor
    ~CLAPI_clCreateSamplerWithProperties() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetSamplerPropertiesString(m_pProperties, m_vecProperties) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateSamplerWithProperties
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateSamplerWithProperties
    /// \param pProperties Parameter for CLAPI_clCreateSamplerWithProperties
    /// \param errcode_ret Parameter for CLAPI_clCreateSamplerWithProperties
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context context,
        const cl_sampler_properties* pProperties,
        cl_int* errcode_ret,
        cl_sampler retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateSamplerWithProperties;
        m_context = context;
        m_pProperties = pProperties;

        int num_properties = 0;

        if (pProperties != NULL)
        {
            // properties is 0 terminated
            while (pProperties[0] != 0 && num_properties < SP_MAX_NUM_CONTEXT_PROPERTIES)
            {
                m_vecProperties.push_back(pProperties[0]);
                pProperties++;
                num_properties++;
            }
        }

        if (num_properties == SP_MAX_NUM_CONTEXT_PROPERTIES)
        {
            //add a dummy value (zero) that tells GetSamplerPropertiesString that the list has been truncated
            m_vecProperties.push_back(0);
        }

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateSamplerWithProperties(const CLAPI_clCreateSamplerWithProperties& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateSamplerWithProperties& operator = (const CLAPI_clCreateSamplerWithProperties& obj);

private:
    cl_context                         m_context;        ///< parameter for clCreateSamplerWithProperties
    const cl_sampler_properties*       m_pProperties;    ///< parameter for clCreateSamplerWithProperties
    std::vector<cl_sampler_properties> m_vecProperties;  ///< vector containing items defined in properties parameter
    cl_int*                            m_errcode_ret;    ///< parameter for clCreateSamplerWithProperties
    cl_int                             m_errcode_retVal; ///< parameter for clCreateSamplerWithProperties
    cl_sampler                         m_retVal;         ///< return value
};

//------------------------------------------------------------------------------------
/// clRetainSampler
//------------------------------------------------------------------------------------
class CLAPI_clRetainSampler : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clRetainSampler() {}

    /// Destructor
    ~CLAPI_clRetainSampler() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_sampler);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clRetainSampler
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param sampler Parameter for CLAPI_clRetainSampler
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_sampler  sampler,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clRetainSampler;
        m_sampler = sampler;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clRetainSampler(const CLAPI_clRetainSampler& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clRetainSampler& operator = (const CLAPI_clRetainSampler& obj);

private:
    cl_sampler m_sampler; ///< parameter for clRetainSampler
    cl_int     m_retVal;  ///< return value
};

//------------------------------------------------------------------------------------
/// clReleaseSampler
//------------------------------------------------------------------------------------
class CLAPI_clReleaseSampler : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clReleaseSampler() {}

    /// Destructor
    ~CLAPI_clReleaseSampler() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_sampler);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clReleaseSampler
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param sampler Parameter for CLAPI_clReleaseSampler
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_sampler  sampler,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clReleaseSampler;
        m_sampler = sampler;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clReleaseSampler(const CLAPI_clReleaseSampler& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clReleaseSampler& operator = (const CLAPI_clReleaseSampler& obj);

private:
    cl_sampler m_sampler; ///< parameter for clReleaseSampler
    cl_int     m_retVal;  ///< return value
};

//------------------------------------------------------------------------------------
/// clGetSamplerInfo
//------------------------------------------------------------------------------------
class CLAPI_clGetSamplerInfo : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetSamplerInfo()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clGetSamplerInfo()
    {
        if (m_param_value != NULL)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_sampler) << s_strParamSeparator
           << CLStringUtils::GetSamplerInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size << s_strParamSeparator
           << CLStringUtils::GetSamplerInfoValueString(m_param_name, m_param_value, m_retVal) << s_strParamSeparator
           << CLStringUtils::GetSizeString(REPLACEDNULLVAL(m_replaced_null_param, m_param_value_size_ret), m_param_value_size_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetSamplerInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param sampler Parameter for CLAPI_clGetSamplerInfo
    /// \param param_name Parameter for CLAPI_clGetSamplerInfo
    /// \param param_value_size Parameter for CLAPI_clGetSamplerInfo
    /// \param param_value Parameter for CLAPI_clGetSamplerInfo
    /// \param param_value_size_ret Parameter for CLAPI_clGetSamplerInfo
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_sampler  sampler,
        cl_sampler_info   param_name,
        size_t   param_value_size,
        void* param_value,
        size_t*  param_value_size_ret,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetSamplerInfo;
        m_sampler = sampler;
        m_param_name = param_name;
        m_param_value_size = param_value_size;
        m_param_value_size_ret = param_value_size_ret;
        m_replaced_null_param = replaced_null_param;
        m_param_value_size_retVal = *param_value_size_ret;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, RETVALMIN(m_param_value_size_retVal, m_param_value_size));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetSamplerInfo(const CLAPI_clGetSamplerInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetSamplerInfo& operator = (const CLAPI_clGetSamplerInfo& obj);

private:
    cl_sampler      m_sampler;                 ///< parameter for clGetSamplerInfo
    cl_sampler_info m_param_name;              ///< parameter for clGetSamplerInfo
    size_t          m_param_value_size;        ///< parameter for clGetSamplerInfo
    void*           m_param_value;             ///< parameter for clGetSamplerInfo
    size_t*         m_param_value_size_ret;    ///< parameter for clGetSamplerInfo
    size_t          m_param_value_size_retVal; ///< parameter for clGetSamplerInfo
    bool            m_replaced_null_param;     ///< flag indicating that we've provided a replacement for a NULL m_param_value_size_ret value
    cl_int          m_retVal;                  ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateProgramWithSource
//------------------------------------------------------------------------------------
class CLAPI_clCreateProgramWithSource : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateProgramWithSource() {}

    /// Destructor
    ~CLAPI_clCreateProgramWithSource() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << m_count << s_strParamSeparator
           << StringUtils::ToHexString(m_strings) << s_strParamSeparator
           << StringUtils::ToHexString(m_lengths) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateProgramWithSource
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateProgramWithSource
    /// \param count Parameter for CLAPI_clCreateProgramWithSource
    /// \param strings Parameter for CLAPI_clCreateProgramWithSource
    /// \param lengths Parameter for CLAPI_clCreateProgramWithSource
    /// \param errcode_ret Parameter for CLAPI_clCreateProgramWithSource
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context  context,
        cl_uint  count,
        const char**   strings,
        const size_t*  lengths,
        cl_int*  errcode_ret,
        cl_program retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateProgramWithSource;
        m_context = context;
        m_count = count;
        m_strings = strings;
        m_lengths = lengths;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateProgramWithSource(const CLAPI_clCreateProgramWithSource& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateProgramWithSource& operator = (const CLAPI_clCreateProgramWithSource& obj);

private:
    cl_context    m_context;        ///< parameter for clCreateProgramWithSource
    cl_uint       m_count;          ///< parameter for clCreateProgramWithSource
    const char**  m_strings;        ///< parameter for clCreateProgramWithSource
    const size_t* m_lengths;        ///< parameter for clCreateProgramWithSource
    cl_int*       m_errcode_ret;    ///< parameter for clCreateProgramWithSource
    cl_int        m_errcode_retVal; ///< parameter for clCreateProgramWithSource
    cl_program    m_retVal;         ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateProgramWithBinary
//------------------------------------------------------------------------------------
class CLAPI_clCreateProgramWithBinary : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateProgramWithBinary()
    {
        m_device_list = NULL;
    }

    /// Destructor
    ~CLAPI_clCreateProgramWithBinary()
    {
        if (m_device_list != NULL)
        {
            FreeArray(m_device_list);
        }

        if (m_lengths != NULL)
        {
            FreeArray(m_lengths);
        }

        if (m_binaries != NULL)
        {
            FreeArray(m_binaries);
        }

        if (m_binary_status != NULL)
        {
            FreeArray(m_binary_status);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << m_num_devices << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_device_list, m_num_devices) << s_strParamSeparator
           << CLStringUtils::GetSizeListString(m_lengths, m_num_devices) << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_binaries, m_num_devices) << s_strParamSeparator
           << CLStringUtils::GetErrorStrings(m_binary_status, m_num_devices) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateProgramWithBinary
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateProgramWithBinary
    /// \param num_devices Parameter for CLAPI_clCreateProgramWithBinary
    /// \param device_list Parameter for CLAPI_clCreateProgramWithBinary
    /// \param lengths Parameter for CLAPI_clCreateProgramWithBinary
    /// \param binaries Parameter for CLAPI_clCreateProgramWithBinary
    /// \param binary_status Parameter for CLAPI_clCreateProgramWithBinary
    /// \param errcode_ret Parameter for CLAPI_clCreateProgramWithBinary
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context  context,
        cl_uint  num_devices,
        const cl_device_id*  device_list,
        const size_t*  lengths,
        const unsigned char**   binaries,
        cl_int*  binary_status,
        cl_int*  errcode_ret,
        cl_program retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateProgramWithBinary;
        m_context = context;
        m_num_devices = num_devices;
        //m_device_list = device_list;
        DeepCopyArray(&m_device_list, device_list, num_devices);
        DeepCopyArray(&m_lengths, lengths, num_devices);
        DeepCopyArray(&m_binaries, binaries, num_devices);
        DeepCopyArray(&m_binary_status, binary_status, num_devices);

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateProgramWithBinary(const CLAPI_clCreateProgramWithBinary& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateProgramWithBinary& operator = (const CLAPI_clCreateProgramWithBinary& obj);

private:
    cl_context            m_context;        ///< parameter for clCreateProgramWithBinary
    cl_uint               m_num_devices;    ///< parameter for clCreateProgramWithBinary
    cl_device_id*         m_device_list;    ///< parameter for clCreateProgramWithBinary
    size_t*               m_lengths;        ///< parameter for clCreateProgramWithBinary
    const unsigned char** m_binaries;       ///< parameter for clCreateProgramWithBinary
    cl_int*               m_binary_status;  ///< parameter for clCreateProgramWithBinary
    cl_int*               m_errcode_ret;    ///< parameter for clCreateProgramWithBinary
    cl_int                m_errcode_retVal; ///< parameter for clCreateProgramWithBinary
    cl_program            m_retVal;         ///< return value
};

//------------------------------------------------------------------------------------
/// clRetainProgram
//------------------------------------------------------------------------------------
class CLAPI_clRetainProgram : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clRetainProgram() {}

    /// Destructor
    ~CLAPI_clRetainProgram() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_program);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clRetainProgram
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param program Parameter for CLAPI_clRetainProgram
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_program  program,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clRetainProgram;
        m_program = program;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clRetainProgram(const CLAPI_clRetainProgram& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clRetainProgram& operator = (const CLAPI_clRetainProgram& obj);

private:
    cl_program m_program; ///< parameter for clRetainProgram
    cl_int     m_retVal;  ///< return value
};

//------------------------------------------------------------------------------------
/// clReleaseProgram
//------------------------------------------------------------------------------------
class CLAPI_clReleaseProgram : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clReleaseProgram() {}

    /// Destructor
    ~CLAPI_clReleaseProgram() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_program);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clReleaseProgram
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param program Parameter for CLAPI_clReleaseProgram
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_program  program,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clReleaseProgram;
        m_program = program;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clReleaseProgram(const CLAPI_clReleaseProgram& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clReleaseProgram& operator = (const CLAPI_clReleaseProgram& obj);

private:
    cl_program m_program; ///< parameter for clReleaseProgram
    cl_int     m_retVal;  ///< return value
};

//------------------------------------------------------------------------------------
/// clBuildProgram
//------------------------------------------------------------------------------------
class CLAPI_clBuildProgram : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clBuildProgram()
    {
        m_device_list = NULL;
    }

    /// Destructor
    ~CLAPI_clBuildProgram()
    {
        if (m_device_list != NULL)
        {
            FreeArray(m_device_list);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_program) << s_strParamSeparator
           << m_num_devices << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_device_list, m_num_devices) << s_strParamSeparator
           << CLStringUtils::GetBuildOptionsString(m_strOptions, m_options, m_strOverriddenOptions, m_bOptionsAppended) << s_strParamSeparator
           << StringUtils::ToHexString(m_pfn_notify) << s_strParamSeparator
           << StringUtils::ToHexString(m_user_data);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clBuildProgram
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param program Parameter for CLAPI_clBuildProgram
    /// \param num_devices Parameter for CLAPI_clBuildProgram
    /// \param device_list Parameter for CLAPI_clBuildProgram
    /// \param options Parameter for CLAPI_clBuildProgram
    /// \param pfn_notify Parameter for CLAPI_clBuildProgram
    /// \param user_data Parameter for CLAPI_clBuildProgram
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_program  program,
        cl_uint  num_devices,
        const cl_device_id*  device_list,
        const char* options,
        void (CL_CALLBACK* pfn_notify1)(cl_program program , void* user_data),
        void* user_data,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clBuildProgram;
        m_program = program;
        m_num_devices = num_devices;
        //m_device_list = device_list;
        DeepCopyArray(&m_device_list, device_list, num_devices);

        // store the original pointer
        m_options = options;

        // check for env var that overrides the build options
        m_strOverriddenOptions = OSUtils::Instance()->GetEnvVar("AMD_OCL_BUILD_OPTIONS");

        // now check for env var that appends build options
        std::string strExtraBldOpts = OSUtils::Instance()->GetEnvVar("AMD_OCL_BUILD_OPTIONS_APPEND");
        m_bOptionsAppended = !strExtraBldOpts.empty();

        if (options != NULL)
        {
            m_strOptions = std::string(options);
            std::vector<std::string> switches;
            StringUtils::Split(switches, m_strOptions, " ", true, true);

            for (std::vector<std::string>::const_iterator it = switches.begin(); it != switches.end(); ++it)
            {
                if (*it == "-ignore-env")
                {
                    m_bOptionsAppended = false;
                    m_strOverriddenOptions = "";
                    break;
                }
            }

            if (m_bOptionsAppended)
            {
                m_strOverriddenOptions = m_strOptions;

                if (!m_strOverriddenOptions.empty())
                {
                    m_strOverriddenOptions += " ";
                }

                m_strOverriddenOptions += strExtraBldOpts;
            }
        }
        else
        {
            if (m_bOptionsAppended)
            {
                m_strOverriddenOptions = strExtraBldOpts;
            }
            else
            {
                m_strOptions = "";
            }
        }

        this->m_pfn_notify = pfn_notify1;
        m_user_data = user_data;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clBuildProgram(const CLAPI_clBuildProgram& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clBuildProgram& operator = (const CLAPI_clBuildProgram& obj);

private:
    cl_program    m_program;              ///< parameter for clBuildProgram
    cl_uint       m_num_devices;          ///< parameter for clBuildProgram
    cl_device_id* m_device_list;          ///< parameter for clBuildProgram
    const char*   m_options;              ///< parameter for clBuildProgram
    std::string   m_strOptions;           ///< std::string version of the options
    std::string   m_strOverriddenOptions; ///< the actual options used by clBuildProgram (may be different from m_strOptions if an env var is set)
    bool          m_bOptionsAppended;     ///< flag indicating if the overridden options were appended (vs. a full replacement)
    void (CL_CALLBACK* m_pfn_notify)(cl_program program , void* user_data); ///< parameter for clBuildProgram
    void*         m_user_data;            ///< parameter for clBuildProgram
    cl_int        m_retVal;               ///< return value
};

//------------------------------------------------------------------------------------
/// clUnloadCompiler
//------------------------------------------------------------------------------------
class CLAPI_clUnloadCompiler : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clUnloadCompiler() {}

    /// Destructor
    ~CLAPI_clUnloadCompiler() {}

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        return "";
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clUnloadCompiler
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clUnloadCompiler;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clUnloadCompiler(const CLAPI_clUnloadCompiler& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clUnloadCompiler& operator = (const CLAPI_clUnloadCompiler& obj);

private:
    cl_int m_retVal; ///< return value
};

//------------------------------------------------------------------------------------
/// clGetProgramInfo
//------------------------------------------------------------------------------------
class CLAPI_clGetProgramInfo : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetProgramInfo()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clGetProgramInfo()
    {
        if (m_param_value != NULL)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_program) << s_strParamSeparator
           << CLStringUtils::GetProgramInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size << s_strParamSeparator
           << CLStringUtils::GetProgramInfoValueString(m_param_name, RETVALMIN(m_param_value_size_retVal, m_param_value_size), m_param_value, m_retVal) << s_strParamSeparator
           << CLStringUtils::GetSizeString(REPLACEDNULLVAL(m_replaced_null_param, m_param_value_size_ret), m_param_value_size_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetProgramInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param program Parameter for CLAPI_clGetProgramInfo
    /// \param param_name Parameter for CLAPI_clGetProgramInfo
    /// \param param_value_size Parameter for CLAPI_clGetProgramInfo
    /// \param param_value Parameter for CLAPI_clGetProgramInfo
    /// \param param_value_size_ret Parameter for CLAPI_clGetProgramInfo
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_program  program,
        cl_program_info   param_name,
        size_t   param_value_size,
        void* param_value,
        size_t*  param_value_size_ret,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetProgramInfo;
        m_program = program;
        m_param_name = param_name;
        m_param_value_size = param_value_size;
        m_param_value_size_ret = param_value_size_ret;
        m_replaced_null_param = replaced_null_param;
        m_param_value_size_retVal = *param_value_size_ret;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, RETVALMIN(m_param_value_size_retVal, m_param_value_size));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetProgramInfo(const CLAPI_clGetProgramInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetProgramInfo& operator = (const CLAPI_clGetProgramInfo& obj);

private:
    cl_program      m_program;                 ///< parameter for clGetProgramInfo
    cl_program_info m_param_name;              ///< parameter for clGetProgramInfo
    size_t          m_param_value_size;        ///< parameter for clGetProgramInfo
    void*           m_param_value;             ///< parameter for clGetProgramInfo
    size_t*         m_param_value_size_ret;    ///< parameter for clGetProgramInfo
    size_t          m_param_value_size_retVal; ///< parameter for clGetProgramInfo
    bool            m_replaced_null_param;     ///< flag indicating that we've provided a replacement for a NULL m_param_value_size_ret value
    cl_int          m_retVal;                  ///< return value
};

//------------------------------------------------------------------------------------
/// clGetProgramBuildInfo
//------------------------------------------------------------------------------------
class CLAPI_clGetProgramBuildInfo : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetProgramBuildInfo()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clGetProgramBuildInfo()
    {
        if (m_param_value != NULL)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_program) << s_strParamSeparator
           << StringUtils::ToHexString(m_device) << s_strParamSeparator
           << CLStringUtils::GetProgramBuildInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size << s_strParamSeparator
           << StringUtils::ToHexString(m_param_value) << s_strParamSeparator
           << CLStringUtils::GetSizeString(REPLACEDNULLVAL(m_replaced_null_param, m_param_value_size_ret), m_param_value_size_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetProgramBuildInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param program Parameter for CLAPI_clGetProgramBuildInfo
    /// \param device Parameter for CLAPI_clGetProgramBuildInfo
    /// \param param_name Parameter for CLAPI_clGetProgramBuildInfo
    /// \param param_value_size Parameter for CLAPI_clGetProgramBuildInfo
    /// \param param_value Parameter for CLAPI_clGetProgramBuildInfo
    /// \param param_value_size_ret Parameter for CLAPI_clGetProgramBuildInfo
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_program  program,
        cl_device_id   device,
        cl_program_build_info   param_name,
        size_t   param_value_size,
        void* param_value,
        size_t*  param_value_size_ret,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetProgramBuildInfo;
        m_program = program;
        m_device = device;
        m_param_name = param_name;
        m_param_value_size = param_value_size;
        m_param_value = param_value;
        m_param_value_size_ret = param_value_size_ret;
        m_replaced_null_param = replaced_null_param;
        m_param_value_size_retVal = *param_value_size_ret;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, RETVALMIN(m_param_value_size_retVal, m_param_value_size));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetProgramBuildInfo(const CLAPI_clGetProgramBuildInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetProgramBuildInfo& operator = (const CLAPI_clGetProgramBuildInfo& obj);

private:
    cl_program            m_program;                 ///< parameter for clGetProgramBuildInfo
    cl_device_id          m_device;                  ///< parameter for clGetProgramBuildInfo
    cl_program_build_info m_param_name;              ///< parameter for clGetProgramBuildInfo
    size_t                m_param_value_size;        ///< parameter for clGetProgramBuildInfo
    void*                 m_param_value;             ///< parameter for clGetProgramBuildInfo
    size_t*               m_param_value_size_ret;    ///< parameter for clGetProgramBuildInfo
    size_t                m_param_value_size_retVal; ///< parameter for clGetProgramBuildInfo
    bool                  m_replaced_null_param;     ///< flag indicating that we've provided a replacement for a NULL m_param_value_size_ret value
    cl_int                m_retVal;                  ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateKernel
//------------------------------------------------------------------------------------
class CLAPI_clCreateKernel : public CLAPIBase
{
    FRIENDTESTCASE(clCreateKernel);
public:
    /// Constructor
    CLAPI_clCreateKernel() {}

    /// Destructor
    ~CLAPI_clCreateKernel() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_program) << s_strParamSeparator
           << CLStringUtils::GetQuotedString(m_str_kernel_name, m_kernel_name) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateKernel
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param program Parameter for CLAPI_clCreateKernel
    /// \param kernel_name Parameter for CLAPI_clCreateKernel
    /// \param errcode_ret Parameter for CLAPI_clCreateKernel
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_program  program,
        const char* kernel_name,
        cl_int*  errcode_ret,
        cl_kernel retVal);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateKernel(const CLAPI_clCreateKernel& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateKernel& operator = (const CLAPI_clCreateKernel& obj);

private:
    cl_program  m_program;         ///< parameter for clCreateKernel
    const char* m_kernel_name;     ///< parameter for clCreateKernel
    std::string m_str_kernel_name; ///< std:string version of the kernel name
    cl_int*     m_errcode_ret;     ///< parameter for clCreateKernel
    cl_int      m_errcode_retVal;  ///< parameter for clCreateKernel
    cl_kernel   m_retVal;          ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateKernelsInProgram
//------------------------------------------------------------------------------------
class CLAPI_clCreateKernelsInProgram : public CLAPIBase
{
    FRIENDTESTCASE(clCreateKernelsInProgram);
public:
    /// Constructor
    CLAPI_clCreateKernelsInProgram()
    {
        m_kernels = NULL;
    }

    /// Destructor
    ~CLAPI_clCreateKernelsInProgram()
    {
        if (m_kernels != NULL)
        {
            FreeArray(m_kernels);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_program) << s_strParamSeparator
           << m_num_kernels << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_kernels, RETVALMIN(m_num_kernels_retVal, m_num_kernels)) << s_strParamSeparator
           << CLStringUtils::GetIntString(REPLACEDNULLVAL(m_replaced_null_param, m_num_kernels_ret), m_num_kernels_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateKernelsInProgram
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param program Parameter for CLAPI_clCreateKernelsInProgram
    /// \param num_kernels Parameter for CLAPI_clCreateKernelsInProgram
    /// \param kernels Parameter for CLAPI_clCreateKernelsInProgram
    /// \param num_kernels_ret Parameter for CLAPI_clCreateKernelsInProgram
    /// \param replaced_null_param flag indicating if the user app passed null to num_kernels_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_program  program,
        cl_uint  num_kernels,
        cl_kernel*  kernels,
        cl_uint* num_kernels_ret,
        bool replaced_null_param,
        cl_int retVal);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateKernelsInProgram(const CLAPI_clCreateKernelsInProgram& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateKernelsInProgram& operator = (const CLAPI_clCreateKernelsInProgram& obj);

private:
    cl_program m_program;             ///< parameter for clCreateKernelsInProgram
    cl_uint    m_num_kernels;         ///< parameter for clCreateKernelsInProgram
    cl_kernel* m_kernels;             ///< parameter for clCreateKernelsInProgram
    cl_uint*   m_num_kernels_ret;     ///< parameter for clCreateKernelsInProgram
    cl_uint    m_num_kernels_retVal;  ///< parameter for clCreateKernelsInProgram
    bool       m_replaced_null_param; ///< flag indicating that we've provided a replacement for a NULL m_num_kernels_ret value
    cl_int     m_retVal;              ///< return value
};

//------------------------------------------------------------------------------------
/// clRetainKernel
//------------------------------------------------------------------------------------
class CLAPI_clRetainKernel : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clRetainKernel() {}

    /// Destructor
    ~CLAPI_clRetainKernel() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_kernel);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clRetainKernel
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param kernel Parameter for CLAPI_clRetainKernel
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_kernel   kernel,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clRetainKernel;
        m_kernel = kernel;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clRetainKernel(const CLAPI_clRetainKernel& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clRetainKernel& operator = (const CLAPI_clRetainKernel& obj);

private:
    cl_kernel m_kernel; ///< parameter for clRetainKernel
    cl_int    m_retVal; ///< return value
};

//------------------------------------------------------------------------------------
/// clReleaseKernel
//------------------------------------------------------------------------------------
class CLAPI_clReleaseKernel : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clReleaseKernel() {}

    /// Destructor
    ~CLAPI_clReleaseKernel() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_kernel);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clReleaseKernel
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param kernel Parameter for CLAPI_clReleaseKernel
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_kernel   kernel,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clReleaseKernel;
        m_kernel = kernel;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clReleaseKernel(const CLAPI_clReleaseKernel& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clReleaseKernel& operator = (const CLAPI_clReleaseKernel& obj);

private:
    cl_kernel m_kernel; ///< parameter for clReleaseKernel
    cl_int    m_retVal; ///< return value
};

//------------------------------------------------------------------------------------
/// clSetKernelArg
//------------------------------------------------------------------------------------
class CLAPI_clSetKernelArg : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clSetKernelArg()
    {
        m_arg_value = NULL;
    }

    /// Destructor
    ~CLAPI_clSetKernelArg()
    {
        if (m_arg_size == sizeof(int*) && m_arg_valueVal != NULL)
        {
            FreeBuffer(m_arg_valueVal);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_kernel) << s_strParamSeparator
           << m_arg_index << s_strParamSeparator
           << m_arg_size << s_strParamSeparator;

        if (m_arg_size == sizeof(int*) && m_arg_valueVal != NULL)
        {
            // see the comment in Create()
            if ((*((int*)m_arg_valueVal)) == 0)
            {
                // If dereferenced value is 0 it means it's not a cl object but a integer, output 0 instead of NULL
                ss << "[0]";
            }
            else
            {
                ss << '[' << StringUtils::ToHexString((*((int*)m_arg_valueVal))) << ']';
            }
        }
        else
        {
            ss << StringUtils::ToHexString(m_arg_value);
        }

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clSetKernelArg
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param kernel Parameter for CLAPI_clSetKernelArg
    /// \param arg_index Parameter for CLAPI_clSetKernelArg
    /// \param arg_size Parameter for CLAPI_clSetKernelArg
    /// \param arg_value Parameter for CLAPI_clSetKernelArg
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_kernel   kernel,
        cl_uint  arg_index,
        size_t   arg_size,
        const void* arg_value,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clSetKernelArg;
        m_kernel = kernel;
        m_arg_index = arg_index;
        m_arg_size = arg_size;
        m_arg_value = arg_value;

        if (m_arg_size == sizeof(int*) && retVal == CL_SUCCESS)
        {
            // If arg size = sizeof( pointer ), we want to dereference it since it helps user to debug.
            // e.g. check whether correct cl_mem object is binded.
            DeepCopyBuffer(&m_arg_valueVal, arg_value, m_arg_size);
        }
        else
        {
            m_arg_valueVal = NULL;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clSetKernelArg(const CLAPI_clSetKernelArg& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clSetKernelArg& operator = (const CLAPI_clSetKernelArg& obj);

private:
    cl_kernel   m_kernel;       ///< parameter for clSetKernelArg
    cl_uint     m_arg_index;    ///< parameter for clSetKernelArg
    size_t      m_arg_size;     ///< parameter for clSetKernelArg
    const void* m_arg_value;    ///< parameter for clSetKernelArg
    void*       m_arg_valueVal; ///< dereferenced value of m_arg_value
    cl_int      m_retVal;       ///< return value
};

//------------------------------------------------------------------------------------
/// clGetKernelInfo
//------------------------------------------------------------------------------------
class CLAPI_clGetKernelInfo : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetKernelInfo()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clGetKernelInfo()
    {
        if (m_param_value != NULL)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_kernel) << s_strParamSeparator
           << CLStringUtils::GetKernelInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size << s_strParamSeparator
           << CLStringUtils::GetKernelInfoValueString(m_param_name, m_param_value, m_retVal) << s_strParamSeparator
           << CLStringUtils::GetSizeString(REPLACEDNULLVAL(m_replaced_null_param, m_param_value_size_ret), m_param_value_size_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetKernelInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param kernel Parameter for CLAPI_clGetKernelInfo
    /// \param param_name Parameter for CLAPI_clGetKernelInfo
    /// \param param_value_size Parameter for CLAPI_clGetKernelInfo
    /// \param param_value Parameter for CLAPI_clGetKernelInfo
    /// \param param_value_size_ret Parameter for CLAPI_clGetKernelInfo
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_kernel   kernel,
        cl_kernel_info param_name,
        size_t   param_value_size,
        void* param_value,
        size_t*  param_value_size_ret,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetKernelInfo;
        m_kernel = kernel;
        m_param_name = param_name;
        m_param_value_size = param_value_size;
        m_param_value_size_ret = param_value_size_ret;
        m_replaced_null_param = replaced_null_param;
        m_param_value_size_retVal = *param_value_size_ret;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, RETVALMIN(m_param_value_size_retVal, m_param_value_size));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetKernelInfo(const CLAPI_clGetKernelInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetKernelInfo& operator = (const CLAPI_clGetKernelInfo& obj);

private:
    cl_kernel      m_kernel;                  ///< parameter for clGetKernelInfo
    cl_kernel_info m_param_name;              ///< parameter for clGetKernelInfo
    size_t         m_param_value_size;        ///< parameter for clGetKernelInfo
    void*          m_param_value;             ///< parameter for clGetKernelInfo
    size_t*        m_param_value_size_ret;    ///< parameter for clGetKernelInfo
    size_t         m_param_value_size_retVal; ///< parameter for clGetKernelInfo
    bool           m_replaced_null_param;     ///< flag indicating that we've provided a replacement for a NULL m_param_value_size_ret value
    cl_int         m_retVal;                  ///< return value
};

//------------------------------------------------------------------------------------
/// clGetKernelWorkGroupInfo
//------------------------------------------------------------------------------------
class CLAPI_clGetKernelWorkGroupInfo : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetKernelWorkGroupInfo()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clGetKernelWorkGroupInfo()
    {
        if (NULL != m_param_value)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_kernel) << s_strParamSeparator
           << StringUtils::ToHexString(m_device) << s_strParamSeparator
           << CLStringUtils::GetKernelWorkGroupInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size << s_strParamSeparator
           << CLStringUtils::GetKernelWorkGroupInfoValueString(m_param_name, m_param_value, m_retVal) << s_strParamSeparator
           << CLStringUtils::GetSizeString(REPLACEDNULLVAL(m_replaced_null_param, m_param_value_size_ret), m_param_value_size_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetKernelWorkGroupInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param kernel Parameter for CLAPI_clGetKernelWorkGroupInfo
    /// \param device Parameter for CLAPI_clGetKernelWorkGroupInfo
    /// \param param_name Parameter for CLAPI_clGetKernelWorkGroupInfo
    /// \param param_value_size Parameter for CLAPI_clGetKernelWorkGroupInfo
    /// \param param_value Parameter for CLAPI_clGetKernelWorkGroupInfo
    /// \param param_value_size_ret Parameter for CLAPI_clGetKernelWorkGroupInfo
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_kernel   kernel,
        cl_device_id   device,
        cl_kernel_work_group_info  param_name,
        size_t   param_value_size,
        void* param_value,
        size_t*  param_value_size_ret,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetKernelWorkGroupInfo;
        m_kernel = kernel;
        m_device = device;
        m_param_name = param_name;
        m_param_value_size = param_value_size;
        m_param_value_size_ret = param_value_size_ret;
        m_replaced_null_param = replaced_null_param;
        m_param_value_size_retVal = *param_value_size_ret;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, RETVALMIN(m_param_value_size_retVal, m_param_value_size));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetKernelWorkGroupInfo(const CLAPI_clGetKernelWorkGroupInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetKernelWorkGroupInfo& operator = (const CLAPI_clGetKernelWorkGroupInfo& obj);

private:
    cl_kernel                 m_kernel;                  ///< parameter for clGetKernelWorkGroupInfo
    cl_device_id              m_device;                  ///< parameter for clGetKernelWorkGroupInfo
    cl_kernel_work_group_info m_param_name;              ///< parameter for clGetKernelWorkGroupInfo
    size_t                    m_param_value_size;        ///< parameter for clGetKernelWorkGroupInfo
    void*                     m_param_value;             ///< parameter for clGetKernelWorkGroupInfo
    size_t*                   m_param_value_size_ret;    ///< parameter for clGetKernelWorkGroupInfo
    size_t                    m_param_value_size_retVal; ///< parameter for clGetKernelWorkGroupInfo
    bool                      m_replaced_null_param;     ///< flag indicating that we've provided a replacement for a NULL m_param_value_size_ret value
    cl_int                    m_retVal;                  ///< return value
};

//------------------------------------------------------------------------------------
/// clWaitForEvents
//------------------------------------------------------------------------------------
class CLAPI_clWaitForEvents : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clWaitForEvents()
    {
        m_event_list = NULL;
    }

    /// Destructor
    ~CLAPI_clWaitForEvents()
    {
        if (m_event_list != NULL)
        {
            FreeArray(m_event_list);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << m_num_events << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_event_list, m_num_events);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clWaitForEvents
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param num_events Parameter for CLAPI_clWaitForEvents
    /// \param event_list Parameter for CLAPI_clWaitForEvents
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_uint  num_events,
        const cl_event*   event_list,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clWaitForEvents;
        m_num_events = num_events;
        //m_event_list = event_list;
        DeepCopyArray(&m_event_list, event_list, num_events);
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clWaitForEvents(const CLAPI_clWaitForEvents& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clWaitForEvents& operator = (const CLAPI_clWaitForEvents& obj);

private:
    cl_uint   m_num_events; ///< parameter for clWaitForEvents
    cl_event* m_event_list; ///< parameter for clWaitForEvents
    cl_int    m_retVal;     ///< return value
};

//------------------------------------------------------------------------------------
/// clGetEventInfo
//------------------------------------------------------------------------------------
class CLAPI_clGetEventInfo : public CLAPIBase
{
public:
    static bool ms_collapseCalls;    ///< flag indicating whether or not consecutive identical CLAPI_clGetEventInfo instances should be collapsed
    size_t      m_consecutiveCount;  ///< number of consectuive identical calls to this API

    /// Constructor
    CLAPI_clGetEventInfo()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clGetEventInfo()
    {
        if (NULL != m_param_value)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_event) << s_strParamSeparator
           << CLStringUtils::GetEventInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size << s_strParamSeparator
           << CLStringUtils::GetEventInfoValueString(m_param_name, m_param_value, m_retVal) << s_strParamSeparator
           << CLStringUtils::GetSizeString(REPLACEDNULLVAL(m_replaced_null_param, m_param_value_size_ret), m_param_value_size_retVal);

        if (m_consecutiveCount > 1)
        {
            ss << " /*" << m_consecutiveCount << " consecutive calls*/";
        }

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetEventInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param event Parameter for CLAPI_clGetEventInfo
    /// \param param_name Parameter for CLAPI_clGetEventInfo
    /// \param param_value_size Parameter for CLAPI_clGetEventInfo
    /// \param param_value Parameter for CLAPI_clGetEventInfo
    /// \param param_value_size_ret Parameter for CLAPI_clGetEventInfo
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_event event,
        cl_event_info  param_name,
        size_t   param_value_size,
        void* param_value,
        size_t*  param_value_size_ret,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetEventInfo;
        m_event = event;
        m_param_name = param_name;
        m_param_value_size = param_value_size;
        m_param_value_size_ret = param_value_size_ret;
        m_replaced_null_param = replaced_null_param;
        m_param_value_size_retVal = *param_value_size_ret;
        m_consecutiveCount = 1;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, RETVALMIN(m_param_value_size_retVal, m_param_value_size));
        }

        m_retVal = retVal;
    }

    /// Checks if a given CLAPI_clGetEventInfo instance has the same parameters as this instance
    /// \param obj the CLAPI_clGetEventInfo instance to compare with
    /// \return true if obj has the same parameters as this instance
    bool SameParameters(const CLAPI_clGetEventInfo* obj) const
    {
        return (obj != NULL &&
                m_type == obj->m_type &&
                m_event == obj->m_event &&
                m_param_name == obj->m_param_name &&
                m_param_value_size == obj->m_param_value_size &&
                m_param_value != NULL && obj->m_param_value != NULL &&
                *(cl_int*)(m_param_value) == *(cl_int*)(obj->m_param_value) &&
                m_param_value_size_retVal == obj->m_param_value_size_retVal);
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetEventInfo(const CLAPI_clGetEventInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetEventInfo& operator = (const CLAPI_clGetEventInfo& obj);

private:
    cl_event      m_event;                   ///< parameter for clGetEventInfo
    cl_event_info m_param_name;              ///< parameter for clGetEventInfo
    size_t        m_param_value_size;        ///< parameter for clGetEventInfo
    void*         m_param_value;             ///< parameter for clGetEventInfo
    size_t*       m_param_value_size_ret;    ///< parameter for clGetEventInfo
    size_t        m_param_value_size_retVal; ///< parameter for clGetEventInfo
    bool          m_replaced_null_param;     ///< flag indicating that we've provided a replacement for a NULL m_param_value_size_ret value
    cl_int        m_retVal;                  ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateUserEvent
//------------------------------------------------------------------------------------
class CLAPI_clCreateUserEvent : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateUserEvent() {}

    /// Destructor
    ~CLAPI_clCreateUserEvent() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateUserEvent
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateUserEvent
    /// \param errcode_ret Parameter for CLAPI_clCreateUserEvent
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context  context,
        cl_int*  errcode_ret,
        cl_event retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateUserEvent;
        m_context = context;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateUserEvent(const CLAPI_clCreateUserEvent& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateUserEvent& operator = (const CLAPI_clCreateUserEvent& obj);

private:
    cl_context m_context;        ///< parameter for clCreateUserEvent
    cl_int*    m_errcode_ret;    ///< parameter for clCreateUserEvent
    cl_int     m_errcode_retVal; ///< parameter for clCreateUserEvent
    cl_event   m_retVal;         ///< return value
};

//------------------------------------------------------------------------------------
/// clRetainEvent
//------------------------------------------------------------------------------------
class CLAPI_clRetainEvent : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clRetainEvent() {}

    /// Destructor
    ~CLAPI_clRetainEvent() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clRetainEvent
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param event Parameter for CLAPI_clRetainEvent
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_event event,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clRetainEvent;
        m_event = event;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clRetainEvent(const CLAPI_clRetainEvent& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clRetainEvent& operator = (const CLAPI_clRetainEvent& obj);

private:
    cl_event m_event;  ///< parameter for clRetainEvent
    cl_int   m_retVal; ///< return value
};

//------------------------------------------------------------------------------------
/// clReleaseEvent
//------------------------------------------------------------------------------------
class CLAPI_clReleaseEvent : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clReleaseEvent() {}

    /// Destructor
    ~CLAPI_clReleaseEvent() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_event);
#ifdef _DEBUG_REF_COUNT_
        ss << "; Ref Counter = " << m_uiRefCount;
#endif
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clReleaseEvent
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param event Parameter for CLAPI_clReleaseEvent
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_event event,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clReleaseEvent;
        m_event = event;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clReleaseEvent(const CLAPI_clReleaseEvent& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clReleaseEvent& operator = (const CLAPI_clReleaseEvent& obj);

private:
    cl_event m_event;      ///< parameter for clReleaseEvent
    cl_int   m_retVal;     ///< return value
#ifdef _DEBUG_REF_COUNT_
public:
    cl_uint  m_uiRefCount; ///< member used when debugging reference count
#endif
};

//------------------------------------------------------------------------------------
/// clSetUserEventStatus
//------------------------------------------------------------------------------------
class CLAPI_clSetUserEventStatus : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clSetUserEventStatus() {}

    /// Destructor
    ~CLAPI_clSetUserEventStatus() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_event) << s_strParamSeparator
           << CLStringUtils::GetExecutionStatusString(m_execution_status);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clSetUserEventStatus
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param event Parameter for CLAPI_clSetUserEventStatus
    /// \param execution_status Parameter for CLAPI_clSetUserEventStatus
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_event event,
        cl_int   execution_status,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clSetUserEventStatus;
        m_event = event;
        m_execution_status = execution_status;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clSetUserEventStatus(const CLAPI_clSetUserEventStatus& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clSetUserEventStatus& operator = (const CLAPI_clSetUserEventStatus& obj);

private:
    cl_event m_event;            ///< parameter for clSetUserEventStatus
    cl_int   m_execution_status; ///< parameter for clSetUserEventStatus
    cl_int   m_retVal;           ///< return value
};

//------------------------------------------------------------------------------------
/// clSetEventCallback
//------------------------------------------------------------------------------------
class CLAPI_clSetEventCallback : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clSetEventCallback() {}

    /// Destructor
    ~CLAPI_clSetEventCallback() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_event) << s_strParamSeparator
           << CLStringUtils::GetExecutionStatusString(m_command_exec_callback_type) << s_strParamSeparator
           << StringUtils::ToHexString(m_pfn_notify) << s_strParamSeparator
           << StringUtils::ToHexString(m_user_data);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clSetEventCallback
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param event Parameter for CLAPI_clSetEventCallback
    /// \param command_exec_callback_type Parameter for CLAPI_clSetEventCallback
    /// \param pfn_notify Parameter for CLAPI_clSetEventCallback
    /// \param user_data Parameter for CLAPI_clSetEventCallback
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_event event,
        cl_int   command_exec_callback_type,
        void (CL_CALLBACK* pfn_notify)(cl_event, cl_int, void*),
        void* user_data,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clSetEventCallback;
        m_event = event;
        m_command_exec_callback_type = command_exec_callback_type;
        this->m_pfn_notify = pfn_notify;
        m_user_data = user_data;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clSetEventCallback(const CLAPI_clSetEventCallback& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clSetEventCallback& operator = (const CLAPI_clSetEventCallback& obj);

private:
    cl_event m_event;                      ///< parameter for clSetEventCallback
    cl_int   m_command_exec_callback_type; ///< parameter for clSetEventCallback
    void (CL_CALLBACK* m_pfn_notify)(cl_event, cl_int, void*); ///< parameter for clSetEventCallback
    void*    m_user_data;                  ///< parameter for clSetEventCallback
    cl_int   m_retVal;                     ///< return value
};

//------------------------------------------------------------------------------------
/// clGetEventProfilingInfo
//------------------------------------------------------------------------------------
class CLAPI_clGetEventProfilingInfo : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetEventProfilingInfo()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clGetEventProfilingInfo()
    {
        if (m_param_value != NULL)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_event) << s_strParamSeparator
           << CLStringUtils::GetProfilingInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size << s_strParamSeparator
           << CLStringUtils::GetProfilingInfoValueString(m_param_name, m_param_value, m_retVal) << s_strParamSeparator
           << CLStringUtils::GetSizeString(REPLACEDNULLVAL(m_replaced_null_param, m_param_value_size_ret), m_param_value_size_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetEventProfilingInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param event Parameter for CLAPI_clGetEventProfilingInfo
    /// \param param_name Parameter for CLAPI_clGetEventProfilingInfo
    /// \param param_value_size Parameter for CLAPI_clGetEventProfilingInfo
    /// \param param_value Parameter for CLAPI_clGetEventProfilingInfo
    /// \param param_value_size_ret Parameter for CLAPI_clGetEventProfilingInfo
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_event event,
        cl_profiling_info param_name,
        size_t   param_value_size,
        void* param_value,
        size_t*  param_value_size_ret,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetEventProfilingInfo;
        m_event = event;
        m_param_name = param_name;
        m_param_value_size = param_value_size;
        m_param_value_size_ret = param_value_size_ret;
        m_replaced_null_param = replaced_null_param;
        m_param_value_size_retVal = *param_value_size_ret;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, RETVALMIN(m_param_value_size_retVal, m_param_value_size));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetEventProfilingInfo(const CLAPI_clGetEventProfilingInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetEventProfilingInfo& operator = (const CLAPI_clGetEventProfilingInfo& obj);

private:
    cl_event          m_event;                   ///< parameter for clGetEventProfilingInfo
    cl_profiling_info m_param_name;              ///< parameter for clGetEventProfilingInfo
    size_t            m_param_value_size;        ///< parameter for clGetEventProfilingInfo
    void*             m_param_value;             ///< parameter for clGetEventProfilingInfo
    size_t*           m_param_value_size_ret;    ///< parameter for clGetEventProfilingInfo
    size_t            m_param_value_size_retVal; ///< parameter for clGetEventProfilingInfo
    bool              m_replaced_null_param;     ///< flag indicating that we've provided a replacement for a NULL m_param_value_size_ret value
    cl_int            m_retVal;                  ///< return value
};

//------------------------------------------------------------------------------------
/// clFlush
//------------------------------------------------------------------------------------
class CLAPI_clFlush : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clFlush() {}

    /// Destructor
    ~CLAPI_clFlush() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clFlush
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param command_queue Parameter for CLAPI_clFlush
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_command_queue  command_queue,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clFlush;
        m_command_queue = command_queue;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clFlush(const CLAPI_clFlush& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clFlush& operator = (const CLAPI_clFlush& obj);

private:
    cl_command_queue m_command_queue; ///< parameter for clFlush
    cl_int           m_retVal;        ///< return value
};

//------------------------------------------------------------------------------------
/// clFinish
//------------------------------------------------------------------------------------
class CLAPI_clFinish : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clFinish() {}

    /// Destructor
    ~CLAPI_clFinish() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clFinish
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param command_queue Parameter for CLAPI_clFinish
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_command_queue  command_queue,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clFinish;
        m_command_queue = command_queue;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clFinish(const CLAPI_clFinish& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clFinish& operator = (const CLAPI_clFinish& obj);

private:
    cl_command_queue m_command_queue; ///< parameter for clFinish
    cl_int           m_retVal;        ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateFromGLBuffer
//------------------------------------------------------------------------------------
class CLAPI_clCreateFromGLBuffer : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateFromGLBuffer() {}

    /// Destructor
    ~CLAPI_clCreateFromGLBuffer() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetMemFlagsString(m_flags) << s_strParamSeparator
           << m_bufobj << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateFromGLBuffer
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateFromGLBuffer
    /// \param flags Parameter for CLAPI_clCreateFromGLBuffer
    /// \param bufobj buffer object
    /// \param errcode_ret Parameter for CLAPI_clCreateFromGLBuffer
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context context,
        cl_mem_flags flags,
        cl_GLuint bufobj,
        cl_int* errcode_ret,
        cl_mem retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateFromGLBuffer;
        m_context = context;
        m_flags = flags;
        m_bufobj = bufobj;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateFromGLBuffer(const CLAPI_clCreateFromGLBuffer& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateFromGLBuffer& operator = (const CLAPI_clCreateFromGLBuffer& obj);

private:
    cl_context   m_context;        ///< parameter for clCreateFromGLBuffer
    cl_mem_flags m_flags;          ///< parameter for clCreateFromGLBuffer
    cl_GLuint    m_bufobj;         ///< parameter for clCreateFromGLBuffer
    cl_int*      m_errcode_ret;    ///< parameter for clCreateFromGLBuffer
    cl_int       m_errcode_retVal; ///< parameter for clCreateFromGLBuffer
    cl_mem       m_retVal;         ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateFromGLTexture2D
//------------------------------------------------------------------------------------
class CLAPI_clCreateFromGLTexture2D : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateFromGLTexture2D() {}

    /// Destructor
    ~CLAPI_clCreateFromGLTexture2D() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetMemFlagsString(m_flags) << s_strParamSeparator
           << CLStringUtils::GetGLTextureTargetString(m_texture_target) << s_strParamSeparator
           << m_miplevel << s_strParamSeparator
           << m_texture << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateFromGLTexture2D
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateFromGLTexture2D
    /// \param flags Parameter for CLAPI_clCreateFromGLTexture2D
    /// \param texture_target texture target
    /// \param miplevel mip level
    /// \param texture texture id
    /// \param errcode_ret Parameter for CLAPI_clCreateFromGLTexture2D
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context context,
        cl_mem_flags flags,
        cl_GLenum texture_target,
        cl_GLint miplevel,
        cl_GLuint texture,
        cl_int* errcode_ret,
        cl_mem retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateFromGLTexture2D;
        m_context = context;
        m_flags = flags;
        m_texture_target = texture_target;
        m_miplevel = miplevel;
        m_texture = texture;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateFromGLTexture2D(const CLAPI_clCreateFromGLTexture2D& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateFromGLTexture2D& operator = (const CLAPI_clCreateFromGLTexture2D& obj);

private:
    cl_context   m_context;        ///< parameter for clCreateFromGLTexture2D
    cl_mem_flags m_flags;          ///< parameter for clCreateFromGLTexture2D
    cl_GLenum    m_texture_target; ///< parameter for clCreateFromGLTexture2D
    cl_GLint     m_miplevel;       ///< parameter for clCreateFromGLTexture2D
    cl_GLuint    m_texture;        ///< parameter for clCreateFromGLTexture2D
    cl_int*      m_errcode_ret;    ///< parameter for clCreateFromGLTexture2D
    cl_int       m_errcode_retVal; ///< parameter for clCreateFromGLTexture2D
    cl_mem       m_retVal;         ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateFromGLTexture3D
//------------------------------------------------------------------------------------
class CLAPI_clCreateFromGLTexture3D : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateFromGLTexture3D() {}

    /// Destructor
    ~CLAPI_clCreateFromGLTexture3D() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetMemFlagsString(m_flags) << s_strParamSeparator
           << CLStringUtils::GetGLTextureTargetString(m_texture_target) << s_strParamSeparator
           << m_miplevel << s_strParamSeparator
           << m_texture << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateFromGLTexture3D
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateFromGLTexture3D
    /// \param flags Parameter for CLAPI_clCreateFromGLTexture3D
    /// \param texture_target texture target
    /// \param miplevel mipmap level
    /// \param texture texture id
    /// \param errcode_ret Parameter for CLAPI_clCreateFromGLTexture3D
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context context,
        cl_mem_flags flags,
        cl_GLenum texture_target,
        cl_GLint miplevel,
        cl_GLuint texture,
        cl_int* errcode_ret,
        cl_mem retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateFromGLTexture3D;
        m_context = context;
        m_flags = flags;
        m_texture_target = texture_target;
        m_miplevel = miplevel;
        m_texture = texture;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateFromGLTexture3D(const CLAPI_clCreateFromGLTexture3D& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateFromGLTexture3D& operator = (const CLAPI_clCreateFromGLTexture3D& obj);

private:
    cl_context   m_context;        ///< parameter for clCreateFromGLTexture3D
    cl_mem_flags m_flags;          ///< parameter for clCreateFromGLTexture3D
    cl_GLenum    m_texture_target; ///< parameter for clCreateFromGLTexture3D
    cl_GLint     m_miplevel;       ///< parameter for clCreateFromGLTexture3D
    cl_GLuint    m_texture;        ///< parameter for clCreateFromGLTexture3D
    cl_int*      m_errcode_ret;    ///< parameter for clCreateFromGLTexture3D
    cl_int       m_errcode_retVal; ///< parameter for clCreateFromGLTexture3D
    cl_mem       m_retVal;         ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateFromGLRenderbuffer
//------------------------------------------------------------------------------------
class CLAPI_clCreateFromGLRenderbuffer : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateFromGLRenderbuffer() {}

    /// Destructor
    ~CLAPI_clCreateFromGLRenderbuffer() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetMemFlagsString(m_flags) << s_strParamSeparator
           << m_renderbuffer << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateFromGLRenderbuffer
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateFromGLRenderbuffer
    /// \param flags Parameter for CLAPI_clCreateFromGLRenderbuffer
    /// \param renderbuffer render buffer
    /// \param errcode_ret Parameter for CLAPI_clCreateFromGLRenderbuffer
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context context,
        cl_mem_flags flags,
        cl_GLuint renderbuffer,
        cl_int* errcode_ret,
        cl_mem retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateFromGLRenderbuffer;
        m_context = context;
        m_flags = flags;
        m_renderbuffer = renderbuffer;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateFromGLRenderbuffer(const CLAPI_clCreateFromGLRenderbuffer& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateFromGLRenderbuffer& operator = (const CLAPI_clCreateFromGLRenderbuffer& obj);

private:
    cl_context   m_context;        ///< parameter for clCreateFromGLRenderbuffer
    cl_mem_flags m_flags;          ///< parameter for clCreateFromGLRenderbuffer
    cl_GLuint    m_renderbuffer;   ///< parameter for clCreateFromGLRenderbuffer
    cl_int*      m_errcode_ret;    ///< parameter for clCreateFromGLRenderbuffer
    cl_int       m_errcode_retVal; ///< parameter for clCreateFromGLRenderbuffer
    cl_mem       m_retVal;         ///< return value
};

//------------------------------------------------------------------------------------
/// clGetGLObjectInfo
//------------------------------------------------------------------------------------
class CLAPI_clGetGLObjectInfo : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetGLObjectInfo() {}

    /// Destructor
    ~CLAPI_clGetGLObjectInfo() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return CLStringUtils::GetErrorString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_memobj) << s_strParamSeparator
           << CLStringUtils::GetGLObjectTypeString(m_gl_object_type, m_gl_object_typeVal) << s_strParamSeparator
           << CLStringUtils::GetIntString(m_gl_object_name, m_gl_object_nameVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetGLObjectInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param memobj memory object
    /// \param gl_object_type gl object type
    /// \param gl_object_name gl object name
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_mem memobj,
        cl_gl_object_type* gl_object_type,
        cl_GLuint* gl_object_name,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetGLObjectInfo;
        m_memobj = memobj;
        m_gl_object_type = gl_object_type;

        if (gl_object_type != NULL)
        {
            m_gl_object_typeVal = *gl_object_type;
        }
        else
        {
            m_gl_object_typeVal = 0;
        }

        m_gl_object_name = gl_object_name;

        if (gl_object_name != NULL)
        {
            m_gl_object_nameVal = *gl_object_name;
        }
        else
        {
            m_gl_object_nameVal = 0;
        }


        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetGLObjectInfo(const CLAPI_clGetGLObjectInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetGLObjectInfo& operator = (const CLAPI_clGetGLObjectInfo& obj);

private:
    cl_mem             m_memobj;            ///< parameter for clGetGLObjectInfo
    cl_gl_object_type* m_gl_object_type;    ///< parameter for clGetGLObjectInfo
    cl_gl_object_type  m_gl_object_typeVal; ///< parameter for clGetGLObjectInfo
    cl_GLuint*         m_gl_object_name;    ///< parameter for clGetGLObjectInfo
    cl_GLuint          m_gl_object_nameVal; ///< parameter for clGetGLObjectInfo
    cl_int             m_retVal;            ///< return value
};

//------------------------------------------------------------------------------------
/// clGetGLTextureInfo
//------------------------------------------------------------------------------------
class CLAPI_clGetGLTextureInfo : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetGLTextureInfo()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clGetGLTextureInfo()
    {
        if (m_param_value != NULL)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return CLStringUtils::GetErrorString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_memobj) << s_strParamSeparator
           << CLStringUtils::GetGLTextureInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size << s_strParamSeparator
           << CLStringUtils::GetGLTextureInfoValueString(m_param_name, m_param_value, m_retVal) << s_strParamSeparator
           << CLStringUtils::GetSizeString(REPLACEDNULLVAL(m_replaced_null_param, m_param_value_size_ret), m_param_value_size_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetGLTextureInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param memobj Parameter for CLAPI_clGetGLTextureInfo
    /// \param param_name Parameter for CLAPI_clGetGLTextureInfo
    /// \param param_value_size Parameter for CLAPI_clGetGLTextureInfo
    /// \param param_value Parameter for CLAPI_clGetGLTextureInfo
    /// \param param_value_size_ret Parameter for CLAPI_clGetGLTextureInfo
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_mem   memobj,
        cl_gl_texture_info param_name,
        size_t   param_value_size,
        void* param_value,
        size_t*  param_value_size_ret,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetGLTextureInfo;
        m_memobj = memobj;
        m_param_name = param_name;
        m_param_value_size = param_value_size;
        m_param_value_size_ret = param_value_size_ret;
        m_replaced_null_param = replaced_null_param;
        m_param_value_size_retVal = *param_value_size_ret;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, RETVALMIN(m_param_value_size_retVal, m_param_value_size));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetGLTextureInfo(const CLAPI_clGetGLTextureInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetGLTextureInfo& operator = (const CLAPI_clGetGLTextureInfo& obj);

private:
    cl_mem             m_memobj;                  ///< parameter for clGetGLTextureInfo
    cl_gl_texture_info m_param_name;              ///< parameter for clGetGLTextureInfo
    size_t             m_param_value_size;        ///< parameter for clGetGLTextureInfo
    void*              m_param_value;             ///< parameter for clGetGLTextureInfo
    size_t*            m_param_value_size_ret;    ///< parameter for clGetGLTextureInfo
    size_t             m_param_value_size_retVal; ///< parameter for clGetGLTextureInfo
    bool               m_replaced_null_param;     ///< flag indicating that we've provided a replacement for a NULL m_param_value_size_ret value
    cl_int             m_retVal;                  ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateEventFromGLsyncKHR
//------------------------------------------------------------------------------------
class CLAPI_clCreateEventFromGLsyncKHR : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateEventFromGLsyncKHR() {}

    /// Destructor
    ~CLAPI_clCreateEventFromGLsyncKHR() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << m_sync << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateEventFromGLsyncKHR
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateEventFromGLsyncKHR
    /// \param sync sync obj
    /// \param errcode_ret Parameter for CLAPI_clCreateEventFromGLsyncKHR
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context  context,
        cl_GLsync sync,
        cl_int*  errcode_ret,
        cl_event retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateEventFromGLsyncKHR;
        m_context = context;
        m_sync = sync;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateEventFromGLsyncKHR(const CLAPI_clCreateEventFromGLsyncKHR& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateEventFromGLsyncKHR& operator = (const CLAPI_clCreateEventFromGLsyncKHR& obj);

private:
    cl_context m_context;        ///< parameter for clCreateEventFromGLsyncKHR
    cl_GLsync  m_sync;           ///< parameter for clCreateEventFromGLsyncKHR
    cl_int*    m_errcode_ret;    ///< parameter for clCreateEventFromGLsyncKHR
    cl_int     m_errcode_retVal; ///< parameter for clCreateEventFromGLsyncKHR
    cl_event   m_retVal;         ///< return value
};

//------------------------------------------------------------------------------------
/// clGetGLContextInfoKHR
//------------------------------------------------------------------------------------
class CLAPI_clGetGLContextInfoKHR : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetGLContextInfoKHR()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clGetGLContextInfoKHR()
    {
        if (m_param_value != NULL)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return CLStringUtils::GetErrorString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetContextPropertiesString(m_properties, m_vecProperties) << s_strParamSeparator
           << CLStringUtils::GetGLContextInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size << s_strParamSeparator
           << CLStringUtils::GetGLContextInfoValueString(m_param_name, RETVALMIN(m_param_value_size_retVal, m_param_value_size), m_param_value, m_retVal) << s_strParamSeparator
           << CLStringUtils::GetSizeString(REPLACEDNULLVAL(m_replaced_null_param, m_param_value_size_ret), m_param_value_size_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetGLContextInfoKHR
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param properties context properties
    /// \param param_name Parameter for CLAPI_clGetGLContextInfoKHR
    /// \param param_value_size Parameter for CLAPI_clGetGLContextInfoKHR
    /// \param param_value Parameter for CLAPI_clGetGLContextInfoKHR
    /// \param param_value_size_ret Parameter for CLAPI_clGetGLContextInfoKHR
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        const cl_context_properties* properties,
        cl_gl_context_info   param_name,
        size_t   param_value_size,
        void* param_value,
        size_t*  param_value_size_ret,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetGLContextInfoKHR;
        m_properties = properties;

        int num_properties = 0;

        if (properties != NULL)
        {
            // properties is 0 terminated
            while (properties[0] != 0 && num_properties < SP_MAX_NUM_CONTEXT_PROPERTIES)
            {
                m_vecProperties.push_back(properties[0]);
                properties++;
                num_properties++;
            }
        }

        if (num_properties == SP_MAX_NUM_CONTEXT_PROPERTIES)
        {
            //add a dummy value (zero) that tells GetContextPropertiesString that the list has been truncated
            m_vecProperties.push_back(0);
        }

        m_param_name = param_name;
        m_param_value_size = param_value_size;
        m_param_value_size_ret = param_value_size_ret;
        m_replaced_null_param = replaced_null_param;
        m_param_value_size_retVal = *param_value_size_ret;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, RETVALMIN(m_param_value_size_retVal, m_param_value_size));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetGLContextInfoKHR(const CLAPI_clGetGLContextInfoKHR& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetGLContextInfoKHR& operator = (const CLAPI_clGetGLContextInfoKHR& obj);

private:
    const cl_context_properties*       m_properties;              ///< parameter for clGetGLContextInfoKHR
    std::vector<cl_context_properties> m_vecProperties;           ///< vector containing items defined in properties parameter
    cl_gl_context_info                 m_param_name;              ///< parameter for clGetGLContextInfoKHR
    size_t                             m_param_value_size;        ///< parameter for clGetGLContextInfoKHR
    void*                              m_param_value;             ///< parameter for clGetGLContextInfoKHR
    size_t*                            m_param_value_size_ret;    ///< parameter for clGetGLContextInfoKHR
    size_t                             m_param_value_size_retVal; ///< parameter for clGetGLContextInfoKHR
    bool                               m_replaced_null_param;     ///< flag indicating that we've provided a replacement for a NULL m_param_value_size_ret value
    cl_int                             m_retVal;                  ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateSubDevicesEXT
//------------------------------------------------------------------------------------
class CLAPI_clCreateSubDevicesEXT : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateSubDevicesEXT()
    {
        m_out_devices = NULL;
    }

    /// Destructor
    ~CLAPI_clCreateSubDevicesEXT()
    {
        if (m_out_devices != NULL)
        {
            FreeArray(m_out_devices);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_in_device) << s_strParamSeparator
           << CLStringUtils::GetPartitionPropertiesExtString(m_vecProperties) << s_strParamSeparator
           << m_num_entries << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_out_devices, RETVALMIN(m_num_devicesVal, m_num_entries)) << s_strParamSeparator
           << CLStringUtils::GetIntString(REPLACEDNULLVAL(m_replaced_null_param, m_num_devices), m_num_devicesVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateSubDevicesEXT
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param in_device Parameter for CLAPI_clCreateSubDevicesEXT
    /// \param partition_properties Parameter for CLAPI_clCreateSubDevicesEXT
    /// \param num_entries Parameter for CLAPI_clCreateSubDevicesEXT
    /// \param out_devices Parameter for CLAPI_clCreateSubDevicesEXT
    /// \param num_devices Parameter for CLAPI_clCreateSubDevicesEXT
    /// \param replaced_null_param Parameter for CLAPI_clCreateSubDevicesEXT
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_device_id     in_device,
        const cl_device_partition_property_ext* partition_properties,
        cl_uint          num_entries,
        cl_device_id*    out_devices,
        cl_uint*         num_devices,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateSubDevicesEXT;
        m_in_device = in_device;

        if (partition_properties != NULL)
        {
            cl_device_partition_property_ext sub_list_terminator = 0;

            if (partition_properties[0] == CL_DEVICE_PARTITION_BY_NAMES_EXT)
            {
                sub_list_terminator = CL_PARTITION_BY_NAMES_LIST_END_EXT;
            }

            while (partition_properties[0] != sub_list_terminator)
            {
                m_vecProperties.push_back(partition_properties[0]);
                partition_properties++;
            }
        }

        m_num_entries = num_entries;
        m_num_devices = num_devices;
        m_replaced_null_param = replaced_null_param;

        if (retVal == CL_SUCCESS)
        {
            m_num_devicesVal = *num_devices;
            DeepCopyArray(&m_out_devices, out_devices, RETVALMIN(m_num_devicesVal, m_num_entries));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateSubDevicesEXT(const CLAPI_clCreateSubDevicesEXT& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateSubDevicesEXT& operator = (const CLAPI_clCreateSubDevicesEXT& obj);

private:
    cl_device_id     m_in_device;           ///< parameter for clCreateSubDevicesEXT
    const cl_device_partition_property_ext*       m_partition_properties; ///< parameter for clCreateSubDevicesEXT
    std::vector<cl_device_partition_property_ext> m_vecProperties;        ///< vector containing items defined in partition_properties parameter
    cl_uint          m_num_entries;         ///< parameter for clCreateSubDevicesEXT
    cl_device_id*    m_out_devices;         ///< parameter for clCreateSubDevicesEXT
    cl_uint*         m_num_devices;         ///< parameter for clCreateSubDevicesEXT
    cl_uint          m_num_devicesVal;      ///< parameter for clCreateSubDevicesEXT
    cl_int           m_retVal;              ///< return value
    bool             m_replaced_null_param; ///< flag indicating that we've provided a replacement for a NULL m_num_devices value
};

//------------------------------------------------------------------------------------
/// clRetainDeviceEXT
//------------------------------------------------------------------------------------
class CLAPI_clRetainDeviceEXT : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clRetainDeviceEXT() {}

    /// Destructor
    ~CLAPI_clRetainDeviceEXT() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_device);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clRetainDeviceEXT
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param device Parameter for CLAPI_clRetainDeviceEXT
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_device_id device,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clRetainDeviceEXT;
        m_device = device;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clRetainDeviceEXT(const CLAPI_clRetainDeviceEXT& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clRetainDeviceEXT& operator = (const CLAPI_clRetainDeviceEXT& obj);

private:
    cl_device_id m_device; ///< parameter for clRetainDeviceEXT
    cl_int       m_retVal; ///< return value
};

//------------------------------------------------------------------------------------
/// clReleaseDeviceEXT
//------------------------------------------------------------------------------------
class CLAPI_clReleaseDeviceEXT : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clReleaseDeviceEXT() {}

    /// Destructor
    ~CLAPI_clReleaseDeviceEXT() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_device);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clReleaseDeviceEXT
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param device Parameter for CLAPI_clReleaseDeviceEXT
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_device_id  device,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clReleaseDeviceEXT;
        m_device = device;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clReleaseDeviceEXT(const CLAPI_clReleaseDeviceEXT& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clReleaseDeviceEXT& operator = (const CLAPI_clReleaseDeviceEXT& obj);

private:
    cl_device_id m_device; ///< parameter for clReleaseDeviceEXT
    cl_int       m_retVal; ///< return value
};

#ifdef _WIN32
//------------------------------------------------------------------------------------
/// clGetDeviceIDsFromD3D10KHR
//------------------------------------------------------------------------------------
class CLAPI_clGetDeviceIDsFromD3D10KHR : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetDeviceIDsFromD3D10KHR()
    {
        m_device_list = NULL;
    }

    /// Destructor
    ~CLAPI_clGetDeviceIDsFromD3D10KHR()
    {
        if (m_device_list != NULL)
        {
            FreeArray(m_device_list);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return CLStringUtils::GetErrorString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_platform) << s_strParamSeparator
           << CLStringUtils::GetD3D10DeviceSourceString(m_d3d_device_source) << s_strParamSeparator
           << StringUtils::ToHexString(m_d3d_object) << s_strParamSeparator
           << CLStringUtils::GetD3D10DeviceSetString(m_d3d_device_set) << s_strParamSeparator
           << m_num_entries << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_device_list, RETVALMIN(m_num_devicesVal, m_num_entries)) << s_strParamSeparator
           << CLStringUtils::GetIntString(REPLACEDNULLVAL(m_replaced_null_param, m_num_devices), m_num_devicesVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetDeviceIDsFromD3D10KHR
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param platform Parameter for CLAPI_clGetDeviceIDsFromD3D10KHR
    /// \param d3d_device_source Parameter for CLAPI_clGetDeviceIDsFromD3D10KHR
    /// \param d3d_object Parameter for CLAPI_clGetDeviceIDsFromD3D10KHR
    /// \param d3d_device_set Parameter for CLAPI_clGetDeviceIDsFromD3D10KHR
    /// \param num_entries Parameter for CLAPI_clGetDeviceIDsFromD3D10KHR
    /// \param device_list Parameter for CLAPI_clGetDeviceIDsFromD3D10KHR
    /// \param num_devices Parameter for CLAPI_clGetDeviceIDsFromD3D10KHR
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_platform_id platform,
        cl_d3d10_device_source_khr d3d_device_source,
        void* d3d_object,
        cl_d3d10_device_set_khr d3d_device_set,
        cl_uint  num_entries,
        cl_device_id*  device_list,
        cl_uint* num_devices,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetDeviceIDsFromD3D10KHR;
        m_platform = platform;
        m_d3d_device_source = d3d_device_source;
        m_d3d_object = d3d_object;
        m_d3d_device_set = d3d_device_set;

        m_num_entries = num_entries;
        m_num_devices = num_devices;
        m_replaced_null_param = replaced_null_param;

        if (retVal == CL_SUCCESS)
        {
            m_num_devicesVal = *num_devices;
            DeepCopyArray(&m_device_list, device_list, RETVALMIN(m_num_devicesVal, m_num_entries));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetDeviceIDsFromD3D10KHR(const CLAPI_clGetDeviceIDsFromD3D10KHR& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetDeviceIDsFromD3D10KHR& operator = (const CLAPI_clGetDeviceIDsFromD3D10KHR& obj);

private:
    cl_platform_id             m_platform;            ///< parameter for clGetDeviceIDsFromD3D10KHR
    cl_d3d10_device_source_khr m_d3d_device_source;   ///< parameter for clGetDeviceIDsFromD3D10KHR
    void*                      m_d3d_object;          ///< parameter for clGetDeviceIDsFromD3D10KHR
    cl_d3d10_device_set_khr    m_d3d_device_set;      ///< parameter for clGetDeviceIDsFromD3D10KHR
    cl_uint                    m_num_entries;         ///< parameter for clGetDeviceIDsFromD3D10KHR
    cl_device_id*              m_device_list;         ///< parameter for clGetDeviceIDsFromD3D10KHR
    cl_uint*                   m_num_devices;         ///< parameter for clGetDeviceIDsFromD3D10KHR
    cl_uint                    m_num_devicesVal;      ///< dereferenced value of a non-null m_num_devices
    bool                       m_replaced_null_param; ///< flag indicating that we've provided a replacement for a NULL m_device_list value
    cl_int                     m_retVal;              ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateFromD3D10BufferKHR
//------------------------------------------------------------------------------------
class CLAPI_clCreateFromD3D10BufferKHR : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateFromD3D10BufferKHR() { }

    /// Destructor
    ~CLAPI_clCreateFromD3D10BufferKHR() { }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetMemFlagsString(m_flags) << s_strParamSeparator
           << StringUtils::ToHexString(m_resource) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateFromD3D10BufferKHR
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateFromD3D10BufferKHR
    /// \param flags Parameter for CLAPI_clCreateFromD3D10BufferKHR
    /// \param resource Parameter for CLAPI_clCreateFromD3D10BufferKHR
    /// \param errcode_ret Parameter for CLAPI_clCreateFromD3D10BufferKHR
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context context,
        cl_mem_flags flags,
        ID3D10Buffer* resource,
        cl_int* errcode_ret,
        cl_mem retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateFromD3D10BufferKHR;
        m_context = context;
        m_flags = flags;
        m_resource = resource;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateFromD3D10BufferKHR(const CLAPI_clCreateFromD3D10BufferKHR& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateFromD3D10BufferKHR& operator = (const CLAPI_clCreateFromD3D10BufferKHR& obj);

private:
    cl_context    m_context;        ///< parameter for clCreateFromD3D10BufferKHR
    cl_mem_flags  m_flags;          ///< parameter for clCreateFromD3D10BufferKHR
    ID3D10Buffer* m_resource;       ///< parameter for clCreateFromD3D10BufferKHR
    cl_int*       m_errcode_ret;    ///< parameter for clCreateFromD3D10BufferKHR
    cl_int        m_errcode_retVal; ///< dereferenced value of a non-null m_errcode_ret
    cl_mem        m_retVal;         ///< parameter for clCreateFromD3D10BufferKHR
};

//------------------------------------------------------------------------------------
/// clCreateFromD3D10Texture2DKHR
//------------------------------------------------------------------------------------
class CLAPI_clCreateFromD3D10Texture2DKHR : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateFromD3D10Texture2DKHR() { }

    /// Destructor
    ~CLAPI_clCreateFromD3D10Texture2DKHR() { }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetMemFlagsString(m_flags) << s_strParamSeparator
           << StringUtils::ToHexString(m_resource) << s_strParamSeparator
           << StringUtils::ToHexString(m_subresource) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateFromD3D10Texture2DKHR
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateFromD3D10Texture2DKHR
    /// \param flags Parameter for CLAPI_clCreateFromD3D10Texture2DKHR
    /// \param resource Parameter for CLAPI_clCreateFromD3D10Texture2DKHR
    /// \param subresource Parameter for CLAPI_clCreateFromD3D10Texture2DKHR
    /// \param errcode_ret Parameter for CLAPI_clCreateFromD3D10Texture2DKHR
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context context,
        cl_mem_flags flags,
        ID3D10Texture2D* resource,
        UINT subresource,
        cl_int* errcode_ret,
        cl_mem retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateFromD3D10Texture2DKHR;
        m_context = context;
        m_flags = flags;
        m_resource = resource;
        m_subresource = subresource;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateFromD3D10Texture2DKHR(const CLAPI_clCreateFromD3D10Texture2DKHR& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateFromD3D10Texture2DKHR& operator = (const CLAPI_clCreateFromD3D10Texture2DKHR& obj);

private:
    cl_context       m_context;        ///< parameter for clCreateFromD3D10Texture2DKHR
    cl_mem_flags     m_flags;          ///< parameter for clCreateFromD3D10Texture2DKHR
    ID3D10Texture2D* m_resource;       ///< parameter for clCreateFromD3D10Texture2DKHR
    UINT             m_subresource;    ///< parameter for clCreateFromD3D10Texture2DKHR
    cl_int*          m_errcode_ret;    ///< parameter for clCreateFromD3D10Texture2DKHR
    cl_int           m_errcode_retVal; ///< dereferenced value of a non-null m_errcode_ret
    cl_mem           m_retVal;         ///< parameter for clCreateFromD3D10Texture2DKHR
};

//------------------------------------------------------------------------------------
/// clCreateFromD3D10Texture3DKHR
//------------------------------------------------------------------------------------
class CLAPI_clCreateFromD3D10Texture3DKHR : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateFromD3D10Texture3DKHR() { }

    /// Destructor
    ~CLAPI_clCreateFromD3D10Texture3DKHR() { }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetMemFlagsString(m_flags) << s_strParamSeparator
           << StringUtils::ToHexString(m_resource) << s_strParamSeparator
           << StringUtils::ToHexString(m_subresource) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateFromD3D10Texture3DKHR
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateFromD3D10Texture3DKHR
    /// \param flags Parameter for CLAPI_clCreateFromD3D10Texture3DKHR
    /// \param resource Parameter for CLAPI_clCreateFromD3D10Texture3DKHR
    /// \param subresource Parameter for CLAPI_clCreateFromD3D10Texture3DKHR
    /// \param errcode_ret Parameter for CLAPI_clCreateFromD3D10Texture3DKHR
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context context,
        cl_mem_flags flags,
        ID3D10Texture3D* resource,
        UINT subresource,
        cl_int* errcode_ret,
        cl_mem retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateFromD3D10Texture3DKHR;
        m_context = context;
        m_flags = flags;
        m_resource = resource;
        m_subresource = subresource;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateFromD3D10Texture3DKHR(const CLAPI_clCreateFromD3D10Texture3DKHR& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateFromD3D10Texture3DKHR& operator = (const CLAPI_clCreateFromD3D10Texture3DKHR& obj);

private:
    cl_context       m_context;        ///< parameter for clCreateFromD3D10Texture3DKHR
    cl_mem_flags     m_flags;          ///< parameter for clCreateFromD3D10Texture3DKHR
    ID3D10Texture3D* m_resource;       ///< parameter for clCreateFromD3D10Texture3DKHR
    UINT             m_subresource;    ///< parameter for clCreateFromD3D10Texture3DKHR
    cl_int*          m_errcode_ret;    ///< parameter for clCreateFromD3D10Texture3DKHR
    cl_int           m_errcode_retVal; ///< dereferenced value of a non-null m_errcode_ret
    cl_mem           m_retVal;         ///< parameter for clCreateFromD3D10Texture3DKHR
};
#endif

//------------------------------------------------------------------------------------
/// clCreateSubDevices
//------------------------------------------------------------------------------------
class CLAPI_clCreateSubDevices : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateSubDevices()
    {
        m_out_devices = NULL;
    }

    /// Destructor
    ~CLAPI_clCreateSubDevices()
    {
        if (m_out_devices != NULL)
        {
            FreeArray(m_out_devices);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_in_device) << s_strParamSeparator
           << CLStringUtils::GetPartitionPropertiesString(m_vecProperties) << s_strParamSeparator
           << m_num_entries << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_out_devices, RETVALMIN(m_num_devicesVal, m_num_entries)) << s_strParamSeparator
           << CLStringUtils::GetIntString(REPLACEDNULLVAL(m_replaced_null_param, m_num_devices), m_num_devicesVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateSubDevices
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param in_device Parameter for CLAPI_clCreateSubDevices
    /// \param partition_properties Parameter for CLAPI_clCreateSubDevices
    /// \param num_entries Parameter for CLAPI_clCreateSubDevices
    /// \param out_devices Parameter for CLAPI_clCreateSubDevices
    /// \param num_devices Parameter for CLAPI_clCreateSubDevices
    /// \param replaced_null_param Parameter for CLAPI_clCreateSubDevices
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_device_id     in_device,
        const cl_device_partition_property* partition_properties,
        cl_uint          num_entries,
        cl_device_id*    out_devices,
        cl_uint*         num_devices,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateSubDevices;
        m_in_device = in_device;

        if (partition_properties != NULL)
        {
            cl_device_partition_property sub_list_terminator = 0;

            if (partition_properties[0] == CL_DEVICE_PARTITION_BY_COUNTS)
            {
                sub_list_terminator = CL_DEVICE_PARTITION_BY_COUNTS_LIST_END;
            }

            while (partition_properties[0] != sub_list_terminator)
            {
                m_vecProperties.push_back(partition_properties[0]);
                partition_properties++;
            }
        }

        m_num_entries = num_entries;
        m_num_devices = num_devices;
        m_replaced_null_param = replaced_null_param;

        if (retVal == CL_SUCCESS)
        {
            m_num_devicesVal = *num_devices;
            DeepCopyArray(&m_out_devices, out_devices, RETVALMIN(m_num_devicesVal, m_num_entries));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateSubDevices(const CLAPI_clCreateSubDevices& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateSubDevices& operator = (const CLAPI_clCreateSubDevices& obj);

private:
    cl_device_id     m_in_device;           ///< parameter for clCreateSubDevices
    const cl_device_partition_property*       m_partition_properties; ///< parameter for clCreateSubDevices
    std::vector<cl_device_partition_property> m_vecProperties;        ///< vector containing items defined in partition_properties parameter
    cl_uint          m_num_entries;         ///< parameter for clCreateSubDevices
    cl_device_id*    m_out_devices;         ///< parameter for clCreateSubDevices
    cl_uint*         m_num_devices;         ///< parameter for clCreateSubDevices
    cl_uint          m_num_devicesVal;      ///< parameter for clCreateSubDevices
    cl_int           m_retVal;              ///< return value
    bool             m_replaced_null_param; ///< flag indicating that we've provided a replacement for a NULL m_num_devices value
};

//------------------------------------------------------------------------------------
/// clRetainDevice
//------------------------------------------------------------------------------------
class CLAPI_clRetainDevice : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clRetainDevice() {}

    /// Destructor
    ~CLAPI_clRetainDevice() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_device);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clRetainDevice
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param device Parameter for CLAPI_clRetainDevice
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_device_id device,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clRetainDevice;
        m_device = device;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clRetainDevice(const CLAPI_clRetainDevice& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clRetainDevice& operator = (const CLAPI_clRetainDevice& obj);

private:
    cl_device_id m_device; ///< parameter for clRetainDevice
    cl_int       m_retVal; ///< return value
};

//------------------------------------------------------------------------------------
/// clReleaseDevice
//------------------------------------------------------------------------------------
class CLAPI_clReleaseDevice : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clReleaseDevice() {}

    /// Destructor
    ~CLAPI_clReleaseDevice() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_device);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clReleaseDevice
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param device Parameter for CLAPI_clReleaseDevice
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_device_id  device,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clReleaseDevice;
        m_device = device;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clReleaseDevice(const CLAPI_clReleaseDevice& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clReleaseDevice& operator = (const CLAPI_clReleaseDevice& obj);

private:
    cl_device_id m_device; ///< parameter for clReleaseDevice
    cl_int       m_retVal; ///< return value
};

//------------------------------------------------------------------------------------
/// clCreateImage
//------------------------------------------------------------------------------------
class CLAPI_clCreateImage : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateImage()
    {
        m_image_format = NULL;
    }

    /// Destructor
    ~CLAPI_clCreateImage()
    {
        if (m_image_format != NULL)
        {
            delete m_image_format;
        }

        if (m_image_desc != NULL)
        {
            delete m_image_desc;
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetMemFlagsString(m_flags) << s_strParamSeparator
           << CLStringUtils::GetImageFormatsString(m_image_format, 1) << s_strParamSeparator
           << CLStringUtils::GetImageDescString(m_image_desc) << s_strParamSeparator
           << StringUtils::ToHexString(m_host_ptr) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateImage
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateImage
    /// \param flags Parameter for CLAPI_clCreateImage
    /// \param image_format Parameter for CLAPI_clCreateImage
    /// \param image_desc Parameter for CLAPI_clCreateImage
    /// \param host_ptr Parameter for CLAPI_clCreateImage
    /// \param errcode_ret Parameter for CLAPI_clCreateImage
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context  context,
        cl_mem_flags   flags,
        const cl_image_format*  image_format,
        const cl_image_desc*  image_desc,
        void* host_ptr,
        cl_int*  errcode_ret,
        cl_mem retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateImage;
        m_context = context;
        m_flags = flags;
        DeepCopyArray(&m_image_format, image_format, 1);
        DeepCopyArray(&m_image_desc, image_desc, 1);
        m_host_ptr = host_ptr;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateImage(const CLAPI_clCreateImage& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateImage& operator = (const CLAPI_clCreateImage& obj);

private:
    cl_context       m_context;         ///< parameter for clCreateImage
    cl_mem_flags     m_flags;           ///< parameter for clCreateImage
    cl_image_format* m_image_format;    ///< parameter for clCreateImage
    cl_image_desc*   m_image_desc;      ///< parameter for clCreateImage
    void*            m_host_ptr;        ///< parameter for clCreateImage
    cl_int*          m_errcode_ret;     ///< parameter for clCreateImage
    cl_int           m_errcode_retVal;  ///< parameter for clCreateImage
    cl_mem           m_retVal;          ///< return value
};

//------------------------------------------------------------------------------------
/// CLAPI_clCreateProgramWithBuiltInKernels
//------------------------------------------------------------------------------------
class CLAPI_clCreateProgramWithBuiltInKernels : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateProgramWithBuiltInKernels()
    {
        m_device_list = NULL;
    }

    /// Destructor
    ~CLAPI_clCreateProgramWithBuiltInKernels()
    {
        if (m_device_list != NULL)
        {
            FreeArray(m_device_list);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << m_num_devices << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_device_list, m_num_devices) << s_strParamSeparator
           << CLStringUtils::GetQuotedString(m_str_kernel_names, m_kernel_names) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of CLAPI_clCreateProgramWithBuiltInKernels
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateProgramWithBuiltInKernels
    /// \param num_devices Parameter for CLAPI_clCreateProgramWithBuiltInKernels
    /// \param device_list Parameter for CLAPI_clCreateProgramWithBuiltInKernels
    /// \param kernel_names Parameter for CLAPI_clCreateProgramWithBuiltInKernels
    /// \param errcode_ret Parameter for CLAPI_clCreateProgramWithBuiltInKernels
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context  context,
        cl_uint  num_devices,
        const cl_device_id*  device_list,
        const char* kernel_names,
        cl_int*  errcode_ret,
        cl_program retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateProgramWithBuiltInKernels;
        m_context = context;
        m_num_devices = num_devices;
        DeepCopyArray(&m_device_list, device_list, num_devices);
        m_kernel_names = kernel_names;

        if (kernel_names != NULL)
        {
            m_str_kernel_names = std::string(kernel_names);
        }
        else
        {
            m_str_kernel_names = "";
        }

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateProgramWithBuiltInKernels(const CLAPI_clCreateProgramWithBuiltInKernels& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateProgramWithBuiltInKernels& operator = (const CLAPI_clCreateProgramWithBuiltInKernels& obj);

private:
    cl_context            m_context;          ///< parameter for clCreateProgramWithBuiltInKernels
    cl_uint               m_num_devices;      ///< parameter for clCreateProgramWithBuiltInKernels
    cl_device_id*         m_device_list;      ///< parameter for clCreateProgramWithBuiltInKernels
    const char*           m_kernel_names;     ///< parameter for clCreateProgramWithBuiltInKernels
    std::string           m_str_kernel_names; ///< std::string version of the kernel names
    cl_int*               m_errcode_ret;      ///< parameter for clCreateProgramWithBuiltInKernels
    cl_int                m_errcode_retVal;   ///< parameter for clCreateProgramWithBuiltInKernels
    cl_program            m_retVal;           ///< return value
};

//------------------------------------------------------------------------------------
/// clCompileProgram
//------------------------------------------------------------------------------------
class CLAPI_clCompileProgram : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCompileProgram()
    {
        m_device_list = NULL;
    }

    /// Destructor
    ~CLAPI_clCompileProgram()
    {
        if (m_device_list != NULL)
        {
            FreeArray(m_device_list);
        }

        if (m_input_headers != NULL)
        {
            FreeArray(m_input_headers);
        }

        if (m_header_include_names != NULL)
        {
            FreeArray(m_header_include_names);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_program) << s_strParamSeparator
           << m_num_devices << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_device_list, m_num_devices) << s_strParamSeparator
           << CLStringUtils::GetBuildOptionsString(m_strOptions, m_options, m_strOverriddenOptions, m_bOptionsAppended) << s_strParamSeparator
           << m_num_input_headers << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_input_headers, m_num_input_headers) << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_header_include_names, m_num_input_headers) << s_strParamSeparator
           << StringUtils::ToHexString(m_pfn_notify) << s_strParamSeparator
           << StringUtils::ToHexString(m_user_data);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCompileProgram
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param program Parameter for CLAPI_clCompileProgram
    /// \param num_devices Parameter for CLAPI_clCompileProgram
    /// \param device_list Parameter for CLAPI_clCompileProgram
    /// \param options Parameter for CLAPI_clCompileProgram
    /// \param num_input_headers Parameter for CLAPI_clCompileProgram
    /// \param input_headers Parameter for CLAPI_clCompileProgram
    /// \param header_include_names Parameter for CLAPI_clCompileProgram
    /// \param pfn_notify Parameter for CLAPI_clCompileProgram
    /// \param user_data Parameter for CLAPI_clCompileProgram
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_program  program,
        cl_uint  num_devices,
        const cl_device_id* device_list,
        const char* options,
        cl_uint num_input_headers,
        const cl_program* input_headers,
        const char** header_include_names,
        void (CL_CALLBACK* pfn_notify)(cl_program program , void* user_data),
        void* user_data,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCompileProgram;
        m_program = program;
        m_num_devices = num_devices;
        DeepCopyArray(&m_device_list, device_list, num_devices);

        // store the original pointer
        m_options = options;

        // check for env var that overrides the build options
        m_strOverriddenOptions = OSUtils::Instance()->GetEnvVar("AMD_OCL_BUILD_OPTIONS");

        // now check for env var that appends build options
        std::string strExtraBldOpts = OSUtils::Instance()->GetEnvVar("AMD_OCL_BUILD_OPTIONS_APPEND");
        m_bOptionsAppended = !strExtraBldOpts.empty();

        if (options != NULL)
        {
            m_strOptions = std::string(options);
            std::vector<std::string> switches;
            StringUtils::Split(switches, m_strOptions, " ", true, true);

            for (std::vector<std::string>::const_iterator it = switches.begin(); it != switches.end(); ++it)
            {
                if (*it == "-ignore-env")
                {
                    m_bOptionsAppended = false;
                    m_strOverriddenOptions = "";
                    break;
                }
            }

            if (m_bOptionsAppended)
            {
                m_strOverriddenOptions = m_strOptions;

                if (!m_strOverriddenOptions.empty())
                {
                    m_strOverriddenOptions += " ";
                }

                m_strOverriddenOptions += strExtraBldOpts;
            }
        }
        else
        {
            if (m_bOptionsAppended)
            {
                m_strOverriddenOptions = strExtraBldOpts;
            }
            else
            {
                m_strOptions = "";
            }
        }

        m_num_input_headers = num_input_headers;
        DeepCopyArray(&m_input_headers, input_headers, num_input_headers);
        DeepCopyArray(&m_header_include_names, header_include_names, num_input_headers);

        this->m_pfn_notify = pfn_notify;
        m_user_data = user_data;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCompileProgram(const CLAPI_clCompileProgram& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCompileProgram& operator = (const CLAPI_clCompileProgram& obj);

private:
    cl_program    m_program;              ///< parameter for clCompileProgram
    cl_uint       m_num_devices;          ///< parameter for clCompileProgram
    cl_device_id* m_device_list;          ///< parameter for clCompileProgram
    const char*   m_options;              ///< parameter for clCompileProgram
    std::string   m_strOptions;           ///< std::string version of the options
    std::string   m_strOverriddenOptions; ///< the actual options used by clCompileProgram (may be different from m_strOptions if an env var is set)
    bool          m_bOptionsAppended;     ///< flag indicating if the overridden options were appended (vs. a full replacement)
    cl_uint       m_num_input_headers;    ///< parameter for clCompileProgram
    cl_program*   m_input_headers;        ///< parameter for clCompileProgram
    const char**  m_header_include_names; ///< parameter for clCompileProgram
    void (CL_CALLBACK* m_pfn_notify)(cl_program program , void* user_data); ///< parameter for clCompileProgram
    void*         m_user_data;            ///< parameter for clCompileProgram
    cl_int        m_retVal;               ///< return value
};

//------------------------------------------------------------------------------------
/// clLinkProgram
//------------------------------------------------------------------------------------
class CLAPI_clLinkProgram : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clLinkProgram()
    {
        m_device_list = NULL;
    }

    /// Destructor
    ~CLAPI_clLinkProgram()
    {
        if (m_device_list != NULL)
        {
            FreeArray(m_device_list);
        }

        if (m_input_programs != NULL)
        {
            FreeArray(m_input_programs);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << m_num_devices << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_device_list, m_num_devices) << s_strParamSeparator
           << CLStringUtils::GetBuildOptionsString(m_strOptions, m_options, m_strOverriddenOptions, m_bOptionsAppended, true) << s_strParamSeparator
           << m_num_input_programs << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_input_programs, m_num_input_programs) << s_strParamSeparator
           << StringUtils::ToHexString(m_pfn_notify) << s_strParamSeparator
           << StringUtils::ToHexString(m_user_data) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clLinkProgram
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clLinkProgram
    /// \param num_devices Parameter for CLAPI_clLinkProgram
    /// \param device_list Parameter for CLAPI_clLinkProgram
    /// \param options Parameter for CLAPI_clLinkProgram
    /// \param num_input_programs Parameter for CLAPI_clLinkProgram
    /// \param input_programs Parameter for CLAPI_clLinkProgram
    /// \param pfn_notify Parameter for CLAPI_clLinkProgram
    /// \param user_data Parameter for CLAPI_clLinkProgram
    /// \param errcode_ret Parameter for CLAPI_clLinkProgram
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context  context,
        cl_uint  num_devices,
        const cl_device_id*  device_list,
        const char* options,
        cl_uint num_input_programs,
        const cl_program* input_programs,
        void (CL_CALLBACK* pfn_notify)(cl_program program , void* user_data),
        void* user_data,
        cl_int* errcode_ret,
        cl_program retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clLinkProgram;
        m_context = context;
        m_num_devices = num_devices;
        //m_device_list = device_list;
        DeepCopyArray(&m_device_list, device_list, num_devices);

        // store the original pointer
        m_options = options;

        // check for env var that overrides the build options
        m_strOverriddenOptions = OSUtils::Instance()->GetEnvVar("AMD_OCL_LINK_OPTIONS");

        // now check for env var that appends build options
        std::string strExtraBldOpts = OSUtils::Instance()->GetEnvVar("AMD_OCL_LINK_OPTIONS_APPEND");
        m_bOptionsAppended = !strExtraBldOpts.empty();

        if (options != NULL)
        {
            m_strOptions = std::string(options);
            std::vector<std::string> switches;
            StringUtils::Split(switches, m_strOptions, " ", true, true);

            for (std::vector<std::string>::const_iterator it = switches.begin(); it != switches.end(); ++it)
            {
                if (*it == "-ignore-env")
                {
                    m_bOptionsAppended = false;
                    m_strOverriddenOptions = "";
                    break;
                }
            }

            if (m_bOptionsAppended)
            {
                m_strOverriddenOptions = m_strOptions;

                if (!m_strOverriddenOptions.empty())
                {
                    m_strOverriddenOptions += " ";
                }

                m_strOverriddenOptions += strExtraBldOpts;
            }
        }
        else
        {
            if (m_bOptionsAppended)
            {
                m_strOverriddenOptions = strExtraBldOpts;
            }
            else
            {
                m_strOptions = "";
            }
        }

        m_num_input_programs = num_input_programs;
        DeepCopyArray(&m_input_programs, input_programs, num_input_programs);

        this->m_pfn_notify = pfn_notify;
        m_user_data = user_data;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clLinkProgram(const CLAPI_clLinkProgram& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clLinkProgram& operator = (const CLAPI_clLinkProgram& obj);

private:
    cl_context    m_context;              ///< parameter for clLinkProgram
    cl_uint       m_num_devices;          ///< parameter for clLinkProgram
    cl_device_id* m_device_list;          ///< parameter for clLinkProgram
    const char*   m_options;              ///< parameter for clLinkProgram
    std::string   m_strOptions;           ///< std::string version of the options
    std::string   m_strOverriddenOptions; ///< the actual options used by clLinkProgram (may be different from m_strOptions if an env var is set)
    bool          m_bOptionsAppended;     ///< flag indicating if the overridden options were appended (vs. a full replacement)
    cl_uint       m_num_input_programs;   ///< parameter for clLinkProgram
    cl_program*   m_input_programs;       ///< parameter for clLinkProgram
    void (CL_CALLBACK* m_pfn_notify)(cl_program program , void* user_data); ///< parameter for clLinkProgram
    void*         m_user_data;            ///< parameter for clLinkProgram
    cl_int*       m_errcode_ret;          ///< parameter for clLinkProgram
    cl_int        m_errcode_retVal;       ///< parameter for clLinkProgram
    cl_program    m_retVal;               ///< return value
};

//------------------------------------------------------------------------------------
/// clUnloadPlatformCompiler
//------------------------------------------------------------------------------------
class CLAPI_clUnloadPlatformCompiler : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clUnloadPlatformCompiler() {}

    /// Destructor
    ~CLAPI_clUnloadPlatformCompiler() {}

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_platform) << s_strParamSeparator;
        return ss.str();
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clUnloadPlatformCompiler
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param platform parameter for clUnloadPlatformCompiler
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_platform_id platform,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_platform = platform;
        m_type = CL_FUNC_TYPE_clUnloadPlatformCompiler;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clUnloadPlatformCompiler(const CLAPI_clUnloadPlatformCompiler& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clUnloadPlatformCompiler& operator = (const CLAPI_clUnloadPlatformCompiler& obj);

private:
    cl_platform_id m_platform; ///< parameter for clUnloadPlatformCompiler
    cl_int         m_retVal;   ///< return value
};

//------------------------------------------------------------------------------------
/// clGetKernelArgInfo
//------------------------------------------------------------------------------------
class CLAPI_clGetKernelArgInfo : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetKernelArgInfo()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clGetKernelArgInfo()
    {
        if (NULL != m_param_value)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return CLStringUtils::GetErrorString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_kernel) << s_strParamSeparator
           << m_arg_index << s_strParamSeparator
           << CLStringUtils::GetKernelArgInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size << s_strParamSeparator
           << CLStringUtils::GetKernelArgInfoValueString(m_param_name, m_param_value, m_retVal) << s_strParamSeparator
           << CLStringUtils::GetSizeString(REPLACEDNULLVAL(m_replaced_null_param, m_param_value_size_ret), m_param_value_size_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of CLAPI_clGetKernelArgInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param kernel parameter for CLAPI_clGetKernelArgInfo
    /// \param arg_index parameter for CLAPI_clGetKernelArgInfo
    /// \param param_name parameter for CLAPI_clGetKernelArgInfo
    /// \param param_value_size parameter for CLAPI_clGetKernelArgInfo
    /// \param param_value parameter for CLAPI_clGetKernelArgInfo
    /// \param param_value_size_ret parameter for CLAPI_clGetKernelArgInfo
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_kernel kernel,
        cl_uint arg_index,
        cl_kernel_arg_info param_name,
        size_t param_value_size,
        void* param_value,
        size_t* param_value_size_ret,
        bool replaced_null_param,
        cl_uint retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;

        m_kernel = kernel;
        m_arg_index = arg_index;

        m_param_name = param_name;
        m_param_value_size = param_value_size;
        m_param_value_size_ret = param_value_size_ret;
        m_replaced_null_param = replaced_null_param;
        m_param_value_size_retVal = *param_value_size_ret;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, RETVALMIN(m_param_value_size_retVal, m_param_value_size));
        }

        m_type = CL_FUNC_TYPE_clGetKernelArgInfo;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetKernelArgInfo(const CLAPI_clGetKernelArgInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetKernelArgInfo& operator = (const CLAPI_clGetKernelArgInfo& obj);

    cl_kernel          m_kernel;                  ///< parameter for CLAPI_clGetKernelArgInfo
    cl_uint            m_arg_index;               ///< parameter for CLAPI_clGetKernelArgInfo
    cl_kernel_arg_info m_param_name;              ///< parameter for CLAPI_clGetKernelArgInfo
    size_t             m_param_value_size;        ///< parameter for CLAPI_clGetKernelArgInfo
    void*              m_param_value;             ///< parameter for CLAPI_clGetKernelArgInfo
    size_t*            m_param_value_size_ret;    ///< parameter for CLAPI_clGetKernelArgInfo
    size_t             m_param_value_size_retVal; ///< parameter for CLAPI_clGetKernelArgInfo
    bool               m_replaced_null_param;     ///< flag indicating that we've provided a replacement for a NULL m_param_value_size_ret value
    cl_uint            m_retVal;                  ///< parameter for CLAPI_clGetKernelArgInfo
};

//------------------------------------------------------------------------------------
/// clGetExtensionFunctionAddressForPlatform
//------------------------------------------------------------------------------------
class CLAPI_clGetExtensionFunctionAddressForPlatform : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetExtensionFunctionAddressForPlatform() { }

    /// Destructor
    ~CLAPI_clGetExtensionFunctionAddressForPlatform() { }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_platform) << s_strParamSeparator
           << CLStringUtils::GetQuotedString(m_str_funcname, m_funcname);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetExtensionFunctionAddressForPlatform
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param platform parameter for CLAPI_clGetExtensionFunctionAddressForPlatform
    /// \param funcname parameter for CLAPI_clGetExtensionFunctionAddressForPlatform
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_platform_id platform,
        const char* funcname,
        void* retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_platform = platform;
        m_funcname = funcname;
        m_str_funcname = std::string(funcname);
        m_type = CL_FUNC_TYPE_clGetExtensionFunctionAddressForPlatform;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetExtensionFunctionAddressForPlatform(const CLAPI_clGetExtensionFunctionAddressForPlatform& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetExtensionFunctionAddressForPlatform& operator = (const CLAPI_clGetExtensionFunctionAddressForPlatform& obj);

    cl_platform_id m_platform;     ///< parameter for CLAPI_clGetExtensionFunctionAddressForPlatform
    const char*    m_funcname;     ///< parameter for CLAPI_clGetExtensionFunctionAddressForPlatform
    std::string    m_str_funcname; ///< std:string version of the function name
    void*          m_retVal;       ///< return value for CLAPI_clGetExtensionFunctionAddressForPlatform
};

//------------------------------------------------------------------------------------
/// clCreateFromGLTexture
//------------------------------------------------------------------------------------
class CLAPI_clCreateFromGLTexture : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreateFromGLTexture() {}

    /// Destructor
    ~CLAPI_clCreateFromGLTexture() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetMemFlagsString(m_flags) << s_strParamSeparator
           << CLStringUtils::GetGLTextureTargetString(m_texture_target) << s_strParamSeparator
           << m_miplevel << s_strParamSeparator
           << m_texture << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreateFromGLTexture
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context Parameter for CLAPI_clCreateFromGLTexture
    /// \param flags Parameter for CLAPI_clCreateFromGLTexture
    /// \param texture_target texture target
    /// \param miplevel mip level
    /// \param texture texture id
    /// \param errcode_ret Parameter for CLAPI_clCreateFromGLTexture
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context context,
        cl_mem_flags flags,
        cl_GLenum texture_target,
        cl_GLint miplevel,
        cl_GLuint texture,
        cl_int* errcode_ret,
        cl_mem retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clCreateFromGLTexture;
        m_context = context;
        m_flags = flags;
        m_texture_target = texture_target;
        m_miplevel = miplevel;
        m_texture = texture;

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreateFromGLTexture(const CLAPI_clCreateFromGLTexture& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreateFromGLTexture& operator = (const CLAPI_clCreateFromGLTexture& obj);

private:
    cl_context   m_context;        ///< parameter for clCreateFromGLTexture
    cl_mem_flags m_flags;          ///< parameter for clCreateFromGLTexture
    cl_GLenum    m_texture_target; ///< parameter for clCreateFromGLTexture
    cl_GLint     m_miplevel;       ///< parameter for clCreateFromGLTexture
    cl_GLuint    m_texture;        ///< parameter for clCreateFromGLTexture
    cl_int*      m_errcode_ret;    ///< parameter for clCreateFromGLTexture
    cl_int       m_errcode_retVal; ///< parameter for clCreateFromGLTexture
    cl_mem       m_retVal;         ///< return value
};

//------------------------------------------------------------------------------------
/// clGetExtensionFunctionAddress
//------------------------------------------------------------------------------------
class CLAPI_clGetExtensionFunctionAddress : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetExtensionFunctionAddress() { }

    /// Destructor
    ~CLAPI_clGetExtensionFunctionAddress() { }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetQuotedString(m_str_funcname, m_funcname);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetExtensionFunctionAddress
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param funcname parameter for CLAPI_clGetExtensionFunctionAddress
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        const char* funcname,
        void* retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_funcname = funcname;
        m_str_funcname = std::string(funcname);
        m_type = CL_FUNC_TYPE_clGetExtensionFunctionAddress;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetExtensionFunctionAddress(const CLAPI_clGetExtensionFunctionAddress& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetExtensionFunctionAddress& operator = (const CLAPI_clGetExtensionFunctionAddress& obj);

    const char* m_funcname;     ///< parameter for CLAPI_clGetExtensionFunctionAddress
    std::string m_str_funcname; ///< std:string version of the function name
    void*       m_retVal;       ///< return value for CLAPI_clGetExtensionFunctionAddress
};

//------------------------------------------------------------------------------------
/// clSVMAlloc
//------------------------------------------------------------------------------------
class CLAPI_clSVMAlloc : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clSVMAlloc() { }

    /// Destructor
    ~CLAPI_clSVMAlloc() { }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetMemFlagsString(m_flags) << s_strParamSeparator
           << m_size << s_strParamSeparator
           << m_alignment;
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clSVMAlloc
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context parameter for CLAPI_clSVMAlloc
    /// \param flags parameter for CLAPI_clSVMAlloc
    /// \param size parameter for CLAPI_clSVMAlloc
    /// \param alignment parameter for CLAPI_clSVMAlloc
    /// \param retVal return value
    /// \param isExtension flag indicating whether the application called the extension API (clSVMAllocAMD) or the real API (clSVMAlloc)
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context context,
        cl_svm_mem_flags flags,
        size_t size,
        cl_uint alignment,
        void* retVal,
        bool isExtension = false)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_context = context;
        m_flags = flags;
        m_size = size;
        m_alignment = alignment;

        if (isExtension)
        {
            m_type = CL_FUNC_TYPE_clSVMAllocAMD;
        }
        else
        {
            m_type = CL_FUNC_TYPE_clSVMAlloc;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clSVMAlloc(const CLAPI_clSVMAlloc& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clSVMAlloc& operator = (const CLAPI_clSVMAlloc& obj);

    cl_context       m_context;   ///< parameter for CLAPI_clSVMAlloc
    cl_svm_mem_flags m_flags;     ///< parameter for CLAPI_clSVMAlloc
    size_t           m_size;      ///< parameter for CLAPI_clSVMAlloc
    cl_uint          m_alignment; ///< parameter for CLAPI_clSVMAlloc
    void*            m_retVal;    ///< return value for CLAPI_clSVMAlloc
};

//------------------------------------------------------------------------------------
/// clSVMFree
//------------------------------------------------------------------------------------
class CLAPI_clSVMFree : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clSVMFree() { }

    /// Destructor
    ~CLAPI_clSVMFree() { }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return "";
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << StringUtils::ToHexString(m_svm_pointer);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clSVMFree
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context parameter for CLAPI_clSVMFree
    /// \param svm_pointer parameter for CLAPI_clSVMFree
    /// \param isExtension flag indicating whether the application called the extension API (clSVMFreeAMD) or the real API (clSVMFree)
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context context,
        void* svm_pointer,
        bool isExtension = false)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_context = context;
        m_svm_pointer = svm_pointer;

        if (isExtension)
        {
            m_type = CL_FUNC_TYPE_clSVMFreeAMD;
        }
        else
        {
            m_type = CL_FUNC_TYPE_clSVMFree;
        }
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clSVMFree(const CLAPI_clSVMFree& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clSVMFree& operator = (const CLAPI_clSVMFree& obj);

    cl_context m_context;     ///< parameter for CLAPI_clSVMFree
    void*      m_svm_pointer; ///< parameter for CLAPI_clSVMFree
};

//------------------------------------------------------------------------------------
/// CLAPI_clSetKernelArgSVMPointer
//------------------------------------------------------------------------------------
class CLAPI_clSetKernelArgSVMPointer : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clSetKernelArgSVMPointer() { }

    /// Destructor
    ~CLAPI_clSetKernelArgSVMPointer() { }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return CLStringUtils::GetErrorString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_kernel) << s_strParamSeparator
           << m_arg_index << s_strParamSeparator
           << StringUtils::ToHexString(m_arg_value);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clSetKernelArgSVMPointer
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param kernel parameter for CLAPI_clSetKernelArgSVMPointer
    /// \param arg_index parameter for CLAPI_clSetKernelArgSVMPointer
    /// \param arg_value parameter for CLAPI_clSetKernelArgSVMPointer
    /// \param retVal return value
    /// \param isExtension flag indicating whether the application called the extension API (clSetKernelArgSVMPointerAMD) or the real API (clSetKernelArgSVMPointer)
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_kernel kernel,
        cl_uint arg_index,
        const void* arg_value,
        cl_int retVal,
        bool isExtension = false)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_kernel = kernel;
        m_arg_index = arg_index;
        m_arg_value = arg_value;

        if (isExtension)
        {
            m_type = CL_FUNC_TYPE_clSetKernelArgSVMPointerAMD;
        }
        else
        {
            m_type = CL_FUNC_TYPE_clSetKernelArgSVMPointer;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clSetKernelArgSVMPointer(const CLAPI_clSetKernelArgSVMPointer& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clSetKernelArgSVMPointer& operator = (const CLAPI_clSetKernelArgSVMPointer& obj);

    cl_kernel   m_kernel;    ///< parameter for CLAPI_clSetKernelArgSVMPointer
    cl_uint     m_arg_index; ///< parameter for CLAPI_clSetKernelArgSVMPointer
    const void* m_arg_value; ///< parameter for CLAPI_clSetKernelArgSVMPointer
    cl_int      m_retVal;    ///< return value for CLAPI_clSetKernelArgSVMPointer
};

//------------------------------------------------------------------------------------
/// CLAPI_clSetKernelExecInfo
//------------------------------------------------------------------------------------
class CLAPI_clSetKernelExecInfo : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clSetKernelExecInfo()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clSetKernelExecInfo()
    {
        if (m_param_value != NULL)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return CLStringUtils::GetErrorString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_kernel) << s_strParamSeparator
           << CLStringUtils::GetKernelExecInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size
           << CLStringUtils::GetKernelExecInfoValueString(m_param_name, m_param_value, m_retVal, m_param_value_size);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clSetKernelExecInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param kernel parameter for CLAPI_clSetKernelExecInfo
    /// \param param_name parameter for CLAPI_clSetKernelExecInfo
    /// \param param_value_size parameter for CLAPI_clSetKernelExecInfo
    /// \param param_value parameter for CLAPI_clSetKernelExecInfo
    /// \param retVal return value
    /// \param isExtension flag indicating whether the application called the extension API (clSetKernelExecInfoAMD) or the real API (clSetKernelExecInfo)
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_kernel kernel,
        cl_kernel_exec_info param_name,
        size_t param_value_size,
        const void* param_value,
        cl_int retVal,
        bool isExtension = false)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_kernel = kernel;
        m_param_name = param_name;
        m_param_value_size = param_value_size;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, m_param_value_size);
        }

        if (isExtension)
        {
            m_type = CL_FUNC_TYPE_clSetKernelExecInfoAMD;
        }
        else
        {
            m_type = CL_FUNC_TYPE_clSetKernelExecInfo;
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clSetKernelExecInfo(const CLAPI_clSetKernelExecInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clSetKernelExecInfo& operator = (const CLAPI_clSetKernelExecInfo& obj);

    cl_kernel   m_kernel;             ///< parameter for CLAPI_clSetKernelExecInfo
    cl_kernel_exec_info m_param_name; ///< parameter for CLAPI_clSetKernelExecInfo
    size_t m_param_value_size;        ///< parameter for CLAPI_clSetKernelExecInfo
    void* m_param_value;              ///< parameter for CLAPI_clSetKernelExecInfo
    cl_int      m_retVal;             ///< return value for CLAPI_clSetKernelExecInfo
};

//------------------------------------------------------------------------------------
/// CLAPI_clCreatePipe
//------------------------------------------------------------------------------------
class CLAPI_clCreatePipe : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clCreatePipe() {}

    /// Destructor
    ~CLAPI_clCreatePipe() {}

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return StringUtils::ToHexString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_context) << s_strParamSeparator
           << CLStringUtils::GetMemFlagsString(m_flags) << s_strParamSeparator
           << m_pipe_packet_size << s_strParamSeparator
           << m_pipe_packet_size << s_strParamSeparator
           << CLStringUtils::GetPipePropertiesString(m_properties, m_vecProperties) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clCreatePipe
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param context parameter for CLAPI_clCreatePipe
    /// \param flags parameter for CLAPI_clCreatePipe
    /// \param pipe_packet_size parameter for CLAPI_clCreatePipe
    /// \param pipe_max_packets parameter for CLAPI_clCreatePipe
    /// \param properties parameter for CLAPI_clCreatePipe
    /// \param errcode_ret parameter for CLAPI_clCreatePipe
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_context context,
        cl_mem_flags flags,
        cl_uint pipe_packet_size,
        cl_uint pipe_max_packets,
        const cl_pipe_properties* properties,
        cl_int* errcode_ret,
        cl_mem retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_context = context;
        m_flags = flags;
        m_pipe_packet_size = pipe_packet_size;
        m_pipe_max_packets = pipe_max_packets;
        m_properties = properties;

        // NOTE: in OCL 2.0 properties MUST be NULL -- so the below code is only being put into place to support future OCL versions where it may be non-NULL
        int num_properties = 0;

        if (properties != NULL)
        {
            // properties is 0 terminated
            while (properties[0] != 0 && num_properties < SP_MAX_NUM_CONTEXT_PROPERTIES)
            {
                m_vecProperties.push_back(properties[0]);
                properties++;
                num_properties++;
            }
        }

        if (num_properties == SP_MAX_NUM_CONTEXT_PROPERTIES)
        {
            //add a dummy value (zero) that tells GetPipePropertiesString that the list has been truncated
            m_vecProperties.push_back(0);
        }

        m_errcode_ret = errcode_ret;

        if (errcode_ret != NULL)
        {
            m_errcode_retVal = *errcode_ret;
        }
        else
        {
            m_errcode_retVal = 0;
        }

        m_type = CL_FUNC_TYPE_clCreatePipe;
        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clCreatePipe(const CLAPI_clCreatePipe& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clCreatePipe& operator = (const CLAPI_clCreatePipe& obj);

    cl_context                m_context;             ///< parameter for CLAPI_clCreatePipe
    cl_mem_flags              m_flags;               ///< parameter for CLAPI_clCreatePipe
    cl_uint                   m_pipe_packet_size;    ///< parameter for CLAPI_clCreatePipe
    cl_uint                   m_pipe_max_packets;    ///< parameter for CLAPI_clCreatePipe
    const cl_pipe_properties* m_properties;          ///< parameter for CLAPI_clCreatePipe
    std::vector<cl_pipe_properties> m_vecProperties; ///< vector containing items defined in properties parameter
    cl_int*                   m_errcode_ret;         ///< parameter for CLAPI_clCreatePipe
    cl_int                    m_errcode_retVal;      ///< parameter for CLAPI_clCreatePipe
    cl_mem                    m_retVal;              ///< return value for CLAPI_clCreatePipe
};

//------------------------------------------------------------------------------------
/// clGetPipeInfo
//------------------------------------------------------------------------------------
class CLAPI_clGetPipeInfo : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clGetPipeInfo()
    {
        m_param_value = NULL;
    }

    /// Destructor
    ~CLAPI_clGetPipeInfo()
    {
        if (m_param_value != NULL)
        {
            FreeBuffer(m_param_value);
        }
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_pipe) << s_strParamSeparator
           << CLStringUtils::GetPipeInfoString(m_param_name) << s_strParamSeparator
           << m_param_value_size << s_strParamSeparator
           << CLStringUtils::GetPipeInfoValueString(m_param_name, m_param_value, m_retVal) << s_strParamSeparator
           << CLStringUtils::GetSizeString(REPLACEDNULLVAL(m_replaced_null_param, m_param_value_size_ret), m_param_value_size_retVal);

        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clGetPipeInfo
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param pipe Parameter for CLAPI_clGetPipeInfo
    /// \param param_name Parameter for CLAPI_clGetPipeInfo
    /// \param param_value_size Parameter for CLAPI_clGetPipeInfo
    /// \param param_value Parameter for CLAPI_clGetPipeInfo
    /// \param param_value_size_ret Parameter for CLAPI_clGetPipeInfo
    /// \param replaced_null_param flag indicating if the user app passed null to param_value_size_ret
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_mem pipe,
        cl_pipe_info param_name,
        size_t param_value_size,
        void* param_value,
        size_t* param_value_size_ret,
        bool replaced_null_param,
        cl_int retVal)
    {
        m_ullStart = ullStartTime;
        m_ullEnd = ullEndTime;
        m_type = CL_FUNC_TYPE_clGetPipeInfo;
        m_pipe = pipe;
        m_param_name = param_name;
        m_param_value_size = param_value_size;
        m_param_value_size_ret = param_value_size_ret;
        m_replaced_null_param = replaced_null_param;
        m_param_value_size_retVal = *param_value_size_ret;

        if (param_value != NULL)
        {
            DeepCopyBuffer(&m_param_value, param_value, RETVALMIN(m_param_value_size_retVal, m_param_value_size));
        }

        m_retVal = retVal;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clGetPipeInfo(const CLAPI_clGetPipeInfo& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clGetPipeInfo& operator = (const CLAPI_clGetPipeInfo& obj);

private:
    cl_mem           m_pipe;                    ///< parameter for clGetPipeInfo
    cl_pipe_info     m_param_name;              ///< parameter for clGetPipeInfo
    size_t           m_param_value_size;        ///< parameter for clGetPipeInfo
    void*            m_param_value;             ///< parameter for clGetPipeInfo
    size_t*          m_param_value_size_ret;    ///< parameter for clGetPipeInfo
    size_t           m_param_value_size_retVal; ///< dereferenced value of m_param_value_size_ret
    bool             m_replaced_null_param;     ///< flag indicating that we've provided a replacement for a NULL m_param_value_size_ret value
    cl_int           m_retVal;                  ///< return value
};

// @}

#endif
