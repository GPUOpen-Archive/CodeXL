#ifndef __AMDTRemoteClientDefs_h
#define __AMDTRemoteClientDefs_h

#include "AMDTRemoteClientBuild.h"

// For the daemon communication definitions.
#include <Daemon/Public Include/dmnDefinitions.h>

typedef AMDT_REMOTE_CLIENT_API void (*AMDTRC_BASIC_CALLBACK)(bool isSuccess, void* params);
typedef AMDT_REMOTE_CLIENT_API void (*AMDTRC_SESSION_STATUS_CALLBACK)(bool isSuccess, DaemonSessionStatus status,  void* params);


#endif // __AMDTRemoteClientDefs_h
