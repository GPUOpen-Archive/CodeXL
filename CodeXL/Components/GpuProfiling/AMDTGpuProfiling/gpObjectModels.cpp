// Qt
#include <QStandardItem>

// AMDTBaseTools
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// local files
#include <AMDTGpuProfiling/gpObjectModels.h>

ObjTreeModel::ObjTreeModel(QDomDocument document, QObject* parent)
    : QStandardItemModel(parent), m_DomDoc(document)
{
    m_rootItem = this->invisibleRootItem();

    refreshTree();
}

ObjTreeModel::~ObjTreeModel()
{

}

void ObjTreeModel::setIncludeDestroyed(int iIncludeDestroyed)
{
    m_iIncludeDestroyed = iIncludeDestroyed;
    refreshTree();
}

void ObjTreeModel::setGroupDestroyed(int iGroupDestroyed)
{
    m_iGroupDestroyed = iGroupDestroyed;
    refreshTree();
}

void ObjTreeModel::refreshTree()
{
    // get list of Devices
    QDomNodeList listDevices = m_DomDoc.elementsByTagName("Device");

    // remove nodes
    m_rootItem->removeRows(0, m_rootItem->rowCount());

    // get list of DX objects
    for (int iDevIdx = 0; iDevIdx < listDevices.count(); iDevIdx++)
    {
        QString strItem = "", strCheck = "";
        QStandardItem* DeviceItem;

        strItem.append(listDevices.at(iDevIdx).nodeName());
        strItem.append(" - ");
        // appending pointer string
        strItem.append(listDevices.at(iDevIdx).attributes().item(0).nodeValue());

        DeviceItem = new QStandardItem(strItem);

        m_rootItem->appendRow(DeviceItem);

        // getting the dx elements
        QDomNode nodeElement = listDevices.at(iDevIdx).firstChildElement();

        while (!nodeElement.isNull())
        {
            QStandardItem* dxObjItem, *newObjItem;
            QStringList ptrStrList;
            bool bDestroyed;

            strCheck = nodeElement.nodeName();
            strCheck = nodeElement.toElement().text();

            dxObjItem = new QStandardItem(nodeElement.nodeName());
            DeviceItem->appendRow(dxObjItem);

            ptrStrList = nodeElement.toElement().text().split(',', QString::SkipEmptyParts);

            for (int iPtrIdx = 0; iPtrIdx < ptrStrList.count(); iPtrIdx++)
            {
                // search for "|d", these are destroyed
                bDestroyed = (ptrStrList[iPtrIdx].right(2).compare("|d") == 0);

                if (bDestroyed)
                {
                    // remove "|d"
                    ptrStrList[iPtrIdx] = ptrStrList[iPtrIdx].left(ptrStrList[iPtrIdx].length() - 2);
                }

                if ((Qt::Unchecked != m_iIncludeDestroyed) || !bDestroyed)
                {
                    newObjItem = new QStandardItem(ptrStrList[iPtrIdx]);

                    if (bDestroyed)
                    {
                        QFont myFont = newObjItem->font();
                        myFont.setStrikeOut(true);
                        newObjItem->setFont(myFont);
                    }

                    dxObjItem->appendRow(newObjItem);
                }
            }

            nodeElement = nodeElement.nextSiblingElement();
        }
    }
}

QVariant ObjTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    GT_UNREFERENCED_PARAMETER(section);
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return tr("AMD Graphics");
    }

    return QVariant();
}

void ObjTreeModel::setDomDoc(QDomDocument document)
{
    m_DomDoc = document;

    refreshTree();
}

// ObjDatabaseModel begin
ObjDatabaseModel::ObjDatabaseModel(QDomDocument document, QObject* parent)
    : QStandardItemModel(parent), m_DomDoc(document)
{
    m_rootItem = this->invisibleRootItem();

    // get list of Devices
    QDomNodeList listInfos = m_DomDoc.elementsByTagName("CreateInfo");

    if (listInfos.count() > 0)
    {
        DisplayObjectNode(listInfos.at(0).firstChildElement());
    }
}

// slot
void ObjDatabaseModel::dBaseObjSelected(const QString strObjectSelected)
{
    DisplayObject(strObjectSelected);
}

