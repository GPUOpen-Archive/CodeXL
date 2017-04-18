#ifndef _DX12_FUNCTION_ENUM_DEFS_H_
#define _DX12_FUNCTION_ENUM_DEFS_H_

// Graphics server
#include "Server/DX12Server/D3D12Enumerations.h"

class DX12FunctionDefs
{
public:
    static eAPIType GetAPIGroupFromAPI(FuncId inAPIFuncId);
};


#endif //_DX12FUNCTION_ENUM_DEFS_H_


