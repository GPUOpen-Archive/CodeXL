///
///  Copyright (c) 2008 - 2013 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file oem_structures.h
/// \brief Contains all OEM or other customer structure declarations for \ALL platforms.
///
/// All data structures used in AMD Display Library (ADL) OEM interfaces should be defined in this header file.
///

#ifndef OEM_STRUCTURES_H_
#define OEM_STRUCTURES_H_

// Place OEM/Partner constants and structure definitions here to be properly picked by Doxygen
/// \addtogroup DEFINES
// @{

/// \defgroup define_oem_constants Non-public Constants for Partners APIs
/// This group of definitions are used by the Partners APIs and are not exposed for public use.
// @{

/// \name Definitions used with
/// ADLDisplayNativeAUXChannelData structure
#define ADL_MAX_ACCESSAUXDATA_SIZE                          (16)		// number of char

/// \name ADLDisplayNativeAUXChannelData.iAuxStatus 
#define  ADL_DISPLAY_AUX_STATUS_UNKNOWN                     (0)
#define  ADL_DISPLAY_AUX_STATUS_SUCESSFUL                   (1)
#define  ADL_DISPLAY_AUX_STATUS_FAILED_CHANNEL_BUSY         (2)
#define  ADL_DISPLAY_AUX_STATUS_FAILED_TIMEOUT              (3)
#define  ADL_DISPLAY_AUX_STATUS_FAILED_PROTOCOL_ERROR       (4)
#define  ADL_DISPLAY_AUX_STATUS_FAILED_NACK                 (5)
#define  ADL_DISPLAY_AUX_STATUS_FAILED_INCOMPLETE           (6)
#define  ADL_DISPLAY_AUX_STATUS_FAILED_OPERATION            (7)
#define  ADL_DISPLAY_AUX_STATUS_FAILED_INVALID_OPERATION    (8)
#define  ADL_DISPLAY_AUX_STATUS_FAILED_TIMEOUT_DEFER        (9)

/// \name ADLDisplayNativeAUXChannelData.iCommand

// Read from the end node sink.
#define  ADL_AUX_CHANNEL_READ                               (0)
// Write to the end node sink.
#define  ADL_AUX_CHANNEL_WRITE                              (1)
// Read from immediate device connected to the graphics card.
// In MST case the command will be sent to the first branch.
#define  ADL_AUX_CHANNEL_IMMEDIATEDEVICE_READ              	(2)
// Write to immediate device connected to the graphics card.
// In MST case the command will be sent to the first branch.
#define  ADL_AUX_CHANNEL_IMMEDIATEDEVICE_WRITE              (3)


/// \name Definitions for DDC VCP code for the backlight brightness of monitor
#define ADL_DDC_VCP_CODE_BACKLIGHT              (0x6b)

// For DDC VCP code buffer size
#define ADL_DDC_SETWRITESIZE                     (8)
#define ADL_DDC_GETRQWRITESIZE                   (6)
#define ADL_DDC_GETREPLYWRITESIZE                (1)
#define ADL_DDC_GETREPLYREADSIZE                 (11)
#define ADL_DDC_MAXREADSIZE                      (131)

// For DDC VCP code buffer offset
#define ADL_DDC_GETRQ_VCPCODE_OFFSET             (4)
#define ADL_DDC_GETRQ_CHK_OFFSET                 (5)

#define ADL_DDC_GETRP_RETURNCODE_OFFSET          (3)
#define ADL_DDC_GETRP_MAXHIGH_OFFSET             (6)
#define ADL_DDC_GETRP_MAXLOW_OFFSET              (7)
#define ADL_DDC_GETRP_CURHIGH_OFFSET             (8)
#define ADL_DDC_GETRP_CURLOW_OFFSET              (9)

#define ADL_DDC_SET_VCPCODE_OFFSET               (4)
#define ADL_DDC_SET_HIGH_OFFSET                  (5)
#define ADL_DDC_SET_LOW_OFFSET                   (6)
#define ADL_DDC_SET_CHK_OFFSET                   (7)


/// \name Definitions for Split display

// ADLSplitDisplayCaps
#define ADL_DISPLAY_SPLITDISPLAY_CAPS_STATE_SUPPORTED			0x00000001
#define ADL_DISPLAY_SPLITDISPLAY_CAPS_STATE_AVAILABLE			0x00000002
#define ADL_DISPLAY_SPLITDISPLAY_CAPS_MODE_LEFT_4_3_SUPPORTED	0x00000004

// ADLSplitDisplayState
#define ADL_DISPLAY_SPLITDISPLAY_STATE_DISABLED		0x00000000
#define ADL_DISPLAY_SPLITDISPLAY_STATE_ENABLED		0x00000001
// Transition state is used to force extended mode during enable Split display in ADL_Display_SplitDisplay_RestoreDesktopConfiguration
#define ADL_DISPLAY_SPLITDISPLAY_STATE_INTRANSITION	0x00000002	

// ADLSplitDisplayMode
#define ADL_DISPLAY_SPLITDISPLAY_MODE_NONE			0x00000000
#define ADL_DISPLAY_SPLITDISPLAY_MODE_LEFT_4_3		0x00000001

// ADLSplitDisplayType
#define ADL_DISPLAY_SPLITDISPLAY_TYPE_DISABLED		0x00000000
#define ADL_DISPLAY_SPLITDISPLAY_TYPE_MAIN			0x00000001
#define ADL_DISPLAY_SPLITDISPLAY_TYPE_SHADOW		0x00000002

