//------------------------------ apFPSDisplayItem.h ------------------------------

... Yaki 04 / 09 / 2005 - This file is not used anymore(we replaced the FPS heads up
                                                        display drawing by a preformance counter)

#ifndef __APFPSDISPLAYITEM
#define __APFPSDISPLAYITEM

// Local:
#include <AMDTAPIClasses/Include/apHeadsUpDisplayItem.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apFPSDisplayItem : public apHeadsUpDisplayItem
// General Description:
//   Represents a GUI item that displays Frame Per Second measurment data.
//
// Author:               Yaki Tebeka
// Creation Date:        8/4/2004
// ----------------------------------------------------------------------------------
class AP_API apFPSDisplayItem : public apHeadsUpDisplayItem
{
public:
    apFPSDisplayItem();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);
};


#endif  // __APFPSDISPLAYITEM
