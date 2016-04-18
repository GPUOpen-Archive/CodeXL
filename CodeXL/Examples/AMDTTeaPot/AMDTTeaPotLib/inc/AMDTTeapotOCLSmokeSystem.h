//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTTeapotOCLSmokeSystem.h
///
//==================================================================================

//------------------------------ AMDTTeapotOCLSmokeSystem.h ------------------------------

#ifndef __AMDTTEAPOTOCLSMOKESYSTEM_H
#define __AMDTTEAPOTOCLSMOKESYSTEM_H

// Forward decelerations:
#include <stdio.h>
#include <stdarg.h>
#if defined(__linux__)
    #include <string.h>
    #include "AMDTMisc.h"
#endif


#include <inc/AMDTOpenGLHelper.h>
#include <inc/AMDTOpenCLHelper.h>
#include <inc/AMDTTimer.h>
#include <inc/AMDTOpenGLMath.h>
#include <inc/AMDTTeapotRenderState.h>
#include <inc/AMDTFluidGrid.h>

// Local definitions
typedef void (*oglProcedureAddress)();

/******************************************************************************
*
* SmokeSystemCommand*
* -------------------
* The smoke simulation is encapsulated in the AMDTTeapotOCLSmokeSystem class
* (see below for details). Parameters like grid size and OpenCL devices
* can be changed while the simulation is running. These change requests
* are made by calling AMDTTeapotOCLSmokeSystem::processSmokeSystemCommand()
* passing in one of the subclasses of class SmokeSystemCommand:
*
*     SmokeSystemCommandToggle       - Toggle one of the flags like CL-GL
*                                      sharing.
*     SmokeSystemCommandSelect       - Request something like grid density
*                                      reset.
*     SmokeSystemCommandGridSize     - Change the grid dimensions.
*     SmokeSystemCommandChangeDevice - Change the OpenCL device.
*
******************************************************************************/
enum SmokeSystemCommandType
{
    SSCMD_TOGGLE,
    SSCMD_SELECT,
    SSCMD_GRID_SIZE,
    SSCMD_CHANGE_DEVICE
};

enum SmokeSystemCommandId
{
    SSCID_ENABLE,
    SSCID_USE_GLCL_SHARING,
    SSCID_SHOW_GRID,
    SSCID_RESET,
    SSCID_GRID_SIZE,
    SSCID_CHANGE_SMOKE_SIM_DEVICE,
    SSCID_CHANGE_VOL_SLICE_DEVICE
};

class SmokeSystemCommand
{
public:
    SmokeSystemCommand(SmokeSystemCommandType type, SmokeSystemCommandId id)
        : _type(type), _id(id)
    {
    }

    virtual ~SmokeSystemCommand()
    {
    }

    virtual SmokeSystemCommandId getId()
    {
        return _id;
    }

    virtual SmokeSystemCommandType getType()
    {
        return _type;
    }

protected:
    SmokeSystemCommand();

    SmokeSystemCommandType  _type;
    SmokeSystemCommandId    _id;
};

class SmokeSystemCommandToggle : public SmokeSystemCommand
{
public:
    SmokeSystemCommandToggle(SmokeSystemCommandId id, bool checked)
        : SmokeSystemCommand(SSCMD_TOGGLE, id), _checked(checked)
    {
    }

    bool        _checked;

protected:
    SmokeSystemCommandToggle();
};

class SmokeSystemCommandSelect : public SmokeSystemCommand
{
public:
    SmokeSystemCommandSelect(SmokeSystemCommandId id)
        : SmokeSystemCommand(SSCMD_SELECT, id)
    {
    }

protected:
    SmokeSystemCommandSelect();
};

class SmokeSystemCommandGridSize : public SmokeSystemCommand
{
public:
    SmokeSystemCommandGridSize(SmokeSystemCommandId id, int x, int y, int z, float spacingMETERS)
        : SmokeSystemCommand(SSCMD_GRID_SIZE, id), _x(x), _y(y), _z(z), _spacingMETERS(spacingMETERS)
    {
    }

    int _x;
    int _y;
    int _z;
    float _spacingMETERS;

protected:
    SmokeSystemCommandGridSize();
};

class SmokeSystemCommandChangeDevice : public SmokeSystemCommand
{
public:
    SmokeSystemCommandChangeDevice(SmokeSystemCommandId id, const OCLDevice* device)
        : SmokeSystemCommand(SSCMD_CHANGE_DEVICE, id), _device(device)
    {
    }

    const OCLDevice* _device;

protected:
    SmokeSystemCommandChangeDevice();
};

/******************************************************************************
*
* ISmokeSystemLogger
* ------------------
* The class that creates an instance of AMDTTeapotOCLSmokeSystem must provide
* an implementation of this interface. It enable the smoke system to report
* progress.
*
******************************************************************************/
class ISmokeSystemLogger
{
public:
    virtual void setProgress(const char* msg) = 0;
};

/******************************************************************************
*
* AMDTTeapotOCLSmokeSystem
* ----------------------
* This is the main class for adding OpenCL smoke simulation to an application.
* It assumes that the OpenGL context has already been created. It assumes that
* the OpenCL shared library can be loaded dynamically - if the library is not
* found, then the simulation will not work. Finally, it assumes that the
* two OpenCL programs source files (tpSmokeSimulation.cl and
* tpVolumeSlicing.cl) are located in the direction "src" in the directory
* where the application is run from.
*
* Essentially, you create an instance of this class, initialize it and then
* call draw() for each time step. The simulation will run in real-time, unless
* draw() is called slower than 20Hz, in which case the maximum time-step is
* held at 50ms and the simulation will appear to run slowly.
*
* By default, the simulation is performed on a density grid 64x64x64. During
* runtime, it is possible to request changes to the grid dimensions and
* changes to the OpenCL devices that will perform the simulation.
*
* The simulation is broken into two pieces. The main work is the Navier-Stokes
* solver (tpSmokeSimulation.cl) and this is best computed on a GPU. The output
* of this stage is an OpenGL 3D texture where the alpha value is set according
* to smoke density. While the smoke simulation is being performed, the volume
* slices (tpVolumeSlicing.cl) are calculated. THe output is a VBO that contains
* triangle vertices that define slices through the grid in the OpenGL model
* space (where the density grid should be drawn). These slices are organized
* back to front (based on viewing position) and the texture coordinates map
* into the 3D texture. Finally glDrawArrays() is called to draw the density
* grid at the end of the time-step.
*
* By default, GL-CL sharing will be used to get maximum performance. This can
* be enabled/disabled at runtime.
*
******************************************************************************/
class AMDTTeapotOCLSmokeSystem
{
public:
    AMDTTeapotOCLSmokeSystem(ISmokeSystemLogger* logger);
    virtual ~AMDTTeapotOCLSmokeSystem();

