//------------------------------ GraphicsServerInitFunc.cpp ------------------------------

// Local:
#include <AMDTOSWrappers/Include/osTransferableObjectCreator.h>
#include <AMDTOSWrappers/Include/osTransferableObjectCreatorsManager.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include "../../Include/GraphicsServerInitFunc.h"


// ---------------------------------------------------------------------------
// Name:        GraphicsServerInterfaceInitFunc
// Description: Initialization function for the GraphicsServerInterface library.
// Return Val:  bool - Success / failure
// Author:      Yaki Tebeka
// Date:        2/6/2004
// Implementation Notes:
//   Registeres all the GraphicsServerInterface transferable objects in the transferable
//   objects creator manager.
// ---------------------------------------------------------------------------
bool GraphicsServerInterfaceInitFunc()
{
    // Verify that this function code is executed only once:
    static bool wasThisFunctionCalled = false;

    if (!wasThisFunctionCalled)
    {
        wasThisFunctionCalled = true;

        // Get the osTransferableObjectCreatorsManager single instance:
        osTransferableObjectCreatorsManager& theTransfetableObsCreatorsManager = osTransferableObjectCreatorsManager::instance();

        // ----------- Register transferable objects creators -----------

        osTransferableObjectCreator<osFilePath> osFilePathCreator;
        theTransfetableObsCreatorsManager.registerCreator(osFilePathCreator);
    }

    return true;
}


