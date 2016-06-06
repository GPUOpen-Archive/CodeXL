//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acRibbonManager.cpp
///
//==================================================================================

//------------------------------ acRibbonManager.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QVBoxLayout>

// Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// Local
#include <AMDTApplicationComponents/inc/acStringConstants.h>
#include <AMDTApplicationComponents/Include/acRibbonManager.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acSplitter.h>
#include <AMDTApplicationComponents/Include/acNavigationChart.h>

static const int RANGE_BOUNDING_LINE_WIDTH = 2;

#define RIBBON_WIDTH 2
#define CLOSED_HEIGHT 16 // height of the image 16 pixels
#define BANNER_SPACING 6
#define MINIMUM_BANNER_OPEN 16

#define LAYOUT_WIDTH 2

#define GRAY_ALPHA 150
#define AC_RIBBON_BANNER_NAME "Banner"
#define TIMELINE_LEGEND_WIDTH 160
#define TIMELINE_RIGHT_MARGIN 10

#define AC_Str_SplitterGradientStyle "QSplitter::handle { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #e0e0e0, stop: 0.4 #d0d0d0, stop: 0.5 #adadad, stop: 0.6 #d0d0d0, stop: 1 #e0e0e0); }"
static const QString& AC_RIBBON_TOOLTIP_BORDER_COLOR = "rgb(185, 185, 185)";
static const QString& AC_RIBBON_TOOLTIP_LINE_COLOR = "rgba(185, 185, 185, 255)";
static const QString& AC_RIBBON_TOOLTIP_BG_COLOR = "rgba(231, 232, 236, 220)";

// global service function for the filters
acRibbonManager* GetManager(QObject* pObject)
{
    acRibbonManager* pManager = nullptr;
    QWidget* pParent = qobject_cast<QWidget*>(pObject->parent());

    while (pParent != nullptr)
    {
        pManager = qobject_cast<acRibbonManager*>(pParent);

        if (pManager != nullptr)
        {
            break;
        }

        pParent = qobject_cast<QWidget*>(pParent->parent());
    }

    return pManager;
}

bool acRibbonTooltipEventFilter::eventFilter(QObject* object, QEvent* event)
{
    if (event != nullptr && object != nullptr)
    {
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::Wheel)
        {
            // let the parent ribbon manager take some extra action before doing the normal action
            // find the manager to execute the action
            acRibbonManager* pManager = GetManager(object);

            if (pManager != nullptr)
            {
                pManager->PassMouseClickBelowToolTip(event);

                // do not handle this event the tooltip control has nothing to do with the mouse click in VS
                return true;
            }
        }
    }

    return QObject::eventFilter(object, event);
}

bool acRibbonEventFilter::eventFilter(QObject* object, QEvent* event)
{
    if (event != nullptr && object != nullptr)
    {
        if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonDblClick || (event->type() == QEvent::Leave))
        {
            // let the parent ribbon manager take some extra action before doing the normal action
            // find the manager to execute the action
            acRibbonManager* pManager = GetManager(object);

            // get index of the ribbon that has the mouse event
            if (pManager != nullptr)
            {
                if (event->type() == QEvent::MouseMove)
                {
                    // handle the mouse move only in the ribbons between the m_pControllerRibbon & m_pBoundFrameRibbonBottom
                    pManager->MouseMoveEvent(object, event);
                }
                else if (event->type() == QEvent::MouseButtonDblClick)
                {
                    // check if this is a banner object, if not do nothing
                    if (object != nullptr && object->objectName() == AC_RIBBON_BANNER_NAME)
                    {
                        // check that this is the left button double click
                        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(event);

                        if (pMouseEvent != nullptr)
                        {
                            if (pMouseEvent->button() == Qt::LeftButton)
                            {
                                pManager->MouseDblClickEvent(object);
                            }
                        }
                    }
                }
                else if (event->type() == QEvent::Leave)
                {
                    pManager->MouseLeaveEvent(object, event);
                }
            }
        }
    }

    return QObject::eventFilter(object, event);
}

acRibbonManagerButton::acRibbonManagerButton(QWidget* pOwningWidget) : QLabel(), m_isInMousePressEvent(false), m_mouseIsInLabel(false), m_pOwningWidget(pOwningWidget)
{

}
acRibbonManagerButton::~acRibbonManagerButton()
{

}

void acRibbonManagerButton::enterEvent(QEvent* pEvent)
{
    QLabel::enterEvent(pEvent);

    m_mouseIsInLabel = true;
}

void acRibbonManagerButton::leaveEvent(QEvent* pEvent)
{
    QLabel::leaveEvent(pEvent);

    m_mouseIsInLabel = false;
}

bool acRibbonManagerButton::event(QEvent* pEvent)
{
    if (pEvent->type() == QEvent::MouseButtonPress)
    {
        m_isInMousePressEvent = true;
    }
    else if (pEvent->type() == QEvent::MouseButtonRelease)
    {
        if (m_isInMousePressEvent && m_mouseIsInLabel)
        {
            emit ButtonPressed(m_pOwningWidget);
        }

        m_isInMousePressEvent = false;
    }

    return QLabel::event(pEvent);
}

acRibbonData::acRibbonData() : m_isVisible(true), m_pButton(nullptr), m_pRibbon(nullptr), m_lastOpenSize(0), m_isFixedSize(false)
{

}

acRibbonWrapper::acRibbonWrapper(QWidget* childWidget) : QWidget(), m_pChildWidget(childWidget)
{

}

acRibbonWrapper::~acRibbonWrapper() {}

QSize acRibbonWrapper::sizeHint() const
{
    QSize retVal = QWidget::sizeHint();

    GT_IF_WITH_ASSERT(m_pChildWidget != nullptr)
    {
        if (!m_pChildWidget->isVisible())
        {
            retVal = QSize(0, 0);
        }
    }
    return retVal;
}

