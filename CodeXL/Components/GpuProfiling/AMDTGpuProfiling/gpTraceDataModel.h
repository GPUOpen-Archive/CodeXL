//------------------------------ GPUSessionDataModel.h ------------------------------

#ifndef _GPUSESSIONDATAMODEL_H_
#define _GPUSESSIONDATAMODEL_H_

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

class GPUSessionTreeItemData;
class gpTraceDataContainer;

// ----------------------------------------------------------------------------------
// Class Name:          GPUSessionDataModel
// General Description: This class is used to load and store the data for a GPU profile
// session. The model contain a parser, a data container, and a data access interface,
// which can be accessed from public when the GPU profile session data needs to be queried
// ----------------------------------------------------------------------------------
class gpTraceDataModel
{
public:

    /// Constructor
    gpTraceDataModel(GPUSessionTreeItemData* pSessionItemData);

    /// Destructor
    ~gpTraceDataModel();

    /// Loads the session data from the file to the data container
    /// \param traceFilePath the requested trace file path
    /// \param wasParseCanceled[output] true if the user clicked cancel while parsing
    /// \param traceDataContainter the container in which to store the trace data
    bool LoadTraceFile(const osFilePath& traceFilePath, bool& wasParseCanceled, gpTraceDataContainer*& pTraceDataContainter);

    /// Accessor
    gpTraceDataContainer* TraceDataContainer() const { return m_pTraceDataContainer; };

private:

    /// The session item data
    GPUSessionTreeItemData* m_pSessionItemData;

    /// Session data container
    gpTraceDataContainer* m_pTraceDataContainer;

};


#endif