    // Initialize the system. This means loading OpenGL and OpenCL, mapping
    // function pointers and creating the OpenGL texture and VBO object.
    // Returns false if there is any failure, in which case getLastError()
    // can be called to get a textural representation of the error.
    bool initialize();

    // Shutdown the simulation, freeing any resources.
    void shutdown();

    // Get the list of OpenCL platforms/devices
    bool getDeviceInfo(const OCLInfo** out_oclInfo, const OCLDevice** out_smokeSimulation, const OCLDevice** out_volumeSlicing);

    // Get the context info
    int getContextInfo(cl_context ctx, cl_context_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret)
    {
        return _clGetContextInfo(ctx, param_name, param_value_size, param_value, param_value_size_ret);
    }

    //DrawInitRes
    enum DrawInitRes
    {
        DRAW_INIT_OK =  0,
        DRAW_INIT_FAIL,
        DRAW_INIT_PROC,
        DRAW_INIT_NONE
    };

    // Run one init cycle of simulation and draw
    // DrawInitRes drawInit(const AMDTTeapotRenderState& state, const Mat4& modelTransformation, float deltaRot);

    // Run one cycle of simulation and draw
    bool draw(const AMDTTeapotRenderState& state, const Mat4& modelTransformation, float deltaRot);

    // Get _logger
    ISmokeSystemLogger* getLogger()
    {
        return _logger;
    }

    bool recalculateKernelParam(const AMDTTeapotRenderState& state, const Mat4& modelTransformation, float deltaRot);

    // Set _enabled flag
    void setEnabled(bool enabled)
    {
        _enabled = enabled;
    }

    // Get _enabled flag
    bool getEnabled()
    {
        return _enabled;
    }

    // IntitCompile Flag
    enum InitCompile
    {
        INIT_COMPILE_START  = 0,
        INIT_COMPILE_PROC,
        INIT_COMPILE_DONE
    };

    // Set _initCompileFlag
    void setInitCompileFlag(InitCompile initCompileFlag)
    {
        _initCompileFlag = initCompileFlag;
    }

    // Get _initCompileFlag
    InitCompile getInitCompileFlag()
    {
        return _initCompileFlag;
    }

    // DirtyFlags
    enum DirtyFlags
    {
        DIRTY_GRID_DIMENSIONS       = (1 << 0),
        DIRTY_SIM_PARAMS            = (1 << 1),
        DIRTY_DEVICES               = (1 << 2),
        DIRTY_RESET_SMOKESIM        = (1 << 3),
        DIRTY_RESET_VOLSLICE        = (1 << 4),
        DIRTY_EXEC_OPTIONS          = (1 << 5)
    };
    unsigned int getDirtyFlags()
    {
        return _dirtyFlags;
    }


    // This structure is used to setup the fluid simulation parameters.
    // At the moment, there is no public interface to change the values
    // runtime, but it would be easy to do so.
    struct SimParams
    {
        // Maximum delta-time. If time step is bigger, it will be clamped to
        // this value.
        float       _maxDeltaTime;

        // Temperature limits
        float       _minTemperature;
        float       _maxTemperature;
        float       _ambientTemperature;

        // Density limits
        float       _minDensity;
        float       _maxDensity;

        // How long it takes for 1.0 density of stationary particles to
        // fall one meter due to gravity.
        float       _timeToFall;

        // Time it takes for 1.0 density of stationary particles to rise
        // one meter at maxiumum temperature.
        float       _timeToRise;

        // Time to dissipate 50% of density field when sim runs at maximum
        // delta time. If sim runs faster, it will take a bit longer to
        // dissipate.
        float       _timeToDissipateHalfDensity;

        // Time to dissipate 50% of temperature field when sim runs at
        // maximum delta time. If sim runs faster, it will take a bit
        // longer to dissipate.
        float       _timeToDissipateHalfTemperature;

        // Vorticity coefficient
        float       _vorticityCoefficient;

        // Input density / sec / cell
        float       _inputDensityPerSec;

        // Input temperature / sec / cell
        float       _inputTemperaturePerSec;
    };

    // Process commands that change the behavior of the smoke system.
    // Use this to make runtime changes to the simulation by passing
    // in the appropriate subclass of SmokeSystemCommand (see class
    // definitions above).
    void processSmokeSystemCommand(SmokeSystemCommand* cmd);

    // Find out if smoke simulation is using GL-CL sharing
    bool usingGLCLSharing();

    // Find out if smoke simulation is active
    bool usingCL() { return _enabled && canUseCL(); };

    // Find out if smoke simulation is ready
    bool canUseCL() { return 0 != (_initStage & INIT_STAGE_CREATE_RESOURCES); };

    // This will perform setup of the Smoke Simulation program for _programs[CALCID_SMOKE_SIMULATION]-
    // create all the memoryobjects and make all the _clSetKernelArg() calls. GL-CL sharing is attempted
    // in this function.
    bool setupSmokeSimulation()
    {
        return setupSmokeSimulation(_programs[CALCID_SMOKE_SIMULATION]);
    }