acRibbonManager::acRibbonManager(QWidget* pParent) : QWidget(pParent),
    m_pMainLayout(nullptr),
    m_pWhiteSpace(nullptr),
    m_pSplitter(nullptr),
    m_pBoundFrameRibbonBottom(nullptr),
    m_pControllerRibbon(nullptr),
    m_pBoundsController(nullptr),
    m_lowBoundRange(0),
    m_highBoundRange(-1),
    m_pBoundingBoxLeftLine(nullptr),
    m_pBoundingBoxRightLine(nullptr),
    m_pBoundingBoxBottomLine(nullptr),
    m_pTooltipLine(nullptr),
    m_pEventFilter(nullptr),
    m_pTooltipEventFilter(nullptr),
    m_pScrollArea(nullptr)
{
    m_pMainLayout = new QVBoxLayout;
    m_pMainLayout->setContentsMargins(0, 0, 0, 0);

    m_pScrollArea = new QScrollArea(this);
    m_pSplitter = new acSplitter();
    m_pScrollArea->setWidget(m_pSplitter);
    m_pScrollArea->setGeometry(m_pSplitter->geometry());
    m_pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setContentsMargins(0, 0, 0, 0);

    m_pSplitter->setChildrenCollapsible(false);
    m_pSplitter->setOrientation(Qt::Vertical);
    m_pSplitter->setContentsMargins(0, 0, 0, 0);
    m_pMainLayout->addWidget(m_pScrollArea);
    setMinimumWidth(300);

    QString style3 = QString(AC_STR_BGWithParam).arg(acQAMD_ORANGE_PRIMARY_COLOUR.name());

    m_pBoundingBoxLeftLine = new QWidget(this);
    m_pBoundingBoxLeftLine->setGeometry(0, 0, 1, height());
    m_pBoundingBoxLeftLine->setStyleSheet(style3);
    m_pBoundingBoxLeftLine->setVisible(false);

    m_pBoundingBoxRightLine = new QWidget(this);
    m_pBoundingBoxRightLine->setGeometry(0, 0, 1, height());
    m_pBoundingBoxRightLine->setStyleSheet(style3);
    m_pBoundingBoxRightLine->setVisible(false);

    m_pBoundingBoxBottomLine = new QWidget(this);
    m_pBoundingBoxBottomLine->setGeometry(0, 0, 1, height());
    m_pBoundingBoxBottomLine->setStyleSheet(style3);
    m_pBoundingBoxBottomLine->setVisible(false);

    m_pTooltipLine = new QWidget(this);
    m_pTooltipLine->setGeometry(0, 0, 1, height());
    m_pTooltipLine->setVisible(false);
    QString tooltipStyle = QString(AC_STR_BGWithParam).arg(AC_RIBBON_TOOLTIP_LINE_COLOR);
    m_pTooltipLine->setStyleSheet(tooltipStyle);
    m_pTooltipLine->setVisible(false);
    m_pTooltipLine->setAttribute(Qt::WA_TransparentForMouseEvents);
    setLayout(m_pMainLayout);

    bool rc = connect(m_pSplitter, SIGNAL(splitterMoved(int, int)), this, SLOT(OnSplitterMoved(int, int)));
    GT_ASSERT(rc);
    rc = connect(m_pScrollArea->verticalScrollBar(), SIGNAL(sliderMoved(int)), this, SLOT(OnSliderMoved(int)));
    GT_ASSERT(rc);

    m_pEventFilter = new acRibbonEventFilter();
    m_pTooltipEventFilter = new acRibbonTooltipEventFilter();

    RecursiveInstallEventFilter(m_pTooltipLine);

    // in VS we need to have a special filter that capture the mouse click on the tooltip controls and pass it to the controls under
    // the tooltip controls for correct handling of focus events and wheel events
    if (GetExecutedApplicationType() == OS_VISUAL_STUDIO_PLUGIN_TYPE)
    {
        m_pTooltipLine->installEventFilter(m_pTooltipEventFilter);
    }

    // Create the white space and add it as the first ribbon (no caption or any thing)
    m_pWhiteSpace = new QWidget;
    m_pWhiteSpace->setMaximumHeight(0);
    m_pWhiteSpace->setMinimumHeight(0);
    AddRibbon(m_pWhiteSpace, "", 0, false, false);
}

acRibbonManager::~acRibbonManager()
{

}

void acRibbonManager::OnSliderMoved(int value)
{
    GT_UNREFERENCED_PARAMETER(value);

    RepositionBoundingFrame();
}

void acRibbonManager::resizeEvent(QResizeEvent* event)
{
    RecalcHeights(event->size().height());

    int numRibbons = m_ribbonDataVector.size();
    GT_IF_WITH_ASSERT(m_pSplitter != nullptr)
    {
        // get current size of all widgets and set it as the last heights for the current added widgets
        QList<int> currentSizes = m_pSplitter->sizes();

        for (int nWidget = 0; nWidget < numRibbons; nWidget++)
        {
            if (m_ribbonDataVector[nWidget].m_isVisible)
            {
                m_ribbonDataVector[nWidget].m_lastOpenSize = currentSizes[nWidget];
            }
        }
    }

    // set the signal with the new sizes
    int legendWidth = acScalePixelSizeToDisplayDPI(TIMELINE_LEGEND_WIDTH);
    int timelineWidth = event->size().width() - legendWidth - TIMELINE_RIGHT_MARGIN;

    if (timelineWidth < legendWidth)
    {
        legendWidth = (event->size().width() - 2 * TIMELINE_RIGHT_MARGIN) / 2;
        timelineWidth = legendWidth + TIMELINE_RIGHT_MARGIN;
    }

    QWidget::resizeEvent(event);

    emit SetElementsNewWidth(legendWidth, timelineWidth);

    RepositionBoundingFrame();

    HideTooltip();
}

void acRibbonManager::RecalcHeights(int height)
{
    int numRibbons = m_ribbonDataVector.size();
    GT_IF_WITH_ASSERT(m_pSplitter != nullptr)
    {
        int totalRatiosHeightOpen = 0;
        int totalRatiosHeightClosed = 0;
        int totalFixedSizeHeight = 0;
        int totalHeight = 0;
        // get current size of all widgets and set it as the last heights for the current added widgets
        QList<int> currentSizes = m_pSplitter->sizes();

        for (int nRibbon = 0; nRibbon < numRibbons ; nRibbon++)
        {
            int bannerHeight = m_ribbonDataVector[nRibbon].m_pButton != nullptr ? CLOSED_HEIGHT + BANNER_SPACING : 0;

            if (m_ribbonDataVector[nRibbon].m_isFixedSize || m_ribbonDataVector[nRibbon].m_ratioHeight > 0)
            {
                totalFixedSizeHeight += ((m_ribbonDataVector[nRibbon].m_isVisible ? m_ribbonDataVector[nRibbon].m_ratioHeight : 0) + bannerHeight);
            }
            else
            {
                if (m_ribbonDataVector[nRibbon].m_isVisible)
                {
                    totalRatiosHeightOpen += abs(m_ribbonDataVector[nRibbon].m_ratioHeight) + bannerHeight;
                }
                else
                {
                    totalRatiosHeightClosed += bannerHeight;
                }
            }

            totalHeight += currentSizes[nRibbon];
        }

        // the height that is left to distribute between not fixed height ribbons
        int totalHeightLeft = height - totalFixedSizeHeight;

        // if the height is enough to do anything then do it
        int heightToDistribute = totalHeightLeft > 0 ? totalHeightLeft : (totalRatiosHeightOpen + totalRatiosHeightClosed);

        for (int nRibbon = 0; nRibbon < numRibbons - 1; nRibbon++)
        {
            int bannerSize = m_ribbonDataVector[nRibbon].m_pButton != nullptr ? CLOSED_HEIGHT + BANNER_SPACING : 0;

            if (m_ribbonDataVector[nRibbon].m_isFixedSize || m_ribbonDataVector[nRibbon].m_ratioHeight > 0)
            {
                currentSizes[nRibbon] = (m_ribbonDataVector[nRibbon].m_isVisible ? m_ribbonDataVector[nRibbon].m_ratioHeight : 0) + bannerSize;
            }
            else
            {
                if (m_ribbonDataVector[nRibbon].m_isVisible)
                {
                    currentSizes[nRibbon] = heightToDistribute * (abs(m_ribbonDataVector[nRibbon].m_ratioHeight) + bannerSize) / totalRatiosHeightOpen;
                }
                else
                {
                    currentSizes[nRibbon] = bannerSize;
                }
            }
        }

        // the last ribbon (white space) has the initial size of 0
        currentSizes[numRibbons - 1] = 0;
        m_pSplitter->setSizes(currentSizes);

        if (totalHeightLeft > 0)
        {
            m_pSplitter->setMinimumHeight(0);
        }
        else
        {
            // the space we have is too small add the scroll area
            m_pSplitter->setMinimumHeight(totalFixedSizeHeight + totalRatiosHeightOpen + totalRatiosHeightClosed);
        }
    }
}

