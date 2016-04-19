//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTTeapotOCLSmokeSystem.cpp
///
//==================================================================================

//------------------------------ AMDTTeapotOCLSmokeSystem.cpp ------------------------------

// Local:
#include <inc/AMDTTeapotOCLSmokeSystem.h>
#include <inc/AMDTTeapotConstants.h>
#include "AMDTMisc.h"
#include "AMDTDebug.h"


// Set KERNEL_DEBUG to one of the following. This sets which OpenCL kernel
// will be used to populate the 3D texture that is rendered. The default is
// DEBUG_NONE which means that the 3D texture will contain the density vales
// for rendering, however it is also possible to display the velocity field
// or the temperature field by setting the appropriate macro.
#define DEBUG_NONE              0
#define DEBUG_TEMPERATURE       1
#define DEBUG_DENSITY           2
#define DEBUG_VELOCITY_VECTOR   3
#define DEBUG_VELOCITY_LENGTH   4
#define DEBUG_PRESSURE          5
#define KERNEL_DEBUG            DEBUG_NONE

// Define the sequence of kernels to execute for the smoke simulation. This
// includes setting which kernels should be iterated multiple times. Also
// defines the OpenCL memory objects (enums starting with MO_) that are
// passed as arguments to each kernel.
const AMDTTeapotOCLSmokeSystem::CalcKernelInfo
AMDTTeapotOCLSmokeSystem::_s_smokeSimulationKernels[] =
{
    {
        KERN_SMOKESIM_APPLY_SOURCES,
        "applySources",
        TP_SMOKE_SIMULATION_KERNEL_APPLY_SOURCES_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_S03D,
            MO_SMOKESIM_T03D,
            MO_SMOKESIM_U03D,
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
    {
        KERN_SMOKESIM_APPLY_BUOYANCY,
        "applyBuoyancy",
        TP_SMOKE_SIMULATION_KERNEL_APPLY_BUOYANCY_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_S03D,
            MO_SMOKESIM_T03D,
            MO_SMOKESIM_U03D,
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
    {
        KERN_SMOKESIM_CALCULATE_CURLU,
        "calculateCurlU",
        TP_SMOKE_SIMULATION_KERNEL_CALCULATE_CURLU_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_U03D,
            MO_SMOKESIM_U13D,         // This will contain curlU
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
    {
        KERN_SMOKESIM_APPLY_VORTICITY,
        "applyVorticity",
        TP_SMOKE_SIMULATION_KERNEL_APPLY_VORTICITY_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_U03D,         // Will be updated based on curlU
            MO_SMOKESIM_U13D,         // Contains curlU
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
    {
        KERN_SMOKESIM_ADVECT_VELOCITY,
        "advectFieldVelocity",
        TP_SMOKE_SIMULATION_KERNEL_ADVECT_VELOCITY_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_U03D,
            MO_SMOKESIM_U13D,
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
    {
        KERN_SMOKESIM_APPLY_VELECITY_BOUNDARY_CONDITION,
        "applyVelocityBoundaryCondition",
        TP_SMOKE_SIMULATION_KERNEL_APPLY_VELECITY_BOUNDARY_CONDITION_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_BOUNDARY_INDEX,
            MO_SMOKESIM_BOUNDARY_SCALE,
            MO_SMOKESIM_U13D,
            -1
        }
    },
    {
        KERN_SMOKESIM_COMPUTE_FIELD_PRESSURE_PREP,
        "computeFieldPressurePrep",
        TP_SMOKE_SIMULATION_KERNEL_COMPUTE_FIELD_PRESSURE_PREP_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_U13D,
            MO_SMOKESIM_TMPSCALAR,
            MO_SMOKESIM_P03D,
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
    {
        KERN_SMOKESIM_COMPUTE_FIELD_PRESSURE_ITER1,
        "computeFieldPressureIter",
        TP_SMOKE_SIMULATION_KERNEL_COMPUTE_FIELD_PRESSURE_ITER_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_TMPSCALAR,
            MO_SMOKESIM_P03D,
            MO_SMOKESIM_P13D,
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },

    {
        KERN_SMOKESIM_COMPUTE_FIELD_PRESSURE_ITER2,
        "computeFieldPressureIter",
        TP_SMOKE_SIMULATION_KERNEL_COMPUTE_FIELD_PRESSURE_ITER_FILE_PATH,
        KERN_SMOKESIM_COMPUTE_FIELD_PRESSURE_ITER1,   // Index of kernel to repeat
        20,                                           // Number of time to repeat
        {
            MO_SMOKESIM_TMPSCALAR,
            MO_SMOKESIM_P13D,
            MO_SMOKESIM_P03D,
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
    {
        KERN_SMOKESIM_APPLY_PRESSURE_BOUNDARY_CONDITION,
        "applyPressureBoundaryCondition",
        TP_SMOKE_SIMULATION_KERNEL_APPLY_PRESSURE_BOUNDARY_CONDITION_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_BOUNDARY_INDEX,
            MO_SMOKESIM_BOUNDARY_SCALE,
            MO_SMOKESIM_P03D,
            -1
        }
    },
    {
        KERN_SMOKESIM_PROJECT_FIELD_VELOCITY,
        "projectFieldVelocity",
        TP_SMOKE_SIMULATION_KERNEL_PROJECT_FIELD_VELOCITY_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_P03D,
            MO_SMOKESIM_U13D,
            MO_SMOKESIM_U03D,
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
    {
        KERN_SMOKESIM_ADVECT_FIELD_DENSITY,
        "advectFieldScalar",
        TP_SMOKE_SIMULATION_KERNEL_ADVECT_FIELD_SCALAR_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_U03D,
            MO_SMOKESIM_S03D,
            MO_SMOKESIM_TMPSCALAR,
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
    {
        KERN_SMOKESIM_DISSIPATE_DENSITY,
        "dissipateDensity",
        TP_SMOKE_SIMULATION_KERNEL_DISSIPATE_DENSITY_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_TMPSCALAR,
            MO_SMOKESIM_S03D,
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
    {
        KERN_SMOKESIM_ADVECT_FIELD_TEMPERATURE,
        "advectFieldScalar",
        TP_SMOKE_SIMULATION_KERNEL_ADVECT_FIELD_SCALAR_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_U03D,
            MO_SMOKESIM_T03D,
            MO_SMOKESIM_TMPSCALAR,
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
    {
        KERN_SMOKESIM_DISSIPATE_TEMPERATURE,
        "dissipateTemperature",
        TP_SMOKE_SIMULATION_KERNEL_DISSIPATE_TEMPERATURE_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_TMPSCALAR,
            MO_SMOKESIM_T03D,
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
#if KERNEL_DEBUG == DEBUG_TEMPERATURE
    {
        KERN_SMOKESIM_DEBUG_TEMPERATURE,
        "debugTemperature",
        TP_SMOKE_SIMULATION_KERNEL_DEBUG_TEMPERATURE_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_T03D,
            MO_SMOKESIM_DENSITY,
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
#elif KERNEL_DEBUG == DEBUG_DENSITY
    {
        KERN_SMOKESIM_DEBUG_DENSITY,
        "debugDensity",
        TP_SMOKE_SIMULATION_KERNEL_DEBUG_DENSITY_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_S03D,
            MO_SMOKESIM_DENSITY,
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
#elif KERNEL_DEBUG == DEBUG_VELOCITY_VECTOR
    {
        KERN_SMOKESIM_DEBUG_VELOCITY_VECTOR,
        "debugVelocityVector",
        TP_SMOKE_SIMULATION_KERNEL_DEBUG_VELOCITY_VECTOR_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_U03D,
            MO_SMOKESIM_DENSITY,
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
#elif KERNEL_DEBUG == DEBUG_VELOCITY_LENGTH
    {
        KERN_SMOKESIM_DEBUG_VELOCITY_LENGTH,
        "debugVelocityLength",
        TP_SMOKE_SIMULATION_KERNEL_DEBUG_VELOCITY_LENGTH_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_U03D,
            MO_SMOKESIM_DENSITY,
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
#elif KERNEL_DEBUG == DEBUG_PRESSURE
    {
        KERN_SMOKESIM_DEBUG_FIELD_PRESSURE,
        "debugFieldPressure",
        TP_SMOKE_SIMULATION_KERNEL_DEBUG_FIELD_PRESSURE_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_P03D,
            MO_SMOKESIM_DENSITY,
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
#else
    {
        KERN_SMOKESIM_CREATE_DENSITY_TEXTURE,
        "createDensityTexture",
        TP_VOLUME_SLICING_KERNEL_CREATE_DENSITY_TEXTURE_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_SMOKESIM_S03D,
            MO_SMOKESIM_DENSITY,
            MO_SMOKESIM_CONSTANTS,
            -1
        }
    },
#endif

    { -1, NULL, NULL, -1, -1, { -1 } }
};


// Define the sequence of kernels and arguments for running the
// volume slicing calculations.
const AMDTTeapotOCLSmokeSystem::CalcKernelInfo
AMDTTeapotOCLSmokeSystem::_s_volumeSlicingKernels[] =
{
    {
        KERN_VOLSLICE_COMPUTE_INTERSECTION,
        "computeIntersection",
        TP_VOLUME_SLICING_KERNEL_FILE_PATH,
        -1,                       // Index of kernel to repeat
        -1,                       // Number of time to repeat
        {
            MO_VOLSLICE_VERTICES,
            MO_VOLSLICE_PARAMS,
            MO_VOLSLICE_CONSTANTS,
            -1
        }
    },
    { -1, NULL, NULL, -1, -1, { -1 } }
};

// Define the programs that need to run. This includes the name of the source
// file and the definition of the sequence of kernels that need to be created
// from the source.
const AMDTTeapotOCLSmokeSystem::CalcProgramInfo
AMDTTeapotOCLSmokeSystem::_s_programs[NUMBER_CALCS] =
{
    { CALCID_SMOKE_SIMULATION, CALCBIT_SMOKE_SIMULATION, TP_SMOKE_SIMULATION_KERNEL_FILE_PATH, _s_smokeSimulationKernels, NUM_SMOKESIM_MEM_OBJECTS },
    { CALCID_VOLUME_SLICING, CALCBIT_VOLUME_SLICING, TP_VOLUME_SLICING_KERNEL_FILE_PATH, _s_volumeSlicingKernels, NUM_VOLSLICE_MEM_OBJECTS }
};


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::AMDTTeapotOCLSmokeSystem
// Description: Constructor
// ---------------------------------------------------------------------------
AMDTTeapotOCLSmokeSystem::AMDTTeapotOCLSmokeSystem(
    ISmokeSystemLogger* logger) :
    _initialized(false),
    _initStage(0),

    _selectedSmokeSimDevice(NULL),
    _selectedVolSliceDevice(NULL),

    _clCreateContext(NULL),
    _clReleaseContext(NULL),
    _clCreateCommandQueue(NULL),
    _clReleaseCommandQueue(NULL),
    _clCreateProgramWithSource(NULL),
    _clReleaseProgram(NULL),
    _clBuildProgram(NULL),
    _clGetProgramInfo(NULL),
    _clGetProgramBuildInfo(NULL),
    _clCreateKernel(NULL),
    _clReleaseKernel(NULL),
    _clSetKernelArg(NULL),
    _clEnqueueNDRangeKernel(NULL),
    _clCreateBuffer(NULL),
    _clCreateFromGLBuffer(NULL),
    _clCreateFromGLTexture3D(NULL),

    // GL_ARB_shader_objects extension functions:
    _isGL_ARB_shader_objectsSupported(false),
    _glCreateShaderObjectARB(NULL),
    _glShaderSourceARB(NULL),
    _glCompileShaderARB(NULL),
    _glGetObjectParameterivARB(NULL),
    _glCreateProgramObjectARB(NULL),
    _glAttachObjectARB(NULL),
    _glLinkProgramARB(NULL),
    _glUseProgramObjectARB(NULL),
    _glGetInfoLogARB(NULL),
    _glGetUniformLocationARB(NULL),
    _glUniform1fARB(NULL),
    _glDeleteObjectARB(NULL),
    _glDetachObjectARB(NULL),

    // GL_ARB_vertex_buffer_object extension functions:
    _isGL_ARB_vertex_buffer_objectSupported(false),
    _glGenBuffersARB(NULL),
    _glBindBufferARB(NULL),
    _glBufferDataARB(NULL),
    _glDeleteBuffersARB(NULL),
    _glBufferSubDataARB(NULL),

    // OpenGL 1.2 function pointers
    _glTexImage3D(NULL),

    // OpenGL 1.5 extension functions:
    _isOpenGL1_5Supported(false),
    _glGenBuffers(NULL),
    _glBindBuffer(NULL),
    _glBufferData(NULL),
    _glDeleteBuffers(NULL),
    _glBufferSubData(NULL),
    _glGetBufferSubData(NULL),

    _gridBuffer(NULL),

    _texDensity(0),
    _texDensityInternalFormat(GL_RGBA32F_ARB),
    _texDensityFormat(GL_RGBA),
    _texDensityType(GL_FLOAT),
    _volSliceVBO(0),
    _programs(NUMBER_CALCS),
    _oclMemGLDensityTexture(NULL),
    _oclMemGLVBO(NULL),

    _dirtyFlags(0),

    _changeExecOptions(EXEC_WITH_GLSHARING),
    _enabled(true),
    _initCompileFlag(INIT_COMPILE_START),
    _execOptions(EXEC_WITH_GLSHARING),
    _showGrid(false),

    _logger(logger)
{
    // Set the grid size.
    setGrid(64, 64, 64, 1.0f / 64.0f);

    // Set the default simulation parameters.
    _simParams._maxDeltaTime = 0.05f;
    _simParams._ambientTemperature = 15.0f;
    _simParams._minTemperature = 15.0f;
    _simParams._maxTemperature = 90.0f;
    _simParams._minDensity = 0.0f;
    _simParams._maxDensity = 99.9f;
    _simParams._timeToFall = 30.0f;
    _simParams._timeToRise = 0.5f;
    _simParams._timeToDissipateHalfDensity = 60.0f;
    _simParams._timeToDissipateHalfTemperature = 60.0f;
    _simParams._inputDensityPerSec = 5.0f;
    _simParams._inputTemperaturePerSec = 500.0f;
    _simParams._vorticityCoefficient = 1.0f;
    _dirtyFlags |= DIRTY_SIM_PARAMS;

    // Get a hold of the OpenCL and OpenGL helper singletons.
    _ogl = AMDTOpenGLHelper::GetInstance();
    _ocl = AMDTOpenCLHelper::GetInstance();

    // Create a list of the OpenCL programs that need to run from the
    // static const data above (_s_programs). This doesn't do any OpenCL
    // calls, just sets up our internal management structures.
    for (int i = 0; i < NUMBER_CALCS; ++i)
    {
        _programs.add(_s_programs[i]);
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::~AMDTTeapotOCLSmokeSystem
// Description: Destructor. Release all OpenGL and OpenCL resources.
// ---------------------------------------------------------------------------
AMDTTeapotOCLSmokeSystem::~AMDTTeapotOCLSmokeSystem()
{
    shutdown();

    // Release our use of the OpenCL and OpenGL helper singletons.
    _ocl->ReleaseInstance();
    _ogl->ReleaseInstance();
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::Initialize
// Description: Initialize OpenGL and OpenCL. Choose the best platform/device
//              for each of the OpenCL programs that needs to run.
// ---------------------------------------------------------------------------
bool AMDTTeapotOCLSmokeSystem::initialize()
{
    bool retVal = false;

    if (!_initialized)
    {
        _initialized = true;

        _initStage = 0;

        _lastError.reset();

        if (initOpenGL())
        {
            _initStage |= INIT_STAGE_OPENGL;

            if (initOpenCL())
            {
                retVal = true;

                _initStage |= INIT_STAGE_OPENCL;
            }
            else
            {
                _lastError.print("ERROR: Failed to initialize openCL.\n");
            }
        }
        else
        {
            _lastError.print("ERROR: Failed to initialize openGL.\n");
        }
    }

    if (!retVal)
    {
        shutdown();
    }

    _logger->setProgress(retVal ? "OK" : _lastError._str);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::initOpenCL
// Description: Read source program files, import OpenCL function pointers
//              and check for GL association of each device. Finally,
//              choose the best OpenCL devices to use by default.
// ---------------------------------------------------------------------------
bool AMDTTeapotOCLSmokeSystem::initOpenCL()
{
    bool ret = false;

    if (_ocl->isReady())
    {
        bool missingKernelSrc = false;
        // Read in all the OpenCL programs and finish if there are problems.
        _logger->setProgress("Reading OpenCL programs from filesystem ...");

        for (int i = _programs.size() - 1; !missingKernelSrc && (i >= 0); --i)
        {
            CalcProgram* program = _programs[i];

            for (int i = 0; !missingKernelSrc && (i < program->_numKernels); ++i)
            {
                CalcKernel& kernel = program->_kernels[i];
                kernel._source = _ocl->readKernelSource(kernel._sourcePath);

                if (kernel._source == NULL)
                {
                    _lastError.printf("ERROR: Problem reading source from \"%ls\".\n",
                                      kernel._sourcePath);
                    missingKernelSrc = true;
                }
            }
        }

        if (!missingKernelSrc)
        {
            // Import OpenCL function addresses
#define OCLIMPORT(var) var = _ocl->var
            OCLIMPORT(_clCreateContext);
            OCLIMPORT(_clReleaseContext);
            OCLIMPORT(_clCreateContextFromType);
            OCLIMPORT(_clGetContextInfo);
            OCLIMPORT(_clCreateCommandQueue);
            OCLIMPORT(_clReleaseCommandQueue);
            OCLIMPORT(_clCreateProgramWithSource);
            OCLIMPORT(_clReleaseProgram);
            OCLIMPORT(_clBuildProgram);
            OCLIMPORT(_clUnloadCompiler);
            OCLIMPORT(_clGetProgramInfo);
            OCLIMPORT(_clGetProgramBuildInfo);
            OCLIMPORT(_clCreateKernel);
            OCLIMPORT(_clReleaseKernel);
            OCLIMPORT(_clSetKernelArg);
            OCLIMPORT(_clEnqueueNDRangeKernel);
            OCLIMPORT(_clEnqueueCopyBufferToImage);
            OCLIMPORT(_clEnqueueMapBuffer);
            OCLIMPORT(_clEnqueueUnmapMemObject);
            OCLIMPORT(_clGetSupportedImageFormats);
            OCLIMPORT(_clCreateBuffer);
            OCLIMPORT(_clCreateImage2D);
            OCLIMPORT(_clCreateImage3D);
            OCLIMPORT(_clReleaseMemObject);
            OCLIMPORT(_clGetKernelWorkGroupInfo);
            OCLIMPORT(_clReleaseEvent);
            OCLIMPORT(_clWaitForEvents);
            OCLIMPORT(_clCreateFromGLBuffer);
            OCLIMPORT(_clCreateFromGLTexture3D);
            OCLIMPORT(_clEnqueueAcquireGLObjects);
            OCLIMPORT(_clEnqueueReleaseGLObjects);
            OCLIMPORT(_clEnqueueWriteBuffer);
            OCLIMPORT(_clEnqueueWriteBufferRect);
            OCLIMPORT(_clFlush);
            OCLIMPORT(_clFinish);
            OCLIMPORT(_clGetGLObjectInfo);

            // Sanity check - if we successfully obtained the function addresses.
            if (_clCreateContext
                && _clReleaseContext
                && _clCreateContextFromType
                && _clGetContextInfo
                && _clCreateCommandQueue
                && _clReleaseCommandQueue
                && _clCreateProgramWithSource
                && _clReleaseProgram
                && _clBuildProgram
                && _clUnloadCompiler
                && _clGetProgramInfo
                && _clGetProgramBuildInfo
                && _clCreateKernel
                && _clReleaseKernel
                && _clSetKernelArg
                && _clEnqueueNDRangeKernel
                && _clGetKernelWorkGroupInfo
                && _clWaitForEvents
                && _clReleaseEvent
                && _clEnqueueWriteBuffer
                && _clFlush
                && _clFinish
               )
            {
                if (_ocl->updateGLCLBinding() && chooseBestDevices())
                {
                    ret = true;
                }
            }
            else
            {
                _lastError.print("ERROR: OpenCL is not exporting all required functions.\n");
            }
        }
    }
    else
    {
        _lastError.printf("ERROR: %s\n", _ocl->getLastError());
    }

    _logger->setProgress(ret ? "OK" : _lastError._str);

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::initOpenGLExtensions
// Description: Import OpenGL function pointers and check that we have
//              required OpenGL extensions.
// ---------------------------------------------------------------------------
bool AMDTTeapotOCLSmokeSystem::initOpenGLExtensions()
{
    bool ret = false;

    if (_ogl->isReady())
    {
        _logger->setProgress("Initializing OpenGL extensions ...");
        // Get OpenGL function pointers
#define OGLIMPORT(var) _ ## var = _ogl->_ ## var

        // GL_ARB_shader_objects extension functions:
        OGLIMPORT(isGL_ARB_shader_objectsSupported);
        OGLIMPORT(glCreateShaderObjectARB);
        OGLIMPORT(glShaderSourceARB);
        OGLIMPORT(glCompileShaderARB);
        OGLIMPORT(glGetObjectParameterivARB);
        OGLIMPORT(glCreateProgramObjectARB);
        OGLIMPORT(glAttachObjectARB);
        OGLIMPORT(glLinkProgramARB);
        OGLIMPORT(glUseProgramObjectARB);
        OGLIMPORT(glGetInfoLogARB);
        OGLIMPORT(glGetUniformLocationARB);
        OGLIMPORT(glUniform1fARB);
        OGLIMPORT(glDeleteObjectARB);
        OGLIMPORT(glDetachObjectARB);

        // GL_ARB_vertex_buffer_object extension functions:
        OGLIMPORT(isGL_ARB_vertex_buffer_objectSupported);
        OGLIMPORT(glGenBuffersARB);
        OGLIMPORT(glBindBufferARB);
        OGLIMPORT(glBufferDataARB);
        OGLIMPORT(glDeleteBuffersARB);
        OGLIMPORT(glBufferSubDataARB);

        // OpenGL 1.2 function pointers
        OGLIMPORT(glTexImage3D);

        // OpenGL 1.5 extension functions:
        OGLIMPORT(isOpenGL1_5Supported);
        OGLIMPORT(glGenBuffers);
        OGLIMPORT(glBindBuffer);
        OGLIMPORT(glBufferData);
        OGLIMPORT(glDeleteBuffers);
        OGLIMPORT(glBufferSubData);
        OGLIMPORT(glGetBufferSubData);

        // GL_ARB_texture_float
        OGLIMPORT(isGL_ARB_texture_floatSupported);

        // The smoke system needs all the following extensions.
        ret = _isGL_ARB_shader_objectsSupported
              && _glTexImage3D
              && (_isGL_ARB_vertex_buffer_objectSupported || _isOpenGL1_5Supported)
              && _isGL_ARB_texture_floatSupported;

        if (!ret)
        {
            _lastError.print("ERROR: The following OpenGL extensions are missing:\n\n");

            if (!_isGL_ARB_shader_objectsSupported)
            {
                _lastError.print("    GL_ARB_shader_objects\n");
            }

            if (!_glTexImage3D)
            {
                _lastError.print("    OpenGL 1.2 - glTexImage3d\n");
            }

            if (!_isGL_ARB_vertex_buffer_objectSupported && !_isOpenGL1_5Supported)
            {
                _lastError.print("    GL_ARB_vertex_buffer_object or OpenGL 1.5\n");
            }

            if (!_isGL_ARB_texture_floatSupported)
            {
                _lastError.print("    GL_ARB_texture_float\n");
            }
        }
    }
    else
    {
        _lastError.print("ERROR: Can't load OpenGL library.");
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::createOpenGLObjects
// Description: Create OpenGL names for density texture and volume slice VBO.
// ---------------------------------------------------------------------------
bool AMDTTeapotOCLSmokeSystem::createOpenGLObjects()
{
    bool retVal = true;
    _logger->setProgress("Creating OpenGL objects ...");

    // Create a texture object
    glGenTextures(1, &_texDensity);

    if (0 == _texDensity)
    {
        _lastError.printf("ERROR: Can't create texture: GL error code = %d\n", glGetError());
        retVal = false;
    }
    else
    {
        // We prefer the OpenGL 1.5 versions of functions over the extension ones:
        _volSliceVBO = 0;

        if (_isOpenGL1_5Supported)
        {
            _glGenBuffers(1, &_volSliceVBO);
        }
        else if (_isGL_ARB_vertex_buffer_objectSupported)
        {
            _glGenBuffersARB(1, &_volSliceVBO);
        }

        if (0 == _volSliceVBO)
        {
            _lastError.printf("ERROR: Can't create VBO: GL error code = %d\n", glGetError());
            retVal = false;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::deleteOpenGLObjects
// Description: Free the texture (density field) and VBO (volume slices).
// ---------------------------------------------------------------------------
void AMDTTeapotOCLSmokeSystem::deleteOpenGLObjects()
{
    glBindTexture(GL_TEXTURE_3D, 0);
    glDeleteTextures(1, &_texDensity);

    if (_isOpenGL1_5Supported)
    {
        _glDeleteBuffers(1, &_volSliceVBO);
    }
    else if (_isGL_ARB_vertex_buffer_objectSupported)
    {
        _glDeleteBuffersARB(1, &_volSliceVBO);
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::initOpenGL
// Description: Initialize OpenGL extensions and create OpenGL objects.
// ---------------------------------------------------------------------------
bool AMDTTeapotOCLSmokeSystem::initOpenGL()
{
    bool retVal = false;

    if (initOpenGLExtensions() && createOpenGLObjects())
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::chooseBestDevices
// Description: Attempt to choose the most optimized configuration for performing
//              the simulation/computations. This will attempt to perform the
//              smoke simulation on a GPU device that shares the GL context
//              and the calculation of the volume slices on the CPU. If the
//              host has Intel CPUs and the Intel platform, then the Intel platform
//              will be used. If the host has AMD CPUs then the AMD platform will
//              be used. The CPU device that can share with the GL context will
//              be given preference.
// ---------------------------------------------------------------------------
bool AMDTTeapotOCLSmokeSystem::chooseBestDevices()
{
    bool retVal = true;
    // Get the OpenCL list of platforms/devices from the OpenCL helper
    // singleton.
    const OCLInfo* clInfo = _ocl->getOpenCLInfo();

    if (!clInfo)
    {
        retVal = false;
    }
    else
    {
        // First GPU device (with preference for GL association).
        const OCLDevice* firstGPUDevice = NULL;
        // First CPU device (with preference for GL association).
        const OCLDevice* firstCPUDevice = NULL;
        const OCLDevice* firstIntelCPUDevice = NULL;
        const OCLDevice* firstAMDCPUDevice = NULL;

        // Go through the list of all platforms and devices.
        for (int i = clInfo->getNumPlatforms() - 1; i >= 0; --i)
        {
            const OCLPlatform* platform = clInfo->getPlatform(i);

            for (int j = platform->getNumDevices() - 1; j >= 0; --j)
            {
                const OCLDevice* device = platform->getDevice(j);

                // What is the device type?
                if (device->getDeviceType() == CL_DEVICE_TYPE_GPU)
                {
                    if (firstGPUDevice)
                    {
                        // Prefer AMD device to non-AMD devices:
                        if (firstGPUDevice->getVendorId() != OCL_VENDOR_AMD && device->getVendorId() == OCL_VENDOR_AMD)
                        {
                            firstGPUDevice = device;
                        }
                        // Prefer devices that support CL-GL interoperability over ones that don't.
                        // But don't replace a non-interop AMD GPU with a non-AMD interop GPU.
                        else if (!firstGPUDevice->getGLAssociation() && device->getGLAssociation() &&
                                 (firstGPUDevice->getVendorId() != OCL_VENDOR_AMD || device->getVendorId() == OCL_VENDOR_AMD))
                        {
                            firstGPUDevice = device;
                        }
                    }
                    else
                    {
                        firstGPUDevice = device;
                    }
                }
                else if (device->getDeviceType() == CL_DEVICE_TYPE_CPU)
                {
                    if (firstCPUDevice)
                    {
                        // Prefer devices that support CL-GL interoperability over ones that don't.
                        if (!firstCPUDevice->getGLAssociation() && device->getGLAssociation())
                        {
                            firstCPUDevice = device;
                        }
                    }
                    else
                    {
                        firstCPUDevice = device;
                    }

                    if (platform->getVendorId() == OCL_VENDOR_INTEL)
                    {
                        if (device->getVendorId() == OCL_VENDOR_INTEL)
                        {
                            if (firstIntelCPUDevice)
                            {
                                // Prefer devices that support CL-GL interoperability over ones that don't.
                                if (!firstIntelCPUDevice->getGLAssociation() && device->getGLAssociation())
                                {
                                    firstIntelCPUDevice = device;
                                }
                            }
                            else
                            {
                                firstIntelCPUDevice = device;
                            }
                        }
                    }
                    else if (platform->getVendorId() == OCL_VENDOR_AMD)
                    {
                        if (device->getVendorId() == OCL_VENDOR_AMD)
                        {
                            if (firstAMDCPUDevice)
                            {
                                // Prefer devices that support CL-GL interoperability over ones that don't.
                                if (!firstAMDCPUDevice->getGLAssociation() && device->getGLAssociation())
                                {
                                    firstAMDCPUDevice = device;
                                }
                            }
                            else
                            {
                                firstAMDCPUDevice = device;
                            }
                        }
                    }
                }
            }
        }

        // Choose the best CPU.
        const OCLDevice* bestCPUDevice = firstCPUDevice;

        // Thanks wpetchang from the AMD Dev Gurus forum for finding the issue here =)
        if (firstIntelCPUDevice)
        {
            bestCPUDevice = firstIntelCPUDevice;
        }

        if (firstAMDCPUDevice)
        {
            bestCPUDevice = firstAMDCPUDevice;
        }

        if (!firstGPUDevice && !bestCPUDevice)
        {
            // No suitable devices!
            retVal = false;
        }
        else
        {
            // Set the best GPU and CPU device that will be used by the programs.
            // If there is no GPU device, use the CPU for both actions, and vice-versa.
            _changeSmokeSimDevice = firstGPUDevice ? firstGPUDevice : bestCPUDevice;
            _changeVolSliceDevice = bestCPUDevice ? bestCPUDevice : firstGPUDevice;
            _dirtyFlags |= DIRTY_DEVICES;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::selectDevices
// Description: Manually specify which devices to use for the different parts of
//              the simulation/computation. This updates _contexts which is a
//              list of all contexts needed (one per device involved in the
//              simulation/calculation) and for each device, which parts of
//              the simulation/computation it is used for.
bool AMDTTeapotOCLSmokeSystem::selectDevices(
    const OCLDevice* smokeSimulation,
    const OCLDevice* volumeSlicing)
{
    CalcContext* context;

    if (smokeSimulation->getPlatform() == volumeSlicing->getPlatform())
    {
        // All calculations on the same platform - create one context.
        context = new CalcContext(smokeSimulation->getPlatform());

        if (_execOptions & EXEC_WITH_GLSHARING)
        {
            context->_requestGLSharing = true;
        }

        if (volumeSlicing == smokeSimulation)
        {
            // Calculations on the same device
            context->addDevice(
                new CalcDevice(
                    CALCBIT_SMOKE_SIMULATION | CALCBIT_VOLUME_SLICING,
                    smokeSimulation));
        }
        else
        {
            // Calculations on two separate devices
            context->addDevice(
                new CalcDevice(CALCBIT_SMOKE_SIMULATION, smokeSimulation));

            context->addDevice(
                new CalcDevice(CALCBIT_VOLUME_SLICING, volumeSlicing));
        }

        _contexts.add(context);
    }
    else
    {
        // Each part of the calculation on a different platform -
        // create two contexts, one on each platform.
        context = new CalcContext(smokeSimulation->getPlatform());

        if (_execOptions & EXEC_WITH_GLSHARING)
        {
            context->_requestGLSharing = true;
        }

        context->addDevice(
            new CalcDevice(CALCBIT_SMOKE_SIMULATION, smokeSimulation));
        _contexts.add(context);

        context = new CalcContext(volumeSlicing->getPlatform());

        if (_execOptions & EXEC_WITH_GLSHARING)
        {
            context->_requestGLSharing = true;
        }

        context->addDevice(
            new CalcDevice(CALCBIT_VOLUME_SLICING, volumeSlicing));
        _contexts.add(context);
    }

    _selectedSmokeSimDevice = smokeSimulation;
    _selectedVolSliceDevice = volumeSlicing;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::initOpenCLResources
// Description: Init && Create all OpenCL contexts, build all programs, create
//              all kernels, create all memory buffers and set all
//              kernel arguments.
bool AMDTTeapotOCLSmokeSystem::initOpenCLResources()
{
    DynStr msg;
    CalcProgram* program = NULL;
    CalcContext* context = NULL;
    CalcDevice* device = NULL;
    cl_int status = -1;
    bool retVal = true;
    bool canRunAll_P = true;


    // First make sure that we have a context in which each program
    // can be run.
    for (int i = _programs.size() - 1; i >= 0; --i)
    {
        program = _programs[i];

        bool foundContext = false;
        bool foundDevice = false;

        for (int j = 0; j < _contexts.size(); ++j)
        {
            context = _contexts.get(j);

            if (context->_calcMask & program->_bit)
            {
                foundContext = true;

                for (int k = 0; k < context->_devices.size(); ++k)
                {
                    device = context->_devices[k];

                    if (device->_calcBits & program->_bit)
                    {
                        foundDevice = true;
                        break;
                    }
                }

                break;
            }
        }

        if (!(foundContext && foundDevice))
        {
            // Can't find a context/device for this program.
            _lastError.print("ERROR: Don't have suitable OpenCL devices to run simulation.");
            retVal = false;
            canRunAll_P = false;
            break;
        }

        program->_calcContext = context;
        program->_calcDevice = device;
    }

    if (canRunAll_P)
    {
        // Go through the list of contexts required and create them, attempting
        // GL-CL association if requested.
        _logger->setProgress("Creating OpenCL contexts ...");

        for (int j = 0; j < _contexts.size(); ++j)
        {
            bool ok = true;     // keep track of status, break if required

            // Create a context for the devices that will be used on this
            // platform.
            context = _contexts.get(j);

            if (context->_clCtx == NULL)
            {
                bool haveGLSharing = true;
                int n = context->_devices.size();
                cl_device_id* devices = new cl_device_id[n];

                for (int k = 0; k < n; ++k)
                {
                    device = context->_devices[k];
                    devices[k] = device->_device->getDeviceID();
                    haveGLSharing &= (device->_device->getGLAssociation()
                                      | device->_device->getGLSharing());
                }

                cl_context_properties p[7];
                p[0] = CL_CONTEXT_PLATFORM;
                p[1] = (cl_context_properties)context->_platform->getPlatformID();
                context->_clCtx = NULL;

                if (context->_requestGLSharing && haveGLSharing)
                {
                    // Attempt to create a context with GL association.
#if defined (__linux__)
                    p[2] = CL_GLX_DISPLAY_KHR;
                    p[3] = (cl_context_properties)glXGetCurrentDisplay();
                    p[4] = CL_GL_CONTEXT_KHR;
                    p[5] = (cl_context_properties)glXGetCurrentContext();
#else // Windows
                    p[2] = CL_WGL_HDC_KHR;
                    p[3] = (cl_context_properties)wglGetCurrentDC();
                    p[4] = CL_GL_CONTEXT_KHR;
                    p[5] = (cl_context_properties)wglGetCurrentContext();
#endif
                    p[6] = 0;
                    context->_clCtx = _clCreateContext(
                                          p,
                                          n,
                                          devices,
                                          NULL,
                                          NULL,
                                          &status);

                    if (status == CL_SUCCESS)
                    {
                        context->_withGLSharing = true;
                    }
                }

                if (context->_clCtx == NULL)
                {
                    // Create a normal context - no GL association.
                    p[2] = 0;
                    context->_clCtx = _clCreateContext(
                                          p,
                                          n,
                                          devices,
                                          NULL,
                                          NULL,
                                          &status);
                }

                delete[] devices;

                if (status != CL_SUCCESS)
                {
                    _lastError.printf("ERROR: Error running _clCreateContext(platform=%s): [%d] %s",
                                      context->_platform->getName(),
                                      status,
                                      _ocl->getOpenCLErrorCodeStr(status));
                    retVal = false;
                    ok = false;
                }
            }

            if (ok)
            {
                // Create command queue for each device in this context.
                _logger->setProgress("Creating OpenCL device command queues ...");

                for (int k = 0; k < context->_devices.size(); ++k)
                {
                    device = context->_devices[k];

                    if (device->_clCmdQueue == NULL)
                    {
                        device->_clCmdQueue = _clCreateCommandQueue(
                                                  context->_clCtx,
                                                  device->_device->getDeviceID(),
                                                  0,//CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
                                                  &status);

                        if (status != CL_SUCCESS)
                        {
                            _lastError.printf("ERROR: Error running _clCreateCommandQueue(platform=%s, device=%s): [%d] %s",
                                              context->_platform->getName(),
                                              device->_device->getName(),
                                              status,
                                              _ocl->getOpenCLErrorCodeStr(status));
                            retVal = false;
                            ok = false;
                        }
                    }
                }

                if (ok)
                {
                    // Query for the list of image formats supported
                    cl_uint numImages;
                    status = _clGetSupportedImageFormats(
                                 context->_clCtx,
                                 CL_MEM_READ_WRITE,
                                 CL_MEM_OBJECT_IMAGE3D,
                                 0,
                                 NULL,
                                 &numImages);

                    if (status != CL_SUCCESS)
                    {
                        _lastError.printf("ERROR: Error calling _clGetSupportedImageFormats(platform=%s, CL_MEM_OBJECT_IMAGE3D): [%d] %s",
                                          context->_platform->getName(),
                                          status,
                                          _ocl->getOpenCLErrorCodeStr(status));
                        retVal = false;
                    }
                    else
                    {
                        delete context->_image3DFormats;
                        context->_image3DFormats = new cl_image_format[numImages];
                        status = _clGetSupportedImageFormats(
                                     context->_clCtx,
                                     CL_MEM_READ_WRITE,
                                     CL_MEM_OBJECT_IMAGE3D,
                                     numImages,
                                     context->_image3DFormats,
                                     &context->_numImage3DFormats);

                        if (status != CL_SUCCESS)
                        {
                            _lastError.printf("ERROR: Error calling _clGetSupportedImageFormats(platform=%s, CL_MEM_OBJECT_IMAGE3D): [%d] %s",
                                              context->_platform->getName(),
                                              status,
                                              _ocl->getOpenCLErrorCodeStr(status));
                            retVal = false;
                        }
                        else
                        {
                            status = _clGetSupportedImageFormats(
                                         context->_clCtx,
                                         CL_MEM_READ_WRITE,
                                         CL_MEM_OBJECT_IMAGE2D,
                                         0,
                                         NULL,
                                         &numImages);

                            if (status != CL_SUCCESS)
                            {
                                _lastError.printf("ERROR: Error calling _clGetSupportedImageFormats(platform=%s, CL_MEM_OBJECT_IMAGE2D): [%d] %s",
                                                  context->_platform->getName(),
                                                  status,
                                                  _ocl->getOpenCLErrorCodeStr(status));
                                retVal = false;
                            }
                            else
                            {
                                delete context->_image2DFormats;
                                context->_image2DFormats = new cl_image_format[numImages];
                                status = _clGetSupportedImageFormats(
                                             context->_clCtx,
                                             CL_MEM_READ_WRITE,
                                             CL_MEM_OBJECT_IMAGE2D,
                                             numImages,
                                             context->_image2DFormats,
                                             &context->_numImage2DFormats);

                                if (status != CL_SUCCESS)
                                {
                                    _lastError.printf("ERROR: Error calling _clGetSupportedImageFormats(platform=%s, CL_MEM_OBJECT_IMAGE2D): [%d] %s",
                                                      context->_platform->getName(),
                                                      status,
                                                      _ocl->getOpenCLErrorCodeStr(status));
                                    retVal = false;
                                }
                                else
                                {
                                    // Go through the list of image formats and store the cl_image_format
                                    // pointers for 2D and 3D float luminance and rgba formats.
                                    for (cl_uint i = 0; i < context->_numImage2DFormats; ++i)
                                    {
                                        cl_image_format* fmt = &context->_image2DFormats[i];

                                        if (fmt->image_channel_data_type == CL_FLOAT)
                                        {
                                            if (fmt->image_channel_order == CL_LUMINANCE)
                                            {
                                                context->_image2Dluminance = fmt;
                                            }
                                            else if (fmt->image_channel_order == CL_RGBA)
                                            {
                                                context->_image2Drgba = fmt;
                                            }
                                        }
                                    }

                                    for (cl_uint i = 0; i < context->_numImage3DFormats; ++i)
                                    {
                                        cl_image_format* fmt = &context->_image3DFormats[i];

                                        if (fmt->image_channel_data_type == CL_FLOAT)
                                        {
                                            if (fmt->image_channel_order == CL_LUMINANCE)
                                            {
                                                context->_image3Dluminance = fmt;
                                            }
                                            else if (fmt->image_channel_order == CL_RGBA)
                                            {
                                                context->_image3Drgba = fmt;
                                            }
                                        }
                                    }

                                    // Make sure that at least CL_RGBA + CL_FLOAT exists
                                    if (!(context->_image2Drgba && context->_image3Drgba))
                                    {
                                        _lastError.printf("ERROR: Platform \"%s\" doesn't support 2D and/or 3D float RGBA images.",
                                                          context->_platform->getName());
                                        retVal = false;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::createOpenCLResources
// Description: Create all OpenCL contexts, build all programs, create
//              all kernels, create all memory buffers and set all
//              kernel arguments.
bool AMDTTeapotOCLSmokeSystem::createOpenCLResources()
{
    bool retVal = true;

    for (int i = _programs.size() - 1; i >= 0; --i)
    {
        if (!createOpenCLKernelProgramResource(_programs[i]))
        {
            retVal = false;
            break;
        }
    }

    // Perform initializations specific to the smoke sim program.
    if (!setupSmokeSimulation(_programs[CALCID_SMOKE_SIMULATION]))
    {
        retVal = false;
    }

    // Perform initializations specific to the volume slicing program.
    if (!setupVolumeSlicing(_programs[CALCID_VOLUME_SLICING]))
    {
        retVal = false;
    }

    _initStage |= INIT_STAGE_CREATE_RESOURCES;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::releaseOpenCLResources
// Description: Release all OpenCL contexts, programs, kernels and
//              memory objects.
bool AMDTTeapotOCLSmokeSystem::releaseOpenCLResources()
{
    bool ok = true;
    cl_int status;
    CalcContext* context;

    _initStage &= (~INIT_STAGE_CREATE_RESOURCES);

    // Release any programs/kernels
    int numProgramsDeleted = 0;

    for (int i = _programs.size() - 1; ok && (i >= 0); --i)
    {
        CalcProgram* program = _programs[i];

        for (int i = 0; ok && (i < program->_numKernels); ++i)
        {
            CalcKernel& kernel = program->_kernels[i];

            if (kernel._clKernel)
            {
                status = _clReleaseKernel(kernel._clKernel);

                if (status != CL_SUCCESS)
                {
                    ok = false;
                }
                else
                {
                    kernel._clKernel = NULL;
                }
            }

            if (ok && kernel._clProgram)
            {
                status = _clReleaseProgram(kernel._clProgram);

                if (status != CL_SUCCESS)
                {
                    ok = false;
                }
                else
                {
                    kernel._clProgram = NULL;
                    numProgramsDeleted++;
                }
            }
        }

        for (int i = program->_numMemObjects - 1; ok && (i >= 0); --i)
        {
            CalcMemory* mem = &program->_memObjects[i];

            if (ok && mem->_clMemory)
            {
                status = _clReleaseMemObject(mem->_clMemory);

                if (status != CL_SUCCESS)
                {
                    ok = false;
                }
                else
                {
                    mem->_clMemory = NULL;
                }
            }
        }
    }

    if (ok)
    {
        if (numProgramsDeleted)
        {
            // Only free compiler resources if we actually created programs.
            _clUnloadCompiler();
        }

        if (!releaseSmokeSimulation())
        {
            ok = false;
        }

        if (ok && !releaseVolumeSlicing())
        {
            ok = false;
        }

        if (ok)
        {
            while (ok && (NULL != (context = _contexts.getHead())))
            {
                // Release any command queues
                CalcDevice* device;

                while (ok && ((device = context->_devices.getHead()) != NULL))
                {
                    if (device->_clCmdQueue)
                    {
                        status = _clReleaseCommandQueue(device->_clCmdQueue);

                        if (status != CL_SUCCESS)
                        {
                            ok = false;
                            break;
                        }
                        else
                        {
                            device->_clCmdQueue = NULL;
                        }
                    }

                    context->_devices.removeHead();
                }

                if (ok)
                {
                    // Now release the OpenCL context
                    if (context->_clCtx)
                    {
                        status = _clReleaseContext(context->_clCtx);

                        if (status != CL_SUCCESS)
                        {
                            ok = false;
                        }
                        else
                        {
                            context->_clCtx = NULL;
                        }
                    }

                    if (ok)
                    {
                        // Remove this context
                        _contexts.removeHead();
                    }
                }
            }
        }
    }

    return ok;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::shutdown()
// Description: Free up all resources allocated during initialization.
// ---------------------------------------------------------------------------
void AMDTTeapotOCLSmokeSystem::shutdown()
{
    if (_ocl->isReady())
    {
        releaseOpenCLResources();
    }

    if (_ogl->isReady())
    {
        deleteOpenGLObjects();
    }

    delete _gridBuffer;
    _gridBuffer = NULL;

    _initStage = 0;
    _dirtyFlags = 0;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::setGrid
// Description: Request new grid dimensions. OpenCL resources will be changed
//              the next time draw() is called.
void AMDTTeapotOCLSmokeSystem::setGrid(
    int numCellsWidth,
    int numCellsDepth,
    int numCellsHeight,
    float cellSpacingMETERS)
{
    _changeNumCellsX = numCellsWidth;
    _changeNumCellsY = numCellsDepth;
    _changeNumCellsZ = numCellsHeight;
    _changeCellSpacingMETERS = cellSpacingMETERS;
    _dirtyFlags |= DIRTY_GRID_DIMENSIONS;
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::draw
// Description: First, if there were any requests for changes to grid
//              dimensions, compute devices etc, OpenCL resources are
//              recreated. Next one step of the simulation is computed.
//              Then the density texture is rendered.
//              Expects to receive the current model transformation
//              that positions the smoke density grid at the desired
//              location in the model (not including _view transform).
bool AMDTTeapotOCLSmokeSystem::draw(
    const AMDTTeapotRenderState& state,
    const Mat4& modelTransformation,
    float deltaRot)
{
    bool ok = true;

    if (_enabled && (_initStage & INIT_STAGE_OPENCL))
    {
        _lastError.reset();

        if (_dirtyFlags & (DIRTY_GRID_DIMENSIONS | DIRTY_DEVICES | DIRTY_EXEC_OPTIONS))
        {
            _logger->setProgress("Releasing OpenCL resources ...");

            if (!releaseOpenCLResources())
            {
                ok = false;
            }
            else
            {
                _dirtyFlags |= DIRTY_DEVICES;
            }

            if (ok && (_dirtyFlags & DIRTY_EXEC_OPTIONS))
            {
                _execOptions = _changeExecOptions;
                _dirtyFlags &= (~DIRTY_EXEC_OPTIONS);
                _dirtyFlags |= DIRTY_DEVICES;
            }

            if (ok && (_dirtyFlags & DIRTY_GRID_DIMENSIONS))
            {
                if (!setupDensityGrid(_changeNumCellsX, _changeNumCellsY, _changeNumCellsZ, _changeCellSpacingMETERS))
                {
                    ok = false;
                }
                else
                {
                    _dirtyFlags |= DIRTY_SIM_PARAMS;
                    _dirtyFlags &= (~DIRTY_GRID_DIMENSIONS);
                }
            }

            if (ok && (_dirtyFlags & DIRTY_DEVICES))
            {
                if (!selectDevices(_changeSmokeSimDevice, _changeVolSliceDevice))
                {
                    ok = false;
                }
                else
                {
                    _dirtyFlags &= (~DIRTY_DEVICES);
                }
            }

            // Initialize the resources:
            if (ok && (initOpenCLResources() && !createOpenCLResources()))
            {
                releaseOpenCLResources();
                ok = false;
            }

            _logger->setProgress(ok ? "OK" : _lastError._str);
        }

        if (ok && (_initStage & INIT_STAGE_CREATE_RESOURCES))
        {
            // If there were any changes to the fluid parameters, recalculate
            // the new parameters for the kernels.
            if (_dirtyFlags & DIRTY_SIM_PARAMS)
            {
                recalculateSmokeSimConstants();
                _dirtyFlags &= (~DIRTY_SIM_PARAMS);
            }

            // Compute the time since the last run (in seconds).
            float deltaTime = (static_cast<float>(_timer.restart()));

            // Carry out one step of the simulation, limitting maximum delta time to 20ms.
            if (compute(state, modelTransformation, deltaTime < _simParams._maxDeltaTime ? deltaTime : _simParams._maxDeltaTime, deltaRot))
            {
                // Render the smoke density.
                render(state, modelTransformation);
            }
            else
            {
                _logger->setProgress("Failed on compute()");
                ok = false;
            }
        }
    }

    return ok;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::initSmokeFields
// Description: Initialize the smoke fields - zero density everywhere,
//              ambient temperature in each cell and zero velocity fields.
//              Initialize the boundary conditions memory objects.
bool AMDTTeapotOCLSmokeSystem::initSmokeFields()
{
    bool retVal = true;
    CalcProgram* program = _programs[CALCID_SMOKE_SIMULATION];
    CalcContext* context = program->_calcContext;
    CalcDevice* device = program->_calcDevice;
    CalcMemory* mem;
    cl_int status;

    size_t scalarSize = sizeof(cl_float) * _fluidGrid._totalNumCells;
    size_t vecSize = scalarSize << 2;
    memset(_gridBuffer, 0, vecSize);

    mem = &program->_memObjects[MO_SMOKESIM_U03D];
    status = _clEnqueueWriteBuffer(
                 device->_clCmdQueue,
                 mem->_clMemory,
                 CL_TRUE,
                 0,
                 vecSize,
                 _gridBuffer,
                 0,
                 NULL,
                 NULL);

    if (status != CL_SUCCESS)
    {
        _lastError.printf("ERROR: Error calling _clEnqueueWriteBuffer(platform=%s, device=%s, MO_SMOKESIM_U03D): [%d] %s",
                          context->_platform->getName(),
                          device->_device->getName(),
                          status,
                          _ocl->getOpenCLErrorCodeStr(status));
        retVal = false;
    }
    else
    {
        cl_float* p;

        mem = &program->_memObjects[MO_SMOKESIM_S03D];
        status = _clEnqueueWriteBuffer(
                     device->_clCmdQueue,
                     mem->_clMemory,
                     CL_TRUE,
                     0,
                     scalarSize,
                     _gridBuffer,
                     0,
                     NULL,
                     NULL);

        if (status != CL_SUCCESS)
        {
            _lastError.printf("ERROR: Error calling _clEnqueueWriteBuffer(platform=%s, device=%s, MO_SMOKESIM_S03D): [%d] %s",
                              context->_platform->getName(),
                              device->_device->getName(),
                              status,
                              _ocl->getOpenCLErrorCodeStr(status));
            retVal = false;
        }
        else
        {
            p = (cl_float*)_gridBuffer;

            for (int i = _fluidGrid._totalNumCells; i > 0; --i)
            {
                *p++ = _smokeSimConstants._ambiantTemperature;
            }

            mem = &program->_memObjects[MO_SMOKESIM_T03D];
            status = _clEnqueueWriteBuffer(
                         device->_clCmdQueue,
                         mem->_clMemory,
                         CL_TRUE,
                         0,
                         scalarSize,
                         _gridBuffer,
                         0,
                         NULL,
                         NULL);

            if (status != CL_SUCCESS)
            {
                _lastError.printf("ERROR: Error calling _clEnqueueWriteBuffer(platform=%s, device=%s, MO_SMOKESIM_T03D): [%d] %s",
                                  context->_platform->getName(),
                                  device->_device->getName(),
                                  status,
                                  _ocl->getOpenCLErrorCodeStr(status));
                retVal = false;
            }
            else
            {
                // Setup boundary conditions - we setup non-slip conditions on all
                // edge cells.
                p = (cl_float*)_gridBuffer;
                cl_int* pi = (cl_int*)(_gridBuffer + vecSize);
                cl_int index = 0;
                cl_int deltaSlice = _fluidGrid._strideZ;
                cl_int deltaRow = _fluidGrid._strideY;
                cl_int deltaIndexZ, deltaIndexY, deltaIndex;

                for (int z = 1; z <= _fluidGrid._numCellsZ; ++z)
                {
                    if (z == 1)
                    {
                        deltaIndexZ = deltaSlice;
                    }
                    else if (z == _fluidGrid._numCellsZ)
                    {
                        deltaIndexZ = -deltaSlice;
                    }
                    else
                    {
                        deltaIndexZ = 0;
                    }

                    for (int y = 1; y <= _fluidGrid._numCellsY; ++y)
                    {
                        if (y == 1)
                        {
                            deltaIndexY = deltaIndexZ + deltaRow;
                        }
                        else if (y == _fluidGrid._numCellsY)
                        {
                            deltaIndexY = deltaIndexZ - deltaRow;
                        }
                        else
                        {
                            deltaIndexY = deltaIndexZ;
                        }

                        for (int x = 1; x <= _fluidGrid._numCellsX; ++x, ++index)
                        {
                            if (x == 1)
                            {
                                deltaIndex = deltaIndexY + 1;
                            }
                            else if (x == _fluidGrid._numCellsX)
                            {
                                deltaIndex = deltaIndexY - 1;
                            }
                            else
                            {
                                deltaIndex = deltaIndexY;
                            }

                            *pi++ = index + deltaIndex;

                            if (deltaIndex)
                            {
                                *p++ = -1.0f;
                                *p++ = -1.0f;
                                *p++ = -1.0f;
                                *p++ = 1.0f;
                            }
                            else
                            {
                                *p++ = 1.0f;
                                *p++ = 1.0f;
                                *p++ = 1.0f;
                                *p++ = 1.0f;
                            }
                        }
                    }
                }

                mem = &program->_memObjects[MO_SMOKESIM_BOUNDARY_INDEX];
                status = _clEnqueueWriteBuffer(
                             device->_clCmdQueue,
                             mem->_clMemory,
                             CL_TRUE,
                             0,
                             scalarSize,
                             _gridBuffer + vecSize,
                             0,
                             NULL,
                             NULL);

                if (status != CL_SUCCESS)
                {
                    _lastError.printf("ERROR: Error calling _clEnqueueWriteBuffer(platform=%s, device=%s, MO_SMOKESIM_BOUNDARY_INDEX): [%d] %s",
                                      context->_platform->getName(),
                                      device->_device->getName(),
                                      status,
                                      _ocl->getOpenCLErrorCodeStr(status));
                    retVal = false;
                }
                else
                {
                    mem = &program->_memObjects[MO_SMOKESIM_BOUNDARY_SCALE];
                    status = _clEnqueueWriteBuffer(
                                 device->_clCmdQueue,
                                 mem->_clMemory,
                                 CL_TRUE,
                                 0,
                                 vecSize,
                                 _gridBuffer,
                                 0,
                                 NULL,
                                 NULL);

                    if (status != CL_SUCCESS)
                    {
                        _lastError.printf("ERROR: Error calling _clEnqueueWriteBuffer(platform=%s, device=%s, MO_SMOKESIM_BOUNDARY_SCALE): [%d] %s",
                                          context->_platform->getName(),
                                          device->_device->getName(),
                                          status,
                                          _ocl->getOpenCLErrorCodeStr(status));
                        retVal = false;
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::initVolumeSlicingData
// Description: Initialize volume slicing constants memory object.
bool AMDTTeapotOCLSmokeSystem::initVolumeSlicingData()
{
    bool retVal = true;
    CalcProgram* program = _programs[CALCID_VOLUME_SLICING];
    CalcContext* context = program->_calcContext;
    CalcDevice* device = program->_calcDevice;
    CalcMemory* mem;
    cl_int status;

    mem = &program->_memObjects[MO_VOLSLICE_CONSTANTS];
    status = _clEnqueueWriteBuffer(
                 device->_clCmdQueue,
                 mem->_clMemory,
                 CL_TRUE,
                 0,
                 mem->_memSize,
                 &_volSliceConstants,
                 0,
                 NULL,
                 NULL);

    if (status != CL_SUCCESS)
    {
        _lastError.printf("ERROR: Error calling _clEnqueueWriteBuffer(platform=%s, device=%s, MO_VOLSLICE_CONSTANTS): [%d] %s",
                          context->_platform->getName(),
                          device->_device->getName(),
                          status,
                          _ocl->getOpenCLErrorCodeStr(status));
        retVal = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::compute
// Description: Carry out one simulation step for given delta time.
bool AMDTTeapotOCLSmokeSystem::compute(
    const AMDTTeapotRenderState& state,
    const Mat4& modelTransformation,
    float deltaTimeSeconds,
    float deltaRot)
{
    bool ok = true;
    cl_int status;
    CalcKernel* kernel = NULL;
    CalcProgram* program = _programs[CALCID_SMOKE_SIMULATION];
    CalcContext* context = program->_calcContext;
    CalcDevice* device = program->_calcDevice;
    CalcProgram* vsProgram = _programs[CALCID_VOLUME_SLICING];
    CalcContext* vsContext = vsProgram->_calcContext;
    CalcDevice* vsDevice = vsProgram->_calcDevice;
    CalcMemory* constantsMem = NULL;

    // Initialize the fields if necessary
    if (_dirtyFlags & DIRTY_RESET_SMOKESIM)
    {
        if (!initSmokeFields())
        {
            ok = false;
        }
        else
        {
            _dirtyFlags &= (~DIRTY_RESET_SMOKESIM);
        }
    }

    // Initialize the volume slicing data
    if (ok && (_dirtyFlags & DIRTY_RESET_VOLSLICE))
    {
        if (!initVolumeSlicingData())
        {
            ok = false;
        }
        else
        {
            _dirtyFlags &= (~DIRTY_RESET_VOLSLICE);
        }
    }

    if (ok)
    {
        // Setup smoke simulation parameters.
        _smokeSimConstants._deltaTimeInSeconds = deltaTimeSeconds;
        _smokeSimConstants._KdissipateDens = 1.0f / (1.0f + deltaTimeSeconds * _smokeSimConstants._KdrDens);
        _smokeSimConstants._KdissipateTemp = 1.0f / (1.0f + deltaTimeSeconds * _smokeSimConstants._KdrTemp);

        // Calculate new position of smoke source and exit velocity and direction.
        // We randomize the smoke calculating a new radial and angular velocity of
        // the center of the source and changing this every few seconds (when to change
        // is also randomized).
        _smokeSourceCurrentDuration += deltaTimeSeconds;

        if (_smokeSourceCurrentDuration >= _smokeSourceNextUpdateTime)
        {
            _smokeSourceNextUpdateTime = 1.0f + rMath::AbsRandf();
            _smokeSourceCurrentDuration = 0.0f;

            _smokeSourceAngularVelocity = (_smokeSourceMaxAngularAccel * rMath::Randf());

            if (_smokeSourceAngularVelocity < -_smokeSourceMaxAngularVelocity)
            {
                _smokeSourceAngularVelocity = -_smokeSourceMaxAngularVelocity;
            }
            else if (_smokeSourceAngularVelocity > _smokeSourceMaxAngularVelocity)
            {
                _smokeSourceAngularVelocity = _smokeSourceMaxAngularVelocity;
            }

            _smokeSourceRadialVelocity = (_smokeSourceMaxRadialAccel * rMath::Randf());

            if (_smokeSourceRadialVelocity < -_smokeSourceMaxRadialVelocity)
            {
                _smokeSourceRadialVelocity = -_smokeSourceMaxRadialVelocity;
            }
            else if (_smokeSourceRadialVelocity > _smokeSourceMaxRadialVelocity)
            {
                _smokeSourceRadialVelocity = _smokeSourceMaxRadialVelocity;
            }
        }

        _smokeSourceAngle += (_smokeSourceAngularVelocity * deltaTimeSeconds);

        if (_smokeSourceAngle < 0.0f)
        {
            do
            {
                _smokeSourceAngle += 360.0f;
            }
            while (_smokeSourceAngle < 0.0f);
        }
        else if (_smokeSourceAngle > 360.f)
        {
            do
            {

                _smokeSourceAngle -= 360.0f;
            }
            while (_smokeSourceAngle > 360.0f);
        }

        _smokeSourceRadius += (_smokeSourceRadialVelocity * deltaTimeSeconds);

        if (_smokeSourceRadius < 0.0f)
        {
            _smokeSourceRadius = 0.0f;
        }
        else if (_smokeSourceRadius > _smokeSourceMaxRadius)
        {
            _smokeSourceRadius = _smokeSourceMaxRadius;
        }

        float rotVelocity = 0.0f;

        if (deltaRot < 0.0f)
        {
            rotVelocity = deltaRot > -3.0f ? deltaRot : -3.0f;
        }
        else if (deltaRot > 0.0f)
        {
            rotVelocity = deltaRot < 3.0f ? deltaRot : 3.0f;
        }

        float angleRAD = rMath::Deg2Rad(_smokeSourceAngle);
        float ca = static_cast<float>(cos(angleRAD));
        float sa = static_cast<float>(sin(angleRAD));
        _smokeSimConstants._sourceCenter.s[0] = _smokeSimConstants._spoutCenter.s[0] + _smokeSourceRadius * ca;
        _smokeSimConstants._sourceCenter.s[1] = _smokeSimConstants._spoutCenter.s[1] + _smokeSourceRadius * sa;
        _smokeSimConstants._sourceVelocity.s[0] = _smokeSourceHorizontalVelocity * ca;
        _smokeSimConstants._sourceVelocity.s[1] = _smokeSourceHorizontalVelocity * (sa + rotVelocity);
        _smokeSimConstants._sourceVelocity.s[2] = _smokeSourceVerticalVelocity;
        _smokeSimConstants._sourceVelocity.s[3] = 0;

        // Clear OpenCL events structure.
        memset(_waitEvents, 0, sizeof(_waitEvents));

        // Smoke simulation - transfer constants/parameters
        constantsMem = &program->_memObjects[MO_SMOKESIM_CONSTANTS];
        status = _clEnqueueWriteBuffer(
                     device->_clCmdQueue,
                     constantsMem->_clMemory,
                     CL_FALSE,
                     0,
                     sizeof(_smokeSimConstants),
                     (void*)&_smokeSimConstants,
                     0,
                     NULL,
                     NULL);

        if (status != CL_SUCCESS)
        {
            _lastError.printf("ERROR: Error calling _clEnqueueWriteBuffer(platform=%s, device=%s, MO_SMOKESIM_CONSTANTS): [%d] %s",
                              context->_platform->getName(),
                              device->_device->getName(),
                              status,
                              _ocl->getOpenCLErrorCodeStr(status));
            ok = false;
        }
    }

    // Smoke simulation - enqueue all the kernels.
    cl_event events[2] = { NULL, NULL };

    for (int i = 0; ok && (i < program->_numKernels); ++i)
    {
        kernel = &program->_kernels[i];

        if (kernel->_clKernel)
        {
            status = _clEnqueueNDRangeKernel(
                         device->_clCmdQueue,
                         kernel->_clKernel,
                         3,
                         NULL,
                         kernel->_globalWorkSize,
                         kernel->_localWorkSize,
                         events[0] ? 1 : 0,
                         events[0] ? events : NULL,
                         &(events[1]));

            if (status != CL_SUCCESS)
            {
                _lastError.printf("ERROR: Error calling _clEnqueueNDRangeKernel(platform=%s, device=%s, kernel=%s): [%d] %s",
                                  context->_platform->getName(),
                                  device->_device->getName(),
                                  kernel->_kernelFuncName,
                                  status,
                                  _ocl->getOpenCLErrorCodeStr(status));
                ok = false;
                break;
            }

            // Swap the events:
            if (events[0] != NULL)
            {
                status = _clReleaseEvent(events[0]);

                if (status != CL_SUCCESS)
                {
                    _lastError.printf("ERROR: Error releasing event after _clEnqueueNDRangeKernel(platform=%s, device=%s, kernel=%s): [%d] %s",
                                      context->_platform->getName(),
                                      device->_device->getName(),
                                      kernel->_kernelFuncName,
                                      status,
                                      _ocl->getOpenCLErrorCodeStr(status));
                }
            }

            events[0] = events[1];
            events[1] = NULL;

            // Deal with jumping back to execute an earlier kernel -
            // running an iteration.
            if (kernel->_repeatKernelIndex >= 0)
            {
                if (kernel->_repeatKernelIteration == 0)
                {
                    kernel->_repeatKernelIteration = kernel->_repeatKernelCount - 1;
                }
                else
                {
                    --kernel->_repeatKernelIteration;
                }

                if (kernel->_repeatKernelIteration > 0)
                {
                    i = kernel->_repeatKernelIndex - 1;
                }
            }
        }
    }

    // Release the last kernel's event:
    if (events[0] != NULL)
    {
        _clReleaseEvent(events[0]);
        events[0] = NULL;
    }

    if (ok)
    {
        // Kick off the smoke simulation
        status = _clFlush(device->_clCmdQueue);

        if (status != CL_SUCCESS)
        {
            _lastError.printf("ERROR: Error calling _clFlush(platform=%s, device=%s): [%d] %s",
                              context->_platform->getName(),
                              device->_device->getName(),
                              status,
                              _ocl->getOpenCLErrorCodeStr(status));
            ok = false;
        }
    }

    if (ok)
    {
        // Setup volume slicing parameters
        _fluidGrid.SetModelTransformation(modelTransformation);
        float v[8 * 3];
        _fluidGrid.GetTransformedVertices(v);

        for (int i = 0; i < 8; ++i)
        {
            int j = i * 3;
            _volSliceParams._tVerts[i].s[0] = v[j++];
            _volSliceParams._tVerts[i].s[1] = v[j++];
            _volSliceParams._tVerts[i].s[2] = v[j];
        }

        _volSliceParams._frontIdx = _fluidGrid.GetClosestIndex(state.getEyePosition());
        Vec3 vNear = _fluidGrid.GetVertex(_volSliceParams._frontIdx);   // !v0 is in world space
        Vec3 vFar = _fluidGrid.GetVertex(AMDTFluidGrid::farIdx[_volSliceParams._frontIdx]);

        // eye (lookat) vector
        Vec3 Np = state.getEyeLookAt() - state.getEyePosition();
        _volSliceParams._view.s[0] = Np._x;
        _volSliceParams._view.s[1] = Np._y;
        _volSliceParams._view.s[2] = Np._z;

        // find nearest plane dist from Origin:
        _numSlicesToRender = _fluidGrid._maxSlices;
        float distNearPlane = (vNear - Vec3(0, 0, 0)) * Np; // projection len over Np
        float distFarPlane = (vFar - Vec3(0, 0, 0)) * Np;
        _volSliceParams._dPlaneIncr = (distFarPlane - distNearPlane) / (float)(_numSlicesToRender + 1);
        _volSliceParams._dPlaneStart = distNearPlane + _volSliceParams._dPlaneIncr;
        _volSliceParams._numSlices = _numSlicesToRender;
    }

    if (ok && _oclMemGLVBO)
    {
        // Acquire the volume slice VBO if we have GL sharing since in this
        // case the kernel will write directly into it.
        status = _clEnqueueAcquireGLObjects(
                     vsDevice->_clCmdQueue,
                     1,
                     &_oclMemGLVBO,
                     0,
                     NULL,
                     NULL
                 );

        if (status != CL_SUCCESS)
        {
            _lastError.printf("ERROR: Error calling _clEnqueueAcquireGLObjects(platform=%s, device=%s): [%d] %s",
                              vsContext->_platform->getName(),
                              vsDevice->_device->getName(),
                              status,
                              _ocl->getOpenCLErrorCodeStr(status));
            ok = false;
        }
    }

    if (ok)
    {
        // Transfer volume slice params to the device
        constantsMem = &vsProgram->_memObjects[MO_VOLSLICE_PARAMS];
        status = _clEnqueueWriteBuffer(
                     vsDevice->_clCmdQueue,
                     constantsMem->_clMemory,
                     CL_FALSE,
                     0,
                     constantsMem->_memSize,
                     (void*)&_volSliceParams,
                     0,
                     NULL,
                     NULL);

        if (status != CL_SUCCESS)
        {
            _lastError.printf("ERROR: Error calling _clEnqueueWriteBuffer(platform=%s, device=%s, MO_VOLSLICE_PARAMS): [%d] %s",
                              vsContext->_platform->getName(),
                              vsDevice->_device->getName(),
                              status,
                              _ocl->getOpenCLErrorCodeStr(status));
            ok = false;
        }
    }

    if (ok)
    {
        // Volume slicing - enqueue the kernel
        kernel = &vsProgram->_kernels[KERN_VOLSLICE_COMPUTE_INTERSECTION];

        if (NULL != kernel->_clKernel)
        {
            kernel->_globalWorkSize[0] = _numSlicesToRender;
            status = _clEnqueueNDRangeKernel(
                         vsDevice->_clCmdQueue,
                         kernel->_clKernel,
                         1,
                         NULL,
                         kernel->_globalWorkSize,
                         NULL,
                         0,
                         NULL,
                         NULL);

            if (status != CL_SUCCESS)
            {
                _lastError.printf("ERROR: Error calling _clEnqueueNDRangeKernel(platform=%s, device=%s, kernel=%s): [%d] %s",
                                  vsContext->_platform->getName(),
                                  vsDevice->_device->getName(),
                                  kernel->_kernelFuncName,
                                  status,
                                  _ocl->getOpenCLErrorCodeStr(status));
                ok = false;
            }
        }
    }


    if (ok && _oclMemGLVBO)
    {
        // Release the VBO if we have GL sharing

        // Wait for the volume slicing to finish so that we can
        // give GL access to the VBO again.
        _clFinish(vsDevice->_clCmdQueue);

        status = _clEnqueueReleaseGLObjects(
                     vsDevice->_clCmdQueue,
                     1,
                     &_oclMemGLVBO,
                     0,
                     NULL,
                     NULL);

        if (status != CL_SUCCESS)
        {
            _lastError.printf("ERROR: Error calling _clEnqueueReleaseGLObjects(platform=%s, device=%s): [%d] %s",
                              vsContext->_platform->getName(),
                              vsDevice->_device->getName(),
                              status,
                              _ocl->getOpenCLErrorCodeStr(status));
            ok = false;
        }
    }
    else
    {
        // Kick off the volume slicing kernels.
        status = _clFlush(vsDevice->_clCmdQueue);

        if (status != CL_SUCCESS)
        {
            _lastError.printf("ERROR: Error calling _clFlush(platform=%s, device=%s): [%d] %s",
                              vsContext->_platform->getName(),
                              vsDevice->_device->getName(),
                              status,
                              _ocl->getOpenCLErrorCodeStr(status));
            ok = false;
        }
    }


    // If GL sharing is active, enqueue a copy operation, otherwise
    // read the density buffer and set the density texture data.
    CalcMemory* srcMem = &program->_memObjects[MO_SMOKESIM_DENSITY];
    void* memPtr = NULL;

    if (ok)
    {
        if (_oclMemGLDensityTexture)
        {
            status = _clEnqueueAcquireGLObjects(
                         device->_clCmdQueue,
                         1,
                         &_oclMemGLDensityTexture,
                         0,
                         NULL,
                         NULL
                     );

            if (status != CL_SUCCESS)
            {
                _lastError.printf("ERROR: Error calling _clEnqueueAcquireGLObjects(platform=%s, device=%s): [%d] %s",
                                  context->_platform->getName(),
                                  device->_device->getName(),
                                  status,
                                  _ocl->getOpenCLErrorCodeStr(status));
                ok = false;
            }
            else
            {
                // Copy the density buffer generated by the smoke sim kernels into
                // the OpenGL 3D RGBA float texture.
                size_t origin[3] = { 0, 0, 0 };
                size_t region[3] = { size_t(_fluidGrid._numCellsX), size_t(_fluidGrid._numCellsY), size_t(_fluidGrid._numCellsZ) };
                status = _clEnqueueCopyBufferToImage(
                             device->_clCmdQueue,
                             srcMem->_clMemory,
                             _oclMemGLDensityTexture,
                             0,
                             origin,
                             region,
                             0,
                             NULL,
                             NULL
                         );

                if (status != CL_SUCCESS)
                {
                    _lastError.printf("ERROR: Error calling _clEnqueueCopyBufferToImage(platform=%s, device=%s): [%d] %s",
                                      context->_platform->getName(),
                                      device->_device->getName(),
                                      status,
                                      _ocl->getOpenCLErrorCodeStr(status));
                    ok = false;
                }
                else
                {
                    status = _clEnqueueReleaseGLObjects(
                                 device->_clCmdQueue,
                                 1,
                                 &_oclMemGLDensityTexture,
                                 0,
                                 NULL,
                                 NULL);

                    if (status != CL_SUCCESS)
                    {
                        _lastError.printf("ERROR: Error calling _clEnqueueReleaseGLObjects(platform=%s, device=%s): [%d] %s",
                                          context->_platform->getName(),
                                          device->_device->getName(),
                                          status,
                                          _ocl->getOpenCLErrorCodeStr(status));
                        ok = false;
                    }
                }
            }
        }
        else
        {
            // No GL-CL sharing. Read the density buffer into host memory and copying
            // into the OpenGL 3D texture.
            memPtr = _clEnqueueMapBuffer(
                         device->_clCmdQueue,
                         srcMem->_clMemory,
                         CL_FALSE,
                         CL_MAP_READ,
                         0,
                         srcMem->_memSize,
                         0,
                         NULL,
                         &_waitEvents[EVT_SMOKESIM_COPY_BUFFER],
                         &status);

            if (status != CL_SUCCESS)
            {
                _lastError.printf("ERROR: Error calling _clEnqueueMapBuffer(platform=%s, device=%s): [%d] %s",
                                  context->_platform->getName(),
                                  device->_device->getName(),
                                  status,
                                  _ocl->getOpenCLErrorCodeStr(status));
                ok = false;
            }
        }
    }

    // Volume slicing - if we don't have GL sharing, we need to copy
    // the vertices into the VBO ourselves.
    CalcMemory* vsSrcMem = &vsProgram->_memObjects[MO_VOLSLICE_VERTICES];
    void* vsMemPtr = NULL;

    if (ok)
    {
        if (NULL == _oclMemGLVBO)
        {
            vsMemPtr = _clEnqueueMapBuffer(
                           vsDevice->_clCmdQueue,
                           vsSrcMem->_clMemory,
                           CL_FALSE,
                           CL_MAP_READ,
                           0,
                           vsSrcMem->_memSize,
                           0,
                           NULL,
                           &_waitEvents[EVT_VOLSLICE_COPY_BUFFER],
                           &status);

            if (status != CL_SUCCESS)
            {
                _lastError.printf("ERROR: Error calling _clEnqueueMapBuffer(platform=%s, device=%s): [%d] %s",
                                  vsContext->_platform->getName(),
                                  vsDevice->_device->getName(),
                                  status,
                                  _ocl->getOpenCLErrorCodeStr(status));
                ok = false;
            }
        }
    }

    if (ok)
    {
        // Kick any remaining enqueues off - e.g. make sure the map operations are executed now.
        status = _clFlush(device->_clCmdQueue);

        if (status != CL_SUCCESS)
        {
            _lastError.printf("ERROR: Error calling _clFlush(platform=%s, device=%s): [%d] %s",
                              context->_platform->getName(),
                              device->_device->getName(),
                              status,
                              _ocl->getOpenCLErrorCodeStr(status));
            ok = false;
        }
    }


    if (ok && (device != vsDevice))
    {
        status = _clFlush(vsDevice->_clCmdQueue);

        if (status != CL_SUCCESS)
        {
            _lastError.printf("ERROR: Error calling _clFlush(platform=%s, device=%s): [%d] %s",
                              vsContext->_platform->getName(),
                              vsDevice->_device->getName(),
                              status,
                              _ocl->getOpenCLErrorCodeStr(status));
            ok = false;
        }
    }

    // If we don't have GL-CL sharing for the 3D texture, wait for
    // the buffer map operations to complete and copy the density buffer
    // into the texture.
    if (ok && memPtr)
    {
        // We mapped the OpenCL density buffer. Copy to OpenGL texture once the
        // mapping has completed.
        status = _clWaitForEvents(1, &_waitEvents[EVT_SMOKESIM_COPY_BUFFER]);

        if (status != CL_SUCCESS)
        {
            _lastError.printf("ERROR: Error calling _clWaitForEvents(platform=%s, device=%s): [%d] %s",
                              context->_platform->getName(),
                              device->_device->getName(),
                              status,
                              _ocl->getOpenCLErrorCodeStr(status));
            ok = false;
        }
        else
        {
            // Copy data into the GL texture.
            glBindTexture(GL_TEXTURE_3D, _texDensity);
            _glTexImage3D(
                GL_TEXTURE_3D,
                0,
                _texDensityInternalFormat,
                _fluidGrid._numCellsX,
                _fluidGrid._numCellsY,
                _fluidGrid._numCellsZ,
                0,
                _texDensityFormat,
                _texDensityType,
                memPtr);
            GLenum glErr = glGetError();
            glBindTexture(GL_TEXTURE_3D, 0);

            // Unmap the density buffer.
            status = _clEnqueueUnmapMemObject(
                         device->_clCmdQueue,
                         srcMem->_clMemory,
                         memPtr,
                         0,
                         NULL,
                         NULL);

            if (glErr)
            {
                _lastError.printf("ERROR: GL error calling glTexImage3D(): %d", glErr);
                ok = false;
            }
            else if (status != CL_SUCCESS)
            {
                _lastError.printf("ERROR: Error calling _clEnqueueUnmapMemObject(platform=%s, device=%s): [%d] %s",
                                  context->_platform->getName(),
                                  device->_device->getName(),
                                  status,
                                  _ocl->getOpenCLErrorCodeStr(status));
                ok = false;
            }
        }
    }


    if (ok && vsMemPtr)
    {
        // We mapped the OpenCL vertex buffer. Copy to OpenGL VBO
        status = _clWaitForEvents(1, &_waitEvents[EVT_VOLSLICE_COPY_BUFFER]);

        if (status != CL_SUCCESS)
        {
            _lastError.printf("ERROR: Error calling _clWaitForEvents(platform=%s, device=%s): [%d] %s",
                              vsContext->_platform->getName(),
                              vsDevice->_device->getName(),
                              status,
                              _ocl->getOpenCLErrorCodeStr(status));
            ok = false;
        }
        else
        {
            GLenum glErr = 0;

            if (_isOpenGL1_5Supported)
            {
                _glBindBuffer(GL_ARRAY_BUFFER, _volSliceVBO);
                _glBufferSubData(
                    GL_ARRAY_BUFFER,
                    0,
                    _numSlicesToRender * 72 * sizeof(GLfloat),
                    vsMemPtr);
                glErr = glGetError();
                _glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            else  if (_isGL_ARB_vertex_buffer_objectSupported)
            {
                _glBindBufferARB(GL_ARRAY_BUFFER_ARB, _volSliceVBO);
                _glBufferSubData(
                    GL_ARRAY_BUFFER_ARB,
                    0,
                    _numSlicesToRender * 72 * sizeof(GLfloat),
                    vsMemPtr);
                glErr = glGetError();
                _glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
            }

            status = _clEnqueueUnmapMemObject(
                         vsDevice->_clCmdQueue,
                         vsSrcMem->_clMemory,
                         vsMemPtr,
                         0,
                         NULL,
                         NULL);

            if (glErr)
            {
                _lastError.printf("ERROR: GL error calling glBufferSubData(): %d", glErr);
                ok = false;
            }
            else if (status != CL_SUCCESS)
            {
                _lastError.printf("ERROR: Error calling _clEnqueueUnmapMemObject(platform=%s, device=%s): [%d] %s",
                                  vsContext->_platform->getName(),
                                  vsDevice->_device->getName(),
                                  status,
                                  _ocl->getOpenCLErrorCodeStr(status));
                ok = false;
            }
        }
    }

    if (ok)
    {
        // Wait for everything to finish.
        status = _clFinish(device->_clCmdQueue);

        if (status != CL_SUCCESS)
        {
            _lastError.printf("ERROR: Error calling _clFinish(platform=%s, device=%s): [%d] %s",
                              context->_platform->getName(),
                              device->_device->getName(),
                              status,
                              _ocl->getOpenCLErrorCodeStr(status));
            ok = false;
        }
    }


    if (ok && (device != vsDevice))
    {
        status = _clFinish(vsDevice->_clCmdQueue);

        if (status != CL_SUCCESS)
        {
            _lastError.printf("ERROR: Error calling _clFinish(platform=%s, device=%s): [%d] %s",
                              vsContext->_platform->getName(),
                              vsDevice->_device->getName(),
                              status,
                              _ocl->getOpenCLErrorCodeStr(status));
            ok = false;
        }
    }

    // Release all events
    for (int i = NUM_WAIT_EVENTS - 1; i >= 0; --i)
    {
        if (_waitEvents[i])
        {
            _clReleaseEvent(_waitEvents[i]);
        }
    }

    return ok;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::render
// Description: Render the 3D texture using the volume slices stored in the VBO.
//              This assumes that the model-_view matrix has been setup with only
//              the _view transformation (since the grid coordinates have already
//              been transformed when computing the volume slices.
bool AMDTTeapotOCLSmokeSystem::render(
    const AMDTTeapotRenderState& state,
    const Mat4& modelTransformation)
{
    (void)(modelTransformation);
    (void)(state);
    _glUseProgramObjectARB(0);

    if (_showGrid)
    {
        _fluidGrid.DrawOutline();
    }

    if (_isOpenGL1_5Supported)
    {
        _glBindBuffer(GL_ARRAY_BUFFER, _volSliceVBO);
    }
    else  if (_isGL_ARB_vertex_buffer_objectSupported)
    {
        _glBindBufferARB(GL_ARRAY_BUFFER_ARB, _volSliceVBO);
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 6 * sizeof(GLfloat), (const GLvoid*)0);
    glTexCoordPointer(3, GL_FLOAT, 6 * sizeof(GLfloat), (const GLvoid*)(sizeof(GLfloat) * 3));

    // Bind the 3D texture
    glEnable(GL_TEXTURE_3D);
    glBindTexture(GL_TEXTURE_3D, _texDensity);
    // Take color directly from texture (don't combine with glColor).
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    //glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
    glEnable(GL_BLEND);
    // use the "under" operator
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw all the triangles.
    glDrawArrays(GL_TRIANGLES, 0, _numSlicesToRender * 4 * 3);

    glDisable(GL_TEXTURE_3D);

    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_3D, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    if (_isOpenGL1_5Supported)
    {
        _glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else  if (_isGL_ARB_vertex_buffer_objectSupported)
    {
        _glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }

    return true;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::getKernelWorkGroupInfo
// Description: For a given kernel, query OpenCL for information about max
//              workgroup sizes, local memory on the device etc.
bool AMDTTeapotOCLSmokeSystem::getKernelWorkGroupInfo(
    const OCLDevice* device,
    CalcKernel* kernel)
{
    bool retVal = true;
    cl_device_id devid = device->getDeviceID();
    cl_int status;
    size_t data_sizet;
    cl_ulong data_ulong;

    status = _clGetKernelWorkGroupInfo(
                 kernel->_clKernel,
                 devid,
                 CL_KERNEL_WORK_GROUP_SIZE,
                 sizeof(data_sizet),
                 &data_sizet,
                 NULL);

    if (status != CL_SUCCESS)
    {
        _lastError.printf("ERROR: Error calling _clGetKernelWorkGroupInfo(kernel=%s, CL_KERNEL_WORK_GROUP_SIZE): [%d] %s",
                          kernel->_kernelFuncName,
                          status,
                          _ocl->getOpenCLErrorCodeStr(status));
        retVal = false;
    }
    else
    {
        kernel->_maxKernelWorkGroupSize = data_sizet;

        status = _clGetKernelWorkGroupInfo(
                     kernel->_clKernel,
                     devid,
                     CL_KERNEL_LOCAL_MEM_SIZE,
                     sizeof(data_ulong),
                     &data_ulong,
                     NULL);

        if (status != CL_SUCCESS)
        {
            _lastError.printf("ERROR: Error calling _clGetKernelWorkGroupInfo(kernel=%s, CL_KERNEL_LOCAL_MEM_SIZE): [%d] %s",
                              kernel->_kernelFuncName,
                              status,
                              _ocl->getOpenCLErrorCodeStr(status));
            retVal = false;
        }
        else
        {
            kernel->_kernelLocalMemSize = data_ulong;

            status = _clGetKernelWorkGroupInfo(
                         kernel->_clKernel,
                         devid,
                         CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                         sizeof(data_sizet),
                         &data_sizet,
                         NULL);

            if (status != CL_SUCCESS)
            {
                _lastError.printf("ERROR: Error calling _clGetKernelWorkGroupInfo(kernel=%s, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE): [%d] %s",
                                  kernel->_kernelFuncName,
                                  status,
                                  _ocl->getOpenCLErrorCodeStr(status));
                retVal = false;
            }
            else
            {
                kernel->_kernelPreferredWorkGroupSizeMultiple = data_sizet;

                // Make sure the kernel local memory usage fits in.
                if (kernel->_kernelLocalMemSize > device->getLocalMemSize())
                {
                    _lastError.printf("ERROR: kernel=%s needs %d bytes of local memory but device only has %d bytes.",
                                      kernel->_kernelFuncName,
                                      kernel->_kernelLocalMemSize,
                                      device->getLocalMemSize());

                    retVal = false;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::setup3DKernelWorkGroupSizes
// Description: Based on OpenCL device specifics (maximum work group size etc)
//              calculate the best way to setup the local work group sizes to
//              cover the entire 3D grid such memory accesses will be
//              coalesced.
bool AMDTTeapotOCLSmokeSystem::setup3DKernelWorkGroupSizes(
    CalcKernel* kernel,
    int width,
    int depth,
    int height)
{
    kernel->_globalWorkSize[0] = width;
    kernel->_globalWorkSize[1] = depth;
    kernel->_globalWorkSize[2] = height;

    if (kernel->_globalWorkSize[0] < kernel->_maxKernelWorkGroupSize)
    {
        kernel->_localWorkSize[0] = kernel->_globalWorkSize[0];
        kernel->_localWorkSize[1] = kernel->_maxKernelWorkGroupSize / kernel->_localWorkSize[0];

        if (kernel->_localWorkSize[1] <= kernel->_globalWorkSize[1])
        {
            kernel->_localWorkSize[2] = 1;
        }
        else
        {
            kernel->_localWorkSize[1] = kernel->_globalWorkSize[1];
            size_t total = kernel->_localWorkSize[0] * kernel->_localWorkSize[1];
            kernel->_localWorkSize[2] =
                kernel->_maxKernelWorkGroupSize / total;
        }
    }
    else
    {
        kernel->_localWorkSize[0] = kernel->_maxKernelWorkGroupSize;
        kernel->_localWorkSize[1] = 1;
        kernel->_localWorkSize[2] = 1;
    }

    return true;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::setupSmokeSimGridMemory
// Description: Create an OpenCL memory buffer that will be used to store
//              data for an entire 3D grid.
bool AMDTTeapotOCLSmokeSystem::setupSmokeSimGridMemory(
    CalcProgram* program,
    SmokeSimMemObject memId,
    size_t numBytesPerCell,
    cl_mem_flags flags)
{
    bool retVal = true;
    cl_int status;
    CalcMemory* mem = &program->_memObjects[memId];
    mem->_memType = CALCMEM_BUFFER;
    mem->_memSize = numBytesPerCell * _fluidGrid._totalNumCells;
    mem->_clMemory = _clCreateBuffer(
                         program->_calcContext->_clCtx,
                         flags,
                         mem->_memSize,
                         NULL,
                         &status);

    if (status != CL_SUCCESS)
    {
        _lastError.printf("ERROR: Error calling _clCreateBuffer(platform=%s, device=%s): [%d] %s",
                          program->_calcContext->_platform->getName(),
                          program->_calcDevice->_device->getName(),
                          status,
                          _ocl->getOpenCLErrorCodeStr(status));
        retVal = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::recalculateSmokeSimConstants
// Description: Recalculate smoke simulation parameters used by the kernels
//              based on the more human-settable parameters in SimParams.
void AMDTTeapotOCLSmokeSystem::recalculateSmokeSimConstants()
{
    // Maximum time step
    _smokeSimConstants._maxDeltaTimeInSeconds = _simParams._maxDeltaTime;

    // Buoyancy coefficients. These are acceleration coefficients
    // that are used to calculation changes to the velocity field.
    // The coefficients are computed from numbers that define how long
    // it takes to travel 1 meter starting from stationary. Hence
    // we use the equation d = a * t^2 / 2. Here d = 1, thus
    // a = 2 / t^2. The units of the velocity field are in grid units,
    // so we need to convert from meters to grid units.
    //
    // Acceleration due to gravity:
    _smokeSimConstants._buoyAlpha = 2.0f / (_simParams._timeToFall * _simParams._timeToFall * _fluidGrid._spacingMETERS);
    // Acceleration due to temperature difference:
    //     dv = (Temp - Tamb) * _buoyBeta * dt
    //     d = (Temp - Tamb) * _buoyBeta * t^2 / 2
    //     1 = (Tmax - Tamb) * _buoyBeta * t^2
    //     _buoyBeta = 2 / ((Tmax - Tamb) * t^2)
    _smokeSimConstants._buoyBeta = 2.0f / ((_simParams._maxTemperature - _simParams._ambientTemperature) * _simParams._timeToRise * _simParams._timeToRise * _fluidGrid._spacingMETERS);

    // Dissipation. v *= 1 / (1 + dt * Kdr). Assume maximum delta time,
    // let's deduce Kdr that will dissipate by 50% (C) after time T.
    //     [1 / (1 + dt(max) * Kdr)]^(T/dt(max)) = C
    //     1 + dt(max) * Kdr = root(T/dt(max))(1/C)
    //     Kdr = (root(T/dt(max))(1/C) - 1) / dt(max)
    _smokeSimConstants._KdrDens = (powf(2.0f, _simParams._maxDeltaTime / _simParams._timeToDissipateHalfDensity) - 1.0f) / _simParams._maxDeltaTime;
    _smokeSimConstants._KdrTemp = (powf(2.0f, _simParams._maxDeltaTime / _simParams._timeToDissipateHalfTemperature) - 1.0f) / _simParams._maxDeltaTime;

    _smokeSimConstants._ambiantTemperature = _simParams._ambientTemperature;
    _smokeSimConstants._KminTemp = _simParams._minTemperature;
    _smokeSimConstants._KmaxTemp = _simParams._maxTemperature;

    _smokeSimConstants._KminDens = _simParams._minDensity;
    _smokeSimConstants._KmaxDens = _simParams._maxDensity;

    _smokeSimConstants._KsDens = _simParams._inputDensityPerSec;
    _smokeSimConstants._KsTemp = _simParams._inputTemperaturePerSec;

    _smokeSimConstants._vorticity = _simParams._vorticityCoefficient;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::setupSmokeSimulation
// Description: Create all memory objects needed for the smoke simulation,
//              do any GL-CL association setup and setup the kernel args.
bool AMDTTeapotOCLSmokeSystem::setupSmokeSimulation(
    CalcProgram* program)
{
    bool retVal = true;
    cl_int status;

    _logger->setProgress("Creating memory objects for smoke simulation ...");

    const OCLDevice* device = program->_calcDevice->_device;
    const OCLPlatform* platform = device->getPlatform();

    // First make sure this device supports 3 work item dimensions
    if (device->getMaxWorkItemDimensions() < 3)
    {
        _lastError.printf("ERROR: 3D work group dimensions not supported: platform=%s, device=%s.",
                          platform->getName(),
                          device->getName());
        retVal = false;
    }
    else
    {
        glGetError();
        // Setup the OpenGL 3D texture
        glBindTexture(GL_TEXTURE_3D, _texDensity);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // clamp the projective texture/shadow map inside the projections edges
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        _glTexImage3D(
            GL_TEXTURE_3D,
            0,
            _texDensityInternalFormat,
            _fluidGrid._numCellsX,
            _fluidGrid._numCellsY,
            _fluidGrid._numCellsZ,
            0,
            _texDensityFormat,
            _texDensityType,
            NULL);

        GLuint glErr = glGetError();

        if (glErr)
        {
            _lastError.printf("ERROR: Can't create the OpenGL 3D texture: %d", glErr);
            retVal = false;
        }
        else
        {
            // Setup constants
            _dirtyFlags |= DIRTY_SIM_PARAMS;
            // Assume equal spacing of grid in all dimensions. Our velocity field
            // is in grid units which means that spacing=1, hence alpha = -spacing^2
            // becomes -1.
            _smokeSimConstants._KpressureJacobiPoissonAlpha = -1.0f;
            // 4 for 2D, need to use 6 for 3D
            _smokeSimConstants._KpressureJacobiPoissonInvBeta = 1.0f / 6.0f;

            // The grid bottom is centered over the teapot spout.
            _smokeSimConstants._spoutCenter.s[0] = (float)(_fluidGrid._numCellsX >> 1);
            _smokeSimConstants._spoutCenter.s[1] = (float)(_fluidGrid._numCellsY >> 1);
            _smokeSimConstants._spoutInvExtent.s[0] = (64.0f / 5.0f) / (float)(_fluidGrid._numCellsX);
            _smokeSimConstants._spoutInvExtent.s[1] = (64.0f / 3.0f) / (float)(_fluidGrid._numCellsY);
            _smokeSimConstants._spoutCenter.s[1] = (float)(_fluidGrid._numCellsY >> 1);
            _smokeSimConstants._sourceCenter.s[0] = (float)(_fluidGrid._numCellsX >> 1);
            _smokeSimConstants._sourceCenter.s[1] = (float)(_fluidGrid._numCellsY >> 1);
            float varianceSquared = 0.2f;
            _smokeSimConstants._sourceDistributionAlpha = -1.0f / (2.0f * varianceSquared);
            _smokeSimConstants._sourceDistributionBeta = static_cast<cl_float>(1.0f / sqrt(2.0f * (float)M_PI * varianceSquared));

            // Setup the smoke random generator
            rMath::SeedRand();
            _smokeSourceAngularVelocity = 0.0f;
            _smokeSourceRadialVelocity = 0.0f;
            _smokeSourceAngle = 00.0f;
            _smokeSourceRadius = 0.0f;
            _smokeSourceMaxRadius = 1.0f / _smokeSimConstants._spoutInvExtent.s[0];
            _smokeSourceMaxAngularVelocity = 360.0f; // deg/sec
            _smokeSourceMaxRadialVelocity = _smokeSourceMaxRadius * 2.0f; // can move from center to edge of spout in 1/2 sec
            _smokeSourceMaxAngularAccel = _smokeSourceMaxAngularVelocity * 10.0f;
            _smokeSourceMaxRadialAccel = _smokeSourceMaxRadialVelocity * 10.0f;
            _smokeSourceNextUpdateTime = 0.0f;
            _smokeSourceCurrentDuration = 0.0f;
            _smokeSourceHorizontalVelocity = 0.3f * _fluidGrid._invSpacingMETERS; // m/s converted to grid units
            _smokeSourceVerticalVelocity = 0.5f * _fluidGrid._invSpacingMETERS; // m/s converted to grid units

            CalcContext* context = program->_calcContext;
            CalcMemory* mem;

            // If we have requested GL sharing, get a cl_mem object for the
            // GL density texture.
            _oclMemGLDensityTexture = NULL;

            if (context->_withGLSharing)
            {
                // Only share if device is GPU.
                if (device->getDeviceType() == CL_DEVICE_TYPE_GPU)
                {
                    _oclMemGLDensityTexture = _clCreateFromGLTexture3D(context->_clCtx, CL_MEM_WRITE_ONLY, GL_TEXTURE_3D, 0, _texDensity, &status);

                    if (status != CL_SUCCESS)
                    {
                        _lastError.printf("WARNING: Error calling _clCreateFromGLTexture3D(platform=%s, device=%s): [%d] %s",
                                          platform->getName(),
                                          device->getName(),
                                          status,
                                          _ocl->getOpenCLErrorCodeStr(status));
                    }
                }
            }

            bool ok = true;
            // Allocate memory for the density buffer. Since this buffer is what is
            // going to be used to transfer data to GL and if we can't share with GL,
            // then we will ask OpenCL to allocate memory internal so that the
            // driver won't allocate memory every computation cycle when we
            // map this memory object.
            ok &= setupSmokeSimGridMemory(
                      program,
                      MO_SMOKESIM_DENSITY,
                      4 * sizeof(cl_float),
                      CL_MEM_READ_WRITE | (NULL == _oclMemGLDensityTexture ? CL_MEM_ALLOC_HOST_PTR : 0));
            // Allocate memory for the other buffers used excusively by the kernels.
            // Density
            ok &= setupSmokeSimGridMemory(program, MO_SMOKESIM_S03D, sizeof(cl_float));
            // Temperature
            ok &= setupSmokeSimGridMemory(program, MO_SMOKESIM_T03D, sizeof(cl_float));
            // Vector field (velocity)
            ok &= setupSmokeSimGridMemory(program, MO_SMOKESIM_U03D, 4 * sizeof(cl_float));
            ok &= setupSmokeSimGridMemory(program, MO_SMOKESIM_U13D, 4 * sizeof(cl_float));
            // Pressure field
            ok &= setupSmokeSimGridMemory(program, MO_SMOKESIM_P03D, sizeof(cl_float));
            ok &= setupSmokeSimGridMemory(program, MO_SMOKESIM_P13D, sizeof(cl_float));
            ok &= setupSmokeSimGridMemory(program, MO_SMOKESIM_TMPSCALAR, sizeof(cl_float));
            ok &= setupSmokeSimGridMemory(program, MO_SMOKESIM_BOUNDARY_INDEX, sizeof(cl_int));
            ok &= setupSmokeSimGridMemory(program, MO_SMOKESIM_BOUNDARY_SCALE, 4 * sizeof(cl_float));

            if (!ok)
            {
                retVal = false;
            }
            else
            {
                // Allocate memory for passing constants to the kernels
                mem = &program->_memObjects[MO_SMOKESIM_CONSTANTS];
                mem->_memType = CALCMEM_BUFFER;
                mem->_memSize = sizeof(SmokeSimConstants);
                mem->_clMemory = _clCreateBuffer(
                                     context->_clCtx,
                                     CL_MEM_READ_ONLY,
                                     mem->_memSize,
                                     NULL,
                                     &status);

                if (status != CL_SUCCESS)
                {
                    _lastError.printf("ERROR: Error calling _clCreateBuffer(platform=%s, device=%s, MO_SMOKESIM_CONSTANTS): [%d] %s",
                                      platform->getName(),
                                      device->getName(),
                                      status,
                                      _ocl->getOpenCLErrorCodeStr(status));
                    retVal = false;
                }
                else
                {
                    bool setOK_P = true;
                    // Setup arguments and ND range for each kernel.
                    _logger->setProgress("Setup kernel args for smoke simulation ...");

                    for (int i = 0; setOK_P && (i < program->_numKernels); ++i)
                    {
                        CalcKernel* kernel = &program->_kernels[i];

                        if (kernel->_clKernel)
                        {
                            int argNum = 0;
                            int memId;

                            while (setOK_P && ((memId = kernel->_args[argNum]) >= 0))
                            {
                                mem = &program->_memObjects[memId];
                                status = _clSetKernelArg(
                                             kernel->_clKernel,
                                             argNum,
                                             sizeof(cl_mem),
                                             (void*)&mem->_clMemory);

                                if (status != CL_SUCCESS)
                                {
                                    _lastError.printf("ERROR: Error calling _clSetKernelArg(platform=%s, device=%s): [%d] %s",
                                                      platform->getName(),
                                                      device->getName(),
                                                      status,
                                                      _ocl->getOpenCLErrorCodeStr(status));
                                    retVal = false;
                                    setOK_P = false;
                                }
                                else
                                {
                                    ++argNum;
                                }
                            }

                            if (setOK_P && getKernelWorkGroupInfo(device, kernel))
                            {
                                if (KERN_SMOKESIM_APPLY_SOURCES == i)
                                {
                                    if (!setup3DKernelWorkGroupSizes(kernel, _fluidGrid._numCellsX, _fluidGrid._numCellsY, 1))
                                    {
                                        retVal = false;
                                    }
                                }
                                else
                                {
                                    if (!setup3DKernelWorkGroupSizes(kernel, _fluidGrid._numCellsX, _fluidGrid._numCellsY, _fluidGrid._numCellsZ))
                                    {
                                        retVal = false;
                                    }
                                }
                            }
                            else
                            {
                                retVal = false;
                            }
                        }
                    }

                    _dirtyFlags |= DIRTY_RESET_SMOKESIM;
                }
            }
        }
    }

    _logger->setProgress(retVal ? "OK" : _lastError._str);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::releaseSmokeSimulation
// Description: Release any additional smoke simulation resources.
bool AMDTTeapotOCLSmokeSystem::releaseSmokeSimulation()
{
    bool retVal = true;
    cl_int status;

    // If we created a memory object associated with the GL texture,
    // free the association.
    if (_oclMemGLDensityTexture)
    {
        status = _clReleaseMemObject(_oclMemGLDensityTexture);

        if (status != CL_SUCCESS)
        {
            retVal = false;
        }
        else
        {
            _oclMemGLDensityTexture = NULL;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::setupVolumeSlicing
// Description: Create all memory objects needed for the volume slicing simulation,
//              do any GL-CL association setup and setup the kernel args.
bool AMDTTeapotOCLSmokeSystem::setupVolumeSlicing(
    CalcProgram* program)
{
    bool ok = true;
    cl_int status = 0;

    _logger->setProgress("Creating memory objects for volume slicing ...");

    const OCLDevice* device = program->_calcDevice->_device;
    const OCLPlatform* platform = device->getPlatform();
    CalcContext* context = program->_calcContext;
    CalcMemory* mem;
    bool tryGLCLSharing = (context->_withGLSharing && (device->getDeviceType() == CL_DEVICE_TYPE_GPU));

    // First let's create the OpenGL VBO that will be used to hold the volume slice vertices.
    // Each slice can have up to 6 vertices. Each vertex has both coordinate
    // and texture coordinate (6 floats). Maximum number of slices is the
    // cube diagonal.
    //int vboNumFloats = _fluidGrid._maxSlices * 6 * 6;
    int vboNumFloats = _fluidGrid._maxSlices * 72; // 4 * 3 * 6
    int vboSize = vboNumFloats * sizeof(GLfloat);

    // See bug 6620 - if VBO is < 2MB, GL-CL sharing doesn't work as expected.
    // So force the vbo size to be at least 2MB even if don't use all of it.
    if (tryGLCLSharing && (vboSize < 2 * 1024 * 1024))
    {
        vboSize = 2 * 1024 * 1024;
    }

    if (_isOpenGL1_5Supported)
    {
        _glBindBuffer(GL_ARRAY_BUFFER, _volSliceVBO);
        _glBufferData(GL_ARRAY_BUFFER, vboSize, NULL, GL_DYNAMIC_DRAW);
        _glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else  if (_isGL_ARB_vertex_buffer_objectSupported)
    {
        _glBindBufferARB(GL_ARRAY_BUFFER_ARB, _volSliceVBO);
        _glBufferDataARB(GL_ARRAY_BUFFER_ARB, vboSize, NULL, GL_DYNAMIC_DRAW_ARB);
        _glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }
    else
    {
        _lastError.print("ERROR: No OpenGL support for VBOs");
        ok = false;
    }

    if (ok)
    {
        // Copy all the grid constants into the OpenCL structures that will be
        // transfered to the device in memory objects. We don't use memcpy because
        // we are copying from host primitives (e.g. float) to OpenCL primitives
        // (e.g. cl_float).
        for (int i = sizeof(_fluidGrid.nSequence) / sizeof(_fluidGrid.nSequence[0]) - 1; i >= 0; --i)
        {
            _volSliceConstants._nSequence[i] = _fluidGrid.nSequence[i];
        }

        for (int i = sizeof(_fluidGrid.v1) / sizeof(_fluidGrid.v1[0]) - 1; i >= 0; --i)
        {
            _volSliceConstants._v1[i] = _fluidGrid.v1[i];
        }

        for (int i = sizeof(_fluidGrid.v2) / sizeof(_fluidGrid.v2[0]) - 1; i >= 0; --i)
        {
            _volSliceConstants._v2[i] = _fluidGrid.v2[i];
        }

        float v[8 * 3];
        _fluidGrid.GetVertsf(v);

        for (int i = 0; i < 8; ++i)
        {
            int j = i * 3;
            _volSliceParams._verts[i].s[0] = v[j + 0];
            _volSliceParams._verts[i].s[1] = v[j + 1];
            _volSliceParams._verts[i].s[2] = v[j + 2];
        }

        // If we have requested GL sharing, get a cl_mem object for the
        // GL VBO that will be used to share the vertex buffer.
        _oclMemGLVBO = NULL;

        if (tryGLCLSharing)
        {
            _oclMemGLVBO = _clCreateFromGLBuffer(
                               context->_clCtx,
                               CL_MEM_WRITE_ONLY,
                               _volSliceVBO,
                               &status);

            if (status != CL_SUCCESS)
            {
                _lastError.printf("WARNING: Error calling _clCreateFromGLBuffer(platform=%s, device=%s): [%d] %s",
                                  platform->getName(),
                                  device->getName(),
                                  status,
                                  _ocl->getOpenCLErrorCodeStr(status));
            }
        }

        // Allocate memory for the vertex buffer. Since this buffer is what is
        // going to be used to transfer data to GL and if we can't share with GL,
        // then we will ask OpenCL to allocate memory internal so that the
        // driver won't allocate memory every computation cycle when we
        // map this memory object.
        mem = &program->_memObjects[MO_VOLSLICE_VERTICES];
        mem->_memType = CALCMEM_BUFFER;
        // Each slice can have up to 6 vertices. Each vertex has both coordinate
        // and texture coordinate (6 floats). Maximum number of slices is the
        // cube diagonal.
        mem->_memSize = vboNumFloats * sizeof(cl_float);

        if (_oclMemGLVBO)
        {
            // This will also ensure that the memory is released by the
            // standard memory release loop.
            mem->_clMemory = _oclMemGLVBO;
        }
        else
        {
            // Allocate memory if we don't have GL sharing.
            mem->_clMemory = _clCreateBuffer(
                                 context->_clCtx,
                                 CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                                 mem->_memSize,
                                 NULL,
                                 &status);

            if (status != CL_SUCCESS)
            {
                _lastError.printf("ERROR: Error calling _clCreateBuffer(platform=%s, device=%s, MO_VOLSLICE_VERTICES): [%d] %s",
                                  platform->getName(),
                                  device->getName(),
                                  status,
                                  _ocl->getOpenCLErrorCodeStr(status));
                ok = false;
            }
        }

        if (ok)
        {
            // Allocate memory for passing constants to the kernels
            mem = &program->_memObjects[MO_VOLSLICE_CONSTANTS];
            mem->_memType = CALCMEM_BUFFER;
            mem->_memSize = sizeof(_volSliceConstants);
            mem->_clMemory = _clCreateBuffer(
                                 context->_clCtx,
                                 CL_MEM_READ_ONLY,
                                 mem->_memSize,
                                 NULL,
                                 &status);

            if (status != CL_SUCCESS)
            {
                _lastError.printf("ERROR: Error calling _clCreateBuffer(platform=%s, device=%s, MO_VOLSLICE_CONSTANTS): [%d] %s",
                                  platform->getName(),
                                  device->getName(),
                                  status,
                                  _ocl->getOpenCLErrorCodeStr(status));
                ok = false;
            }
            else
            {
                // Allocate memory for passing constants to the kernels
                mem = &program->_memObjects[MO_VOLSLICE_PARAMS];
                mem->_memType = CALCMEM_BUFFER;
                mem->_memSize = sizeof(_volSliceParams);
                mem->_clMemory = _clCreateBuffer(
                                     context->_clCtx,
                                     CL_MEM_READ_ONLY,
                                     mem->_memSize,
                                     NULL,
                                     &status);

                if (status != CL_SUCCESS)
                {
                    _lastError.printf("ERROR: Error calling _clCreateBuffer(platform=%s, device=%s, MO_VOLSLICE_PARAMS): [%d] %s",
                                      platform->getName(),
                                      device->getName(),
                                      status,
                                      _ocl->getOpenCLErrorCodeStr(status));
                    ok = false;
                }
                else
                {
                    // Setup arguments and ND range for each kernel.
                    _logger->setProgress("Setup kernel args for volume slicing ...");

                    for (int i = 0; ok && (i < program->_numKernels); ++i)
                    {
                        CalcKernel* kernel = &program->_kernels[i];

                        if (kernel->_clKernel)
                        {
                            int argNum = 0;
                            int memId;

                            while (ok && ((memId = kernel->_args[argNum]) >= 0))
                            {
                                mem = &program->_memObjects[memId];
                                status = _clSetKernelArg(
                                             kernel->_clKernel,
                                             argNum,
                                             sizeof(cl_mem),
                                             (void*)&mem->_clMemory);

                                if (status != CL_SUCCESS)
                                {
                                    _lastError.printf("ERROR: Error calling _clSetKernelArg(platform=%s, device=%s): [%d] %s",
                                                      platform->getName(),
                                                      device->getName(),
                                                      status,
                                                      _ocl->getOpenCLErrorCodeStr(status));
                                    ok = false;
                                    break;
                                }
                                else
                                {
                                    ++argNum;
                                }
                            }

                            if (ok && !(getKernelWorkGroupInfo(device, kernel)))
                            {
                                ok = false;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (ok)
        {
            _dirtyFlags |= DIRTY_RESET_VOLSLICE;
        }
    }

    return ok;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::releaseSmokeSimulation
// Description: Release any additional volume slicing resources.
bool AMDTTeapotOCLSmokeSystem::releaseVolumeSlicing()
{
    _oclMemGLVBO = NULL;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::setupDensityGrid
// Description: Recalculate all grid parameters based on new dimensions and
//              allocate a memory buffer that will be used to setup device
//              memory buffers with initial values.
bool AMDTTeapotOCLSmokeSystem::setupDensityGrid(
    int numCellsX,
    int numCellsY,
    int numCellsZ,
    float spacingMETERS)
{
    _fluidGrid.Setup(numCellsX, numCellsY, numCellsZ, spacingMETERS);

    delete _gridBuffer;
    _gridBuffer = new char[(4 * sizeof(cl_float) + sizeof(cl_int)) * _fluidGrid._totalNumCells];

    return true;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::getDeviceInfo
// Description: Public method that enables the outside to get a hold of the
//              list of OpenCL platforms/devices and which devices are
//              currently running the different parts of the simulation.
bool AMDTTeapotOCLSmokeSystem::getDeviceInfo(
    const OCLInfo** out_oclInfo,
    const OCLDevice** out_smokeSimulation,
    const OCLDevice** out_volumeSlicing)
{
    *out_oclInfo = _ocl->getOpenCLInfo();
    *out_smokeSimulation = _changeSmokeSimDevice;
    *out_volumeSlicing = _changeVolSliceDevice;
    return ((*out_oclInfo != NULL) && (*out_smokeSimulation != NULL) && (*out_volumeSlicing != NULL));
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::usingGLCLSharing
// Description: Find out if the current simulation is using GL-CL sharing.
bool AMDTTeapotOCLSmokeSystem::usingGLCLSharing()
{
    return (NULL != _oclMemGLDensityTexture);
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::processSmokeSystemCommand
// Description: Public method that enables the outside to request changes to
//              the simulation - grid size, devices that run the simulation etc.
//              The changes will be applied the next time draw() is called.
void AMDTTeapotOCLSmokeSystem::processSmokeSystemCommand(
    SmokeSystemCommand* cmd)
{
    switch (cmd->getType())
    {
        case SSCMD_SELECT:
        {
            switch (cmd->getId())
            {
                case SSCID_RESET:
                    _dirtyFlags |= (DIRTY_RESET_SMOKESIM | DIRTY_RESET_VOLSLICE);
                    break;

                default:
                    break;
            }
        }
        break;

        case SSCMD_TOGGLE:
        {
            SmokeSystemCommandToggle* toggle = static_cast<SmokeSystemCommandToggle*>(cmd);

            switch (cmd->getId())
            {
                case SSCID_ENABLE:
                    _enabled = toggle->_checked;
                    break;

                case SSCID_USE_GLCL_SHARING:
                    if (toggle->_checked)
                    {
                        if (!(_changeExecOptions & EXEC_WITH_GLSHARING))
                        {
                            _changeExecOptions |= EXEC_WITH_GLSHARING;
                            _dirtyFlags |= DIRTY_EXEC_OPTIONS;
                            setInitCompileFlag(INIT_COMPILE_START);
                        }
                    }
                    else
                    {
                        if (_changeExecOptions & EXEC_WITH_GLSHARING)
                        {
                            _changeExecOptions &= (~EXEC_WITH_GLSHARING);
                            _dirtyFlags |= DIRTY_EXEC_OPTIONS;
                            setInitCompileFlag(INIT_COMPILE_START);
                        }
                    }

                    break;

                case SSCID_SHOW_GRID:
                    _showGrid = toggle->_checked;

                    break;

                default:
                    break;
            }
        }
        break;

        case SSCMD_GRID_SIZE:
        {
            SmokeSystemCommandGridSize* grid = static_cast<SmokeSystemCommandGridSize*>(cmd);
            setGrid(grid->_x, grid->_y, grid->_z, grid->_spacingMETERS);
            setInitCompileFlag(INIT_COMPILE_START);
        }
        break;

        case SSCMD_CHANGE_DEVICE:
        {
            SmokeSystemCommandChangeDevice* changeDevice = static_cast<SmokeSystemCommandChangeDevice*>(cmd);

            switch (changeDevice->getId())
            {
                case SSCID_CHANGE_SMOKE_SIM_DEVICE:
                    _changeSmokeSimDevice = changeDevice->_device;
                    _dirtyFlags |= DIRTY_DEVICES;
                    setInitCompileFlag(INIT_COMPILE_START);
                    break;

                case SSCID_CHANGE_VOL_SLICE_DEVICE:
                    _changeVolSliceDevice = changeDevice->_device;
                    _dirtyFlags |= DIRTY_DEVICES;
                    setInitCompileFlag(INIT_COMPILE_START);
                    break;

                default:
                    break;
            }
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::getLastError
// Description: Get the last error message that was generated during
//              initialization or the last time the simulation ran a step.
const char* AMDTTeapotOCLSmokeSystem::getLastError()
{
    const char* retVal = NULL;

    if ('\0' != _lastError._str[0])
    {
        retVal = _lastError._str;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::createOpenCLProgramResource
// Description: Create all OpenCL contexts, build all programs, create
//              all kernels, create all memory buffers and set all
//              kernel arguments for the program.
bool AMDTTeapotOCLSmokeSystem::createOpenCLKernelProgramResource(
    CalcProgram* program)
{
    bool ok = true;
    CalcContext* context = program->_calcContext;
    CalcDevice* device = program->_calcDevice;
    cl_int status;
    DynStr msg;
    char str[1024];

    if (program->_calcContext && program->_calcDevice)
    {
        for (int i = 0; ok && (i < program->_numKernels); ++i)
        {
            CalcKernel& kernel = program->_kernels[i];

            if (kernel._clProgram == NULL)
            {
                msg.reset();
                msg.printf("Creating program \"%S\" ...", kernel._sourcePath);
                _logger->setProgress(msg._str);

                const char* source = kernel._source;
                kernel._clProgram = _clCreateProgramWithSource(
                                        program->_calcContext->_clCtx,
                                        1,
                                        &source,
                                        NULL,
                                        &status);

                if (status != CL_SUCCESS)
                {
                    _lastError.printf("ERROR: Error calling _clCreateProgramWithSource(platform=%s, device=%s, file=%S): [%d] %s",
                                      context->_platform->getName(),
                                      device->_device->getName(),
                                      kernel._sourcePath,
                                      status,
                                      _ocl->getOpenCLErrorCodeStr(status));
                    ok = false;
                    break;
                }
            }

            if (ok)
            {
                sprintf(str,
                        "-D GRID_NUM_CELLS_X=%d -D GRID_NUM_CELLS_Y=%d -D GRID_NUM_CELLS_Z=%d"
                        " -D GRID_INV_SPACING=%ff -D GRID_SPACING=%ff"
                        " -D GRID_SHIFT_X=%d -D GRID_SHIFT_Y=%d -D GRID_SHIFT_Z=%d"
                        " -D GRID_STRIDE_Y=%d -D GRID_STRIDE_SHIFT_Y=%d"
                        " -D GRID_STRIDE_Z=%d -D GRID_STRIDE_SHIFT_Z=%d"
                        " -I res",
                        _fluidGrid._numCellsX,
                        _fluidGrid._numCellsY,
                        _fluidGrid._numCellsZ,
                        1.0f,
                        1.0f,
                        _fluidGrid._shiftX,
                        _fluidGrid._shiftY,
                        _fluidGrid._shiftZ,
                        _fluidGrid._strideY,
                        _fluidGrid._strideShiftY,
                        _fluidGrid._strideZ,
                        _fluidGrid._strideShiftZ
                       );
                const cl_device_id devid = program->_calcDevice->_device->getDeviceID();

                msg.reset();
                msg.printf("Building program \"%S\" ...", kernel._sourcePath);
                _logger->setProgress(msg._str);

                status = _clBuildProgram(
                             kernel._clProgram,
                             1,
                             &devid,
                             str,
                             NULL,
                             NULL);

                char* log = NULL;
                size_t logSize;

                if (_clGetProgramBuildInfo(
                        kernel._clProgram,
                        devid,
                        CL_PROGRAM_BUILD_LOG,
                        0,
                        NULL,
                        &logSize) == CL_SUCCESS)
                {
                    log = new char[logSize];

                    if (_clGetProgramBuildInfo(
                            kernel._clProgram,
                            devid,
                            CL_PROGRAM_BUILD_LOG,
                            logSize,
                            log,
                            NULL) != CL_SUCCESS)
                    {
                        delete[] log;
                        log = NULL;
                    }
                }

                if (status != CL_SUCCESS)
                {
                    _lastError.printf("ERROR: Error calling _clBuildProgram(platform=%s, device=%s, file=%S): [%d] %s",
                                      context->_platform->getName(),
                                      device->_device->getName(),
                                      kernel._sourcePath,
                                      status,
                                      _ocl->getOpenCLErrorCodeStr(status));

                    if (NULL != log)
                    {
                        _lastError.print(log);
                        delete[] log;
                    }

                    ok = false;
                }
                else
                {
                    delete[] log;

                    // Create all the kernels that are in this program
                    msg.reset();
                    msg.printf("Creating kernels for program \"%S\" ...", program->_sourcePath);
                    _logger->setProgress(msg._str);

                    if (NULL != kernel._kernelFuncName && kernel._clKernel == NULL)
                    {
                        kernel._clKernel = _clCreateKernel(
                                               kernel._clProgram, kernel._kernelFuncName, &status);

                        if (status != CL_SUCCESS)
                        {
                            _lastError.printf("ERROR: Error calling _clCreateKernel(platform=%s, device=%s, file=%S, kernel=%s): [%d] %s",
                                              context->_platform->getName(),
                                              device->_device->getName(),
                                              kernel._sourcePath,
                                              kernel._kernelFuncName,
                                              status,
                                              _ocl->getOpenCLErrorCodeStr(status));
                            ok = false;
                        }
                        else
                        {
                            //The program has to contain additional kernel
                            if (kernel._repeatKernelCount > 0)
                            {
                                CalcKernel& kernelRep = program->_kernels[kernel._repeatKernelIndex];
                                kernelRep._clKernel = _clCreateKernel(
                                                          kernelRep._clProgram, kernelRep._kernelFuncName, &status);

                                if (status != CL_SUCCESS)
                                {
                                    _lastError.printf("ERROR: Error calling _clCreateKernel(platform=%s, device=%s, file=%S, kernel=%s): [%d] %s",
                                                      context->_platform->getName(),
                                                      device->_device->getName(),
                                                      kernelRep._sourcePath,
                                                      kernelRep._kernelFuncName,
                                                      status,
                                                      _ocl->getOpenCLErrorCodeStr(status));
                                    ok = false;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return ok;
}
// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::createOpenCLProgramResource
// Description: Create all OpenCL contexts, build all programs, create
//              all kernels, create all memory buffers and set all
//              kernel arguments for the program.
bool AMDTTeapotOCLSmokeSystem::createOpenCLProgramResource(
    CalcProgram* program)
{
    bool retVal = true;

    CalcContext* context = program->_calcContext;
    CalcDevice* device = program->_calcDevice;
    cl_int status;
    DynStr msg;
    char str[1024];

    if (program->_source && program->_calcContext && program->_calcDevice)
    {
        if (program->_clProgram == NULL)
        {
            msg.reset();
            msg.printf("Creating program \"%S\" ...", program->_sourcePath);
            _logger->setProgress(msg._str);

            const char* source = program->_source;
            program->_clProgram = _clCreateProgramWithSource(
                                      program->_calcContext->_clCtx,
                                      1,
                                      &source,
                                      NULL,
                                      &status);

            if (status != CL_SUCCESS)
            {
                _lastError.printf("ERROR: Error calling _clCreateProgramWithSource(platform=%s, device=%s, file=%S): [%d] %s",
                                  context->_platform->getName(),
                                  device->_device->getName(),
                                  program->_sourcePath,
                                  status,
                                  _ocl->getOpenCLErrorCodeStr(status));
                retVal = false;
            }
        }
        else
        {
            sprintf(str,
                    "-D GRID_NUM_CELLS_X=%d -D GRID_NUM_CELLS_Y=%d -D GRID_NUM_CELLS_Z=%d"
                    " -D GRID_INV_SPACING=%ff -D GRID_SPACING=%ff"
                    " -D GRID_SHIFT_X=%d -D GRID_SHIFT_Y=%d -D GRID_SHIFT_Z=%d"
                    " -D GRID_STRIDE_Y=%d -D GRID_STRIDE_SHIFT_Y=%d"
                    " -D GRID_STRIDE_Z=%d -D GRID_STRIDE_SHIFT_Z=%d",
                    _fluidGrid._numCellsX,
                    _fluidGrid._numCellsY,
                    _fluidGrid._numCellsZ,
                    1.0f,
                    1.0f,
                    _fluidGrid._shiftX,
                    _fluidGrid._shiftY,
                    _fluidGrid._shiftZ,
                    _fluidGrid._strideY,
                    _fluidGrid._strideShiftY,
                    _fluidGrid._strideZ,
                    _fluidGrid._strideShiftZ
                   );
            const cl_device_id devid = program->_calcDevice->_device->getDeviceID();

            msg.reset();
            msg.printf("Building program \"%S\" ...", program->_sourcePath);
            _logger->setProgress(msg._str);

            status = _clBuildProgram(
                         program->_clProgram,
                         1,
                         &devid,
                         str,
                         NULL,
                         NULL);

            char* log = NULL;
            size_t logSize;

            if (_clGetProgramBuildInfo(
                    program->_clProgram,
                    devid,
                    CL_PROGRAM_BUILD_LOG,
                    0,
                    NULL,
                    &logSize) == CL_SUCCESS)
            {
                log = new char[logSize];

                if (_clGetProgramBuildInfo(
                        program->_clProgram,
                        devid,
                        CL_PROGRAM_BUILD_LOG,
                        logSize,
                        log,
                        NULL) != CL_SUCCESS)
                {
                    delete[] log;
                    log = NULL;
                }
            }

            if (status != CL_SUCCESS)
            {
                _lastError.printf("ERROR: Error calling _clBuildProgram(platform=%s, device=%s, file=%S): [%d] %s",
                                  context->_platform->getName(),
                                  device->_device->getName(),
                                  program->_sourcePath,
                                  status,
                                  _ocl->getOpenCLErrorCodeStr(status));

                if (NULL != log)
                {
                    _lastError.print(log);
                    delete[] log;
                }

                retVal = false;
            }
            else
            {
                delete[] log;
                //Create all the kernels that are in this program
                msg.reset();
                msg.printf("Creating kernels for program \"%S\" ...", program->_sourcePath);
                _logger->setProgress(msg._str);

                for (int i = 0; i < program->_numKernels; ++i)
                {
                    CalcKernel& kernel = program->_kernels[i];

                    if (NULL != kernel._kernelFuncName && kernel._clKernel == NULL)
                    {
                        kernel._clKernel = _clCreateKernel(
                                               program->_clProgram, kernel._kernelFuncName, &status);

                        if (status != CL_SUCCESS)
                        {
                            _lastError.printf("ERROR: Error calling _clCreateKernel(platform=%s, device=%s, file=%S, kernel=%s): [%d] %s",
                                              context->_platform->getName(),
                                              device->_device->getName(),
                                              program->_sourcePath,
                                              kernel._kernelFuncName,
                                              status,
                                              _ocl->getOpenCLErrorCodeStr(status));
                            retVal = false;
                            break;
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOCLSmokeSystem::recalculateKernelParam
// Description: Recalculate the new parameters for the kernels
bool AMDTTeapotOCLSmokeSystem::recalculateKernelParam(
    const AMDTTeapotRenderState& state,
    const Mat4& modelTransformation,
    float deltaRot)
{
    bool retVal = true;

    // If there were any changes to the fluid parameters, recalculate
    // the new parameters for the kernels.
    if (_dirtyFlags & DIRTY_SIM_PARAMS)
    {
        recalculateSmokeSimConstants();
        _dirtyFlags &= (~DIRTY_SIM_PARAMS);
    }

    // Compute the time since the last run (in seconds).
    float deltaTime = (static_cast<float>(_timer.restart()));

    // Carry out one step of the simulation, limitting maximum delta time to 20ms.
    if (compute(state, modelTransformation, deltaTime < _simParams._maxDeltaTime ? deltaTime : _simParams._maxDeltaTime, deltaRot))
    {
        // Render the smoke density.
        render(state, modelTransformation);
    }
    else
    {
        retVal = false;
    }

    return retVal;
}