// Flags for ADL_Display_SplitDisplay_RestoreDesktopConfiguration
#define ADL_DISPLAY_SPLITDISPLAY_DESKTOPCONFIGURATION_VALID						0x00000001
#define ADL_DISPLAY_SPLITDISPLAY_DESKTOPCONFIGURATION_INVALID_PRIMARY_RESTORED	0x00000002
#define ADL_DISPLAY_SPLITDISPLAY_DESKTOPCONFIGURATION_INVALID_POSITION_RESTORED	0x00000004
#define ADL_DISPLAY_SPLITDISPLAY_DESKTOPCONFIGURATION_INTRANSITION_EXTENDED_RESTORED	0x00000008
#define ADL_DISPLAY_SPLITDISPLAY_DESKTOPCONFIGURATION_FAILED	0x00000010 // need to restore, but failed.

// Flags for ADL_Display_SplitDisplay_Caps
#define ADL_DISPLAY_SPLITDISPLAY_CAPS_SUPPORTED		0x00000001

#define ADL_VARIBRIGHT_ENABLED			( 1 << 0 )
#define ADL_VARIBRIGHT_DEFAULTENABLED		( 1 << 1 )

/// \name Definitions for ADL_Display_WriteAndReadI2CLargePayload

// Maximum payload size supported by ADL_Display_WriteAndReadI2CLargePayload
#define ADL_DL_I2C_MAXLARGEPAYLOADSIZE	144

/// \name Definitions for AVIVO color adjustment type
#define ADL_AVIVO_ADJUSTMENT_BRIGHTNESS            0x00000001
#define ADL_AVIVO_ADJUSTMENT_COLOR_VIBRANCE        0x00000002
#define ADL_AVIVO_ADJUSTMENT_CONTRAST              0x00000003
#define ADL_AVIVO_ADJUSTMENT_FLESH_TONE_CORRECTION 0x00000004
#define ADL_AVIVO_ADJUSTMENT_GAMMA_RED             0x00000005
#define ADL_AVIVO_ADJUSTMENT_GAMMA_GREEN           0x00000006
#define ADL_AVIVO_ADJUSTMENT_GAMMA_BLUE            0x00000007
#define ADL_AVIVO_ADJUSTMENT_HUE                   0x00000008
#define ADL_AVIVO_ADJUSTMENT_SATURATION            0x00000009
#define ADL_AVIVO_ADJUSTMENT_TINT                  0x0000000A

// \name Constant values for AVIVO brightness
#define ADL_AVIVO_BRIGHTNESS_DEFAULT                        0

/// \name Constant values for AVIVO color vibrance
#define ADL_AVIVO_COLOR_VIBRANCE_ENABLE                     0
#define ADL_AVIVO_COLOR_VIBRANCE_ENABLE_DEFAULT             0
#define ADL_AVIVO_COLOR_VIBRANCE_DEFAULT                    0
#define ADL_AVIVO_COLOR_VIBRANCE_STEP                       1
#define ADL_AVIVO_COLOR_VIBRANCE_MIN                        1
#define ADL_AVIVO_COLOR_VIBRANCE_MAX                      100

// \name Constant values for AVIVO contrast
#define ADL_AVIVO_CONTRAST_DEFAULT                        100 
 
/// \name Constant values for AVIVO flesh tone
#define ADL_AVIVO_FLESH_TONE_ENABLE                         0
#define ADL_AVIVO_FLESH_TONE_ENABLE_DEFAULT                 0
#define ADL_AVIVO_FLESH_TONE_DEFAULT                        0
#define ADL_AVIVO_FLESH_TONE_STEP                           1
#define ADL_AVIVO_FLESH_TONE_MIN                            1
#define ADL_AVIVO_FLESH_TONE_MAX                          100

// \name Constant values for AVIVO gamma
#define ADL_AVIVO_GAMMA_DEFAULT                             0

// \name Constant values for AVIVO hue/tint
#define ADL_AVIVO_HUE_DEFAULT                               0

// \name Constant values for AVIVO saturation
#define ADL_AVIVO_SATURATION_DEFAULT                        0

/// \defgroup define_ADLI2CLargePayload Flags for ADLI2CLargePayload structure
/// This group of definitions define the flags for the \ref ADLI2CLargePayload structure which is passed
/// into the \ref ADL_Display_WriteAndReadI2CLargePayload function.
// @{
/// Skip the segment offset on I2C transactions when segment is zero  
#define ADL_DL_I2C_FLAG_SKIPEMBEDDEDOFFSET          0x00000001
// @}

// @}
// @}

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing data of native aux channel of DisplayPort display.
///
/// This structure is used to read or write native aux channel of DP display.
/// This structure is used by the ADL_Display_NativeAUXChannel_Access() function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayNativeAUXChannelData
{

/// Size of this struct. The caller should set the size.
  int  iSize;
/// The result status of the function
  int  iAuxStatus;
/// Command. Currently only Read and Write are supported
  int  iCommand;
/// Address of native AUX channel
  int  iAddress;
/// Size of buffer, holds the number of valid bytes in cData.
  int  iDataSize;
/// Data buffer
  char cData[ADL_MAX_ACCESSAUXDATA_SIZE];
} ADLDisplayNativeAUXChannelData;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing the gamut matrix.
///
/// This structure is used to get and set the gamut matrix.
/// This structure is used by the ADL_Display_GamutMapping_Get/Set functions.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct  ADLDisplayGamutMatrix
{
   int  iSize;			            // Size of the structure.
   int  iMatrixSettings[3][3];		// The 3*3 Gamut Matrix.
   int  iOffsets[3];			    // The offset.
} ADLDisplayGamutMatrix;