void acRibbonManager::AddRibbon(QWidget* pRibbon, QString ribbonName, int initialHeight, bool isFixedSize, bool hasCloseButton, bool isOpen)
{
    GT_IF_WITH_ASSERT(pRibbon != nullptr && m_pSplitter != nullptr)
    {
        QLabel* pButton = nullptr;
        QWidget* pWrapper = nullptr;
        QLabel* pLabel = nullptr;

        // this will holds the ribbon an banner or just the ribbon
        QVBoxLayout* pVBox = new QVBoxLayout;
        pVBox->setContentsMargins(0, 0, 0, 0);
        pRibbon->setContentsMargins(0, 0, 0, 0);

        if (hasCloseButton)
        {
            // create the layout that holds the button, ribbon name and the ribbon itself
            QHBoxLayout* pHBox = new QHBoxLayout;
            QPixmap buttonIcon;
            bool rc = acSetIconInPixmap(buttonIcon, isOpen ? AC_ICON_RIBBON_CLOSE : AC_ICON_RIBBON_OPEN, AC_16x16_ICON);
            GT_ASSERT(rc);
            pButton = new acRibbonManagerButton(pRibbon);
            pButton->setPixmap(buttonIcon);
            pButton->setContentsMargins(0, 0, 0, 0);
            pButton->setEnabled(true);
            int closeHeight = acScalePixelSizeToDisplayDPI(CLOSED_HEIGHT);
            pButton->setMaximumSize(closeHeight, closeHeight);
            pButton->setMinimumHeight(closeHeight);
            rc = connect(pButton, SIGNAL(ButtonPressed(QWidget*)), this, SLOT(OnButtonPressed(QWidget*)));

            // create the custom button
            pLabel = new QLabel(ribbonName);
            pLabel->setContentsMargins(0, 0, 0, 0);
            pLabel->setStyleSheet("QLabel { text-align:left; }");
            pLabel->setAttribute(Qt::WA_TranslucentBackground);
            pLabel->setWindowOpacity(true);
            pHBox->addWidget(pButton);
            pHBox->addWidget(pLabel);
            pHBox->setContentsMargins(0, 0, 0, 0);
            QWidget* pVWidget = new acRibbonWrapper(pRibbon);
            pVWidget->setLayout(pHBox);
            pVWidget->setStyleSheet("background-color:#dbdbdb; border: 1px solid #b9b9b9; ");
            pVWidget->setContentsMargins(0, 0, 0, 0);
            pVWidget->setMaximumHeight(closeHeight);

            // enable mouse tracking on the caption also
            pVWidget->setMouseTracking(true);
            pLabel->setMouseTracking(true);
            pButton->setMouseTracking(true);

            // give the banner objects name so they can be identified in the filter:
            pVWidget->setObjectName(AC_RIBBON_BANNER_NAME);
            pLabel->setObjectName(AC_RIBBON_BANNER_NAME);
            pButton->setObjectName(AC_RIBBON_BANNER_NAME);

            // Create the vertical layout with the hbox and the widget itself
            pVBox->addWidget(pVWidget);
        }

        // create a wrapper widget for the layout
        pVBox->addWidget(pRibbon);
        pWrapper = new QWidget();
        pWrapper->setContentsMargins(RIBBON_MANAGER_SPACE, 0, 0, 0);
        pWrapper->setLayout(pVBox);
        pWrapper->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        m_pTooltipLine->raise();

        acRibbonData ribbonData;
        ribbonData.m_isVisible = isOpen;
        ribbonData.m_pRibbon = pRibbon;
        ribbonData.m_pButton = pButton;
        ribbonData.m_pWrapper = pWrapper;
        ribbonData.m_pLabel = pLabel;
        ribbonData.m_isFixedSize = isFixedSize;
        ribbonData.m_isEnabled = true;
        ribbonData.m_ratioHeight = initialHeight;
        ribbonData.m_lastOpenSize = initialHeight;

        // add the ribbon before the white space widget
        int numWidgets = m_pSplitter->count();

        if (numWidgets > 0)
        {
            bool handleEnabled = true;
            // need to check if the last handle is disabled so it will be moved
            QSplitterHandle* pLastHandle = m_pSplitter->handle(m_ribbonDataVector.size() - 1);
            GT_IF_WITH_ASSERT(pLastHandle != nullptr)
            {
                handleEnabled = pLastHandle->isEnabled();
            }

            m_pSplitter->insertWidget(numWidgets - 1, pWrapper);
            // store the white space data
            acRibbonData oldWhiteSpace = m_ribbonDataVector[m_ribbonDataVector.size() - 1];
            m_ribbonDataVector[m_ribbonDataVector.size() - 1] = ribbonData;
            m_ribbonDataVector.push_back(oldWhiteSpace);

            QSplitterHandle* pBeforeLastHandle = m_pSplitter->handle(m_ribbonDataVector.size() - 2);
            GT_IF_WITH_ASSERT(pBeforeLastHandle != nullptr)
            {
                pBeforeLastHandle->setEnabled(handleEnabled);
                pLastHandle->setEnabled(true);
            }
        }
        else
        {
            m_pSplitter->addWidget(pWrapper);
            m_ribbonDataVector.push_back(ribbonData);
        }

        if (!isOpen && hasCloseButton)
        {
            pRibbon->setVisible(false);
            // hide the handle after the ribbon if the ribbon is by default closed
            QSplitterHandle* pHandle = m_pSplitter->handle(m_ribbonDataVector.size() - 1);
            GT_IF_WITH_ASSERT(pHandle != nullptr)
            {
                pHandle->setEnabled(false);
            }
        }

        RecursiveInstallEventFilter(pWrapper);

        int numRibbons = m_ribbonDataVector.size();

        // check if the ribbon above the added ribbon is fixed size then hide its splitter
        // start this check for more then 2 ribbons (do not count the white space as a ribbon)
        if (numRibbons > 2)
        {
            // need to bypass qt error of operator[]
            int currentRibbon = IndexOfRibbon(pRibbon);

            if (m_ribbonDataVector[currentRibbon - 1].m_isFixedSize)
            {

                QSplitterHandle* pHandle = m_pSplitter->handle(currentRibbon);

                if (pHandle != nullptr)
                {
                    pHandle->setEnabled(false);
                }
            }
        }
    }
}

void acRibbonManager::RecursiveInstallEventFilter(QWidget* pWidget)
{
    GT_IF_WITH_ASSERT(pWidget != nullptr)
    {
        // install the event filter on the widget
        pWidget->installEventFilter(m_pEventFilter);
        // then install on all the children if it has any:
        QList<QWidget*> childWidgets = pWidget->findChildren<QWidget*>();
        int numChildren = childWidgets.size();

        for (int nChild = 0; nChild < numChildren; nChild++)
        {
            // do not add to the banner qlabel and acRibbonManagerButton since there is a widget under them that will cover the case or we'll get double events
            if (childWidgets[nChild]->objectName() == AC_RIBBON_BANNER_NAME)
            {
                QString className(childWidgets[nChild]->metaObject()->className());

                if (className == "QWidget")
                {
                    childWidgets[nChild]->installEventFilter(m_pEventFilter);
                }
            }
            else
            {
                childWidgets[nChild]->installEventFilter(m_pEventFilter);
            }
        }
    }
}

int acRibbonManager::IndexOfRibbon(QWidget* pWidget)
{
    int retVal = -1;

    int numRibbons = m_ribbonDataVector.size();

    for (int nRibbon = 0; nRibbon < numRibbons; nRibbon++)
    {
        if (m_ribbonDataVector[nRibbon].m_pRibbon == pWidget)
        {
            retVal = nRibbon;
            break;
        }
    }

    return retVal;
}

