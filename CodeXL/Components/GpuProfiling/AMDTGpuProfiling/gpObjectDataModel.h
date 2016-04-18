//------------------------------ GPUObjectDataModel.h ------------------------------

#ifndef _GPOBJECTDATAMODEL_H_
#define _GPOBJECTDATAMODEL_H_

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

class GPUSessionTreeItemData;
class gpObjectDataContainer;

// ----------------------------------------------------------------------------------
// Class Name:          GPUSessionDataModel
// General Description: This class is used to load and store the data for a GPU profile
// session. The model contain a parser, a data container, and a data access interface,
// which can be accessed from public when the GPU profile session data needs to be queried
// ----------------------------------------------------------------------------------
class gpObjectDataModel
{
public:

    /// Constructor
    gpObjectDataModel(GPUSessionTreeItemData* pSessionItemData);

    /// Destructor
    ~gpObjectDataModel();

    /// Loads the session data from the file to the data container
    /// \param ObjectFilePath the requested Object file path
    /// \param wasParseCanceled[output] true if the user clicked cancel while parsing
    /// \param ObjectDataContainter the container in which to store the Object data
    bool LoadObjectFile(const osFilePath& ObjectFilePath, bool& wasParseCanceled, gpObjectDataContainer*& pObjectDataContainter);

    /// Accessor
    gpObjectDataContainer* ObjectDataContainer() const { return m_pObjectDataContainer; };

private:

    /// The session item data
    GPUSessionTreeItemData* m_pSessionItemData;

    /// Session data container
    gpObjectDataContainer* m_pObjectDataContainer;

};

#endif // _GPOBJECTDATAMODEL_H_