/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing the CurrentPixelClock
///
/// This structure is used to get the Current Pixel Clock.
/// This structure is used by the ADL_Display_CurrentPixelClock_Get() API
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLCurrentPixelClock
{
/// Size of this struct. The caller should set the size.
	long iSize;
/// Center Frequency
    long iCenterFrequency;
/// Frequency Spread
    long iFrequencySpread;
/// Frequency Spread Type
    long iFrequencySpreadType;
/// Padding 
    long iPadding[12];
} ADLCurrentPixelClock;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing the Pixel Clock Capabilities
///
/// This structure is used to get the Pixel Clock Caps
/// This structure is used by the ADL_Display_PixelClockCaps_Get() API
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLPixelClockCaps
{
/// Size of this struct. The caller should set the size.
    long iSize;
/// Minimal Frequency
    long iMinFrequency;
/// Maximum Frequency
    long iMaxFrequency;
/// Padding 
    long iPadding[13];
} ADLPixelClockCaps;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing the Pixel Clock Allowable Range
///
/// This structure is used to set the Pixel Clock Allowable Range
/// This structure is used by the ADL_Display_PixelClockAllowableRange_Set() API
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLPixelClockRange
{
/// Size of this struct. The caller should set the size.
    long iSize;
/// Minimal Frequency
    long iMinFrequency;
/// Maximum Frequency
    long iMaxFrequency;
/// Padding 
    long iPadding[13];
} ADLPixelClockRange;


/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the Split Display feature.
///
/// This structure is used to store the Split Display Caps information.  
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct tagADLSplitDisplayCaps
{
	/// The logic adapter index for the current Split Display.
   	int iAdapterIndex; 

	/// The displayID for the current Split Display.
    ADLDisplayID displayID; 

	/// The bit mask identifies the bits this structure is currently using. Refer to ADL_DISPLAY_SPLITDISPLAY_CAPS_XXX
    int  iSplitDisplayCapsMask; 

	/// The bit mask identifies the caps status. Refer to ADL_DISPLAY_SPLITDISPLAY_CAPS_XXX
    int  iSplitDisplayCapsValue; 
} ADLSplitDisplayCaps, *LPADLSplitDisplayCaps;


/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the Split Display feature.
///
/// This structure is used to store the Split Display State information.  
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct tagADLSplitDisplayState
{
	/// The adapter index for the current Split Display.
   	int iAdapterIndex; 

	/// The displayID for the current Split Display.
    ADLDisplayID displayID;

	/// The bit mask identifies the bits this structure is currently using. Refer to ADL_DISPLAY_SPLITDISPLAY_STATE_XXX
    int  iSplitDisplayStateMask; 

	/// The bit mask identifies the state status. Refer to ADL_DISPLAY_SPLITDISPLAY_STATE_XXX
	int  iSplitDisplayStateValue; 

} ADLSplitDisplayState, *LPADLSplitDisplayState;

     
/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the Split Display feature.
///
/// This structure is used to store the Split Display Mode information.  
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct tagADLSplitDisplayMode
{
	/// The adapter index for the current Split Display.
   	int iAdapterIndex; 

	/// The displayID for the current Split Display.
	ADLDisplayID displayID; 

	/// The bit mask identifies the bits this structure is currently using. Refer to ADL_DISPLAY_SPLITDISPLAY_MODE_XXX
	int  iSplitDisplayModeMask; 

	///< The bit mask identifies the mode status. Refer to ADL_DISPLAY_SPLITDISPLAY_MODE_XXX
	int  iSplitDisplayModeValue; 

} ADLSplitDisplayMode, *LPADLSplitDisplayMode;


/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the Split Display feature.
///
/// This structure is used to store the Split Display Type information.  
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////       
typedef struct tagADLSplitDisplayType
{
	/// The adapter index for the Split Display.
	int iAdapterIndex; 

	/// The displayID for the Split Display.
    ADLDisplayID displayID; 
       
	/// The bit mask identifies the bits this structure is currently using. Refer to ADL_DISPLAY_SPLITDISPLAY_TYPE_XXX 
	int  iSplitDisplayTypeMask; 
    
	/// The bit mask identifies the type status. Refer to ADL_DISPLAY_SPLITDISPLAY_TYPE_XXX  
	int  iSplitDisplayTypeValue; 
} ADLSplitDisplayType, *LPADLSplitDisplayType;


