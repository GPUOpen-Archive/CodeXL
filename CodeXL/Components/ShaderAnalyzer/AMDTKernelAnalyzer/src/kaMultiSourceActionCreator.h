//------------------------------ kaMultiSourceActionCreator.h ------------------------------

#ifndef __KAMULYISOURCEACTIONCREATOR_H
#define __KAMULYISOURCEACTIONCREATOR_H


// Infra:
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>

// local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           kaMultiSourceActionCreator : public afActionCreatorAbstract
// General Description:  This class is used for handling the creation and execution of
//                       all actions related to the source code editing
// Author:               Sigal Algranaty
// Creation Date:        1/9/2011
// ----------------------------------------------------------------------------------
class KA_API kaMultiSourceActionCreator : public afActionCreatorAbstract
{

public:

    kaMultiSourceActionCreator();
    ~kaMultiSourceActionCreator();

    // Virtual functions that needs to be implemented:

    // Menu position.
    // Each hierarchy on the ,emu include name/priority.
    // If separator is needed after the item then 's' after the priority is needed
    // in case of a sub menu if one item is marked with an 's' it is enough to mark a separator after it:
    virtual gtString menuPosition(int actionIndex, afActionPositionData& positionData);

    // Toolbar position. separators are defined by "/":
    // Position is defined as in menus:
    virtual gtString toolbarPosition(int actionIndex);

    // Get the group command ids for the action:
    virtual void groupAction(int actionIndex);

protected:

    // Each derived class must populate the vector of supported command Ids
    virtual void populateSupportedCommandIds();

};



#endif //__KAMULYISOURCEACTIONCREATOR_H