    // This will perform setup of the Volume Slicing program for _programs[CALCID_VOLUME_SLICING]-
    // create all the memory objects and make all the _clSetKernelArg() calls. GL-CL sharing is attempted
    // in this function.
    bool setupVolumeSlicing()
    {
        return setupVolumeSlicing(_programs[CALCID_VOLUME_SLICING]);
    }

    // Return the last encountered error
    const char* getLastError();

private:
    // Helper instances
    AMDTOpenCLHelper* _ocl;
    AMDTOpenGLHelper* _ogl;

    // Flags to keep track of library initialization.
    bool _initialized;
    enum SmokeInitStage
    {
        INIT_STAGE_OPENGL           = (1 << 0),
        INIT_STAGE_OPENCL           = (1 << 1),
        INIT_STAGE_OPENCL_SETUP     = (1 << 2),
        INIT_STAGE_CREATE_RESOURCES = (1 << 3)
    };
    unsigned int _initStage;

    // OpenCL devices currently performing the different parts
    // of the simulation.
    const OCLDevice* _selectedSmokeSimDevice;
    const OCLDevice* _selectedVolSliceDevice;

    // OpenCL function pointers
    clCreateContext_fn                  _clCreateContext;
    clReleaseContext_fn                 _clReleaseContext;
    clCreateCommandQueue_fn             _clCreateCommandQueue;
    clReleaseCommandQueue_fn            _clReleaseCommandQueue;
    clCreateProgramWithSource_fn        _clCreateProgramWithSource;
    clReleaseProgram_fn                 _clReleaseProgram;
    clBuildProgram_fn                   _clBuildProgram;
    clUnloadCompiler_fn                 _clUnloadCompiler;
    clGetProgramInfo_fn                 _clGetProgramInfo;
    clGetProgramBuildInfo_fn            _clGetProgramBuildInfo;
    clCreateKernel_fn                   _clCreateKernel;
    clReleaseKernel_fn                  _clReleaseKernel;
    clSetKernelArg_fn                   _clSetKernelArg;
    clEnqueueNDRangeKernel_fn           _clEnqueueNDRangeKernel;
    clEnqueueCopyBufferToImage_fn       _clEnqueueCopyBufferToImage;
    clEnqueueMapBuffer_fn               _clEnqueueMapBuffer;
    clEnqueueUnmapMemObject_fn          _clEnqueueUnmapMemObject;
    clGetSupportedImageFormats_fn       _clGetSupportedImageFormats;
    clCreateBuffer_fn                   _clCreateBuffer;
    clCreateImage2D_fn                  _clCreateImage2D;
    clCreateImage3D_fn                  _clCreateImage3D;
    clReleaseMemObject_fn               _clReleaseMemObject;
    clCreateFromGLBuffer_fn             _clCreateFromGLBuffer;
    clCreateFromGLTexture3D_fn          _clCreateFromGLTexture3D;
    clGetKernelWorkGroupInfo_fn         _clGetKernelWorkGroupInfo;
    clWaitForEvents_fn                  _clWaitForEvents;
    clReleaseEvent_fn                   _clReleaseEvent;
    clEnqueueAcquireGLObjects_fn        _clEnqueueAcquireGLObjects;
    clEnqueueReleaseGLObjects_fn        _clEnqueueReleaseGLObjects;
    clEnqueueWriteBuffer_fn             _clEnqueueWriteBuffer;
    clEnqueueWriteBufferRect_fn         _clEnqueueWriteBufferRect;
    clFlush_fn                          _clFlush;
    clFinish_fn                         _clFinish;
    clGetGLObjectInfo_fn                _clGetGLObjectInfo;
    clCreateContextFromType_fn          _clCreateContextFromType;
    clGetContextInfo_fn                 _clGetContextInfo;

    // GL_ARB_shader_objects extension functions:
    bool _isGL_ARB_shader_objectsSupported;
    PFNGLCREATESHADEROBJECTARBPROC _glCreateShaderObjectARB;
    PFNGLSHADERSOURCEARBPROC _glShaderSourceARB;
    PFNGLCOMPILESHADERARBPROC _glCompileShaderARB;
    PFNGLGETOBJECTPARAMETERIVARBPROC _glGetObjectParameterivARB;
    PFNGLCREATEPROGRAMOBJECTARBPROC _glCreateProgramObjectARB;
    PFNGLATTACHOBJECTARBPROC _glAttachObjectARB;
    PFNGLLINKPROGRAMARBPROC _glLinkProgramARB;
    PFNGLUSEPROGRAMOBJECTARBPROC _glUseProgramObjectARB;
    PFNGLGETINFOLOGARBPROC _glGetInfoLogARB;
    PFNGLGETUNIFORMLOCATIONARBPROC _glGetUniformLocationARB;
    PFNGLUNIFORM1FARBPROC _glUniform1fARB;
    PFNGLDELETEOBJECTARBPROC _glDeleteObjectARB;
    PFNGLDETACHOBJECTARBPROC _glDetachObjectARB;

    // GL_ARB_vertex_buffer_object extension functions:
    bool _isGL_ARB_vertex_buffer_objectSupported;
    PFNGLGENBUFFERSARBPROC _glGenBuffersARB;
    PFNGLBINDBUFFERARBPROC _glBindBufferARB;
    PFNGLBUFFERDATAARBPROC _glBufferDataARB;
    PFNGLDELETEBUFFERSARBPROC _glDeleteBuffersARB;
    PFNGLBUFFERSUBDATAARBPROC _glBufferSubDataARB;

    // OpenGL 1.2 function pointers
    PFNGLTEXIMAGE3DPROC         _glTexImage3D;

    // OpenGL 1.5 extension functions:
    bool _isOpenGL1_5Supported;
    PFNGLGENBUFFERSPROC _glGenBuffers;
    PFNGLBINDBUFFERPROC _glBindBuffer;
    PFNGLBUFFERDATAPROC _glBufferData;
    PFNGLDELETEBUFFERSPROC _glDeleteBuffers;
    PFNGLBUFFERSUBDATAPROC _glBufferSubData;
    PFNGLGETBUFFERSUBDATAPROC _glGetBufferSubData;

