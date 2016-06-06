//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acQTextFilterCtrl.cpp
///
//==================================================================================

//------------------------------ acQTextFilterCtrl.cpp ------------------------------

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <QTableWidget>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>

// Local:
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acQTextFilterCtrl.h>
#include <inc/acStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        acQTextFilterCtrl::acQTextFilterCtrl
// Description: Constructor
// Arguments:   QWidget* pParent
// Author:      Yoni Rabin
// Date:        24/4/2012
// ---------------------------------------------------------------------------
acQTextFilterCtrl::acQTextFilterCtrl(QWidget* pParent)
    : QLineEdit(pParent),
      m_defaultNoFilterString(AC_STR_breakpointsTextFilterInitialText),
      m_defaultNoFilterTruncString(AC_STR_breakpointsTextFilterInitialText),
      m_filtering(false),
      m_isFilterResultOutputEmpty(false)
{
    // Ugly hack to avoid scenario in which the user presses backspace on the default string
    int defaultStringLength = m_defaultNoFilterString.length();

    if (defaultStringLength > 1)
    {
        m_defaultNoFilterTruncString = m_defaultNoFilterString.left(defaultStringLength - 2);
    }

    m_paletteRedWhite.setColor(this->backgroundRole(), Qt::red);
    m_paletteRedWhite.setColor(QPalette::Text, Qt::white);

    m_paletteWhiteBlack.setColor(this->backgroundRole(), Qt::white);
    m_paletteWhiteBlack.setColor(QPalette::Text, Qt::black);

    m_paletteWhiteGray.setColor(this->backgroundRole(), Qt::white);
    m_paletteWhiteGray.setColor(QPalette::Text, acQLIST_EDITABLE_ITEM_COLOR);

    // Set the initial filter style:
    setInitializeFilterStyle();
}

// ---------------------------------------------------------------------------
// Name:        acQTextFilterCtrl::~acQTextFilterCtrl
// Description: Destructor
// Author:      Yoni Rabin
// Date:        24/4/2012
// ---------------------------------------------------------------------------
acQTextFilterCtrl::~acQTextFilterCtrl()
{
    // Clear the stored list content:
    m_listFullContent.deleteElementsAndClear();
}

// ---------------------------------------------------------------------------
// Name:        acQTextFilterCtrl::initialize
// Description: This function should be called after the list control is filled with all items.
//              It gives the filter control over the list content.
// Arguments:   acListCtrl* pList
// Author:      Yoni Rabin
// Date:        24/4/2012
// Implementation notes:
//   Copies the initial list content into the m_listFullContent vector.
// ---------------------------------------------------------------------------
void acQTextFilterCtrl::initialize(acListCtrl* pList)
{
    // Store the controlled list control pointer:
    m_pList = pList;

    // Iterate over the list items:
    int numItems = m_pList->rowCount();

    for (int i = 0; i < numItems; ++i)
    {
        // Get the current list item:
        QTableWidgetItem* pOrigListItem = m_pList->item(i, 0);
        GT_ASSERT(pOrigListItem != NULL);

        // Copy it into our new list:
        QTableWidgetItem* pListItemCopy = new QTableWidgetItem(*pOrigListItem);

        m_listFullContent.push_back(pListItemCopy);
    }

    // Check that we added all the elements:
    GT_ASSERT((unsigned int)numItems == m_listFullContent.size());

    connect(this, SIGNAL(textChanged(QString)), this, SLOT(onFilterTextChanged(QString)));
    connect(this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(onCursorPositionChanged(int, int)));
}

// ---------------------------------------------------------------------------
// Name:        acQTextFilterCtrl::terminate
// Description: This function might be called after finishing the use of the Text Filter.
//  It resets the _pControlledlistCtrl with its initial content.
// Author:      Yoni Rabin
// Date:        24/4/2012
// ---------------------------------------------------------------------------
void acQTextFilterCtrl::terminate()
{
    GT_IF_WITH_ASSERT(m_pList != NULL)
    {
        m_pList->clearList();

        // Iterate over the full list items:
        int numItems = m_listFullContent.size();

        for (int i = 0; i < numItems; ++i)
        {
            QTableWidgetItem* pItem = m_listFullContent[i];
            GT_IF_WITH_ASSERT(pItem != NULL)
            {
                m_pList->addRow(pItem->text(), pItem->data(Qt::UserRole));
            }
        }
    }

    setInitializeFilterStyle();
}