void acRibbonManager::SetBoundFrameControlRibbons(acNavigationChart* pBoundWidgetController, QWidget* pControllerRibbon, QWidget* pRibbonBottom)
{
    // We can only set the bounds once since there is no sense in changing the bounds and there is no requirement for this at this stage
    // The setting of the filter will be done only for the ribbons between the bound and the bottom ribbon (including)
    // we also must set the bounds after all the ribbons were added and the lower bound ribbon is already defined
    GT_IF_WITH_ASSERT(pBoundWidgetController != nullptr && pControllerRibbon != nullptr && pRibbonBottom != nullptr)
    {
        int controllerIndex = IndexOfRibbon(pControllerRibbon);
        int bottomIndex = IndexOfRibbon(pRibbonBottom);

        GT_IF_WITH_ASSERT(m_pBoundsController == nullptr && controllerIndex != -1 && bottomIndex != -1 && controllerIndex < bottomIndex)
        {
            m_pBoundFrameRibbonBottom = pRibbonBottom;
            m_pControllerRibbon = pControllerRibbon;
            m_pBoundsController = pBoundWidgetController;

            bool rc = connect(pBoundWidgetController, SIGNAL(BoundingChanged(int&, int&)), this, SLOT(OnBoundChanged(int&, int&)));
            GT_ASSERT(rc);

            /// create a connection for the SetNewElementsWidth signal to handle the resize event
            for (int nRibbon = controllerIndex; nRibbon <= bottomIndex; nRibbon++)
            {
                GT_IF_WITH_ASSERT(m_ribbonDataVector[nRibbon].m_pRibbon != nullptr)
                {
                    bool rc = connect(this, SIGNAL(SetElementsNewWidth(int, int)), m_ribbonDataVector[nRibbon].m_pRibbon, SLOT(OnSetElementsNewWidth(int, int)));
                    GT_ASSERT(rc);
                }
            }

            // create qlabels. one for each ribbon (including the ribbon the contains the controller)
            for (int nRibbon = controllerIndex; nRibbon <= bottomIndex; nRibbon++)
            {
                QLabel* pLabel = new QLabel(this);
                QColor gray(acQAMD_GRAY3_COLOUR);
                gray.setAlpha(GRAY_ALPHA);

                QString style = QString(AC_STR_CSS_TooltipStyle).arg(AC_RIBBON_TOOLTIP_BORDER_COLOR).arg(AC_RIBBON_TOOLTIP_BG_COLOR);
                pLabel->setStyleSheet(style);
                pLabel->setTextFormat(Qt::RichText);
                pLabel->setVisible(false);
                m_tooltipLabels.push_back(pLabel);
                m_pTooltipLine->stackUnder(pLabel);

                // only in vs add the filter
                if (GetExecutedApplicationType() == OS_VISUAL_STUDIO_PLUGIN_TYPE)
                {
                    pLabel->installEventFilter(m_pTooltipEventFilter);
                }

            }
        }
    }
}

void acRibbonManager::RepositionBoundingFrame()
{
    bool showFrame = false;

    if (nullptr != m_pBoundsController && m_pBoundFrameRibbonBottom != nullptr && m_pSplitter != nullptr)
    {
        m_pBoundsController->GetBounds(m_lowBoundRange, m_highBoundRange);
        // these points are in the m_pBoundsController coords system
        QPoint leftPoint(m_lowBoundRange, m_pBoundsController->height());
        QPoint rightPoint(m_highBoundRange, m_pBoundsController->height());

        // convert the two points to the global coord and back to the ribbon manager system. then the X position cam be used
        QPoint globalLeft = m_pBoundsController->mapToGlobal(leftPoint);
        QPoint globalRight = m_pBoundsController->mapToGlobal(rightPoint);

        QPoint localLeft = mapFromGlobal(globalLeft);
        QPoint localRight = mapFromGlobal(globalRight);

        // use the top from the GetControllerAxisBottomCoordinate function and bottom from the GetRibbonBottomCoordinate
        // Get the position of the controlling ribbon
        QPoint leftTop(localLeft.x() - RIBBON_WIDTH / 2, GetControllerAxisBottomCoordinate());
        QPoint leftBottom(localLeft.x() + RIBBON_WIDTH / 2 - (1 - RIBBON_WIDTH % 2), GetRibbonBottomCoordinate(m_pBoundFrameRibbonBottom) - RIBBON_WIDTH + m_pSplitter->handleWidth());
        QPoint rightTop(localRight.x() - RIBBON_WIDTH / 2, GetControllerAxisBottomCoordinate());
        QPoint rightBottom(localRight.x() + RIBBON_WIDTH / 2 - (1 - RIBBON_WIDTH % 2), GetRibbonBottomCoordinate(m_pBoundFrameRibbonBottom) - 1 + m_pSplitter->handleWidth());


        m_pBoundingBoxLeftLine->setGeometry(QRect(leftTop, leftBottom));
        m_pBoundingBoxLeftLine->raise();

        m_pBoundingBoxRightLine->setGeometry(QRect(rightTop, rightBottom));
        m_pBoundingBoxRightLine->raise();

        m_pBoundingBoxBottomLine->setGeometry(QRect(leftBottom, rightBottom));
        m_pBoundingBoxBottomLine->raise();

        showFrame = true;
    }

    m_pBoundingBoxLeftLine->setVisible(showFrame);
    m_pBoundingBoxRightLine->setVisible(showFrame);
    m_pBoundingBoxBottomLine->setVisible(showFrame);
    update();
}

void acRibbonManager::OnBoundChanged(int& lowBound, int& highBound)
{
    m_lowBoundRange = lowBound;
    m_highBoundRange = highBound;

    RepositionBoundingFrame();
}

void acRibbonManager::OnButtonPressed(QWidget* pWidget)
{
    GT_IF_WITH_ASSERT(pWidget != nullptr)
    {
        int index = IndexOfRibbon(pWidget);

        GT_IF_WITH_ASSERT(index != -1)
        {
            if (m_ribbonDataVector[index].m_isEnabled)
            {
                ToggleRibbonState(index);
            }
        }
    }
}

void acRibbonManager::ToggleRibbonState(int index)
{
    int numRibbons = m_ribbonDataVector.size();
    GT_IF_WITH_ASSERT(index >= 0 && index < m_ribbonDataVector.size())
    {
        // store the last visible height for restoring
        int height = m_ribbonDataVector[index].m_pWrapper->height();

        if (m_ribbonDataVector[index].m_isVisible)
        {
            m_ribbonDataVector[index].m_lastOpenSize = height;
        }

        // set the visibility. the pointer of the widget can't be nullptr since it was equal to pWidget in order to get index != -1
        m_ribbonDataVector[index].m_isVisible = !m_ribbonDataVector[index].m_isVisible;
        m_ribbonDataVector[index].m_pRibbon->setVisible(m_ribbonDataVector[index].m_isVisible);

        if (m_ribbonDataVector[index].m_isVisible)
        {
            OpenRibbon(index);
        }
        else
        {
            CloseRibbon(index);
        }

        // set the pixmap correctly
        QPixmap actionIcon;
        bool rc = acSetIconInPixmap(actionIcon, m_ribbonDataVector[index].m_isVisible ? AC_ICON_RIBBON_CLOSE : AC_ICON_RIBBON_OPEN, AC_16x16_ICON);
        GT_ASSERT(rc);
        m_ribbonDataVector[index].m_pButton->setPixmap(actionIcon);

        // Set the splitter handle state
        if (index < numRibbons - 1)
        {
            // the splitter handle is the one above the ribbon (0 index is hidden)
            QSplitterHandle* pHandle = m_pSplitter->handle(index + 1);
            GT_IF_WITH_ASSERT(pHandle != nullptr)
            {
                pHandle->setEnabled(m_ribbonDataVector[index].m_isVisible && !m_ribbonDataVector[index].m_isFixedSize);
            }
        }

        CheckGroupEndRibbonsCase(m_ribbonDataVector[index].m_pRibbon);

        RepositionBoundingFrame();
    }
}

