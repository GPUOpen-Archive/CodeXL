//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Collections of miscellaneous support fuinctions.
//==============================================================================

#include "misc.h"
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osSystemError.h>
#include "OSWrappers.h"

#if defined (_WIN32)
    #include <windows.h>
    #pragma warning( push, 3 )  // boost generates level 4 warnings. Suppress these
    #include "boost/date_time/posix_time/posix_time.hpp" //include all types plus i/o
    #pragma warning (pop)
    using namespace boost::posix_time;
#else
    #include <math.h>
    #include <unistd.h>   // for getcwd
    #include "WinDefs.h"
#endif

#include "PerfStudioServer_Version.h"

gtASCIIString FormatText(const char* fmt, ...)
{
    char str[10240];
    va_list arg_ptr;

    va_start(arg_ptr, fmt);
    vsprintf_s(str, 10240, fmt, arg_ptr);
    va_end(arg_ptr);

    return str;
}

std::string FormatString(const char* fmt, ...)
{
    char str[10240];
    va_list arg_ptr;

    va_start(arg_ptr, fmt);
    vsprintf_s(str, 10240, fmt, arg_ptr);
    va_end(arg_ptr);

    return std::string(str);
}

#ifdef _LINUX
// On Linux, remove the boost dependency
const char* GetMonthString(int monthIndex)
{
    static const char* months[] =
    {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    return months[monthIndex];
}
#endif

//----------------------------------------------------------------
// Returns a string with the current date and time
//----------------------------------------------------------------
gtASCIIString GetTimeStr()
{
#ifdef _WIN32
    ptime time(second_clock::local_time());
    return to_simple_string(time).c_str();
#else
    gtASCIIString timeString;
    time_t t = ::time(0);           // get time now
    struct tm* now = localtime(&t);
    return FormatText("%d-%s-%02d %02d:%02d:%02d", now->tm_year + 1900, GetMonthString(now->tm_mon), now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
#endif
}

gtASCIIString GetMicroTimeStr()
{
#ifdef _WIN32
    ptime time(microsec_clock::local_time());
    return to_simple_string(time).c_str();
#else
    gtASCIIString timeString;
    time_t t = ::time(0);           // get time now
    struct timeval tv;
    gettimeofday(&tv, NULL);        // get time in microseconds
    struct tm* now = localtime(&t);
    return FormatText("%d-%s-%02d %02d:%02d:%02d.%06d", now->tm_year + 1900, GetMonthString(now->tm_mon), now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, tv.tv_usec);
#endif
}

/// Define common message box flags
const UINT CommonMessageBoxFlags = MB_OK | MB_SETFOREGROUND | MB_TASKMODAL;

void MessageBoxStop(gtASCIIString s)
{
    gtASCIIString sMsg = "Stop: ";
    sMsg += s;
    OSWrappers::MessageBox((const char*)sMsg.asCharArray(), (const char*)PERFSTUDIOSERVER_APP_NAME, MB_ICONSTOP | CommonMessageBoxFlags);
}

void MessageBoxInfo(gtASCIIString s)
{
    gtASCIIString sMsg = "Info: ";
    sMsg += s;
    OSWrappers::MessageBox((const char*)sMsg.asCharArray(), (const char*)PERFSTUDIOSERVER_APP_NAME, MB_ICONINFORMATION | CommonMessageBoxFlags);
}

void MessageBoxWarning(gtASCIIString s)
{
    gtASCIIString sMsg = "Warning: ";
    sMsg += s;
    OSWrappers::MessageBox((const char*)sMsg.asCharArray(), (const char*)PERFSTUDIOSERVER_APP_NAME, MB_ICONWARNING | CommonMessageBoxFlags);
}

void MessageBoxError(gtASCIIString s)
{
    gtASCIIString sMsg = "Error: ";
    sMsg += s;
    OSWrappers::MessageBox((const char*)sMsg.asCharArray(), (const char*)PERFSTUDIOSERVER_APP_NAME, MB_ICONERROR | CommonMessageBoxFlags);
}

gtASCIIString DumpHex(unsigned char* pData, unsigned long dwSize, unsigned long dwLength)
{
    gtASCIIString out;

    while (dwSize > 0)
    {
        unsigned long len = dwSize;

        if (dwSize > dwLength)
        {
            len = dwLength;
        }

        out += FormatText("0x%p: ", pData);

        for (unsigned long i = 0; i < len; i++)
        {
            out += FormatText("%02x ", pData[i]);
        }

        for (unsigned long i = len; i < dwLength; i++)
        {
            out += FormatText("   ");
        }

        out += FormatText("     ");

        for (unsigned long i = 0; i < len; i++)
        {
            if (pData[i] > 32 && pData[i] < 'z')
            {
                out += FormatText("%c", pData[i]);
            }
            else
            {
                out += FormatText(".");
            }
        }

        for (unsigned long i = len; i < dwLength; i++)
        {
            out += FormatText(" ");
        }

        out += FormatText("\n");

        pData += len;
        dwSize -= len;
    }

    return out;
}


//----------------------------------------------------------------
/// Returns a string with the hex value on the char
/// \param dec Input char
/// \return Hex string
//----------------------------------------------------------------
std::string CharToHex(char dec)
{
    char dig1 = (dec & 0xF0) >> 4;
    char dig2 = (dec & 0x0F);

    if (0 <= dig1 && dig1 <= 9) { dig1 += '0'; }

    if (10 <= dig1 && dig1 <= 15) { dig1 += 'a' - 10; }

    if (0 <= dig2 && dig2 <= 9) { dig2 += '0'; }

    if (10 <= dig2 && dig2 <= 15) { dig2 += 'a' - 10; }

    std::string r;
    r.append(&dig1, 1);
    r.append(&dig2, 1);
    return r;
}

//----------------------------------------------------------------
/// URL encode an string, replace illegal chars with %hex
/// \param in Input string
/// \return string
//----------------------------------------------------------------
std::string UrlEncode(std::string in)
{
    std::string::size_type ini = 0;
    std::string out;

    for (std::string::size_type loc = 0; loc < in.length(); loc++)
    {
        char c = in[loc];

        if (c == 0)
        {
            out += std::string(&in[ini], loc - ini);
            break;
        }
        else if ((c == '<') || (c == '>') || (c == '\n') || (c == '\r') || (c == '&'))
        {
            out += std::string(&in[ini], loc - ini) + std::string("%") + CharToHex(c);
            ini = loc + 1; //+1 to skip the illegal character
        }

    }

    return out;
}
/// Error return string
char strError [64];

/// Set the error return string
/// \param eVal Input error val
/// \return Error string
const char* GetErrorString(unsigned int eVal)
{
    switch (eVal)
    {
        case S_OK:
        {
            strcpy_s(strError, 64, "S_OK");
            break;
        }

        case S_FALSE:
        {
            strcpy_s(strError, 64, "S_FALSE");
            break;
        }

        default:
        {
            sprintf_s(strError, 64, "%d unsupported error value", eVal);
            break;
        }
    }

    return strError;
}

//
/// Returns a thread's ID, this function helps debugging by, for example forcing the returned value to be always the same
//
DWORD GetThreadsID()
{
    return osGetCurrentThreadId();
}

//
/// This class helps detect reentrance problems
//
RefTrackerCounter::RefTrackerCounter()
{
    m_IsUsingExternalMutex = false;
    m_pmutex = new mutex();
}

RefTrackerCounter::~RefTrackerCounter()
{
    if (m_IsUsingExternalMutex == false)
    {
        delete m_pmutex;
    }
}

RefTrackerCounter::RefTrackerCounter(mutex* pM)
{
    m_IsUsingExternalMutex = true;

    m_pmutex = pM;
}

void RefTrackerCounter::UseExternalMutex(mutex* pM)
{
    if (m_IsUsingExternalMutex == false)
    {
        delete m_pmutex;
    }

    m_IsUsingExternalMutex = true;

    m_pmutex = pM;


}

void RefTrackerCounter::operator++(int)
{
    ScopeLock lock(m_pmutex);

    UINT32 dwThreadId = GetThreadsID();
    std::map<UINT32, int>::iterator it = m_mapInsideWrapper.find(dwThreadId);

    if (it != m_mapInsideWrapper.end())
    {
        it->second++;
    }
    else
    {
        m_mapInsideWrapper[dwThreadId] = 1;
    }
}

void RefTrackerCounter::operator--(int)
{
    ScopeLock lock(m_pmutex);

    UINT32 dwThreadId = GetThreadsID();
    std::map<UINT32, int>::iterator it = m_mapInsideWrapper.find(dwThreadId);

    if (it != m_mapInsideWrapper.end())
    {
        it->second--;
    }
    else
    {
        //not found? this should be impossible, assert!!
        PsAssert(it != m_mapInsideWrapper.end());
    }
}

bool RefTrackerCounter::operator==(UINT32 v)
{
    ScopeLock lock(m_pmutex);

    UINT32 dwThreadId = GetThreadsID();
    std::map<UINT32, int>::iterator it = m_mapInsideWrapper.find(dwThreadId);

    if (it != m_mapInsideWrapper.end())
    {
        return it->second == (int)v;
    }

    //not found? then its value is zero
    return (v == 0);
}

bool RefTrackerCounter::operator>(UINT32 v)
{
    ScopeLock lock(m_pmutex);

    UINT32 dwThreadId = GetThreadsID();
    std::map<UINT32, int>::iterator it = m_mapInsideWrapper.find(dwThreadId);

    if (it != m_mapInsideWrapper.end())
    {
        return (UINT32)it->second > v;
    }

    return false;
}

UINT32 RefTrackerCounter::GetRef()
{
    ScopeLock lock(m_pmutex);

    UINT32 dwThreadId = GetThreadsID();
    std::map<UINT32, int>::iterator it = m_mapInsideWrapper.find(dwThreadId);

    if (it != m_mapInsideWrapper.end())
    {
        return it->second;
    }

    //zero
    return 0;
}

#if defined (_WIN32)
string GetErrorStringFromHRESULT(HRESULT hRes)
{
    switch (hRes)
    {
        case E_UNEXPECTED :
            return string("E_UNEXPECTED");

        case E_NOTIMPL :
            return string("E_NOTIMPL");

        case E_OUTOFMEMORY :
            return string("E_OUTOFMEMORY");

        case E_INVALIDARG :
            return string("E_INVALIDARG");

        case E_NOINTERFACE  :
            return string("E_NOINTERFACE");

        case E_POINTER :
            return string("E_POINTER");

        case E_HANDLE :
            return string("E_HANDLE");

        case E_ABORT :
            return string("E_ABORT");

        case E_FAIL :
            return string("E_FAIL");

        case E_ACCESSDENIED :
            return string("E_ACCESSDENIED");

        default:
            return string("UNKNOWN");
    }
}
#endif // _WIN32

/// Tests a result for failure and aserts and logs and error.
/// \param error The result to test
/// \param strMessage The message to log
void hResultLogAndAssert(INT32 error, const char* strMessage)
{
    PsAssert(error == NO_ERROR);

    if (error != NO_ERROR)
    {
        Log(logERROR, "%s", strMessage);
    }
}

/// Decodes a URL encoded string.
/// \param pszDecodedOut The decoded output string
/// \param nBufferSize The size of the output string (must be at least the size of pszEncodedIn)
/// \param pszEncodedIn The encoded input string
/// \param bIgnoreLinefeed Flag to ignore the linefeed command.
void URLDecode(char* pszDecodedOut, size_t nBufferSize, const char* pszEncodedIn, bool bIgnoreLinefeed)
{
    memset(pszDecodedOut, 0, nBufferSize);

    enum DecodeState_e
    {
        STATE_SEARCH = 0, ///< searching for an ampersand to convert
        STATE_CONVERTING, ///< convert the two proceeding characters from hex
    };

    DecodeState_e state = STATE_SEARCH;

    for (unsigned int i = 0; i < strlen(pszEncodedIn) - 1; ++i)
    {
        switch (state)
        {
            case STATE_SEARCH:
            {
                if (pszEncodedIn[i] != '%')
                {
                    strncat_s(pszDecodedOut, nBufferSize, &pszEncodedIn[i], 1);
                    break;
                }

                // We are now converting
                state = STATE_CONVERTING;
            }
            break;

            case STATE_CONVERTING:
            {
                // Conversion complete (i.e. don't convert again next iter)
                state = STATE_SEARCH;

                // Create a buffer to hold the hex. For example, if %20, this
                // buffer would hold 20 (in ASCII)
                char pszTempNumBuf[3] = {0};
                strncpy_s(pszTempNumBuf, 3, &pszEncodedIn[i], 2);

                // Ensure both characters are hexadecimal
                bool bBothDigits = true;

                for (int j = 0; j < 2; ++j)
                {
                    if (!isxdigit(pszTempNumBuf[j]))
                    {
                        bBothDigits = false;
                    }
                }

                if (!bBothDigits)
                {
                    break;
                }

                // Convert two hexadecimal characters into one character
                int nAsciiCharacter;
                sscanf_s(pszTempNumBuf, "%x", &nAsciiCharacter);

                // Ensure we aren't going to overflow
                assert(strlen(pszDecodedOut) < nBufferSize);

                // Concatenate this character onto the output
                int linefeed = 0x0D;

                // Ignore the linefeed
                if (!(bIgnoreLinefeed == true && nAsciiCharacter == linefeed))
                {
                    strncat_s(pszDecodedOut, nBufferSize, (char*)&nAsciiCharacter, 1);
                }

                // Skip the next character
                i++;
            }
            break;
        }
    }
}

#ifdef _WIN32
bool enable_token_privilege(HANDLE htok, LPCTSTR szPrivilege, TOKEN_PRIVILEGES* tpOld)
{
    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (LookupPrivilegeValue(0, szPrivilege, &tp.Privileges[0].Luid))
    {
        DWORD cbOld = sizeof(*tpOld);

        if (AdjustTokenPrivileges(htok, FALSE, &tp, cbOld, tpOld, &cbOld))
        {
            return (ERROR_NOT_ALL_ASSIGNED != osGetLastSystemError());
        }
        else
        {
            Log(logWARNING, "enable_token_privilege\n");

            return false;
        }
    }
    else
    {
        Log(logWARNING, "enable_token_privilege\n");

        return (FALSE);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Enable privilege
/// \param szPrivilege Privilige
//////////////////////////////////////////////////////////////////////////////////////////////////////
bool enable_privilege(LPCTSTR szPrivilege)
{
    bool bReturn = FALSE;
    HANDLE hToken;
    TOKEN_PRIVILEGES tpOld;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        Log(logWARNING, "enable_privilege\n");

        return false;
    }

    bReturn = enable_token_privilege(hToken, szPrivilege, &tpOld);
    CloseHandle(hToken);

    return bReturn;
}


std::string GetThreadString()
{
    std::string str ;

    str = " ThreadID: " ;

    void* p = GetCurrentThread();

    std::stringstream ss;

    ss << (size_t)p;

    str.append(ss.str());

    return str;
}
#endif // def _WIN32

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Designed to work on binary data wherte strstr function cannot be used
/// Searches for a substring within the buf
/// \param buf String to search within
/// \param len Length of the input string
/// \param s The substring to search for
//////////////////////////////////////////////////////////////////////////////////////////////////////
long find_string_in_buf(unsigned char* buf, size_t len, const char* s)
{
    size_t i, j;
    size_t slen = strlen(s);
    size_t imax = len - slen - 1;
    long ret = -1;
    int match;

    for (i = 0; i < imax; i++)
    {
        match = 1;

        for (j = 0; j < slen; j++)
        {
            if (buf[i + j] != s[j])
            {
                match = 0;
                break;
            }
        }

        if (match)
        {
            ret = (long)i;
            break;
        }
    }

    return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// A hacky function to detect the end of an ASCI  string within a binary file
/// \param buf The input data
/// \param len Length of the input data
/// \param start The index into the main string to start searching from
/// \param str Records the ASCI section in a reference that can be read outside of this function
//////////////////////////////////////////////////////////////////////////////////////////////////////
long find_non_ascii(unsigned char* buf, long len, long start, std::string& str)
{
    unsigned char max = 0 ;
    unsigned char min = 255;

    while (start < len)
    {
        unsigned char val = buf[start];

        str += val;

        if (val < min)
        {
            min = val;
        }

        if (val > max)
        {
            max = val;
        }

        if (val > 169)
        {
            return start;
        }

        if (val < 0)
        {
            return start;
        }

        start++;
    }

    return -1;
}

/// Get the GPS dir name
/// \return String name
const char* GetPerfStudioDirName()
{
    static const char* PERFSTUDIO_DIR_NAME = "GPUPerfStudio";
    return PERFSTUDIO_DIR_NAME;
}

//-----------------------------------------------------------------------------
float HalfToFloat(unsigned short half)
{
    float result;

    unsigned short sign = (half & 0x8000) >> 15;
    unsigned short exponent = (half & 0x7c00) >> 10;
    unsigned short mantissa = (half & 0x03ff);

    if (exponent == 0 && mantissa == 0)
    {
        result = pow(-1.0f, (float)sign) * 0.0f;
    }
    else if (exponent == 0 && mantissa != 0)
    {
        result = pow(-1.0f, (float)sign) * pow(2.0f, -14.0f) * ((float)mantissa / pow(2.0f, 10.0f));
    }
    else
    {
        result = pow(-1.0f, (float)sign) * pow(2.0f, (float)exponent - 15.0f) * (1 + ((float)mantissa / pow(2.0f, 10.0f)));
    }

    return result;
}

//--------------------------------------------------------------
/// Gets the directory of the current module
/// \param out if functions returns true, contains the path to the current module
/// \return true if the path could could be found; false otherwise
//--------------------------------------------------------------
bool GetModuleDirectory(gtASCIIString& out)
{
    char sDir[PS_MAX_PATH];
#if defined (WIN32)
    DWORD len = GetModuleFileName(NULL, sDir, PS_MAX_PATH);
    const char separator = '\\';
#elif defined (_LINUX)

    if (program_invocation_name[0] == '/')
    {
        // file contains full path already, so don't append path
        sprintf_s(sDir, PS_MAX_PATH, "%s", program_invocation_name);
    }
    else
    {
        char currentDir[PS_MAX_PATH];
        char* buf = NULL;
        buf = getcwd(currentDir, PS_MAX_PATH);

        if (buf != NULL)
        {
            sprintf_s(sDir, PS_MAX_PATH, "%s/%s", currentDir, program_invocation_name);
        }
        else
        {
            LogConsole(logERROR, "failed to get current directory\n");
            return false;
        }
    }

    DWORD len = ::strlen(sDir);
    const char separator = '/';
#endif

    if (len > 0)
    {
        DWORD i;

        for (i = len; i > 0; i--)   // note it doesn't check sDir[0]
            if (sDir[i] == separator)
            {
                sDir[i + 1] = '\0';
                out = gtASCIIString(sDir);   // + 1 to include the last '\'
                return true;
            }

        return false;
    }

    return false;
}

#if defined (_WIN32)
//-----------------------------------------------------------------------------
/// Parse app name and show reminder messages where applicable
/// Params: -
/// char * szAppName - name of application, must be in all lower case
//-----------------------------------------------------------------------------
void ShowLauncherReminder(const char* szAppName)
{
#ifdef CODEXL_GRAPHICS
    GT_UNREFERENCED_PARAMETER(szAppName);

    return;
#else
    int iRmdMsg = 0;

    if (strstr(szAppName, "steam.exe") != NULL)
    {
        iRmdMsg = RMD_MSG_OL_STEAM;
    }
    else if (strstr(szAppName, "origin.exe") != NULL)
    {
        iRmdMsg = RMD_MSG_OL_ORIGIN;
    }
    else if (strstr(szAppName, "uplay.exe") != NULL)
    {
        iRmdMsg = RMD_MSG_OL_UPLAY;
    }

    if (0 < iRmdMsg)
    {
        LauncherReminderMessage(iRmdMsg);
    }

#endif

}

//-----------------------------------------------------------------------------
/// Parse app name and show reminder messages where applicable
/// Params: -
/// wchar_t * szAppName - name of application, must be in all lower case
//-----------------------------------------------------------------------------
void ShowLauncherReminder(const wchar_t* szAppName)
{
#ifdef CODEXL_GRAPHICS
    GT_UNREFERENCED_PARAMETER(szAppName);

    return;
#else

    int iRmdMsg = 0;

    if (wcsstr(szAppName, L"steam.exe") != NULL)
    {
        iRmdMsg = RMD_MSG_OL_STEAM;
    }
    else if (wcsstr(szAppName, L"origin.exe") != NULL)
    {
        iRmdMsg = RMD_MSG_OL_ORIGIN;
    }
    else if (wcsstr(szAppName, L"uplay.exe") != NULL)
    {
        iRmdMsg = RMD_MSG_OL_UPLAY;
    }

    if (0 < iRmdMsg)
    {
        LauncherReminderMessage(iRmdMsg);
    }

#endif

}

//-----------------------------------------------------------------------------
/// Show dismissable reminder messages
/// Params: -
/// int iRmdMsg - index of messages
//-----------------------------------------------------------------------------
void LauncherReminderMessage(int iRmdMsg)
{
    TCHAR szMessage[1024] = { 0 };

    switch (iRmdMsg)
    {
        case RMD_MSG_OL_STEAM:
            sprintf_s(szMessage, 1024, TEXT("The in-game Steam overlay prevents GPU PerfStudio from working.\n\nPlease disable it in the \"Steam/Settings/In-Game\" user interface.\n\nMake sure that the \"Enable the Steam Overlay while in-game\" option is turned off.\n\nMore details are available in the GPU PerfClient Help pages in the \"Connecting To An Application/Disable In-Game Overlay Functionality\" section."));
            break;

        case RMD_MSG_OL_ORIGIN:
            sprintf_s(szMessage, 1024, TEXT("The in-game Origin overlay prevents GPU PerfStudio from working.\n\nPlease disable it in the \"Origin/Application Settings/Origin In Game\" settings user interface. \n\nMake sure that the \"Enable Origin In Game\" option is turned off.\n\nMore details are available in the GPU PerfClient Help pages in the \"Connecting To An Application/Disable In-Game Overlay Functionality\" section."));
            break;

        case RMD_MSG_OL_UPLAY:
            sprintf_s(szMessage, 1024, TEXT("The in-game UPlay overlay prevents GPU PerfStudio from working.\n\nPlease disable it in the \"UPlay/Settings/General\" user interface. \n\nMake sure that the \"Enable in-game overlay for supported games\" option is turned off.\n\nMore details are available in the GPU PerfClient Help pages in the \"Connecting To An Application/Disable In-Game Overlay Functionality\" section."));
            break;
    }

    if (RMD_MSG_OL_NONE < iRmdMsg)
    {
        // show reminder dialogue with dismiss check box. To re-enable message, delete reg keys
        MessageBoxWarning(szMessage);
    }
}

#endif // (_WIN32)