    bool _isGL_ARB_texture_floatSupported;

    // Memory buffer used to set initial values for the simulation
    // and copy them into the OpenCL memory objects. The buffer
    // is big enough to hold 4-floats per grid cell.
    char* _gridBuffer;

    // OpenGL objects
    GLuint _texDensity;
    const GLenum _texDensityInternalFormat;
    const GLenum _texDensityFormat;
    const GLenum _texDensityType;
    GLuint  _volSliceVBO;

    // Internal Id for the parts of the simulation.
    enum CalcId
    {
        CALCID_SMOKE_SIMULATION  = 0,
        CALCID_VOLUME_SLICING,
        NUMBER_CALCS
    };
    // Parts of the simulation expressed as a bitfield.
    // This is used to set which parts of the simulation runs
    // on a given device (remembering that more than one part
    // can run on the same device.
    enum CalcBits
    {
        CALCBIT_SMOKE_SIMULATION        = (1 << 0),
        CALCBIT_VOLUME_SLICING          = (1 << 1)
    };

    // This template class defines a simple list that owns
    // objects of type T added to it. If the list is destroyed,
    // the objects are deleted.
    template <class T>
    class CalcList
    {
    private:
        int             _num;
        T*              _list;

    private:
        CalcList(const CalcList& rhs);
        CalcList& operator=(const CalcList& rhs)
        {
            return *this;
        }

    public:
        CalcList() :
            _num(0),
            _list(NULL)
        {
        }

        ~CalcList()
        {
            T* curr;
            T* next = _list;

            while (next)
            {
                curr = next;
                next = curr->_next;
                delete curr;
            }
        }

        void add(T* elem)
        {
            elem->_next = _list;
            _list = elem;
            _num++;
        }

        void remove(T* elem)
        {
            T* curr = _list;
            T* prev = curr;

            do
            {
                if (curr == elem)
                {
                    if (curr == prev)
                    {
                        // Remove head.
                        _list = curr->_next;
                    }
                    else
                    {
                        prev->_next = curr->_next;
                    }

                    delete curr;
                    --_num;
                    break;
                }

                prev = curr;
                curr = curr->_next;
            }
            while (curr);
        }

        int size()
        {
            return _num;
        }

        T* get(int index) const
        {
            T* retVal = NULL;

            if (index >= 0 && index < _num)
            {
                T* curr = _list;

                while (index--)
                {
                    curr = curr->_next;
                }

                retVal = curr;
            }

            return retVal;
        }

        T* operator[](int index) const
        {
            return get(index);
        }

        T* getHead() const
        {
            return _list;
        }

        void removeHead()
        {
            T* head = _list;
            _list = head->_next;
            delete head;
            --_num;
        }
    };

    // CalcDevice is a class used to manage information about a given
    // OpenCL device. It holds the cl_command_queue that was created
    // for this device and a bitfield that indicates which parts of
    // the simulation will run on this device.
    class CalcDevice
    {
        friend class CalcDevices;

    public:
        CalcDevice*             _next;

    private:
        CalcDevice();
        CalcDevice(const CalcDevice& rhs);
        CalcDevice& operator=(const CalcDevice& rhs)
        {
            (void)(rhs);
            return *this;
        }

    public:
        const unsigned int      _calcBits;
        const OCLDevice*        _device;
        cl_command_queue        _clCmdQueue;

        CalcDevice(unsigned int calcBits, const OCLDevice* device) :
            _calcBits(calcBits),
            _device(device)
        {
            _next = NULL;
            _clCmdQueue = NULL;
        }
    };

    typedef CalcList<CalcDevice> CalcDevices;

    // CalcContext is a class used to manage information about a given
    // OpenCL context. It holds the cl_context pointer, a pointer to
    // information about the OpenCL platform, a list of CalcDevices
    // created with the context, a bitfield of which parts of the
    // simulation will be performed on this platform and whether
    // or not CL-GL sharing can be attempted using this context
    // i.e. if the context was created using the current OpenGL
    // context.
    class CalcContext
    {
        friend class CalcContexts;

    public:
        CalcContext*            _next;

    private:
        CalcContext();
        CalcContext(const CalcContext& rhs);
        CalcContext& operator=(const CalcContext& rhs)
        {
            (void)(rhs);
            return *this;
        }

    public:
        unsigned int            _calcMask;   // Which bits of calc performed in this context
        const OCLPlatform*      _platform;
        cl_context              _clCtx;
        CalcDevices             _devices;
        cl_image_format*        _image2DFormats;
        cl_uint                 _numImage2DFormats;
        cl_image_format*        _image3DFormats;
        cl_uint                 _numImage3DFormats;
        const cl_image_format*  _image2Drgba;
        const cl_image_format*  _image2Dluminance;
        const cl_image_format*  _image3Drgba;
        const cl_image_format*  _image3Dluminance;
        bool                    _requestGLSharing;
        bool                    _withGLSharing;

        CalcContext(const OCLPlatform* platform) :
            _platform(platform)
        {
            _next = NULL;
            _calcMask = 0;
            _clCtx = NULL;
            _image2DFormats = NULL;
            _numImage2DFormats = 0;
            _image3DFormats = NULL;
            _numImage3DFormats = 0;
            _image2Drgba = NULL;
            _image2Dluminance = NULL;
            _image3Drgba = NULL;
            _image3Dluminance = NULL;
            _requestGLSharing = false;
            _withGLSharing = false;
        }

        ~CalcContext()
        {
            delete [] _image2DFormats;
            delete [] _image3DFormats;
        }

        void addDevice(CalcDevice* device)
        {
            _calcMask |= device->_calcBits;
            _devices.add(device);
        }

        void removeDevice(CalcDevice* device)
        {
            _devices.remove(device);

            // Reset the mask for the remaining devices
            _calcMask = 0;

            for (int i = _devices.size() - 1; i >= 0; --i)
            {
                _calcMask |= _devices[i]->_calcBits;
            }
        }
    };

