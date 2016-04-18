//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTTeapotConstants.h
///
//==================================================================================

//------------------------------ AMDTTeapotConstants.h ------------------------------

#ifndef __AMDTTEAPOTCONSTANTS_H
#define __AMDTTEAPOTCONSTANTS_H

// "Commands" menu:
#define ID_COMMAND_MAKE_CRASH                   111
#define ID_COMMAND_OUTPUT_SAMPLE_DEBUG          112
#define ID_COMMAND_GENERATE_OPENGL_ERROR        113
#define ID_COMMAND_GENERATE_BREAK_POINT         114
#define ID_COMMAND_DETECTED_ERROR               115
#define ID_COMMAND_TOGGLE_SHADING_PARAMETERS    116
#define ID_COMMAND_TOGGLE_MODEL_VIEW_MATRIX     117
#define ID_COMMAND_TOGGLE_FRAGMENT_SHADERS      118
#define ID_COMMAND_GENERATE_OPENCL_ERROR        119


// "View" menu:
#define ID_VIEW_TOGGLE_BACKGROUND_COLOR         121
#define ID_VIEW_TOGGLE_SHADING_PROGRAM          122
#define ID_VIEW_TOGGLE_WIREFRAME_MODE           123
#define ID_VIEW_INCREASE_TEXTURE_INFLUENCE      124
#define ID_VIEW_DECREASE_TEXTURE_INFLUENCE      125
#define ID_VIEW_TOGGLE_GEOMETRY_SHADER          126
#define ID_VIEW_INCREASE_SPIKINESS              127
#define ID_VIEW_DECREASE_SPIKINESS              128

// "Smoke" menu:
#define ID_SMOKE_ENABLED                        129
#define ID_SMOKE_USE_GLCL_SHARING               130
#define ID_SMOKE_SHOW_GRID                      131
#define ID_SMOKE_RESET                          132
#define ID_SMOKE_GRID_32_32_64                  133
#define ID_SMOKE_GRID_64_64_64                  134
#define ID_SMOKE_GRID_64_64_128                 135
#define ID_SMOKE_GRID_128_128_128               136
#define ID_SMOKE_GRID_128_128_256               137
#define ID_SMOKE_SHOW_ERROR                     138

// "Help" menu:
#define ID_HELP_ABOUT                           139

// "File" menu:
#define ID_FILE_EXIT                            140

// Used for automatic code generation
#define ID_DUMMY                                -1

// Smoke Sim device menu IDs
#define ID_SMOKE_SIM_DEVICE_MIN_ID              200
#define ID_SMOKE_SIM_DEVICE_MAX_ID              249

// Volume slice device menu IDs
#define ID_VOL_SLICE_DEVICE_MIN_ID              250
#define ID_VOL_SLICE_DEVICE_MAX_ID              299

// Menu string constants:

// "File" menu
#define TP_MENU_FILE        L"&File"
#define TP_MENU_FILE_EXIT   L"E&xit\tEsc"

// "Commands" menu
#define TP_MENU_COMMANDS L"&Commands"
#define TP_MENU_COMMANDS_MAKE_CRASH L"C&rash Application"
#define TP_MENU_COMMANDS_OUTPUT_SAMPLE_DEBUG L"Output a Sample Debug &String\tCtrl+D"
#define TP_MENU_COMMANDS_OUTPUT_SAMPLE_DEBUG_DESCRIPTION_LINUX "Generating a sample string to stdout.\nIf debbugging with CodeXL, this will only be seen in the log file."
#define TP_MENU_COMMANDS_GENERATE_OPENGL_ERROR L"Generate an Open&GL Error\tCtrl+E"
#define TP_MENU_COMMANDS_GENERATE_OPENGL_ERROR_DESCRIPTION_LINUX "Generating an OpenGL Error.\nTo see this error in CodeXL, enable \"Break on OpenGL Errors\"."
#define TP_MENU_COMMANDS_GENERATE_OPENCL_ERROR L"Generate an Open&CL Error\tCtrl+L"
#define TP_MENU_COMMANDS_GENERATE_BREAK_POINT L"Generate a &Breakpoint Event\tCtrl+P"
#define TP_MENU_COMMANDS_DETECTED_ERROR L"Generate a &Detected Error\tCtrl+T"
#define TP_MENU_COMMANDS_TOGGLE_SHADING_PARAMETERS L"Replace S&hading Parameters\tCtrl+O"
#define TP_MENU_COMMANDS_TOGGLE_MODEL_VIEW_MATRIX L"Replace Model &View Matrix\tCtrl+M"
#define TP_MENU_COMMANDS_TOGGLE_FRAGMENT_SHADERS L"Replace &Fragment Shader\tCtrl+Z"

