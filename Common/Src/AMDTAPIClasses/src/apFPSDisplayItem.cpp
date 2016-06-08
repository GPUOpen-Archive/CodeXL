//------------------------------ apFPSDisplayItem.cpp ------------------------------

... Yaki 04 / 09 / 2005 - This file is not used anymore(we replaced the FPS heads up
                                                        display drawing by a preformance counter)

// Infra:
#include <AMDTOSWrappers/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apFPSDisplayItem.h>


// ---------------------------------------------------------------------------
// Name:        apFPSDisplayItem::apFPSDisplayItem
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        9/2/2005
// ---------------------------------------------------------------------------
apFPSDisplayItem::apFPSDisplayItem()
{
}


// ---------------------------------------------------------------------------
// Name:        apFPSDisplayItem::type
// Description: Returns my transferable object type.
// Author:      Yaki Tebeka
// Date:        9/2/2005
// ---------------------------------------------------------------------------
osTransferableObjectType apFPSDisplayItem::type() const
{
    return OS_TOBJ_ID_FPS_DISPLAY_ITEM;
}


// ---------------------------------------------------------------------------
// Name:        apFPSDisplayItem::writeSelfIntoChannel
// Description: Writes self into a channel.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/2/2005
// ---------------------------------------------------------------------------
bool apFPSDisplayItem::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Nothing to write:
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apFPSDisplayItem::readSelfFromChannel
// Description: Reads my self from a channel.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/2/2005
// ---------------------------------------------------------------------------
bool apFPSDisplayItem::readSelfFromChannel(osChannel& ipcChannel)
{
    // Nothing to read:
    return true;
}