    typedef CalcList<CalcContext> CalcContexts;

    // Storage for the list of contexts/devices that are currently
    // running the simulation.
    CalcContexts _contexts;

    // This structure is used to define static information about the
    // OpenCL kernels in a program.
    struct CalcKernelInfo
    {
        // Enum for this kernel.
        int             _index;

        // Name of the kernel in the source program.
        const char*     _kernelFuncName;

        // The source path to the OpenCL program file.
        const wchar_t*  _sourcePath;

        // It is possible to goto another kernel in the list for a given
        // repeat count. This is used to perform iteration of the Jacobi
        // Poisson solver.
        int             _repeatKernelIndex;     // Index of kernel to go to
        int             _repeatKernelCount;     // How many times to go there
        // List of memory object Ids (see the enums below) that are passed
        // to this kernel (we only pass memory objects).
        int             _args[10];
    };
    // This array defines the sequence of kernels that need to be
    // run for the smoke simulation.
    static const CalcKernelInfo _s_smokeSimulationKernels[];
    // This array defines the sequence of kernels that need to be
    // run for the volume slicing calculation.
    static const CalcKernelInfo _s_volumeSlicingKernels[];

    // This structure is used to define static information about the
    // OpenCL programs that are used in the simulation.
    struct CalcProgramInfo
    {
        // Enum for this part of the simulation.
        CalcId                  _id;
        // The bitfield value for this part of the simulation (see enum CalcBits).
        unsigned int            _bit;
        // The source path to the OpenCL program file.
        const wchar_t*          _sourcePath;
        // Pointer to a null-terminated array of information about kernels
        // that need to be executed from this source program.
        const CalcKernelInfo*   _kernels;
        // The total number of memory objects that need to be created for this
        // part of the simulation.
        int                     _numMemObjects;
    };
    // This defines the list of OpenCL programs.
    static const CalcProgramInfo _s_programs[NUMBER_CALCS];

    // This class holds dynamic information about a kernel. It is built
    // from static information stored in CalcKernelInfo. It is used to
    // hold the cl_kernel pointer, OpenCL information about the kernel
    // and the global and local work group sizes that are passed to
    // _clEnqueueNDRangeKernel.
    class CalcKernel
    {
    private:
        CalcKernel(const CalcKernel& rhs);
        CalcKernel& operator=(const CalcKernel& rhs)
        {
            (void)(rhs);
            return *this;
        }

    public:
        const char*     _kernelFuncName;
        const wchar_t*  _sourcePath;
        char*           _source;     // Program(Kernel) source read from sourcePath
        cl_program      _clProgram;  // OpenCL program
        cl_kernel       _clKernel;
        size_t          _maxKernelWorkGroupSize;
        cl_ulong        _kernelLocalMemSize;
        size_t          _kernelPreferredWorkGroupSizeMultiple;
        size_t          _globalWorkSize[3];
        size_t          _localWorkSize[3];
        int             _repeatKernelIndex;
        int             _repeatKernelCount;
        int             _repeatKernelIteration;
        int             _args[10];

        CalcKernel() :
            _kernelFuncName(NULL),
            _clProgram(NULL),
            _clKernel(NULL),
            _repeatKernelIndex(-1),
            _repeatKernelCount(-1),
            _repeatKernelIteration(0)
        {
        }

        ~CalcKernel()
        {
        }
    };

    // CalcMemory is a class used to manage data about an OpenCL memory
    // object that has been created - cl_mem pointer, memory type,
    // memory size.
    enum CalcMemoryType
    {
        CALCMEM_NONE = 0,
        CALCMEM_BUFFER,
        CALCMEM_IMAGE2D,
        CALCMEM_IMAGE3D
    };
    class CalcMemory
    {
    private:
        CalcMemory(const CalcMemory& rhs);
        CalcMemory& operator=(const CalcMemory& rhs)
        {
            (void)(rhs);
            return *this;
        }

    public:
        CalcMemoryType  _memType;
        cl_mem          _clMemory;
        size_t          _memSize;

        CalcMemory() :
            _memType(CALCMEM_NONE),
            _clMemory(NULL),
            _memSize(0)
        {
        }

        ~CalcMemory()
        {
        }
    };

    // CalcProgram is used to hold all information about a program (part of
    // the simulation) - cl_program pointer, kernels and memory that have
    // been created.
    class CalcProgram
    {
    private:
        CalcProgram();
        CalcProgram(const CalcProgram& rhs);
        CalcProgram& operator=(const CalcProgram& rhs)
        {
            (void)(rhs);
            return *this;
        }

    public:
        const CalcId        _id;
        const unsigned int  _bit;
        const wchar_t*      _sourcePath;

        char*               _source;     // Program source read from sourcePath
        CalcContext*        _calcContext;// Context that will perform the calc
        CalcDevice*         _calcDevice; // Device that will perform the calc
        cl_program          _clProgram;  // OpenCL program
        CalcKernel*         _kernels;    // Kernels in the program
        int                 _numKernels; // The number of kernels in the program.
        CalcMemory*         _memObjects; // Memory objects that have been created
        int                 _numMemObjects; // Number of memory objects.

        CalcProgram(const CalcProgramInfo& info) :
            _id(info._id),
            _bit(info._bit),
            _sourcePath(info._sourcePath)
        {
            _source = NULL;
            _calcContext = NULL;
            _calcDevice = NULL;
            _clProgram = NULL;
            _kernels = NULL;
            _numKernels = 0;
            _memObjects = NULL;
            _numMemObjects = 0;

            const CalcKernelInfo* kernelInfo = info._kernels;

            while (kernelInfo->_index >= 0)
            {
                if (kernelInfo->_index > _numKernels)
                {
                    _numKernels = kernelInfo->_index;
                }

                kernelInfo++;
            }

            _numKernels++;
            _kernels = new CalcKernel[_numKernels];
            kernelInfo = info._kernels;

            while (kernelInfo->_index >= 0)
            {
                CalcKernel* k = &_kernels[kernelInfo->_index];
                k->_kernelFuncName = kernelInfo->_kernelFuncName;
                k->_sourcePath = kernelInfo->_sourcePath;
                k->_repeatKernelIndex = kernelInfo->_repeatKernelIndex;
                k->_repeatKernelCount = kernelInfo->_repeatKernelCount;
                k->_repeatKernelIteration = 0;
                memcpy(k->_args, kernelInfo->_args, sizeof(kernelInfo->_args));
                kernelInfo++;
            }

            if (info._numMemObjects)
            {
                _memObjects = new CalcMemory[info._numMemObjects];
                _numMemObjects = info._numMemObjects;
            }
        }

