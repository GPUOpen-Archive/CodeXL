//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suSpiesUtilitiesModuleInitializer.h
///
//==================================================================================

//------------------------------ suSpiesUtilitiesModuleInitializer.h ------------------------------

#ifndef __SUSPIESUTILITIESMODULEINITIALIZER_H
#define __SUSPIESUTILITIESMODULEINITIALIZER_H

// ----------------------------------------------------------------------------------
// Class Name:   suSpiesUtilitiesModuleInitializer
//
// General Description:
//  Initializes the spies utilities module.
//
// Author:               Yaki Tebeka
// Creation Date:        5/1/2010
// ----------------------------------------------------------------------------------
class suSpiesUtilitiesModuleInitializer
{
public:
    static suSpiesUtilitiesModuleInitializer& instance();
    virtual ~suSpiesUtilitiesModuleInitializer();

private:
    // Only my instance method should create me:
    suSpiesUtilitiesModuleInitializer();

    // Holds this class single instance:
    static suSpiesUtilitiesModuleInitializer* _pMySingleInstance;
};


#endif //__SUSPIESUTILITIESMODULEINITIALIZER_H

