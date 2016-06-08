//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acImageManagerModel.cpp
///
//==================================================================================

//------------------------------ acImageManagerModel.cpp ------------------------------

#include <math.h>

// Qt:
#include <QHeaderView>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTApplicationComponents/Include/acImageManager.h>
#include <AMDTApplicationComponents/Include/acImageManagerModel.h>


// ---------------------------------------------------------------------------
// Name:        acImageManagerModel::acImageManagerModel
// Description: Constructor
// Arguments:   acImageManager* pManager
// Author:      Sigal Algranaty
// Date:        21/6/2012
// ---------------------------------------------------------------------------
acImageManagerModel::acImageManagerModel(acImageManager* pManager) : m_pImageManager(pManager)
{
}

// ---------------------------------------------------------------------------
// Name:        acImageManagerModel::~acImageManagerModel
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        21/6/2012
// ---------------------------------------------------------------------------
acImageManagerModel::~acImageManagerModel()
{

}


// ---------------------------------------------------------------------------
// Name:        acImageManagerModel::rowCount
// Description: Return the table row count
// Arguments:   const QModelIndex &parent
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        21/6/2012
// ---------------------------------------------------------------------------
int acImageManagerModel::rowCount(const QModelIndex& parent) const
{
    GT_UNREFERENCED_PARAMETER(parent);

    int retVal = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pImageManager != NULL)
    {
        if ((m_pImageManager->managerMode() == AC_MANAGER_MODE_STANDARD_ITEM) || (m_pImageManager->managerMode() == AC_MANAGER_MODE_TEXT_ITEM))
        {
            retVal = 1;
        }
        else if (m_pImageManager->managerMode() == AC_MANAGER_MODE_CUBEMAP_TEXTURE)
        {
            retVal = 3;
        }
        else if (m_pImageManager->managerMode() == AC_MANAGER_MODE_THUMBNAIL_VIEW)
        {
            int colCount = columnCount();
            float rowsCountF = 0;

            if (colCount > 0)
            {
                rowsCountF = (float)m_pImageManager->m_imageItems.size() / colCount;
            }

            retVal = ceil(rowsCountF);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acImageManagerModel::columnCount
// Description: Returns the columns amount for the view
// Arguments:   const QModelIndex &parent
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        12/7/2012
// ---------------------------------------------------------------------------
int acImageManagerModel::columnCount(const QModelIndex& parent) const
{
    GT_UNREFERENCED_PARAMETER(parent);

    int retVal = 1;
    GT_IF_WITH_ASSERT(m_pImageManager != NULL)
    {
        if ((m_pImageManager->m_managerMode == AC_MANAGER_MODE_STANDARD_ITEM) || (m_pImageManager->m_managerMode == AC_MANAGER_MODE_TEXT_ITEM))
        {
            retVal = 1;
        }
        else if (m_pImageManager->m_managerMode == AC_MANAGER_MODE_THUMBNAIL_VIEW)
        {
            float viewWidth = (float)m_pImageManager->size().width();
            float thumbWidth = (float)(AC_IMAGES_MANAGER_THUMBNAIL_SIZE + 2 * AC_IMAGES_MANAGER_THUMBNAIL_MARGIN);
            float amountOfCols = viewWidth / thumbWidth;
            retVal = floor(amountOfCols);
        }
        else if (m_pImageManager->m_managerMode == AC_MANAGER_MODE_CUBEMAP_TEXTURE)
        {
            retVal = 4;
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        acImageManagerModel::data
// Description: Return the data for the requested item
// Arguments:   const QModelIndex &index
//              int role
// Return Val:  QVariant
// Author:      Sigal Algranaty
// Date:        12/7/2012
// ---------------------------------------------------------------------------
QVariant acImageManagerModel::data(const QModelIndex& index, int role) const
{
    QVariant retVal;

    if (index.isValid())
    {
        if ((role == Qt::ToolTipRole) || (role == Qt::StatusTipRole))
        {
            GT_IF_WITH_ASSERT(m_pImageManager != NULL)
            {
                // Get the item for this index:
                acImageItem* pItem = m_pImageManager->getItem(index);

                if (pItem != NULL)
                {
                    if (!pItem->toolTipText().isEmpty())
                    {
                        retVal = pItem->toolTipText().asASCIICharArray();
                    }
                }
            }
        }

        else if (role == Qt::TextAlignmentRole)
        {
            return  Qt::AlignLeft + Qt::AlignVCenter;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acImageManagerModel::headerData
// Description: Header data for a certain item
// Arguments:   int section
//              Qt::Orientation orientation
//              int role
// Return Val:  QVariant
// Author:      Sigal Algranaty
// Date:        12/7/2012
// ---------------------------------------------------------------------------
QVariant acImageManagerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    GT_UNREFERENCED_PARAMETER(section);
    GT_UNREFERENCED_PARAMETER(orientation);
    GT_UNREFERENCED_PARAMETER(role);

    QVariant retVal;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acImageManagerModel::flags
// Description: Item flags
// Arguments:   const QModelIndex &index
// Return Val:  Qt::ItemFlags
// Author:      Sigal Algranaty
// Date:        12/7/2012
// ---------------------------------------------------------------------------
Qt::ItemFlags acImageManagerModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags retVal = Qt::ItemIsEnabled;

    if (index.isValid())
    {
        retVal = QAbstractTableModel::flags(index);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageManagerModel::updateModel
// Description: Refreshes the model
// Author:      Sigal Algranaty
// Date:        25/6/2012
// ---------------------------------------------------------------------------
void acImageManagerModel::updateModel()
{
    // Sanity check
    GT_IF_WITH_ASSERT(m_pImageManager != NULL)
    {
        int currentColCount = m_pImageManager->horizontalHeader()->count();
        int futureColCount = columnCount();

        if (currentColCount < futureColCount)
        {
            beginInsertColumns(QModelIndex(), currentColCount + 1, futureColCount);
            endInsertColumns();
            emit headerDataChanged(Qt::Horizontal, currentColCount + 1, futureColCount);
        }

        else if (currentColCount > futureColCount)
        {
            beginRemoveColumns(QModelIndex(), futureColCount + 1, currentColCount);
            endRemoveColumns();
            emit headerDataChanged(Qt::Horizontal, futureColCount + 1, currentColCount);
        }

        int currentRowCount = m_pImageManager->verticalHeader()->count();
        int futureRowCount = rowCount();

        if (currentRowCount < futureRowCount)
        {
            beginInsertRows(QModelIndex(), currentRowCount + 1, futureRowCount);
            endInsertRows();
            emit headerDataChanged(Qt::Vertical, currentRowCount + 1, futureRowCount);
        }

        else if (currentRowCount > futureRowCount)
        {
            beginRemoveRows(QModelIndex(), futureRowCount + 1, currentRowCount);
            endRemoveRows();
            emit headerDataChanged(Qt::Vertical, futureRowCount + 1, currentRowCount);
        }
    }
}