// show object details id'ed by Handle
void ObjDatabaseModel::DisplayObject(QString strHandle)
{
    QString strItem = "", strCheck = "";

    // search for dom object with correct handle
    // get list of Devices
    QDomNodeList listInfos = m_DomDoc.elementsByTagName("CreateInfo");

    // get list of DX objects
    for (int iInfoIdx = 0; iInfoIdx < listInfos.count(); iInfoIdx++)
    {
        // sample string for nodeValue() "0x000000DA83E60200"
        strCheck = listInfos.at(iInfoIdx).attributes().item(0).nodeValue();

        if (0 == strHandle.compare(strCheck, Qt::CaseInsensitive))
        {
            // Object found
            DisplayObjectNode(listInfos.at(iInfoIdx).firstChildElement());
        }
    }
}

// show specified XML obj node, this is the node after info handle, i.e. the node of "ID3D12RootSignature", "ID3D12ComittedResource", "ID3D12CommandList"
void ObjDatabaseModel::DisplayObjectNode(QDomNode domObjectNode)
{
    //    domObjectNode contains the CreateInfo node
    QString strName = "", strCheck = "", strValue = "";
    QStandardItem* pNextTreeItem;

    // getting the dx elements
    m_rootItem->removeRows(0, m_rootItem->rowCount());
    pNextTreeItem = m_rootItem;

    // 1st element is the object type label, only one of these
    strName = domObjectNode.nodeName();
    strCheck = domObjectNode.toElement().text();

    // top node, Object type string,
    QDomNode nodeElement = domObjectNode.firstChildElement();

    // send object type to edit box
    emit(ObjDisplayed(domObjectNode.nodeName()));

    while (!nodeElement.isNull())
    {
        QList<QStandardItem*> rowItems;

        strName = nodeElement.nodeName();
        strCheck = nodeElement.toElement().text();

        if (nodeElement.hasAttributes())
        {
            strValue = nodeElement.attributes().item(0).nodeValue();
            rowItems << new QStandardItem(nodeElement.attributes().item(0).nodeValue());

            if (nodeElement.hasChildNodes() && nodeElement.firstChild().isText())
            {
                strCheck = nodeElement.toElement().text();
                rowItems << new QStandardItem(nodeElement.toElement().text());
            }
            else
            {
                strName = nodeElement.nodeName();
                rowItems << new QStandardItem(nodeElement.nodeName());
            }

            pNextTreeItem->appendRow(rowItems);

            // recursive function to fill table
            if (nodeElement.firstChild().childNodes().count() > 0)
            {
                FillTree(rowItems.first(), nodeElement);
            }
        }
        else
        {
            // TODO invalid node assert
        }

        nodeElement = nodeElement.nextSiblingElement();
    }
}

// recursive function to build Object Database tree
void ObjDatabaseModel::FillTree(QStandardItem* pNextTreeItem, QDomNode elementNode)
{
    GT_UNREFERENCED_PARAMETER(pNextTreeItem);
    GT_UNREFERENCED_PARAMETER(elementNode);

    QString strName = "", strCheck = "", strValue = "";
    QDomNode nodeElement = elementNode.firstChildElement();

    strCheck = nodeElement.nodeName();
    strCheck = nodeElement.toElement().text();

    while (!nodeElement.isNull())
    {
        QList<QStandardItem*> rowItems;

        strName = nodeElement.nodeName();
        strCheck = nodeElement.toElement().text();

        if (nodeElement.hasAttributes())
        {
            strValue = nodeElement.attributes().item(0).nodeValue();
            rowItems << new QStandardItem(nodeElement.attributes().item(0).nodeValue());

            if (nodeElement.hasChildNodes() && nodeElement.firstChild().isText())
            {
                strCheck = nodeElement.toElement().text();
                rowItems << new QStandardItem(nodeElement.toElement().text());
            }
            else
            {
                strName = nodeElement.nodeName();
                rowItems << new QStandardItem(nodeElement.nodeName());
            }

            pNextTreeItem->appendRow(rowItems);

            // recursive function to fill table
            if (nodeElement.childNodes().count() > 0)
            {
                FillTree(rowItems.first(), nodeElement);
            }
        }
        else
        {
            // TODO invalid node assert
        }

        nodeElement = nodeElement.nextSiblingElement();
    }

}

ObjDatabaseModel::~ObjDatabaseModel()
{

}

QVariant ObjDatabaseModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const
{
    GT_UNREFERENCED_PARAMETER(section);
    GT_UNREFERENCED_PARAMETER(orientation);
    GT_UNREFERENCED_PARAMETER(role);

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
            case 0:
                return tr("Description");

            case 1:
                return tr("Data");

            default:
                return QVariant();
        }
    }

    return QVariant();
}