// ---------------------------------------------------------------------------
// Name:        acQTextFilterCtrl::onEmptyFilterResult
// Description: Is called when the output result of the filter is empty.
// Author:      Yoni Rabin
// Date:        24/4/2012
// ---------------------------------------------------------------------------
void acQTextFilterCtrl::onEmptyFilterResult()
{
    // Make the text white on red to indicate no item matching the filter:
    m_isFilterResultOutputEmpty = true;
    setPalette(m_paletteRedWhite);
}

// ---------------------------------------------------------------------------
// Name:        acQTextFilterCtrl::onNoneEmptyFilterResult
// Description: Is called when the output result of the filter is not empty
// Author:      Yoni Rabin
// Date:        24/4/2012
// ---------------------------------------------------------------------------
void acQTextFilterCtrl::onNoneEmptyFilterResult()
{
    // If the current filter result is not empty, no need to change style:
    if (m_isFilterResultOutputEmpty)
    {
        setPalette(m_paletteWhiteBlack);
        m_isFilterResultOutputEmpty = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        acQTextFilterCtrl::setInitializeFilterStyle
// Description: initialize the filter text and color
// Author:      Yoni Rabin
// Date:        24/4/2012
// ---------------------------------------------------------------------------
void acQTextFilterCtrl::setInitializeFilterStyle()
{
    // Only initialize filter text if the filter result is not empty:
    if (!m_isFilterResultOutputEmpty)
    {
        m_filterString.clear();
        setText(m_defaultNoFilterString);
        setPalette(m_paletteWhiteGray);
    }
}

// ---------------------------------------------------------------------------
// Name:        acQTextFilterCtrl::onFilterTextChanged
// Description: Populates the list with entries which match the filter
//              Called when the filtered text is changed
// Arguments:   QString filterText
// Return Val:  void
// Author:      Yoni Rabin
// Date:        25/4/2012
// Implementation Note: AND logic on the filter string tokens (delimited by " ,;")
// ---------------------------------------------------------------------------
void acQTextFilterCtrl::onFilterTextChanged(QString filterText)
{
    // If we are not in the middle of a previous operation:
    if (!m_filtering)
    {
        m_filtering = true;

        // Sanity check - do we have a list that we are controlling:
        if (m_pList != NULL)
        {
            //The output string to put in the filter:
            QString strOutFilter;

            // If the filter is empty - reset the default string:
            if (filterText.isEmpty())
            {
                m_filterString.clear();

                strOutFilter = m_defaultNoFilterString;
                setPalette(m_paletteWhiteGray);
            }
            // If the user hit backspace on the default string:
            else if (m_defaultNoFilterTruncString == filterText)
            {
                m_filterString.clear();
                strOutFilter.clear();
                setPalette(m_paletteWhiteBlack);
            }
            else if (m_filterString.isEmpty())
            {
                // If the default string was previously in the text box - just set the filter to the last char, e.g.  "filter here...A" becomes "A":
                if (filterText.startsWith(m_defaultNoFilterString))
                {
                    strOutFilter = filterText.mid(m_defaultNoFilterString.length());
                }
                else
                {
                    strOutFilter = filterText;
                }

                m_filterString = strOutFilter;
                setPalette(m_paletteWhiteBlack);
            }
            else
            {
                // copy the new string into the filter
                m_filterString = filterText;
                strOutFilter = filterText;
            }

            // Convert from QString to gtString:
            // Transform the filter text to lower case format:
            m_filterString = m_filterString.toLower();

            // Clear the current list control content:
            m_pList->clearList();

            // Split the filter text into tokens:
            QRegExp splitterRegex("[\\s ,;]+");
            QStringList patternsList = m_filterString.split(splitterRegex, QString::SkipEmptyParts);
            int numItems = m_listFullContent.size();

            // We separate to empty & non empty filters for performance considerations:
            // Check if we have any tokens to filter on:
            if (patternsList.isEmpty())
            {
                // For each item in our full content vector:
                for (int i = 0; i < numItems; ++i)
                {
                    QTableWidgetItem* pItem = m_listFullContent[i];
                    m_pList->addRow(pItem->text(), pItem->data(Qt::UserRole));
                }
            }
            else
            {
                // For each item in our full content vector:
                for (int i = 0; i < numItems; ++i)
                {
                    // Check if the item passes the filter:
                    bool currentItemPassesFilter = true;
                    QTableWidgetItem* pItem = m_listFullContent[i];
                    GT_IF_WITH_ASSERT(pItem != NULL)
                    {
                        QString itemName = pItem->text().toLower();

                        // For each token in the pattern string check if it exists in the entry (AND logic):
                        foreach (QString pattern, patternsList)
                        {
                            // If the current token is not a part of the string:
                            if (itemName.indexOf(pattern) < 0)
                            {
                                // The string failed to match the filter, no need to check the other tokens:
                                currentItemPassesFilter = false;
                                break;
                            }
                        }

                        // If all tokens match:
                        if (currentItemPassesFilter)
                        {
                            // Add the current item into the list control:
                            m_pList->addRow(pItem->text(), pItem->data(Qt::UserRole));
                        }
                    }
                }

                // If the list control is empty after applying the filter:
                if (m_pList->rowCount() == 0)
                {
                    onEmptyFilterResult();
                }
                else
                {
                    onNoneEmptyFilterResult();
                }
            }

            //Set the text:
            emit textChanged(strOutFilter);
            setText(strOutFilter);
        }

        m_filtering = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        acQTextFilterCtrl::onCursorPositionChanged
// Description: We need this event for the first time someone moves the cursor
//              to correctly get rid of the initial "filter here..." text
// Arguments:   int x
//              int y
// Author:      Yoni Rabin
// Date:        16/5/2012
// ---------------------------------------------------------------------------
void acQTextFilterCtrl::onCursorPositionChanged(int x, int y)
{
    GT_UNREFERENCED_PARAMETER(x);
    GT_UNREFERENCED_PARAMETER(y);

    // If we are not in the middle of a previous operation:
    if (!m_filtering)
    {
        m_filtering = true;

        // If the text is still set to "filter here...":
        if (m_defaultNoFilterString == text())
        {
            // Initialize the text box:
            m_filterString.clear();
            setText(m_filterString);
            setPalette(m_paletteWhiteBlack);
        }
        else if (text().isEmpty())
        {
            m_filterString.clear();
            setText(m_defaultNoFilterString);
            setPalette(m_paletteWhiteGray);
        }

        m_filtering = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        acQTextFilterCtrl::isDefaultString
// Description: Check if current string is the default string
// Return Val:  bool - Success / failure.
// Author:      Yoni Rabin
// Date:        27/6/2012
// ---------------------------------------------------------------------------
bool acQTextFilterCtrl::isDefaultString()
{
    QString origText = text();
    return origText == m_defaultNoFilterString;
}

// ---------------------------------------------------------------------------
// Name:        acQTextFilterCtrl::focusInEvent
// Description: Derived focus in method
// Arguments:   QFocusEvent* pEvent
// Return Val:  void
// Author:      Yoni Rabin
// Date:        27/6/2012
// ---------------------------------------------------------------------------
void acQTextFilterCtrl::focusInEvent(QFocusEvent* pEvent)
{
    QLineEdit::focusInEvent(pEvent);
    emit(focused(true));
}

// ---------------------------------------------------------------------------
// Name:        acQTextFilterCtrl::focusOutEvent
// Description: Derived focus out method
// Arguments:   QFocusEvent* pEvent
// Return Val:  void
// Author:      Yoni Rabin
// Date:        27/6/2012
// ---------------------------------------------------------------------------
void acQTextFilterCtrl::focusOutEvent(QFocusEvent* pEvent)
{
    QLineEdit::focusOutEvent(pEvent);

    if (text().isEmpty())
    {
        blockSignals(true);
        setPalette(m_paletteWhiteGray);
        setText(m_defaultNoFilterString);
        blockSignals(false);
    }

    emit(focused(false));
}



// ---------------------------------------------------------------------------
// Name:        acQTextFilterCtrl::clear
// Description: deletes the string and sets color to black
// Return Val:  void
// Author:      Yoni Rabin
// Date:        27/6/2012
// ---------------------------------------------------------------------------
void acQTextFilterCtrl::clear()
{
    blockSignals(true);
    setText("");
    setPalette(m_paletteWhiteBlack);
    m_filterString.clear();
    blockSignals(false);
}