/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing version information 
///
/// This structure is used to store software version information, description of the display device and a web link to the latest installed Catalyst drivers.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLVersionsInfo
{
	/// Driver Release (Packaging) Version (e.g. 8.71-100128n-094835E-ATI)
	char strDriverVer[ADL_MAX_PATH];
	/// Catalyst Version(e.g. "10.1").
	char strCatalystVersion[ADL_MAX_PATH];
	/// Web link to an XML file with information about the latest AMD drivers and locations (e.g. "http://www.amd.com/us/driverxml" )
	char strCatalystWebLink[ADL_MAX_PATH];

} ADLVersionsInfo, *LPADLVersionsInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the data packet of a display.
///
/// This structure is used to get and set the data packet of a display.
/// This structure is used by the ADL_Display_InfoPacket_Get() and ADL_Display_InfoPacket_Set() functions.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct  ADLDisplayDataPacket
{
/// Size of the structure
	int iSize;
/// One of these values:\n \ref ADL_DL_DISPLAY_DATA_PACKET__INFO_PACKET_RESET or \ref ADL_DL_DISPLAY_DATA_PACKET__INFO_PACKET_SET
	int iFlags;
/// Data Packet Type \ref define_display_packet
	int iPacketType;
///  Structure containing the packet info of a display.
	ADLInfoPacket sInfoPacket;
/// Structure containing the AVI packet info of a display
	ADLAVIInfoPacket sAviInfoPacket;
// Reserved
	char aucReserved[3];
}ADLDisplayDataPacket;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about I2C.
///
/// This structure is used to store the I2C information for the current adapter.
/// This structure is used by \ref ADL_Display_WriteAndReadI2CLargePayload
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct tagADLI2CLargePayload
{
/// Size of the structure
    int iSize;
/// Numerical value representing hardware I2C.
    int iLine;
/// The 7-bit I2C slave device address.
    int iAddress;
/// The offset of the data from the address.
    int iOffset;
/// Read from or write to slave device. \ref ADL_DL_I2C_ACTIONREAD or \ref ADL_DL_I2C_ACTIONWRITE
    int iAction;
/// I2C clock speed in KHz.
    int iSpeed;
/// I2C option flags.  \ref define_ADLI2CLargePayload
	int iFlags;
/// A numerical value representing the number of bytes to be sent or received on the I2C bus.
    int iDataSize;
/// Address of the characters which are to be sent or received on the I2C bus.
    char *pcData;              
} ADLI2CLargePayload;




///\brief Structure containing information about driver point coordinates
///
/// This structure is used to store the driver point coodinates for gamut and white point
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLPoint
{
   /// x coordinate
    int          iX   ;
   /// y coordinate
    int          iY   ;
    
} ADLPoint ;               
/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about driver supported gamut coordinates
///
/// This structure is used to store the driver supported supported gamut coordinates
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLGamutCoordinates
{
   /// red channel chromasity coordinate
    ADLPoint      Red   ;
    /// green channel chromasity coordinate
    ADLPoint      Green ;
    /// blue channel chromasity coordinate
    ADLPoint      Blue  ;
    
} ADLGamutCoordinates ;


/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about driver  gamut space , whether it is related to source or to destination, overlay or graphics
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef  struct ADLGamutReference
{
   /// mask whether it is related to source or to destination, overlay or graphics
    int      iGamutRef;        
    
}ADLGamutReference;
/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about driver current gamut space , parent struct for ADLGamutCoordinates and ADLWhitePoint
/// This structure is used to get/set driver supported gamut space
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef  struct ADLGamutData
{
    ///used as mask and could be 4 options
    ///BIT_0 If flag ADL_GAMUT_REFERENCE_SOURCE is asserted set operation is related to gamut source ,
    ///if not gamut destination
    ///BIT_1 If flag ADL_GAMUT_GAMUT_VIDEO_CONTENT is asserted 
    ///BIT_2,BIT_3 used as mask and could be 4 options custom (2) + predefined (2)
    ///0.  Gamut predefined,        white point predefined -> 0                | 0
    ///1.  Gamut predefined,        white point custom     -> 0                | ADL_CUSTOM_WHITE_POINT
    ///2.  White point predefined,  gamut custom           -> 0                | ADL_CUSTOM_GAMUT
    ///3.  White point custom,      gamut custom           -> ADL_CUSTOM_GAMUT | ADL_CUSTOM_WHITE_POINT
    int        iFeature;        
    
    ///one of ADL_GAMUT_SPACE_CCIR_709 - ADL_GAMUT_SPACE_CIE_RGB
    int         iPredefinedGamut;
    
    ///one of ADL_WHITE_POINT_5000K - ADL_WHITE_POINT_9300K
    int         iPredefinedWhitePoint;
    
    ///valid when in mask avails ADL_CUSTOM_WHITE_POINT 
    ADLPoint             CustomWhitePoint;
    
    ///valid when in mask avails ADL_CUSTOM_GAMUT 
    ADLGamutCoordinates  CustomGamut;   
    
} ADLGamutData;

