//=============================================================
// (c) 2012 Advanced Micro Devices, Inc.
//
/// \author franksw
/// \version $Revision: $
/// \brief  The readme describing the AMDT CodeAnalyst component
//
//=============================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author: $
// Change list:    $Change: $
//=============================================================

The session window and profile logic are under the CpuProfiling folder.
The framework interactions are under the Framework folder.

As good a place as any to describe the issues run into while building a minimal solution which
holds the framework dll and the stand-alone implementation, with the CodeAnalyst component.

1) AMDTApplication depends on the following dlls, obtained from various sub-directories in 
	Depot\Common\Lib\:
	AMDTQtControls(-d).dll
	AMDTQtControls(_d).dll
	FreeImage.dll
	qscintillaAmdDt474(d)2.dll
	QtCoreAmdDt474(d)4.dll
	QtGuiAmdDt474(d)4.dll
	QtOpenGLAmdDt474(d)4.dll
	wxbase292u(d)_vc_custom.dll
	wxmsw292u(d)_adv_vc_custom.dll
	wxmsw292u(d)_aui_vc_custom.dll
	wxmsw292u(d)_core_vc_custom.dll
	wxmsw292u(d)_gl_vc_custom.dll
	wxmsw292u(d)_html_vc_custom.dll
  wxmsw292u(d)_stc_vc_custom.dll
