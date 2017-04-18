#ifndef __beDriverUtils_h
#define __beDriverUtils_h

// Infra.
#include <AMDTBaseTools/Include/gtString.h>

class beDriverUtils
{
public:
    static bool GetDriverPackagingVersion(gtString& driverPackagingVersion);
private:
    beDriverUtils();
    ~beDriverUtils();
};

#endif // __beDriverUtils_h