// "View" menu
#define TP_MENU_VIEW L"&View"
#define TP_MENU_VIEW_TOGGLE_BACKGROUND_COLOR L"&Replace Background Color\tCtrl+B"
#define TP_MENU_VIEW_TOGGLE_SHADING_PROGRAM L"Use &Shading Program\tCtrl+S"
#define TP_MENU_VIEW_TOGGLE_GEOMETRY_SHADER L"Use &Geometry Shader \tCtrl+G"
#define TP_MENU_VIEW_TOGGLE_WIREFRAME_MODE L"&Wire-frame mode\tCtrl+W"
#define TP_MENU_VIEW_INCREASE_TEXTURE_INFLUENCE L"&Increases Texture Influence\t+"
#define TP_MENU_VIEW_DECREASE_TEXTURE_INFLUENCE L"&Decreases Texture Influence\t-"
#define TP_MENU_VIEW_INCREASE_SPIKINESS L"I&ncrease Spikiness\t]"
#define TP_MENU_VIEW_DECREASE_SPIKINESS L"D&ecrease Spikiness\t["

// "Smoke" menu
#define TP_MENU_SMOKE L"&OpenCL Smoke"
#define TP_MENU_SMOKE_ENABLED L"Enabled"
#define TP_MENU_SMOKE_USE_GLCL_SHARING L"Use GL-CL sharing"
#define TP_MENU_SMOKE_SHOW_GRID L"Show grid"
#define TP_MENU_SMOKE_RESET L"Reset"
#define TP_MENU_SMOKE_GRID_32_32_64 L"32 x 32 x 64"
#define TP_MENU_SMOKE_GRID_64_64_64 L"64 x 64 x 64"
#define TP_MENU_SMOKE_GRID_64_64_128 L"64 x 64 x 128"
#define TP_MENU_SMOKE_GRID_128_128_128 L"128 x 128 x 128"
#define TP_MENU_SMOKE_GRID_128_128_256 L"128 x 128 x 256"
#define TP_SUBMENU_SMOKE_SIM L"Choose Smoke Sim Device..."
#define TP_SUBMENU_VOL_SLICE L"Choose Volume Slice Device..."
#define TP_MENU_SMOKE_SHOW_ERROR L"Show errors\tCtrl+H"
#define TP_SMOKE_ERROR_DIALOG_TITLE L"Smoke System Errors"
#define TP_SMOKE_DIALOG_HEADER L"The smoke system reports the following messages:"
#define TP_SMOKE_DIALOG_NO_ERROR L"No errors reported by the smoke system."

// "Help" menu
#define TP_MENU_HELP L"&Help"
#define TP_MENU_HELP_ABOUT L"&About CodeXL Teapot Example..."

// Main window (frame) position and size:
#define TP_MAIN_WINDOW_POS_X 0
#define TP_MAIN_WINDOW_POS_Y 0
#define TP_MAIN_WINDOW_WIDTH 400
#define TP_MAIN_WINDOW_HEIGHT 400

// Rendering constants:
#define TP_AUTO_ROTATION_FACTOR 0.1F

// String constants:
#define TP_MAIN_WINDOW_CAPTION_STR L"  CodeXL Teapot Example"
#define TP_DRAWING_TEAPOT_MARKER_STR "Drawing a teapot (scale = %f)"
#define TP_SETTING_UP_MATERIAL_MARKER_STR "Setting up material"
#define TP_GENERAL_INIT_MARKER_STR "Performing general OpenGL initializations"
#define TO_CREATION_GLSL_OBJECTS_MARKER_STR "Creating GLSL programs and shaders"
#define TP_DRAWING_SCENE_MARKER_STR "Drawing scene objects"
#define TP_HELP_DIALOG_CAPTION_STR L"AMD OpenGL teapot help"
#define TP_OPEN_GL_ERROR_EXAMPLE_STR L"Generating OpenGL error example"
#define TP_OPEN_CL_ERROR_EXAMPLE_STR L"Generating OpenCL error example"
#define TP_CRASH_EXAMPLE_STR L"Generating application crash example"
#define TP_BREAK_POINT_EXAMPLE_STR L"Generating breakpoint exception example"
#define TP_DEBUG_STRING_EXAMPLE_STR L"A sample output debug string"
#define TP_ERROR_SRT L"Error"
#define TP_ERROR_TITLE L"OpenCL Errors"
#define TP_ABOUT_DIALOG_TITLE L"About AMD OpenGL Teapot"
#define TP_ABOUT_DIALOG_TITLE_LINUX L"AMD OpenGL Teapot"
#define TP_ABOUT_DIALOG_APPLICATION_DESCRIPTION L"\nCodeXL Teapot Example"
#define TP_ABOUT_DIALOG_COPYRIGHT L"\xA9 2004 - 2016 Advanced Micro Devices, Inc. All Rights Reserved.\n"
#define TP_ABOUT_DIALOG_WEBSITE_URL "http://gpuopen.com/"