/*
ADLGamutData gamut; 
if( ADL_Display_Gamut_Get (... &gamut ) == ADL_OK )
{
    ADLGamutReference   ref;
    
    Following bits are valid for iGamutRef when set operation is executed:
    00 and 10 produce same result destination gamut is applied to graphics and overlay
        01 gamut source for graphics only
        11 gamut source for overlay only
        
        reg.iGamutRef =0;
    reg.iGamutRef = ADL_GAMUT_REFERENCE_SOURCE|ADL_GAMUT_GAMUT_VIDEO_CONTENT;
    get reference source for overlay;
    
    reg.iGamutRef = ADL_GAMUT_REFERENCE_SOURCE;
    get reference source for overlay;
    
    reg.iGamutRef = ADL_GAMUT_GAMUT_VIDEO_CONTENT;
    get reference destination for overlay;
    
    if( ADL_Display_Gamut_Get (..,reg &gamut ) == ADL_OK )
    {
        
        int                  iPredefinedGamut;
        int                  iPredefinedWhitePoint;
        ADLPoint             CustomWhitePoint;
        ADLGamutCoordinates  CustomGamut;   
        
        the rule how to read the  gamut using  iFeature
            and assigned variables are valid.
            
            if( gamut.iFeature & ADL_CUSTOM_WHITE_POINT )
            {
                CustomWhitePoint      = gamut.CustomWhitePoint;
            }
            else
            {
                iPredefinedWhitePoint = gamut.iPredefinedWhitePoint;            
            }
            
            if( gamut.iFeature & ADL_CUSTOM_GAMUT )
            {
                CustomGamut       = gamut.CustomGamut;
            }
            else
            {
                iPredefinedGamut = gamut.iPredefinedGamut;            
            }        
    }
}
                  
                    
*/

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about driver supported gamut spaces , capability method
///
/// This structure is used to get driver all supported gamut spaces
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLGamutInfo
{
    
    ///Any combination of following ADL_GAMUT_SPACE_CCIR_709 - ADL_GAMUT_SPACE_CUSTOM
    int    SupportedGamutSpace;   
    
    ///Any combination of following ADL_WHITE_POINT_5000K - ADL_WHITE_POINT_CUSTOM
    int    SupportedWhitePoint;   
    
} ADLGamutInfo;

/*
ADLGamutInfo info; 
ADLGamutReference   ref;

reg.iGamutRef =0;
reg.iGamutRef = ADL_GAMUT_REFERENCE_SOURCE|ADL_GAMUT_GAMUT_VIDEO_CONTENT;

get capabilities for reference source and overlay
if( ADL_Display_Gamut_GetCapabilities (...ref,  &info ) == ADL_OK )
{
    
    the rule how to use the  gamut info 
        string localization for gamut names and white points are omitted.
        
        external CComboBox * pGamutCb ;
    external CComboBox * pWhitePontCb ;
    
    int iSelected ;
    
    if( info.SupportedGamutSpace & ADL_GAMUT_SPACE_CCIR_709 )
    {
        iSelected = pGamutCb->AddString( L"GAMUT_SPACE_CCIR_709" );
        pGamutCb->SetItemData(iSelected, ADL_GAMUT_SPACE_CCIR_709 );                       
    }
    //etc check all available declared flags
    if( info.SupportedWhitePoint & ADL_WHITE_POINT_5000K )
    {
        iSelected = pWhitePontCb->AddString( L"ADL_WHITE_POINT_5000K" );
        pWhitePontCb->SetItemData(iSelected, ADL_WHITE_POINT_5000K );                       
    }
    
    
    if( !info.SupportedGamutSpace & ADL_GAMUT_SPACE_CUSTOM )
    {  
        //application should disable to allow user to use custom gamut
    }
    
    if( !info.SupportedGamutSpace & ADL_WHITE_POINT_CUSTOM )
    {  
        //application should disable to allow user to use custom white point
    }
}
                  
         driver uses following tables and the values would be divided by 10000
         const  GamutSpaceEntry GamutSpace::gamutArray [] = 
                  {
                  primary name               index x_red  y_red  x_gr  y_gr  x_blue y_blue  
                  { "sRgb HDTV, CCIR709",     1,   6400, 3300,  3000, 6000, 1500,  600 } ,
                  { "NTSC(1953)m, CCIR601-1", 2,   6700, 3300,  2100, 7100, 1400,  800 } ,
                  { "Adobe Rgb(1988)"       , 4,   6400, 3300,  2100, 7100, 1500,  600 } ,
                  { "CIE Rgb"               , 8,   7350, 2650,  2740, 7170, 1670,  90  } 
                  
                  } ;
                    
           using CIE 1964
           const  WhitePointCoodinatesEntry GamutSpace::whitePointArray [] = 
                      {
                      primary name   index x_white  y_white  
                      { "5000K Horizon Light. ICC profile PCS"              1,   3477, 3595,   } ,
                      { "6500K Noon Daylight: Television, sRGB color space" 2,   3127, 3310,   } ,
                      { "7500K North sky Daylight "                         4,   2996, 3174,   } ,
                      { "9300K"                                             8,   2866, 2950,   } 
                      
                      } ;
                        
*/

