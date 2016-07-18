//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Template class used to control how items are displayed on the HUD overlay.
//==============================================================================

#ifndef SURFACESECTION_H
#define SURFACESECTION_H

#include <AMDTBaseTools/Include/gtASCIIString.h>
#include "CommandProcessor.h"
#include "HUDTextureVisualization.h"
#include "misc.h"


//=============================================================================
/// An arry of SurfaceSectionHUD elements to simplify having a bunch of textures
/// that can be displayed on the HUD
//=============================================================================
template < class DerivedHUDTV > class HUDTextureVisualizationArray : public CommandProcessor
{
public:

    /// Constructor
    /// \param pElementName the name to give each element in this array
    HUDTextureVisualizationArray(const char* pElementName)
    {
        if (pElementName == NULL)
        {
            pElementName = "Item";
        }

        m_pElementArray = NULL;
        m_pElementName = pElementName;
        m_uArraySize = 0;
    }

    /// Destructor
    virtual ~HUDTextureVisualizationArray()
    {
        ClearArray();
    }

    /// Resizes the array
    /// Clears all internal arrays and resizes them
    bool Resize(unsigned int uNumItems)
    {
        DerivedHUDTV* pTmp = NULL;

        if (PsNewArray< DerivedHUDTV >(pTmp, uNumItems) == false)
        {
            return false;
        }

        // allocation was successful, so clear the current arrays
        // and recreate them
        m_Commands.clear();
        m_Processors.clear();
        m_uArraySize = uNumItems;

        ClearArray();

        m_pElementArray = pTmp;

        gtASCIIString name;
        gtASCIIString id;

        for (unsigned int i = 0; i < uNumItems; i++)
        {
            name = "";
            id = "";
            name.appendFormattedString("%s %u", m_pElementName, i);
            id.appendFormattedString("%u", i);
            AddProcessor(m_pElementName,
                         (const char*) name.asCharArray(),
                         (const char*) id.asCharArray(),
                         "",
                         NO_DISPLAY,
                         m_pElementArray[i]);

            m_pElementArray[i].m_uIndex = i;
        }

        return true;
    }

    /// Array index operator
    /// \param uIndex the index to get the element at
    /// \return the element at the specified array index
    DerivedHUDTV& operator [](unsigned int uIndex)
    {
        return m_pElementArray[ uIndex ];
    }

    /// Accessor to the size of the array
    /// \return the number of elements in the array
    unsigned int Size()
    {
        return m_uArraySize;
    }

    /// Sets the pipeline stage that this object belongs to.
    /// \param eStage the SHADER_TYPE
    void SetStage(PIPELINE_STAGE eStage)
    {
        for (unsigned int i = 0 ; i < m_uArraySize ; i ++)
        {
            m_pElementArray[i].SetStage(eStage) ;
        }
    }

private:

    // No additional settings
    virtual std::string GetDerivedSettings() { return ""; }

    /// Clears the array of elements
    void ClearArray()
    {
        SAFE_DELETE_ARRAY(m_pElementArray);
    }

    /// pointer to the element array
    DerivedHUDTV* m_pElementArray;

    /// the number of elements in the array
    unsigned int m_uArraySize;

    /// the shared name of each element
    const char* m_pElementName;
};


#endif // SURFACESECTION_H