// Vertex, geometry and fragment shader code file paths:
#define TP_MAC_OS_X_RESOURCES_DIR L"./AMDTTeapot.app/Contents/Resources"
#define TP_VERTEX_SHADER_CODE_FILE_PATH L"res/tpVertexShader.glsl"
#define TP_GEOMTERY_SHADER_CODE_FILE_PATH L"res/tpGeometryShader.glsl"
#define TP_FRAGMENT_SHADER_CODE_FILE_PATH1 L"res/tpFragmentShader.glsl"
#define TP_FRAGMENT_SHADER_CODE_FILE_PATH2 L"res/tpFragmentShader.catchMeIfYouCan.glsl"

// Texture file path:
#define TP_TEXTURE_FILE_PATH_BMP "tpLogo-white.bmp"

// AMD logo path:
#define TP_ABOUT_DIALOG_AMDT_LOGO_PATH L"AMDT-CodeXLLogo.png"

// OpenCL kernels
#define TP_SMOKE_SIMULATION_KERNEL_APPLY_SOURCES_FILE_PATH L"res/tpApplySources.cl"
#define TP_SMOKE_SIMULATION_KERNEL_APPLY_BUOYANCY_FILE_PATH L"res/tpApplyBuoyancy.cl"
#define TP_SMOKE_SIMULATION_KERNEL_CALCULATE_CURLU_FILE_PATH L"res/tpCalculateCurlU.cl"
#define TP_SMOKE_SIMULATION_KERNEL_APPLY_VORTICITY_FILE_PATH L"res/tpApplyVorticity.cl"
#define TP_SMOKE_SIMULATION_KERNEL_ADVECT_VELOCITY_FILE_PATH L"res/tpAdvectFieldVelocity.cl"
#define TP_SMOKE_SIMULATION_KERNEL_APPLY_VELECITY_BOUNDARY_CONDITION_FILE_PATH L"res/tpApplyVelocityBoundaryCondition.cl"
#define TP_SMOKE_SIMULATION_KERNEL_COMPUTE_FIELD_PRESSURE_PREP_FILE_PATH L"res/tpComputeFieldPressurePrep.cl"
#define TP_SMOKE_SIMULATION_KERNEL_COMPUTE_FIELD_PRESSURE_ITER_FILE_PATH L"res/tpComputeFieldPressureIter.cl"
#define TP_SMOKE_SIMULATION_KERNEL_APPLY_PRESSURE_BOUNDARY_CONDITION_FILE_PATH L"res/tpApplyPressureBoundaryCondition.cl"
#define TP_SMOKE_SIMULATION_KERNEL_PROJECT_FIELD_VELOCITY_FILE_PATH L"res/tpProjectFieldVelocity.cl"
#define TP_SMOKE_SIMULATION_KERNEL_ADVECT_FIELD_SCALAR_FILE_PATH L"res/tpAdvectFieldScalar.cl"
#define TP_SMOKE_SIMULATION_KERNEL_DISSIPATE_DENSITY_FILE_PATH L"res/tpDissipateDensity.cl"
#define TP_SMOKE_SIMULATION_KERNEL_ADVECT_FIELD_TEMPERATURE_FILE_PATH L"res/tpAdvectFieldScalar.cl"
#define TP_SMOKE_SIMULATION_KERNEL_DISSIPATE_TEMPERATURE_FILE_PATH L"res/tpDissipateTemperature.cl"
#define TP_SMOKE_SIMULATION_KERNEL_DEBUG_TEMPERATURE_FILE_PATH L"res/tpDebugTemperature.cl"
#define TP_SMOKE_SIMULATION_KERNEL_DEBUG_DENSITY_FILE_PATH L"res/tpDebugDensity.cl"
#define TP_SMOKE_SIMULATION_KERNEL_DEBUG_VELOCITY_VECTOR_FILE_PATH L"res/tpDebugVelocityVector.cl"
#define TP_SMOKE_SIMULATION_KERNEL_DEBUG_VELOCITY_LENGTH_FILE_PATH L"res/tpDebugVelocityLength.cl"
#define TP_SMOKE_SIMULATION_KERNEL_DEBUG_FIELD_PRESSURE_FILE_PATH L"res/tpDebugFieldPressure.cl"
#define TP_VOLUME_SLICING_KERNEL_CREATE_DENSITY_TEXTURE_FILE_PATH L"res/tpCreateDensityTexture.cl"
#define TP_SMOKE_SIMULATION_KERNEL_FILE_PATH L"res/tpSmokeSimulation.cl"
#define TP_VOLUME_SLICING_KERNEL_FILE_PATH L"res/tpVolumeSlicing.cl"

// Resources path:
#define TP_RESOURCES_PATH "res/"


#endif //__AMDTTEAPOTCONSTANTS_H