        ~CalcProgram()
        {
            delete _source;
            delete [] _kernels;
            delete [] _memObjects;
        }
    };

    // Maintain a list of programs (parts of the simulation).
    class CalcPrograms
    {
    private:
        const int       _num;
        CalcProgram**   _array;

    private:
        CalcPrograms();
        CalcPrograms(const CalcPrograms& rhs);
        CalcPrograms& operator=(const CalcPrograms& rhs)
        {
            (void)(rhs);
            return *this;
        }

    public:
        CalcPrograms(int num) :
            _num(num)
        {
            _array = new CalcProgram*[num];

            for (int i = num - 1; i >= 0; --i)
            {
                _array[i] = NULL;
            }
        }

        ~CalcPrograms()
        {
            for (int i = _num - 1; i >= 0; --i)
            {
                delete _array[i];
            }

            delete [] _array;
        }

        bool add(const CalcProgramInfo& info)
        {
            bool retVal = false;

            if (info._id >= 0 && info._id < _num)
            {
                delete _array[info._id];
                _array[info._id] = new CalcProgram(info);
                retVal = true;
            }

            return retVal;
        }

        int size()
        {
            return _num;
        }

        CalcProgram* operator[](int index) const
        {
            CalcProgram* retVal = NULL;

            if (index >= 0 && index < _num)
            {
                retVal = _array[index];
            }

            return retVal;
        }
    };
    // Storage for the OpenCL programs that have been loaded.
    CalcPrograms _programs;

    // This time is used to calculate the time since the last step was calculated.
    // It ensures that we attempt to run in real-time.
    AMDTTimer _timer;

    // Smoke Simulation memory objects
    enum SmokeSimMemObject
    {
        MO_SMOKESIM_CONSTANTS = 0,
        MO_SMOKESIM_DENSITY,
        MO_SMOKESIM_U03D,
        MO_SMOKESIM_U13D,
        MO_SMOKESIM_S03D,
        MO_SMOKESIM_T03D,
        MO_SMOKESIM_TMPSCALAR,
        MO_SMOKESIM_P03D,
        MO_SMOKESIM_P13D,
        MO_SMOKESIM_BOUNDARY_INDEX,
        MO_SMOKESIM_BOUNDARY_SCALE,
        NUM_SMOKESIM_MEM_OBJECTS
    };
    // Smoke Simulation kernels.
    enum SmokeSimKernels
    {
        KERN_SMOKESIM_APPLY_SOURCES = 0,
        KERN_SMOKESIM_APPLY_BUOYANCY,
        KERN_SMOKESIM_CALCULATE_CURLU,
        KERN_SMOKESIM_APPLY_VORTICITY,
        KERN_SMOKESIM_ADVECT_VELOCITY,
        KERN_SMOKESIM_APPLY_VELECITY_BOUNDARY_CONDITION,
        KERN_SMOKESIM_COMPUTE_FIELD_PRESSURE_PREP,
        KERN_SMOKESIM_COMPUTE_FIELD_PRESSURE_ITER1,
        KERN_SMOKESIM_COMPUTE_FIELD_PRESSURE_ITER2,
        KERN_SMOKESIM_APPLY_PRESSURE_BOUNDARY_CONDITION,
        KERN_SMOKESIM_PROJECT_FIELD_VELOCITY,
        KERN_SMOKESIM_ADVECT_FIELD_DENSITY,
        KERN_SMOKESIM_DISSIPATE_DENSITY,
        KERN_SMOKESIM_ADVECT_FIELD_TEMPERATURE,
        KERN_SMOKESIM_DISSIPATE_TEMPERATURE,
        KERN_SMOKESIM_CREATE_DENSITY_TEXTURE,
        KERN_SMOKESIM_DEBUG_DENSITY,
        KERN_SMOKESIM_DEBUG_TEMPERATURE,
        KERN_SMOKESIM_DEBUG_VELOCITY_VECTOR,
        KERN_SMOKESIM_DEBUG_VELOCITY_LENGTH,
        KERN_SMOKESIM_DEBUG_FIELD_PRESSURE,
        KERN_SMOKESIM_DEBUG_FIELD_DIVERGENCE
    };

    // The OpenCL memory object associated with the 3D texture used to store density
    // for rendering. If GL-CL sharing wasn't supported, this will be NULL.
    cl_mem _oclMemGLDensityTexture;

    // This structure is stored in an OpenCL memory buffer and transfered to the
    // device before each simulation step. It contains all the parameters needed
    // by the kernels.
    struct SmokeSimConstants
    {
        cl_float    _buoyAlpha;          // buoyancy force constant
        cl_float    _buoyBeta;           // buoyancy force constant
        cl_float    _vorticity;          // _vorticity constant
        cl_float    _KsDens;             // Multiplier for density source
        cl_float    _KsTemp;             // Multiplier for temperature source

        // dissipation constants and coefficients
        cl_float    _KminDens;
        cl_float    _KmaxDens;
        cl_float    _KminTemp;
        cl_float    _KmaxTemp;

        cl_float    _KdrDens;
        cl_float    _KdrTemp;
        cl_float    _KdissipateDens;     // 1.0f / (1.0f + dTime * _KdrDens)
        cl_float    _KdissipateTemp;     // 1.0f / (1.0f + dTime * _KdrTemp)