/////////////////////////////////////////////////////////////////////////////////////////////
/// TO BE REMOVED - Replaced by ADL_Display_RegammaCoeffEx
///\brief Structure containing information about driver supported re gamma coefficients used to build re gamma curve
///
/// This structure is used to get/set driver re gamma coefficients used to build ideal re gamma curve
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLReGammaCoefficients
{
    /// flag ADL_EDID_REGAMMA_COEFFICIENTS set or not 
    int     Feature;       
    /// uses divider ADL_REGAMMA_COEFFICIENT_A0_DIVIDER
    int     CoefficientA0; 
    /// uses divider ADL_REGAMMA_COEFFICIENT_A1A2A3_DIVIDER
    int     CoefficientA1;
    /// uses divider ADL_REGAMMA_COEFFICIENT_A1A2A3_DIVIDER
    int     CoefficientA2; 
    /// uses divider ADL_REGAMMA_COEFFICIENT_A1A2A3_DIVIDER
    int     CoefficientA3; 
    
}ADLReGammaCoefficients;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///\brief display gamma RAMP about gamma to programm the regamma  LUT.
/// \nosubgrouping
///////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_Display_GammaRamp
{
   //For gamma ramp we use ushort intentionally    because the range of the values are 0-FFFF & we packed 
   //the array in same way as OS Windows API Set/GetGammaRamp
   //256 ushorts for red, 256 ushorts for green & 256 ushorts for blue
   unsigned short  gamma[256*3];  

} ADL_Display_GammaRamp;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TO BE REMOVED - Replaced by ADL_Display_RegammaCoeffEx
///\brief Structure containing information about driver supported re gamma coefficients used to build re gamma curve
/// This structure is used to get/set driver re gamma coefficients used to build ideal re gamma curve
/// \nosubgrouping
///////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_Display_RegammaCoeff
{
    /// uses divider ADL_REGAMMA_COEFFICIENT_A0_DIVIDER
    int     CoefficientA0[3]; 
    /// uses divider ADL_REGAMMA_COEFFICIENT_A1A2A3_DIVIDER
    int     CoefficientA1[3];
    /// uses divider ADL_REGAMMA_COEFFICIENT_A1A2A3_DIVIDER
    int     CoefficientA2[3]; 
    /// uses divider ADL_REGAMMA_COEFFICIENT_A1A2A3_DIVIDER
    int     CoefficientA3[3]; 

} ADL_Display_RegammaCoeff;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about driver supported re-gamma coefficients used to build re-gamma curve
/// This structure is used to get/set driver re-gamma coefficients that are used to build an ideal re-gamma curve
/// \nosubgrouping
///////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_Display_RegammaCoeffEx
{
    /// uses divider defined in adl_defines.h: ADL_REGAMMA_COEFFICIENT_A0_DIVIDER
    int     CoefficientA0[3]; 
    /// uses divider defined in adl_defines.h: ADL_REGAMMA_COEFFICIENT_A1A2A3_DIVIDER
    int     CoefficientA1[3];
    /// uses divider defined in adl_defines.h: ADL_REGAMMA_COEFFICIENT_A1A2A3_DIVIDER
    int     CoefficientA2[3]; 
    /// uses divider defined in adl_defines.h: ADL_REGAMMA_COEFFICIENT_A1A2A3_DIVIDER
    int     CoefficientA3[3];
	/// uses divider defined in adl_defines.h: ADL_REGAMMA_COEFFICIENT_A1A2A3_DIVIDER
	int		Gamma[3];

} ADL_Display_RegammaCoeffEx;

/////////////////////////////////////////////////////////////////////////////////////////////
/// TO BE REMOVED - Replaced by ADLRegammaEx
///\brief Structure containing information about driver supported re gamma coefficients used to build re gamma curve
///
/// This structure is used to get/set driver re gamma coefficients used to build ideal re gamma curve
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLRegamma
{
    int     Feature;                         //flag ADL_USE_GAMMA_RAMP
	ADL_Display_GammaRamp    gamma;          //use when ADL_USE_GAMMA_RAMP is set
	ADL_Display_RegammaCoeff coefficients;   //use when ADL_USE_GAMMA_RAMP is not set
    
}ADLRegamma;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about driver supported re-gamma coefficients used to build re-gamma curve
///
/// This structure is used to get/set driver re-gamma coefficients used to build ideal re-gamma curve
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLRegammaEx
{
    int                      	Feature;        	//flag ADL_USE_GAMMA_RAMP
	ADL_Display_GammaRamp		gamma;				//use when ADL_USE_GAMMA_RAMP is set
	ADL_Display_RegammaCoeffEx 	coefficientsEx;   	//use when ADL_USE_GAMMA_RAMP is not set
    
}ADLRegammaEx;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///\brief Enum containing information about Display Monitor Power State.
///
/// This enum is used to set Display Monitor Power State on given adapter.
/// This enum is used by \ref ADL_Display_MonitorPowerState_Set 
/// \nosubgrouping
///////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum ADL_Display_MonitorPowerState
{
    /// Power Off State
    ADL_Display_MonitorPowerState_OFF  = 3,
    /// Power On State
    ADL_Display_MonitorPowerState_ON   = 0,

} ADL_Display_MonitorPowerState;


