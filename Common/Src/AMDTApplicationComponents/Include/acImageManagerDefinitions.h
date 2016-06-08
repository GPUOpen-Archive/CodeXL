//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acImageManagerDefinitions.h
///
//==================================================================================

//------------------------------ acImageManagerDefinitions.h ------------------------------

#ifndef __ACIMAGEMANAGERDEFINITIONS_H
#define __ACIMAGEMANAGERDEFINITIONS_H


// Sizing definitions:
#define AC_IMAGE_MANAGER_IMAGE_BORDER_SIZE 1
#define AC_IMAGE_MANAGER_IMAGE_BORDER_COLOR Qt::black
#define AC_IMAGE_MANAGER_TOP_MARGIN 7
#define AC_IMAGE_MANAGER_IMAGE_TO_LABEL_MARGIN 2
#define AC_IMAGE_MANAGER_THUMBNAIL_BG_COLOR QColor(236, 233, 216, 255)

// ----------------------------------------------------------------------------------
// Class Name:          AC_API acDisplayedImageProperties
// General Description: This class is used to represent properties for a displayed
//                      item. It contain data used for displaying the object.
// Author:              Sigal Algranaty
// Creation Date:       8/11/2010
// ----------------------------------------------------------------------------------
class AC_API acDisplayedItemProperties
{
public:

    acDisplayedItemProperties():
        _actionsMask(AC_IMAGE_CHANNEL_RED | AC_IMAGE_CHANNEL_GREEN | AC_IMAGE_CHANNEL_BLUE | AC_IMAGE_CHANNEL_ALPHA),
        _zoomLevel(100), _position(0, 0), _rotateAngle(0), _activePage(0), _isNormalized(false), _minValue(0), _maxValue(0), _arePropertiesValid(false) {}

    acDisplayedItemProperties(const acDisplayedItemProperties& other)
    {
        _actionsMask = other._actionsMask;
        _zoomLevel = other._zoomLevel;
        _position = other._position;
        _rotateAngle = other._rotateAngle;
        _activePage = other._activePage;
        _isNormalized = other._isNormalized;
        _minValue = other._minValue;
        _maxValue = other._maxValue;
        _arePropertiesValid = other._arePropertiesValid;
    }

    // Indicates which of the image actions are applied for the item:
    unsigned int _actionsMask;

    // Preserves item properties
    int _zoomLevel;
    QPoint _position;
    double _rotateAngle;
    int _activePage;
    bool _isNormalized;
    double _minValue;
    double _maxValue;

    // True iff the image properties are valid:
    bool _arePropertiesValid;

};



#endif //__ACIMAGEMANAGERDEFINITIONS_H