        // constants for Jacobi Poisson iteration used to deduce pressure.
        cl_float    _KpressureJacobiPoissonAlpha;
        cl_float    _KpressureJacobiPoissonInvBeta;

        cl_float    _ambiantTemperature;
        cl_float    _deltaTimeInSeconds;
        cl_float    _maxDeltaTimeInSeconds;

        // Smoke source
        cl_float    _sourceDistributionAlpha; // -1 / (2 * var^2)
        cl_float    _sourceDistributionBeta; // 1 / sqrt(2 * pi * var^2)
        cl_float2   _spoutCenter;        // Center of tea spout in grid coords
        cl_float2   _spoutInvExtent;     // (1 / maxXRadius, 1 / maxYRadius)
        cl_float2   _sourceCenter;       // Center of maximum smoke density
        cl_float2   ___alignSourceVelocity__;
        cl_float4   _sourceVelocity;     // Velocity vector of smoke leaving spout.
    };

    // Host storage for the smoke simulation parameters.
    SmokeSimConstants _smokeSimConstants;

    // Volume Slicing memory objects
    enum VolumeSlicingMemObjects
    {
        MO_VOLSLICE_VERTICES,
        MO_VOLSLICE_PARAMS,
        MO_VOLSLICE_CONSTANTS,
        NUM_VOLSLICE_MEM_OBJECTS
    };
    // Volume slicing kernels.
    enum VolumeSlicingKernels
    {
        KERN_VOLSLICE_COMPUTE_INTERSECTION = 0,
    };

    // This structure is stored in an OpenCL memory buffer. It contains constants
    // needed by the kernels. It is transfered to the device once.
    struct VolumeSlicingConstants
    {
        cl_int _v1[12];
        cl_int _v2[12];
        cl_int _nSequence[64];
    };
    // This structure is stored in an OpenCL memory buffer and transfered to the
    // device before each simulation step. It contains all the parameters needed
    // by the kernels.
    struct VolumeSlicingParams
    {
        // View vector pointing from the plane towards the eye position.
        cl_float3  _view;

        // Canonical vertices of the grid (-0.5 or 0.5)
        cl_float3  _verts[8];

        // Grid vertices after model transformation
        cl_float3  _tVerts[8];

        // Distance of the starting plane from the origin (model space)
        cl_float   _dPlaneStart;

        // Distance between each plane (in model space)
        cl_float   _dPlaneIncr;

        // The grid corner that is closest to the viewer.
        cl_int     _frontIdx;

        // The number of slices from closest corner to farthest corner.
        cl_int     _numSlices;
    };

    // The OpenCL memory object associated with the VBO used to store slice
    // triangles. If GL-CL sharing wasn't supported, this will be NULL.
    cl_mem _oclMemGLVBO;

    // Host storage for volume slicing constants.
    VolumeSlicingConstants _volSliceConstants;
    // Host storage for volume slicing parameters.
    VolumeSlicingParams _volSliceParams;

    // Keep track of how many volume slices we need to compute.
    int _numSlicesToRender;

    // Storage for OpenCL events.
    enum SmokeSimEvents
    {
        EVT_SMOKESIM_COPY_BUFFER = 0,
        EVT_VOLSLICE_COPY_BUFFER,
        NUM_WAIT_EVENTS
    };
    cl_event _waitEvents[NUM_WAIT_EVENTS];

    // Fluid grid instance. This object keeps track of the grid
    // size and OpenGL corner coordinates.
    AMDTFluidGrid _fluidGrid;

    // Parameters used to randomized smoke source location and speed.
    float _smokeSourceAngularVelocity;  // deg/sec
    float _smokeSourceRadialVelocity;   // grid cells / sec
    float _smokeSourceAngle;
    float _smokeSourceRadius;
    float _smokeSourceMaxRadius;
    float _smokeSourceMaxAngularAccel;
    float _smokeSourceMaxRadialAccel;
    float _smokeSourceMaxAngularVelocity;
    float _smokeSourceMaxRadialVelocity;
    float _smokeSourceCurrentDuration;
    float _smokeSourceNextUpdateTime;
    float _smokeSourceVerticalVelocity;
    float _smokeSourceHorizontalVelocity;

    // Track configuration changes that need to be made before
    // the next step in the simulation.
    enum ExecutionOptions
    {
        EXEC_WITH_GLSHARING = (1 << 0)
    };

    unsigned int    _dirtyFlags;
    int             _changeNumCellsX;
    int             _changeNumCellsY;
    int             _changeNumCellsZ;
    float           _changeCellSpacingMETERS;
    const OCLDevice*    _changeSmokeSimDevice;
    const OCLDevice*    _changeVolSliceDevice;
    unsigned int        _changeExecOptions;

    // Storage for the current fluid simulation parameters.
    SimParams       _simParams;

    // Whether or not to run the simulation and render.
    bool _enabled;

    // Whether or not the kernels compilation was done in independent thread
    InitCompile _initCompileFlag;

    // Bitfield of ExecutionOptions.
    unsigned int _execOptions;

    // Whether or not to render the outline of the grid.
    bool _showGrid;

    // This defines a simple dynamic memory-storage string. It is used to
    // accumulate error messages.
    struct DynStr
    {
        char*   _str;
        size_t  _size;
        size_t  _bufSize;

        DynStr()
        {
            _bufSize = 4096;
            _str = (char*)malloc(_bufSize);
            reset();
        }

        ~DynStr()
        {
            free(_str);
        }

        void reset()
        {
            _size = 0;
            _str[0] = '\0';
        }

        bool resize(size_t size)
        {
            bool ret = false;
            char* newBuf = (char*)realloc(_str, size);

            if (newBuf)
            {
                _str = newBuf;
                _bufSize = size;
                ret = true;
            }

            return ret;
        }

        void print(const char* str)
        {
            size_t len = strlen(str);

            if (_size + len + 1 > _bufSize)
            {
                resize(_size + len + 1);
            }

            const char* src = str;
            char* dst = _str + _size;
            size_t cnt = (_size + len < _bufSize) ? len : (_bufSize - _size - 1);

            while (cnt-- > 0)
            {
                *dst++ = *src++;
                ++_size;
            }

            *dst = '\0';
        }

