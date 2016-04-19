//------------------------------ gpAbstractTraceDataAccessInterface.h ------------------------------

#ifndef __GPABSTRACTTRACEDATAACCESSINTERFACE_H
#define __GPABSTRACTTRACEDATAACCESSINTERFACE_H

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>

/// This class implement an abstract data access for trace files
class AMDT_GPU_PROF_API gpAbstractTraceDataAccessInterface
{
public:

    gpAbstractTraceDataAccessInterface(const osFilePath& dataFilePath) { m_dataFilePath = dataFilePath }
    ~gpAbstractTraceDataAccessInterface();

    /// Virtual functions for the concrete data access classes
    virtual bool LoadDataFromFile() = 0;

    virtual int GetAPINumber(APIToTrace api);
protected:

    /// File path for the file containing the data
    osFilePath m_dataFilePath;
};

#endif // __GPABSTRACTTRACEDATAACCESSINTERFACE_H
