//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppAidFunctions.cpp
///
//==================================================================================

// Qt:
#include <AMDTApplicationComponents/Include/acIcons.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTPowerProfiling/src/ppAidFunctions.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTPowerProfileApi.h>

// ---------------------------------------------------------------------------
QString ppAidFunction::CounterCategoryToStr(AMDTPwrCategory categoryID)
{
    AMDTResult ret = AMDT_ERROR_FAIL;
    QString categoryName;
    AMDTPwrCategoryInfo category;

    ret = AMDTPwrGetCategoryInfo(categoryID, &category);

    if(AMDT_STATUS_OK == ret)
    {
        categoryName = (char*)category.m_name;
    }
    return categoryName;
}

// ---------------------------------------------------------------------------
acIconId ppAidFunction::GetCategoryIconId(AMDTPwrCategory categoryID)
{
    acIconId categoryIdIcon;

    switch (categoryID)
    {
        case AMDT_PWR_CATEGORY_POWER:
            categoryIdIcon = AC_ICON_PROFILE_PWR_COUNTER_POWER;
            break;

        case AMDT_PWR_CATEGORY_FREQUENCY:
            categoryIdIcon = AC_ICON_PROFILE_PWR_COUNTER_FREQUENCY;
            break;

        case AMDT_PWR_CATEGORY_CURRENT:
            categoryIdIcon = AC_ICON_PROFILE_PWR_COUNTER_CURRENT;
            break;

        case AMDT_PWR_CATEGORY_VOLTAGE:
            categoryIdIcon = AC_ICON_PROFILE_PWR_COUNTER_VOLTAGE;
            break;

        case AMDT_PWR_CATEGORY_DVFS:
            categoryIdIcon = AC_ICON_PROFILE_PWR_COUNTER_DVFS;
            break;

        case AMDT_PWR_CATEGORY_PROCESS:
            categoryIdIcon = AC_ICON_PROFILE_PWR_COUNTER_PID;
            break;

        case AMDT_PWR_CATEGORY_TEMPERATURE:
            categoryIdIcon = AC_ICON_PROFILE_PWR_COUNTER_TEMPRATURE;
            break;

        case AMDT_PWR_CATEGORY_COUNT:
            categoryIdIcon = AC_ICON_PROFILE_PWR_COUNTER_COUNT;
            break;

        case AMDT_PWR_CATEGORY_ENERGY:
            categoryIdIcon = AC_ICON_PROFILE_PWR_COUNTER_POWER;
            break;

        default:
            categoryIdIcon = AC_ICON_EMPTY;
            break;
    }

    return categoryIdIcon;
}

// ---------------------------------------------------------------------------