void acRibbonManager::CheckGroupEndRibbonsCase(QWidget* pWidget)
{
    int numRibbons = m_ribbonDataVector.size();
    GT_IF_WITH_ASSERT(pWidget != nullptr)
    {
        int index = IndexOfRibbon(pWidget);

        if (!m_ribbonDataVector[index].m_isVisible)
        {
            // check if from this ribbon until the end all ribbons are closed or fixed then the handle above should also be disabled
            // also do not take into account the white space ribbon
            int nRibbon = numRibbons - 2;

            for (; nRibbon >= 0; nRibbon--)
            {
                if (m_ribbonDataVector[nRibbon].m_isVisible && !m_ribbonDataVector[nRibbon].m_isFixedSize)
                {
                    break;
                }
            }

            if (index > nRibbon)
            {
                QSplitterHandle* pHandle = m_pSplitter->handle(nRibbon + 1);
                GT_IF_WITH_ASSERT(pHandle != nullptr)
                {
                    pHandle->setEnabled(false);
                }
            }

        }
        else
        {
            // if it was a part of a group of closed ribbons until the last ribbon and now we opened this ribbon which split the closed ribbons group into two, make sure the handle above the first
            // closed ribbon should be enabled
            // find the first visible ribbon that is not pWidget but we must first pass through pWidget
            // also do not take into account the white space ribbon
            bool foundWidget = false;
            int nRibbon = numRibbons - 2;

            for (; nRibbon >= 0; nRibbon--)
            {
                if (m_ribbonDataVector[nRibbon].m_isVisible && !m_ribbonDataVector[nRibbon].m_isFixedSize)
                {
                    if (!foundWidget && m_ribbonDataVector[nRibbon].m_pRibbon == pWidget)
                    {
                        foundWidget = true;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            if (foundWidget)
            {
                // we found the the correct order of closed ribbons and widget so enable the handle
                QSplitterHandle* pHandle = m_pSplitter->handle(nRibbon + 1);
                GT_IF_WITH_ASSERT(pHandle != nullptr)
                {
                    pHandle->setEnabled(true);

                    // After enabling part of an end group we need to check the group after that ribbon:
                    // also do not take into account the white space ribbon
                    if (index < numRibbons - 2)
                    {
                        CheckGroupEndRibbonsCase(m_ribbonDataVector[index + 1].m_pRibbon);
                    }
                }
            }
        }
    }
}

void acRibbonManager::CloseRibbon(int index)
{
    int numRibbons = m_ribbonDataVector.size();
    // Get the sizes of the current state
    GT_IF_WITH_ASSERT(m_pSplitter != nullptr && index < numRibbons)
    {
        QList<int> currentSizes = m_pSplitter->sizes();

        // add the difference of ribbon size to a different ribbon
        // first try to add it to a ribbon after the closed ribbon so the button will not move
        // but if that is not possible add it to a ribbon before
        int deltaSize = currentSizes[index] - CLOSED_HEIGHT;

        // save current sized and set the closed ribbon closed height
        m_ribbonDataVector[index].m_lastOpenSize = currentSizes[index];
        currentSizes[index] = CLOSED_HEIGHT;

        // hide the widget so splitter will allow us to really close it
        GT_IF_WITH_ASSERT(m_ribbonDataVector[index].m_pRibbon != nullptr)
        {
            m_ribbonDataVector[index].m_pRibbon->setVisible(false);
        }
        // add the delta to a widget after the current index but only to an open ribbon
        // we can always add the needed delta at the end to the white space widget
        bool deltaWasAdded = false;

        for (int nRibbon = index + 1; nRibbon < numRibbons && !deltaWasAdded; nRibbon++)
        {
            if (m_ribbonDataVector[nRibbon].m_isVisible && !m_ribbonDataVector[nRibbon].m_isFixedSize)
            {
                deltaWasAdded = AddHeightToRibbon(currentSizes, nRibbon, deltaSize);
            }
        }

        StoreLastOpenHeights(currentSizes);

        m_pSplitter->setSizes(currentSizes);
    }
}

bool acRibbonManager::AddHeightToRibbon(QList<int>& ribbonsSizes, int ribbonIndex, int& deltaSize)
{
    bool retVal = false;

    int numRibbons = m_ribbonDataVector.size();
    GT_IF_WITH_ASSERT(ribbonIndex >= 0 && ribbonIndex < numRibbons && ribbonsSizes.size() == numRibbons)
    {
        // if we are checking the last white space ribbon make sure it has the needed space
        if (ribbonIndex == numRibbons - 1)
        {
            GT_IF_WITH_ASSERT(m_ribbonDataVector[ribbonIndex].m_pRibbon != nullptr)
            {
                m_ribbonDataVector[ribbonIndex].m_pRibbon->setMaximumHeight(ribbonsSizes[ribbonIndex] + deltaSize);
                m_ribbonDataVector[ribbonIndex].m_pRibbon->setMinimumHeight(ribbonsSizes[ribbonIndex] + deltaSize);
            }
        }

        ribbonsSizes[ribbonIndex] += deltaSize;
        retVal = true;
    }

    return retVal;
}

void acRibbonManager::OpenRibbon(int index)
{
    int numRibbons = m_ribbonDataVector.size();
    // Get the sizes of the current state
    GT_IF_WITH_ASSERT(m_pSplitter != nullptr && index < numRibbons)
    {
        QList<int> currentSizes = m_pSplitter->sizes();

        // show the widget
        GT_IF_WITH_ASSERT(m_ribbonDataVector[index].m_pRibbon != nullptr)
        {
            m_ribbonDataVector[index].m_pRibbon->setVisible(true);
        }

        // when opening a ribbon try and take the space from the ribbons that come after the opened ribbon
        // there is always the white space ribbon to take from since it was used to add space when closing the ribbons
        int deltaSize = m_ribbonDataVector[index].m_lastOpenSize - currentSizes[index];
        currentSizes[index] = m_ribbonDataVector[index].m_lastOpenSize;

        for (int nRibbon = index + 1; nRibbon < numRibbons; nRibbon++)
        {
            if (m_ribbonDataVector[nRibbon].m_isVisible && !m_ribbonDataVector[nRibbon].m_isFixedSize)
            {
                GT_IF_WITH_ASSERT(m_ribbonDataVector[nRibbon].m_pWrapper != nullptr && m_ribbonDataVector[nRibbon].m_pRibbon != nullptr)
                {
                    int bannerHeight = m_ribbonDataVector[nRibbon].m_pButton != nullptr ? CLOSED_HEIGHT + BANNER_SPACING + MINIMUM_BANNER_OPEN : 0;
                    int minRibbonHeight = m_ribbonDataVector[nRibbon].m_pWrapper->minimumHeight() + bannerHeight;

                    // if it is the last white space ribbon take special action and make it small enough for the delta
                    if (nRibbon == numRibbons - 1)
                    {
                        minRibbonHeight = 0;
                    }

                    // do not change in open ribbons that have max size and if we are in the max size region.
                    int amountToReduce = (currentSizes[nRibbon] - minRibbonHeight > deltaSize) ? deltaSize : currentSizes[nRibbon] - minRibbonHeight;

                    if (nRibbon == numRibbons - 1)
                    {
                        m_ribbonDataVector[nRibbon].m_pRibbon->setMaximumHeight(currentSizes[nRibbon] - amountToReduce);
                        m_ribbonDataVector[nRibbon].m_pRibbon->setMinimumHeight(currentSizes[nRibbon] - amountToReduce);
                    }

                    if (currentSizes[nRibbon] > minRibbonHeight)
                    {
                        // remove from the size of the ribbon maximum a delta that will not reduce it below its min height
                        currentSizes[nRibbon] -= amountToReduce;
                        deltaSize -= amountToReduce;
                    }
                }
            }

            // if enough place was found in ribbons then stop
            if (deltaSize == 0)
            {
                break;
            }
        }

        // if not enough space was found look in the other direction
        if (deltaSize > 0)
        {
            for (int nRibbon = index - 1; nRibbon >= 0; nRibbon--)
            {
                if (m_ribbonDataVector[nRibbon].m_isVisible && !m_ribbonDataVector[nRibbon].m_isFixedSize)
                {
                    GT_IF_WITH_ASSERT(m_ribbonDataVector[nRibbon].m_pWrapper != nullptr)
                    {
                        int bannerHeight = m_ribbonDataVector[nRibbon].m_pButton != nullptr ? CLOSED_HEIGHT + BANNER_SPACING + MINIMUM_BANNER_OPEN : 0;
                        int minRibbonHeight = m_ribbonDataVector[nRibbon].m_pWrapper->minimumHeight() + bannerHeight;

                        if (currentSizes[nRibbon] > minRibbonHeight)
                        {
                            // remove from the size of the ribbon maximum a delta that will not reduce it below its min height
                            int amountToReduce = (currentSizes[nRibbon] - minRibbonHeight > deltaSize) ? deltaSize : currentSizes[nRibbon] - minRibbonHeight;
                            currentSizes[nRibbon] -= amountToReduce;
                            deltaSize -= amountToReduce;
                        }
                    }
                }

                // if enough place was found in ribbons then stop
                if (deltaSize == 0)
                {
                    break;
                }
            }
        }

        // if there is still some delta then we did not find enough room so do not increase this view too full size, try to reduce it by delta:
        if (deltaSize > 0)
        {
            if (currentSizes[index] > deltaSize)
            {
                currentSizes[index] -= deltaSize;
            }
        }

        StoreLastOpenHeights(currentSizes);

        m_pSplitter->setSizes(currentSizes);
    }
}

void acRibbonManager::OnSplitterMoved(int pos, int index)
{
    GT_UNREFERENCED_PARAMETER(pos);

    // need to check if the splitter after the index moved is closed then actually move the splitter after that one
    GT_IF_WITH_ASSERT(m_pSplitter != nullptr)
    {
        // set the sizes again based on the current new position
        QList<int> currentSizes = m_pSplitter->sizes();
        int numRibbons = m_ribbonDataVector.size();

        // we need to take action only if the handle moved was of a closed ribbon, a fixed ribbon size or an open ribbon when the size of the ribbon was
        // smaller of the banner

        // find the delta how much it was moved by looking the current size and
        // resize the current ribbon to the correct size
        // check if this is done for an open fixed size ribbon or a closed ribbon
        int deltaSize;

        if (!m_ribbonDataVector[index].m_isVisible)
        {
            deltaSize = currentSizes[index] - CLOSED_HEIGHT;
            currentSizes[index] = CLOSED_HEIGHT;
        }
        else
        {
            deltaSize = currentSizes[index] - m_ribbonDataVector[index].m_lastOpenSize;
        }

        // if the open ribbon reach the minimum banner size + x pixels, stop the pushing
        bool needToStop = false;
        int indexBefore = index - 1;
        int indexAfter = index;

        while (!m_ribbonDataVector[indexAfter].m_isVisible)
        {
            indexAfter++;
        }

        // the way splitter handles are enabled, the index before must be visible and the one after might be visible if not all the ribbons are closed and there is a problem
        GT_IF_WITH_ASSERT(indexBefore >= 0 && indexAfter < numRibbons)
        {
            // need to check two ribbons height, before and after the splitter handle moved
            int bannerSizeAfter = m_ribbonDataVector[indexAfter].m_pButton != nullptr ? CLOSED_HEIGHT + BANNER_SPACING + MINIMUM_BANNER_OPEN : 0;
            int bannerSizeBefore = m_ribbonDataVector[indexBefore].m_pButton != nullptr ? CLOSED_HEIGHT + BANNER_SPACING + MINIMUM_BANNER_OPEN : 0;

            if (currentSizes[indexBefore] - deltaSize < bannerSizeBefore || currentSizes[indexAfter] + deltaSize < bannerSizeAfter)
            {
                needToStop = true;

                for (int nRibbon = 0; nRibbon < numRibbons; nRibbon++)
                {
                    if (m_ribbonDataVector[nRibbon].m_isVisible)
                    {
                        currentSizes[nRibbon] = m_ribbonDataVector[nRibbon].m_lastOpenSize;
                    }
                    else
                    {
                        currentSizes[nRibbon] = CLOSED_HEIGHT + BANNER_SPACING + MINIMUM_BANNER_OPEN;
                    }
                }
            }
        }

        // if we can continue pushing, do it for all the closed ribbons from this point if the ribbon is closed
        if (!needToStop)
        {
            // move all closed ribbons under the current one if there are any until the first one that is not closed
            if (!m_ribbonDataVector[index].m_isVisible || m_ribbonDataVector[index].m_isFixedSize)
            {
                int nRibbon = index + 1;

                for (; nRibbon < numRibbons; nRibbon++)
                {
                    if (!m_ribbonDataVector[nRibbon].m_isVisible)
                    {
                        currentSizes[nRibbon] = CLOSED_HEIGHT;
                    }
                    else if (m_ribbonDataVector[nRibbon].m_isFixedSize)
                    {
                        currentSizes[nRibbon] = m_ribbonDataVector[nRibbon].m_lastOpenSize;
                    }
                    else
                    {
                        break;
                    }
                }

                if (nRibbon < numRibbons)
                {
                    bool deltaAdded = false;

                    for (; nRibbon < numRibbons; nRibbon++)
                    {
                        // at this point ignore minimum size. it needs to take into account qwidgets with no layouts and with layouts
                        if (0 < currentSizes[nRibbon] + deltaSize)
                        {
                            // increase/decrease size of the first visible ribbon by delta
                            currentSizes[nRibbon] += deltaSize;
                            deltaAdded = true;
                            break;
                        }
                    }

                    if (!deltaAdded)
                    {
                        // can't move splitter. copy old sizes
                        for (int nRibbon = 0; nRibbon < numRibbons; nRibbon++)
                        {
                            if (m_ribbonDataVector[nRibbon].m_isVisible)
                            {
                                currentSizes[nRibbon] = m_ribbonDataVector[nRibbon].m_lastOpenSize;
                            }
                        }
                    }
                }
            }
        }

        StoreLastOpenHeights(currentSizes);

        m_pSplitter->setSizes(currentSizes);

        RepositionBoundingFrame();
    }
}

void acRibbonManager::StoreLastOpenHeights(QList<int>& currentSizes)
{
    // copy the current sized to the last opened size if visible
    // update ratios
    int numRibbons = m_ribbonDataVector.size();

    for (int nRibbon = 0; nRibbon < numRibbons; nRibbon++)
    {
        if (m_ribbonDataVector[nRibbon].m_isVisible)
        {
            m_ribbonDataVector[nRibbon].m_lastOpenSize = currentSizes[nRibbon];
            m_ribbonDataVector[nRibbon].m_ratioHeight = (m_ribbonDataVector[nRibbon].m_ratioHeight > 0 ? currentSizes[nRibbon] : -currentSizes[nRibbon]);
        }
    }
}

int acRibbonManager::IsRibbonUnderPosition(QPoint& globalPos)
{
    int retVal = -1;

    // check if it is in one of the bounds ribbons
    int controllerIndex = IndexOfRibbon(m_pControllerRibbon);
    int bottomIndex = IndexOfRibbon(m_pBoundFrameRibbonBottom);

    for (int nRibbon = controllerIndex + 1; nRibbon <= bottomIndex; nRibbon++)
    {
        GT_IF_WITH_ASSERT(m_ribbonDataVector[nRibbon].m_pWrapper != nullptr)
        {
            QPoint localPos = m_ribbonDataVector[nRibbon].m_pWrapper->mapFromGlobal(globalPos);

            if (m_ribbonDataVector[nRibbon].m_pWrapper->rect().contains(localPos))
            {
                retVal = nRibbon;
                break;
            }
        }
    }

    return retVal;
}


void acRibbonManager::MouseMoveEvent(QObject* object, QEvent* event)
{
    // convert the position of the event to the x of the m_pBoundsController if there is any. from that convert to the time in the m_pBoundsController
    if (m_pBoundsController != nullptr && m_pBoundFrameRibbonBottom != nullptr && event != nullptr)
    {
        int controllerIndex = IndexOfRibbon(m_pControllerRibbon);
        int bottomIndex = IndexOfRibbon(m_pBoundFrameRibbonBottom);

        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(event);
        QPoint globalPos = pMouseEvent->globalPos();

        int foundRibbon = IsRibbonUnderPosition(globalPos);
        if (foundRibbon != -1)
        {

            GT_IF_WITH_ASSERT(pMouseEvent != nullptr && m_pTooltipLine != nullptr && m_pBoundingBoxBottomLine != nullptr)
            {
                // the x coord of the mouse event in the ribbon is the same for the m_pBoundsController so it can be used to get the time there:
                // convert the x coord from the qwidget that got the event to the m_pBoundsController coords for testing the range
                QPoint boundLocal = m_pBoundsController->mapFromGlobal(pMouseEvent->globalPos());
                QCPAxis* pAxis = m_pBoundsController->GetActiveRangeXAxis();

                if (pAxis != nullptr && boundLocal.x() >= pAxis->axisRect()->left() && boundLocal.x() <= pAxis->axisRect()->right())
                {
                    // convert the mouse x pos to the client coord
                    QPoint localPos = mapFromGlobal(pMouseEvent->globalPos());
                    double coordTime = pAxis->pixelToCoord(boundLocal.x());

                    ShowTooltip(object, event, coordTime, localPos, true);
                }
                else
                {
                    m_pTooltipLine->setVisible(false);

                    emit ShowTimeLine(false, 0);

                    // hide the labels
                    for (int nRibbon = controllerIndex; nRibbon < bottomIndex; nRibbon++)
                    {
                        GT_IF_WITH_ASSERT(m_tooltipLabels[nRibbon - controllerIndex] != nullptr)
                        {
                            m_tooltipLabels[nRibbon - controllerIndex]->setVisible(false);
                        }
                    }
                }
            }
        }
        else
        {
            HideTooltip();
        }
    }
}

void acRibbonManager::ShowTooltip(QObject* object, QEvent* event, double coordTime, QPoint& tooltipPos, bool shouldEmitSyncSignal)
{
    int controllerIndex = IndexOfRibbon(m_pControllerRibbon);
    int bottomIndex = IndexOfRibbon(m_pBoundFrameRibbonBottom);

    QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(event);

    int yPos = GetControllerAxisBottomCoordinate();
    QPoint leftTop(tooltipPos.x(), yPos);
    QPoint rightBottom(tooltipPos.x() + 1, GetRibbonBottomCoordinate(m_pBoundFrameRibbonBottom) - 1);

    m_pTooltipLine->setGeometry(QRect(leftTop, rightBottom));
    m_pTooltipLine->setVisible(true);

    if (shouldEmitSyncSignal)
    {
        emit ShowTimeLine(true, coordTime);
    }
    // set the labels text and position
    for (int nRibbon = controllerIndex; nRibbon <= bottomIndex; nRibbon++)
    {
        QLabel* pCurrentLabel = nullptr;
        GT_IF_WITH_ASSERT(nRibbon - controllerIndex >= 0 && nRibbon - controllerIndex < m_tooltipLabels.size())
        {
            pCurrentLabel = m_tooltipLabels[nRibbon - controllerIndex];
        }
        GT_IF_WITH_ASSERT(pCurrentLabel != nullptr)
        {
            // special case for the label of the controller text and y position
            if (nRibbon == controllerIndex)
            {
                pCurrentLabel->setText(m_pBoundsController->TimeToString(m_pBoundsController->GetNavigationUnitsX(), coordTime, true));
            }
            else
            {
                GT_IF_WITH_ASSERT(m_ribbonDataVector[nRibbon].m_pRibbon != nullptr)
                {
                    acIRibbonTime* pTooltipInterface = dynamic_cast<acIRibbonTime*>(m_ribbonDataVector[nRibbon].m_pRibbon);

                    if (pTooltipInterface != nullptr)
                    {
                        // check if the ribbon has the object that got the message as a child
                        bool hasObject = false;
                        QList<QObject*> childObjects = m_ribbonDataVector[nRibbon].m_pRibbon->findChildren<QObject*>();
                        // add the ribbon itself as an item to check
                        childObjects.push_back(m_ribbonDataVector[nRibbon].m_pRibbon);
                        int numObjects = childObjects.size();

                        for (int nObject = 0; nObject < numObjects; nObject++)
                        {
                            if (childObjects[nObject] == object)
                            {
                                hasObject = true;
                                break;
                            }
                        }

                        QString tooltip;

                        if (hasObject)
                        {
                            tooltip = pTooltipInterface->GetTooltip(coordTime, pMouseEvent);
                        }
                        else
                        {
                            tooltip = pTooltipInterface->GetTooltip(coordTime, nullptr);
                        }

                        pCurrentLabel->setText(tooltip);
                    }
                }
            }

            pCurrentLabel->adjustSize();
            int labelWidth = pCurrentLabel->width();
            int labelHeight = pCurrentLabel->height();
            int labelYPos = yPos;

            if (nRibbon > controllerIndex)
            {
                GT_IF_WITH_ASSERT(m_ribbonDataVector[nRibbon].m_pWrapper != nullptr)
                {
                    labelYPos = m_ribbonDataVector[nRibbon].m_pWrapper->geometry().top() + CLOSED_HEIGHT;
                }
            }

            // Calculate the label x coordinate. If the label has enough space to be painted with the line coord as
            // left coordinate, draw it there. If not, calculate the left most coordinate that leaves enough space for
            // the label
            int rightMostPos = m_pBoundingBoxBottomLine->geometry().right();
            int leftPos = tooltipPos.x();

            if ((leftPos + labelWidth) > rightMostPos)
            {
                leftPos = leftPos - labelWidth;
            }

            pCurrentLabel->setGeometry(leftPos, labelYPos, labelWidth, labelHeight);

            pCurrentLabel->setVisible(m_ribbonDataVector[nRibbon].m_isVisible && !pCurrentLabel->text().isEmpty());
        }
    }
}

void acRibbonManager::MouseLeaveEvent(QObject* object, QEvent* event)
{
    GT_UNREFERENCED_PARAMETER(object);
    GT_UNREFERENCED_PARAMETER(event);

    bool hideTooltip = true;

    QPoint globalPos = QCursor::pos();

    if (IsRibbonUnderPosition(globalPos) != -1)
    {
        hideTooltip = false;
    }

    if (hideTooltip)
    {
        HideTooltip();
    }
}

void acRibbonManager::MouseDblClickEvent(QObject* object)
{
    // get the ribbon the object click is referencing to
    int numRibbons = m_ribbonDataVector.size();
    bool ribbonFound = false;

    for (int nRibbon = 0; nRibbon < numRibbons && !ribbonFound; nRibbon++)
    {
        GT_IF_WITH_ASSERT(m_ribbonDataVector[nRibbon].m_pWrapper != nullptr)
        {
            QList<QWidget*> childWidgets = m_ribbonDataVector[nRibbon].m_pWrapper->findChildren<QWidget*>();
            int numChildren = childWidgets.size();

            for (int nWidget = 0; nWidget < numChildren; nWidget++)
            {
                if (childWidgets[nWidget] == object)
                {
                    if (m_ribbonDataVector[nRibbon].m_isEnabled)
                    {
                        ToggleRibbonState(nRibbon);
                    }

                    ribbonFound = true;
                    break;
                }
            }
        }
    }
}

int acRibbonManager::GetRibbonBottomCoordinate(QWidget* pRibbon)
{
    // by default return the bottom of the ribbon manager
    int retVal = geometry().bottom();

    GT_IF_WITH_ASSERT(pRibbon != nullptr && m_pScrollArea != nullptr && m_pScrollArea->verticalScrollBar() != nullptr)
    {
        int ribbonIndex = IndexOfRibbon(pRibbon);

        GT_IF_WITH_ASSERT(ribbonIndex != -1)
        {
            GT_IF_WITH_ASSERT(m_ribbonDataVector[ribbonIndex].m_pWrapper != nullptr)
            {
                QRect wrapperRect = m_ribbonDataVector[ribbonIndex].m_pWrapper->geometry();
                retVal = wrapperRect.bottom();

                if (m_pScrollArea->verticalScrollBar()->isVisible())
                {
                    retVal -= m_pScrollArea->verticalScrollBar()->value();
                }
            }
        }
    }

    return retVal;
}

int acRibbonManager::GetControllerAxisBottomCoordinate()
{
    int retVal = 0;

    if (m_pBoundsController != nullptr && m_pScrollArea != nullptr && m_pScrollArea->verticalScrollBar() != nullptr)
    {
        QCPAxis* pCurrentAxis = m_pBoundsController->GetActiveRangeXAxis();
        GT_IF_WITH_ASSERT(pCurrentAxis != nullptr)
        {
            QCPAxisRect* axisRect = pCurrentAxis->axisRect();
            QRect outerRect = axisRect->outerRect();
            QPoint bottomLeft = outerRect.bottomLeft();
            int padding = pCurrentAxis->padding() + (pCurrentAxis->tickLengthOut() > pCurrentAxis->subTickLengthOut() ? pCurrentAxis->tickLengthOut() : pCurrentAxis->subTickLengthOut());
            bottomLeft.setY(bottomLeft.y() - padding);
            QPoint bottomLeftGlobal = m_pBoundsController->mapToGlobal(bottomLeft);

            QPoint bottomLeftLocal = mapFromGlobal(bottomLeftGlobal);

            retVal = bottomLeftLocal.y();

            if (m_pScrollArea->verticalScrollBar()->isVisible())
            {
                retVal -= m_pScrollArea->verticalScrollBar()->value();
            }
        }
    }

    return retVal;
}

void acRibbonManager::HideTooltip()
{
    emit ShowTimeLine(false, 0);

    GT_IF_WITH_ASSERT(m_pTooltipLine != nullptr)
    {
        m_pTooltipLine->setVisible(false);
    }
    int numLabels = m_tooltipLabels.size();

    for (int nLabel = 0; nLabel < numLabels; nLabel++)
    {
        GT_IF_WITH_ASSERT(m_tooltipLabels[nLabel] != nullptr)
        {
            m_tooltipLabels[nLabel]->setVisible(false);
        }
    }
}

void acRibbonManager::ChangeRibbonName(QWidget* pRibbon, QString newRibbonName)
{
    int numRibbons = m_ribbonDataVector.size();
    bool ribbonFound = false;

    for (int nRibbon = 0; nRibbon < numRibbons && !ribbonFound; nRibbon++)
    {
        if (m_ribbonDataVector[nRibbon].m_pRibbon == pRibbon)
        {
            // get the qlabel and change its name
            GT_IF_WITH_ASSERT(m_ribbonDataVector[nRibbon].m_pLabel != nullptr)
            {
                m_ribbonDataVector[nRibbon].m_pLabel->setText(newRibbonName);
            }
        }
    }
}

void acRibbonManager::ChangeRibbonInitialHeight(QWidget* pRibbon, int newInitialHeight)
{
    int numRibbons = m_ribbonDataVector.size();
    bool ribbonFound = false;

    for (int nRibbon = 0; nRibbon < numRibbons && !ribbonFound; nRibbon++)
    {
        if (m_ribbonDataVector[nRibbon].m_pRibbon == pRibbon)
        {
            m_ribbonDataVector[nRibbon].m_ratioHeight = newInitialHeight;
        }
    }
}

void acRibbonManager::LastRibbonCheck(bool openAction)
{
    // count the number of open ribbons, if there is only one mark it based on the action that is being done
    int numOpenRibbons = 0;
    int openRibbonIndex = -1;
    int numRibbons = m_ribbonDataVector.size();
    int disabledRibbon = -1;

    for (int nRibbon = 0; nRibbon < numRibbons; nRibbon++)
    {
        // check only the ribbons that have close button
        if (m_ribbonDataVector[nRibbon].m_pButton != nullptr && m_ribbonDataVector[nRibbon].m_isVisible)
        {
            numOpenRibbons++;
            openRibbonIndex = nRibbon;
        }

        if (!m_ribbonDataVector[nRibbon].m_isEnabled)
        {
            disabledRibbon = nRibbon;
        }
    }

    // if there is only one ribbon change its status based on the action done
    // or there is a disabled ribbon and we are in an open action
    if ((1 == numOpenRibbons && !openAction) || (disabledRibbon != -1 && openAction))
    {
        QPixmap actionIcon;
        bool rc = acSetIconInPixmap(actionIcon, openAction ? AC_ICON_RIBBON_CLOSE : AC_ICON_RIBBON_CLOSE_DISABLED, AC_16x16_ICON);
        GT_ASSERT(rc);
        GT_IF_WITH_ASSERT(m_ribbonDataVector[openRibbonIndex].m_pButton)
        {
            m_ribbonDataVector[openRibbonIndex].m_pButton->setPixmap(actionIcon);
            m_ribbonDataVector[openRibbonIndex].m_isEnabled = openAction;
        }
    }
}

void acRibbonManager::PassMouseClickBelowToolTip(QEvent* pEvent)
{
    QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
    QWheelEvent* pWheelEvent = dynamic_cast<QWheelEvent*>(pEvent);

    if (pMouseEvent != nullptr || pWheelEvent != nullptr)
    {
        // find the control under the tooltip and create a new event and pass it to the control
        // check only in the tooltip area
        QPoint globalPos;
        if (pMouseEvent != nullptr)
        {
            globalPos = pMouseEvent->globalPos();
        }
        else
        {
            globalPos = pWheelEvent->globalPos();
        }

        int foundRibbon = IsRibbonUnderPosition(globalPos);

        if (foundRibbon != -1)
        {
            GT_IF_WITH_ASSERT(m_ribbonDataVector[foundRibbon].m_pRibbon != nullptr)
            {
                if (pMouseEvent != nullptr)
                {
                    // set the focus to the control under the tooltip control
                    m_ribbonDataVector[foundRibbon].m_pRibbon->setFocus();
                }
                else
                {
                    QPoint eventPosLocal = m_ribbonDataVector[foundRibbon].m_pRibbon->mapFromGlobal(pWheelEvent->globalPos());

                    QWheelEvent* pCorrectedWheelEvent = new QWheelEvent(eventPosLocal, pWheelEvent->globalPos(),
                        pWheelEvent->pixelDelta(), pWheelEvent->angleDelta(), pWheelEvent->angleDelta().ry(), pWheelEvent->orientation(), pWheelEvent->buttons(), Qt::NoModifier, pWheelEvent->phase());
                    qApp->notify(m_ribbonDataVector[foundRibbon].m_pRibbon, pCorrectedWheelEvent);
//                    m_ribbonDataVector[foundRibbon].m_pRibbon->event(pEvent);
                }
            }
        }
    }
}

void acRibbonManager::OnShowTimeLine(bool visible, double timePos)
{
    if (m_pBoundsController != nullptr)
    {
        // tooltip pos in the bounds controller coords, need to convert to global and then to local coord
        QPoint tooltipPos = QPoint(m_pBoundsController->GetActiveRangeXAxis()->coordToPixel(timePos), 0);
        QPoint globalPos = m_pBoundsController->mapToGlobal(tooltipPos);
        QPoint localPos = mapFromGlobal(globalPos);

        if (visible)
        {
            ShowTooltip(nullptr, nullptr, timePos, localPos, false);
        }
        else
        {
            HideTooltip();
        }
    }
    else
    {
        HideTooltip();
    }
}
