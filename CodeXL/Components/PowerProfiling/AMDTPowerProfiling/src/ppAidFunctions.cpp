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

// ---------------------------------------------------------------------------
QString ppAidFunction::CounterCategoryToStr(AMDTPwrCategory categoryID)
{
    QString categoryName;

    switch (categoryID)
    {
        case AMDT_PWR_CATEGORY_POWER:
            categoryName = PP_STR_PowerCategoryName;
            break;

        case AMDT_PWR_CATEGORY_CURRENT:
            categoryName = PP_STR_CurrentCategoryName;
            break;

        case AMDT_PWR_CATEGORY_VOLTAGE:
            categoryName = PP_STR_VoltageCategoryName;
            break;

        case AMDT_PWR_CATEGORY_DVFS:
            categoryName = PP_STR_DVFSCategoryName;
            break;

        case AMDT_PWR_CATEGORY_PROCESS:
            categoryName = PP_STR_ProcessCategoryName;
            break;

        case AMDT_PWR_CATEGORY_TEMPERATURE:
            categoryName = PP_STR_TemperatureCategoryName;
            break;

        case AMDT_PWR_CATEGORY_FREQUENCY:
            categoryName = PP_STR_FrequencyCategoryName;
            break;

        case AMDT_PWR_CATEGORY_COUNT:
            categoryName = PP_STR_CountCategoryName;
            break;

        default:
            categoryName = PP_STR_UnsupportedCategoryName;
            break;
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

        default:
            categoryIdIcon = AC_ICON_EMPTY;
            break;
    }

    return categoryIdIcon;
}

// ---------------------------------------------------------------------------
