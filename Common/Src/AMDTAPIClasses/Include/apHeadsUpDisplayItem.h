//------------------------------ apHeadsUpDisplayItem.h ------------------------------

... Yaki 04 / 09 / 2005 - This file is not used anymore(we replaced the FPS heads up
                                                        display drawing by a preformance counter)

#ifndef __APHEADSUPDISPLAYITEM
#define __APHEADSUPDISPLAYITEM

// Infra:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>

// Local:
#include <AMDTOSWrappers/Include/osTransferableObject.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apHeadsUpDisplayItem : public osTransferableObject
// General Description:
//   Represents a GUI item that can be displayed in the application Heads up display.
//
// Author:               Yaki Tebeka
// Creation Date:        8/4/2004
// ----------------------------------------------------------------------------------
class AP_API apHeadsUpDisplayItem : public osTransferableObject
{
public:
    apHeadsUpDisplayItem();
};


#endif  // __APHEADSUPDISPLAYITEM