        void printv(const char* fmt, va_list args)
        {
            // Make sure we have enough space.
            size_t cnt = _bufSize - _size;

            if (cnt < 1024)
            {
                resize(_size + 1024);
                cnt = _bufSize - _size;
            }

            int n = vsnprintf(_str + _size, cnt, fmt, args);

            if (n < 0)
            {
                // Error occurred.
                _str[_size] = '\0';
            }
            else if ((size_t)n >= cnt)
            {
                // Need more space.
                resize(_size + (size_t)n + 1);
                cnt = _bufSize - _size;
                n = vsnprintf(_str + _size, cnt, fmt, args);

                if (n < 0 || (size_t)n >= cnt)
                {
                    // Error occurred.
                    _str[_size] = '\0';
                }
                else
                {
                    _size += (size_t)n;
                }
            }
            else
            {
                _size += (size_t)n;
            }
        }

        void printf(const char* fmt, ...)
        {
            va_list args;
            va_start(args, fmt);
            printv(fmt, args);
            va_end(args);
        }
    };

    // String used to hold the last error message.
    DynStr _lastError;

    // Keep the logger object that is used to tell the application of
    // progress when running the simulation (mainly during
    // initialization and setting up OpenCL resources).
    ISmokeSystemLogger* _logger;

private:
    // Hide the default constructor. You need to pass certain data when
    // instantiating this class.
    AMDTTeapotOCLSmokeSystem();

    // Initialize the OpenCL system - choose platform, create contexts.
    bool initOpenCL();

    // Initialize the OpenGL system - lookup function pointers and create
    // texture and VBO object names.
    bool initOpenGL();

    // Import OpenGL function pointers and check that we have required
    // extensions.
    bool initOpenGLExtensions();

    // Create all the OpenGL objects (textures/buffers).
    bool createOpenGLObjects();

    // Delete all the OpenGL objects that have been created.
    void deleteOpenGLObjects();

    // Create all OpenCL contexts, command queues, memory objects.
    // Build the program.
    bool createOpenCLProgramResource(CalcProgram* program);

    // Create all OpenCL contexts, command queues, memory objects.
    // Build the (kernel) program.
    bool createOpenCLKernelProgramResource(CalcProgram* program);

    // Build all the programs.
    bool createOpenCLResources();

    // Create all OpenCL contexts, command queues, memory objects.
    bool initOpenCLResources();

    // Release all OpenCL resources
    bool releaseOpenCLResources();

    // Set grid dimensions. This is used to request a change, doesn't actually
    // change anything.
    void setGrid(int numCellsWidth, int numCellsDepth, int numCellsHeight, float cellSpacingMETERS);

    // Setup the density grid dimensions and parameters - recalculation all
    // grid parameters.
    bool setupDensityGrid(int numCellsX, int numCellsY, int numCellsZ, float spacingMETERS);

    // This performs one computation step (smoke simulation, volume slicing).
    bool compute(const AMDTTeapotRenderState& state, const Mat4& modelTransformation, float deltaTimeSeconds, float deltaRot);

    // This renders the smoke density texture.
    bool render(const AMDTTeapotRenderState& state, const Mat4& modelTransformation);

    // This is used to make OpenCL kernel info requests to determine maximum values for
    // work group sizes etc.
    bool getKernelWorkGroupInfo(const OCLDevice* device, CalcKernel* kernel);

    // This is used to setup the global and local work group sizes for a given kernel.
    // This is for kernels that need to process the 3D grid.
    bool setup3DKernelWorkGroupSizes(CalcKernel* kernel, int width, int depth, int height);

    // This function will initialize all the smoke memory objects to default values -
    // zero velocity field, zero density, ambient temperature.
    bool initSmokeFields();

    // This will perform setup of the Smoke Simulation program - create all the memory
    // objects and make all the _clSetKernelArg() calls. GL-CL sharing is attempted
    // in this function.
    bool setupSmokeSimulation(CalcProgram* program);

    // Release any special resources that were created for the Smoke Simulation. Releasing
    // kernels and memory objects is done automatically (since the data is all available
    // in the CalcProgram structure).
    bool releaseSmokeSimulation();

    // Create a OpenCL buffer that will be used to hold data for every cell of the grid.
    bool setupSmokeSimGridMemory(
        CalcProgram* program,
        SmokeSimMemObject memId,
        size_t numBytesPerCell,
        cl_mem_flags flags = CL_MEM_READ_WRITE);

    // This will perform setup of the Volume Slicing program - create all the memory
    // objects and make all the _clSetKernelArg() calls. GL-CL sharing is attempted
    // in this function.
    bool setupVolumeSlicing(CalcProgram* program);

    // Release any special resources that were created for Volume Slicing. Releasing
    // kernels and memory objects is done automatically (since the data is all available
    // in the CalcProgram structure).
    bool releaseVolumeSlicing();

    // Initialize the volume slicing memory objects. This is done once after the
    // OpenCL devices have been set/changed.
    bool initVolumeSlicingData();

    // Given simulation parameters (SimParams), calculate the SmokeSimConstants values
    // that are passed to the smoke simulation kernels.
    void recalculateSmokeSimConstants();

    // Attempt to choose the most optimized configuration for performing
    // the simulation/computations. This will attempt to perform the
    // smoke simulation on a GPU device that shares the GL context
    // and the calculation of the volume slices on the CPU. If the
    // host has Intel CPUs and the Intel platform, then the Intel platform
    // will be used. If the host has AMD CPUs then the AMD platform will
    // be used. The CPU device that can share with the GL context will
    // be given preference.
    bool chooseBestDevices();

    // Manually specify which devices to use for the different parts of
    // the simulation/computation.
    bool selectDevices(const OCLDevice* smokeSimulation, const OCLDevice* volumeSlicing);
};


#endif //__AMDTTEAPOTOCLSMOKESYSTEM_H