/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing MM video data information.
///
/// This structure is used to store the MM video data information.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLMMVideoData
{
	/// Video item index
	int iVideoItemIndex;
	/// Min value from controller video info
	float fMinValue;
	/// Max value from controller video info
	float fMaxValue;
	/// Default value from controller video info
	float fDefaultValue;
	/// Step
	float fStep;
	/// Current value from controller video info
	float fCurrentValue;
	/// Temporary value
	float fTemporaryValue;
	/// Video adjustable boolean value
	int bVideoAdjustable;
	/// Enable and enableDefault: ENABLE_DEFAULT_MASK = 0x01, ENABLE_CURRENT_MASK = 0x02
	unsigned iEnable;
} ADLMMVideoData, *LPADLMMVideoData;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing MM video set data information.
///
/// This structure is used to set MM video data.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLMMVideoSetData
{
	/// Video item index 
	int iVideoItemIndex;
	/// Current value from controller video data </remarks>
	float fCurrentValue;
	/// Boolean value. If it is true, it is final setting, otherwise, it is temporary setting 
	int bCommited;
	/// Boolean value. If it is true, it is currently enabled, otherwise, it is disabled 
	int bEnabled;
}ADLMMVideoSetData, *LPADLMMVideoSetData;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing MM video capability data
///
/// Video Capability Data Structure, result provided by MM video driver
/// \nosubgrouping
/////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLMMVideoCaps
{
	/// Validate fields
	int iValidFields;
	/// DeinterlaceCaps IDs
	int iDeinterlaceCaps;
	/// PostProcessingCaps IDs
	int iPostProcessingCaps;
	/// ColorManagementCaps IDs, such as MMCAPS_COLOR_VIBRANCE/FLESHTONE/TRUEWHITE
	int iColorManagementCaps;
	/// CSCCaps IDs
	int iCSCCaps;
	/// OTMCaps IDs, such as MMCAPS_OTM_OVERLAY/VMR
	int iOTMCaps;
}ADLMMVideoCaps, *LPADLMMVideoCaps;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing MM video generic capability data
///
/// Video Capability Data Structure, result provided by MM video driver
/// \nosubgrouping
/////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLMMVideoGenericCaps
{
	/// Validate fields
	int iValidFields;
	/// GenericCaps IDs
	int iGenericCaps;
}ADLMMVideoGenericCaps, *LPADLMMVideoGenericCaps;

/////////////////////////////////////////////////////////////////////////////////////////////
/// Structures and definitions for the Multi-Media Re-architecture
/////////////////////////////////////////////////////////////////////////////////////////////


/// Definitions for ClientID
    
typedef enum ADL_FEATURE_CLIENT_ID
{
    ADL_FEATURE_CLIENT_ID_ADL = 0,
    ADL_FEATURE_CLIENT_ID_CCC = 1,
    ADL_FEATURE_CLIENT_ID_MMD = 2,
    ADL_FEATURE_CLIENT_ID_INVALID = 0xffffffff
} ADL_FEATURE_CLIENT_ID;


/// Size in bytes of the Feature Name
#define ADL_FEATURE_NAME_LENGTH 	16	

/// Definitions for ADLFeatureCaps.iFeatureProperties

/// Feature is SUPPORTED on this Adapter
#define ADL_FEATURE_PROPERTIES_SUPPORTED (1 << 0)

/// Feature's STATE. If not set, feature should be GRAYED OUT
#define ADL_FEATURE_PROPERTIES_STATE (1 << 1)

/// Feature is available for Advanced users
#define ADL_FEATURE_PROPERTIES_ADVANCED (1 << 2)

/// Feature is available for Standard users
#define ADL_FEATURE_PROPERTIES_STANDARD (1 << 3)

/// Feature has User-Controlled Boolean
#define ADL_FEATURE_PROPERTIES_TOGGLE (1 << 4)

/// Toggle Boolean Grays/Un-grays Adjustment
#define ADL_FEATURE_PROPERTIES_OVERRIDE (1 << 5)

/// Display related feature. If  0: Render related feature 
#define ADL_FEATURE_PROPERTIES_ISDISPLAY (1 << 6)

/// 0: Nothing, 1: int range, 2: float range, 3: ENUM
#define ADL_FEATURE_PROPERTIES_ADJUSTMASK ((1 << 7) | (1 << 8))

/// int range
#define ADL_FEATURE_PROPERTIES_INT_RANGE   (1 << 7)

/// float range
#define ADL_FEATURE_PROPERTIES_FLOAT_RANGE (2 << 7)

/// ENUM
#define ADL_FEATURE_PROPERTIES_ENUM        (3 << 7)

/// Definitions for ADLFeatureCaps.iControlType
/// CheckBox
#define ADL_FEATURE_CONTROL_TYPE_CHECKBOX 0

/// Radio button
#define ADL_FEATURE_CONTROL_TYPE_RBUTTON  1

/// Definitions for ADLFeatureCaps.iControlStyle and ADLFeatureCaps.iAdjustmentStyle

/// Plain style
#define ADL_FEATURE_STYLE_PLAIN 0

/// Stylized
#define ADL_FEATURE_STYLE_STYLIZED 1

/// Definitions for ADLFeatureCaps.iAdjustmentType

/// Slider
#define ADL_FEATURE_ADJUSTMENT_TYPE_SLIDER 0

/// Spin Box
#define ADL_FEATURE_ADJUSTMENT_TYPE_SPINBOX 1

/// Slider + Spin Box
#define ADL_FEATURE_ADJUSTMENT_TYPE_SLIDER_SBOX 2

/// Text entry box
#define ADL_FEATURE_ADJUSTMENT_TYPE_TEXT 3			

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the Multimedia Feature Name
///
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLFeatureName
{
/// The Feature Name
	char FeatureName[ADL_FEATURE_NAME_LENGTH];
}	ADLFeatureName , *LPADLFeatureName;



