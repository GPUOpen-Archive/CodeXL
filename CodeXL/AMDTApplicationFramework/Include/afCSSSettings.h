//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afCSSSettings.h
///
//==================================================================================

#ifndef __AFCSSSETTINGS_H
#define __AFCSSSETTINGS_H

/// Project settings CSS style settings:
#define AF_STR_WhiteBG "background-color: white;"
#define AF_STR_BGWithParam "background-color: %1;"
#define AF_STR_CSSBold "font-weight:bold;"
#define AF_STR_BlueFont "color: blue;"
#define AF_STR_settingsContainerCSS "QFrame { border-style: 1px solid Gray; background-color: white;}"
#define AF_STR_captionLabelStyleSheet "background-color:#EEEEEE; text-align:left; font-weight:bold; height:25px; padding-top: 3px; padding-bottom: 3px; margin-top:10px;"
#define AF_STR_captionLabelStyleSheetMain "background-color:#EEEEEE; text-align:left; font-weight:bold; height:25px; padding-top: 3px; padding-bottom: 3px; "
#define AF_STR_treeWidgetWithBorderStyleSheet "QTreeWidget { border: 1px solid Gray; background-color: white;}"
#define AF_STR_groupBoxWhiteBG "QGroupBox{ background-color: white; margin: 0px; border-style: none;}"
#define AF_STR_grayBorderWhiteBackgroundTE "QTextEdit { border: 1px solid Gray; background-color: white;}"
#define AF_STR_noBorderAlignLeft "text-align:left; border: none;"
#define AF_STR_StartupDialogPushButtonCSS "QPushButton {" \
    "margin: 0px;" \
    "padding-top: 0px;" \
    "padding-right: 5px;" \
    "padding-left: 5px;" \
    "padding-bottom: 0px;" \
    "color: black;" \
    "border: none;" \
    "}" \
    "QPushButton:pressed {" \
    "border-color: #85BCFF;" \
    "border-style: outset;" \
    "border-radius: 3px;" \
    "border-width: 1px;" \
    "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #CFE5FF, stop: 1 #D8EBFF);" \
    "}" \
    "QPushButton:hover {" \
    "border-color: #85BCFF;" \
    "border-style: outset;" \
    "border-radius: 3px;" \
    "border-width: 1px;" \
    "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #CFE5FF, stop: 1 #D8EBFF);" \
    "}"
#endif //__AFCSSSETTINGS_H