/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about MM Feature Capabilities.
///
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLFeatureCaps
{
/// The Feature Name
	ADLFeatureName	Name;
//	char strFeatureName[ADL_FEATURE_NAME_LENGTH];

/// Group ID. All Features in the same group are shown sequentially in the same UI Page.
	int  iGroupID;

/// Visual ID. Places one or more features in a Group Box. If zero, no Group Box is added.
	int  iVisualID;

/// Page ID. All Features with the same Page ID value are shown together on the same UI page.
	int iPageID;

/// Feature Property Mask. Indicates which are the valid bits for iFeatureProperties.
	int iFeatureMask;	

/// Feature Property Values. See definitions for ADL_FEATURE_PROPERTIES_XXX
	int  iFeatureProperties; 
	
/// Apperance of the User-Controlled Boolean.
	int  iControlType; 
	
/// Style of the User-Controlled Boolean.
	int  iControlStyle; 
	
/// Apperance of the Adjustment Controls.
	int  iAdjustmentType; 
	
/// Style of the Adjustment Controls.
	int  iAdjustmentStyle; 

/// Default user-controlled boolean value. Valid only if ADLFeatureCaps supports user-controlled boolean.
   int bDefault;

/// Minimum integer value. Valid only if ADLFeatureCaps indicates support for integers.
	int iMin;

/// Maximum integer value. Valid only if ADLFeatureCaps indicates support for integers.
	int iMax;

/// Step integer value. Valid only if ADLFeatureCaps indicates support for integers.
	int iStep;

/// Default integer value. Valid only if ADLFeatureCaps indicates support for integers.
	int iDefault;

/// Minimum float value. Valid only if ADLFeatureCaps indicates support for floats.
	float fMin;

/// Maximum float value. Valid only if ADLFeatureCaps indicates support for floats.
	float fMax;

/// Step float value. Valid only if ADLFeatureCaps indicates support for floats.
	float fStep;

/// Default float value. Valid only if ADLFeatureCaps indicates support for floats.
	float fDefault;

/// The Mask for available bits for enumerated values.(If ADLFeatureCaps supports ENUM values)
	int EnumMask;

} ADLFeatureCaps, *LPADLFeatureCaps;


/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about MM Feature Values.
///
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLFeatureValues
{
/// The Feature Name
	ADLFeatureName	Name;
//	char strFeatureName[ADL_FEATURE_NAME_LENGTH];

/// User controlled Boolean current value. Valid only if ADLFeatureCaps supports Boolean.
	int bCurrent;

/// Current integer value. Valid only if ADLFeatureCaps indicates support for integers.
	int iCurrent;

/// Current float value. Valid only if ADLFeatureCaps indicates support for floats.
	float fCurrent;

/// The States for the available bits for enumerated values. 
	int EnumStates;

} ADLFeatureValues, *LPADLFeatureValues;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Remote Display caps.
///
/// This structure is used to store the information about remote display capabilities
/// This structure is used by the ADL_RemoteDisplay_Display_Acquire() function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLRemoteDisplayCaps
{
/// Size of the structure
    int iSize;
/// Size of EDID data
    int iEdidSize;
/// EDID data
    char cEDIDData[ADL_MAX_OVERRIDEEDID_SIZE];
/// Reserved, Remote display capabilities, fill with 0xFF
    char cReserved[512];
} ADLRemoteDisplayCaps;


/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Define Remote Display handle
///
/// This handle is used by the ADL_RemoteDisplay_Display_Acquire() and ADL_RemoteDisplay_Display_Release() functions.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef int ADLRemoteDisplayHandle;


/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Contains information about capabilities of specific Video color feature.
///
/// The structure is used with ADL_MMD_VideoColor_Caps call
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLVideoColorCaps
{	
	/// Will be set to 0 if the Video color feature is not supported. Will be set to "Not zero" value otherwise
	int iSupported;
	/// Will be set to 0 if the Video color feature is not enabled. Will be set to "Not zero" value otherwise
	int iEnabled;
	/// Minimum valid value of the Video Color feature
	float fMinValue;
	/// Maximum valid value of the Video Color feature
	float fMaxValue;
	/// Default factory value of the Video Color feature
	float fDefaultValue;
	///	Increment step of the Video Color Feature
	float fStep;
} ADLVideoColorCaps, *LPADLVideoColorCaps;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///\brief Enum defines numeric id for Video Color features such as brightness, contrast, hue 
///
/// This enum is used to identify Avivo video feature to get or set the value or to retrieve the feature capabilities.
/// This enum is used by \ref ADL_MMD_VideoColor_Caps, \ref ADL_MMD_VideoColor_Set, \ref ADL_MMD_VideoColor_Get
/// \nosubgrouping
///////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum ADLVideoColorFeature
{
	ADL_VIDEO_COLOR_FEATURE_BRIGHTNESS			    =0,
	ADL_VIDEO_COLOR_PROPERTY_CONTRAST				=1,
	ADL_VIDEO_COLOR_PROPERTY_HUE					=2,
	ADL_VIDEO_COLOR_PROPERTY_SATURATION				=3,	
	ADL_VIDEO_COLOR_PROPERTY_COLORVIBRANCE			=10,
	ADL_VIDEO_COLOR_PROPERTY_FLESHTONE				=11,
	ADL_VIDEO_COLOR_PROPERTY_GAMMA					=16,
} ADLVideoColorFeature;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the overlap offset info for all the displays for each SLS mode.
///
/// This structure is used to store the no. of overlapped modes for each SLS Mode once user finishes the configuration from Overlap Widget
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLSLSOverlappedMode
{
	/// the SLS mode for which the overlap is configured
	ADLMode SLSMode;
	/// the number of target displays in SLS.
	int iNumSLSTarget;
    /// the first target array index in the target array
	int iFirstTargetArrayIndex;
}ADLSLSTargetOverlap, *LPADLSLSTargetOverlap;

#endif /* OEM_STRUCTURES_H_ */
