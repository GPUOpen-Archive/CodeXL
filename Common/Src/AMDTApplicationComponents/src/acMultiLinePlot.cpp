//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acMultiLinePlot.cpp
///
//==================================================================================


// Warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra
#include  <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acMultiLinePlot.h>
#include <AMDTApplicationComponents/Include/acMultiLinePlotData.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>

#define AC_PLOT_TICK_LEN_OUT 2
#define AC_PLOT_TICK_LEN_IN  1
#define AC_PLOT_TICK_LABEL_PADDING 10
#define AC_PLOT_DEFAULT_ROW_HEIGHT 20

#define AC_PLOT_DEFAULT_X_RANGE      2
#define AC_PLOT_DEFAULT_Y_RANGE     6
#define AC_PLOT_DEFAULT_Y_STEPS_NUM 5

#define AC_PLOT_SELECTIONTOLERANCE_PIXELS 15

QString acMultiLinePlot::m_sTooltipHTMLStyle = "";

void acSingleLineGraph::draw(QCPPainter* painter)
{
    if (!m_isHidden)
    {
        acVectorLineGraph::draw(painter);
    }
}

acMultiLinePlot::acMultiLinePlot(bool isCumulative, bool isLastGrapTotal)
{
    m_unitsStr = "";
    m_pPlotInfoTable = NULL;
    m_isCumulative = isCumulative;
    m_isLastGraphTotal = isLastGrapTotal;
    m_isLastGraphTotal = false;
    m_pValuesTextLabel = NULL;
    m_addRemoveRowNum = 0;
    m_pCustomPlot = new acCustomPlot(NULL);

    m_pCustomPlot->xAxis->setRange(0, AC_PLOT_DEFAULT_X_RANGE);
    m_pCustomPlot->yAxis->setRange(0, AC_PLOT_DEFAULT_Y_RANGE);
    m_pCustomPlot->yAxis->setAutoTickLabels(true);
    m_pCustomPlot->yAxis->setAutoTickCount(AC_PLOT_DEFAULT_Y_STEPS_NUM);

    m_pCustomPlot->xAxis->setAutoTickLabels(true);
    m_pCustomPlot->xAxis->setAutoTicks(true);
    m_pCustomPlot->xAxis->setSubTickLength(0);

    m_pCustomPlot->xAxis->setTickLabelPadding(AC_PLOT_TICK_LABEL_PADDING);
    m_pCustomPlot->xAxis->setTickLengthOut(AC_PLOT_TICK_LEN_OUT);
    m_pCustomPlot->xAxis->setTickLengthIn(AC_PLOT_TICK_LEN_IN);

    m_pCustomPlot->yAxis->setAutoTickStep(true);

    // how many ticks between each jump
    m_pCustomPlot->yAxis->setAutoSubTicks(false);
    m_pCustomPlot->yAxis->setSubTickCount(0);
    m_pCustomPlot->yAxis->setTickLabelColor(Qt::black);

    connect(m_pCustomPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(OnPlotHovered(QMouseEvent*)));

    m_pCustomPlot->xAxis->rescale();
    m_pCustomPlot->setContentsMargins(0, 0, 0, 0);

    m_pCustomPlot->setSelectionTolerance(AC_PLOT_SELECTIONTOLERANCE_PIXELS);
}


void acMultiLinePlot::InitPlotAxesLabels(const QString& xLabel, const QString& yLabel)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pCustomPlot != NULL)
    {
        QFont font;
        font.setBold(true);

        // xAxis Label
        if (!xLabel.isEmpty())
        {
            m_pCustomPlot->xAxis->setLabel(xLabel);
            m_pCustomPlot->xAxis->setLabelColor(Qt::black);
            m_pCustomPlot->xAxis->setLabelFont(font);
        }

        // yAxis Label
        if (!yLabel.isEmpty())
        {
            m_pCustomPlot->yAxis->setLabel(yLabel);
            m_pCustomPlot->yAxis->setLabelColor(Qt::black);
            m_pCustomPlot->yAxis->setLabelFont(font);
        }
    }
}


void acMultiLinePlot::ClearGraph()
{
    int graphsCount = m_pCustomPlot->graphCount();

    if (graphsCount > 0)
    {
        m_pGraphsVec.clear();
        m_pCustomPlot->clearGraphs();
    }
}

void acMultiLinePlot::InitPlotWithEmptyGraphs(QVector<acMultiLinePlotItemData*>& pCurrentGraphItemsData, bool allowReplot)
{
    ClearGraph();

    GT_IF_WITH_ASSERT(NULL != m_pCustomPlot && pCurrentGraphItemsData.count() > 0)
    {
        QColor graphColor;

        for (int i = 0; i < pCurrentGraphItemsData.count(); i++)
        {
            acSingleLineGraph* pCurrentGraph = InitPlotInner(pCurrentGraphItemsData[i], graphColor);
            GT_IF_WITH_ASSERT(NULL != pCurrentGraph)
            {
                QPen pen(graphColor);

                // if cumulative graph
                if (m_isCumulative)
                {
                    pCurrentGraph->setAntialiasedFill(false);

                    if ((!m_isLastGraphTotal) || (i != pCurrentGraphItemsData.count() - 1))
                    {
                        // last graph is the total - don't fill between this graph to the prev graph:
                        // set fill between this graph and the prev graph
                        pCurrentGraph->setBrush(QBrush(graphColor, Qt::SolidPattern));
                        pCurrentGraph->setPen(graphColor);
                        pCurrentGraph->setSelectedBrush(QBrush(graphColor, Qt::SolidPattern));

                        QColor alphaColor = Qt::black;
                        alphaColor.setAlpha(20);
                        pCurrentGraph->setSelectedPen(alphaColor);
                    }
                    else
                    {
                        pCurrentGraph->setSelectedPen(QPen(graphColor, 2));
                    }

                    // set fill between graphs  (no need for the first graph)
                    if (i > 0)
                    {
                        pCurrentGraph->setChannelFillGraph(m_pGraphsVec[i - 1]);
                    }
                }
                else
                {
                    pCurrentGraph->setPen(pen);
                    pCurrentGraph->setSelectedPen(pen);
                }
            }
        }

        m_pCustomPlot->setInteraction(QCP::iSelectPlottables);

        // add connect to selected graph changed (for choosing the appropriate legend item)
        bool rc = connect(m_pCustomPlot, SIGNAL(selectionChangedByUser()), this, SLOT(OnGraphSelectionChanged()), Qt::UniqueConnection);
        GT_ASSERT(rc);

        if (allowReplot)
        {
            Replot();
        }
    }
}

void acMultiLinePlot::InitPlotGraphs(QVector<acMultiLinePlotItemData*>& pCurrentGraphItemsData, bool allowReplot)
{
    ClearGraph();
    double maxValue = 0;
    double minNegValue = 0;

    GT_IF_WITH_ASSERT(NULL != m_pCustomPlot && pCurrentGraphItemsData.count() > 0)
    {
        QVector<double> cumulativeValVec;

        // this vector will not incclude the disabled graphs values
        QVector<double> totalValVec;
        QColor graphColor;
        int graphsNum = pCurrentGraphItemsData.count();

        for (int i = 0; i < graphsNum; i++)
        {
            // init new line graph and add it to plot
            acMultiLinePlotItemData* pCurrentGraphItem = pCurrentGraphItemsData[i];
            acSingleLineGraph* pCurrentGraph = InitPlotInner(pCurrentGraphItem, graphColor);

            GT_IF_WITH_ASSERT(NULL != pCurrentGraph)
            {
                QPen pen(graphColor);

                if (m_isCumulative)
                {
                    // The selection color should be the graph color:
                    pCurrentGraph->setSelectedBrush(QBrush(graphColor, Qt::SolidPattern));

                    QColor alphaColor = Qt::black;
                    alphaColor.setAlpha(20);
                    pCurrentGraph->setSelectedPen(alphaColor);
                }
                else
                {
                    pCurrentGraph->setPen(pen);

                    // Set the selected pen:
                    pCurrentGraph->setSelectedPen(pen);
                }

                // if cumulative graph
                if (m_isCumulative)
                {
                    pCurrentGraph->setAntialiasedFill(false);

                    if (i == 0)
                    {
                        // init cumulative vector with 0 value
                        for (int k = 0; k < pCurrentGraphItem->GetValueVec().size(); k++)
                        {
                            cumulativeValVec << 0;
                            totalValVec << 0;
                        }
                    }

                    // cumulate into vector
                    for (int j = 0; j < pCurrentGraphItem->GetValueVec().size(); j++)
                    {
                        cumulativeValVec[j] += (pCurrentGraphItem->GetValueVec())[j];

                        // sum this graph value into totalValVec only if it is not disabled
                        if (!pCurrentGraph->ShouldBeDisabled())
                        {
                            totalValVec[j] += (pCurrentGraphItem->GetValueVec())[j];
                            maxValue = totalValVec[j] > maxValue ? totalValVec[j] : maxValue;
                        }
                    }

                    if (m_isLastGraphTotal)
                    {
                        // last graph is the total - dont fill between this graph to the prev graph
                        if (i != pCurrentGraphItemsData.count() - 1)
                        {
                            // set fill between this graph and the prev graph
                            pCurrentGraph->setBrush(QBrush(graphColor, Qt::SolidPattern));
                            pCurrentGraph->setPen(graphColor);
                        }
                        else
                        {
                            pCurrentGraph->setPen(pen);
                            pCurrentGraph->setSelectedPen(QPen(graphColor, 2));
                        }
                    }
                    else
                    {
                        pCurrentGraph->setBrush(QBrush(graphColor, Qt::SolidPattern));
                        pCurrentGraph->setPen(graphColor);
                    }

                    // set fill between graphs  (no need for the first graph)
                    if (i > 0)
                    {
                        pCurrentGraph->setChannelFillGraph(m_pGraphsVec[i - 1]);
                    }

                    if (m_isLastGraphTotal && i == graphsNum - 1)
                    {
                        // if the total graph - set the new total values as the graph data
                        // not including the disabled graphs values
                        pCurrentGraph->SetVectorData(pCurrentGraphItem->GetKeyVec(), totalValVec);
                    }
                    else
                    {
                        // set the new cumulated values as the graph data
                        pCurrentGraph->SetVectorData(pCurrentGraphItem->GetKeyVec(), cumulativeValVec);
                    }
                }
                else
                {
                    QVector<double>& valueVec = pCurrentGraphItem->GetValueVec();
                    int count = valueVec.size();

                    // in regular graphs (not cumulative - just set data as read from DB
                    pCurrentGraph->SetVectorData(pCurrentGraphItem->GetKeyVec(), valueVec);

                    for (int nVal = 0; nVal < count; nVal++)
                    {
                        maxValue = valueVec[nVal] > maxValue ? valueVec[nVal] : maxValue;

                        // set the min value only if it is negative - otherwise the lower range value will be set to 0
                        if (valueVec[nVal] < 0)
                        {
                            minNegValue = minNegValue < valueVec[nVal] ? minNegValue : valueVec[nVal];
                        }
                    }
                }
            }
        }

        m_pCustomPlot->setInteraction(QCP::iSelectPlottables);

        // add connect to selected graph changed (for choosing the appropriate legend item)
        connect(m_pCustomPlot, SIGNAL(selectionChangedByUser()), this, SLOT(OnGraphSelectionChanged()), Qt::UniqueConnection);

        // reset YAxis range if needed
        ResetGraphYRange(ceil(maxValue), floor(minNegValue));

        if (allowReplot)
        {
            Replot();
        }
    }
}

acSingleLineGraph* acMultiLinePlot::InitPlotInner(acMultiLinePlotItemData* pCurrentGraphItem, QColor& graphColor)
{
    acSingleLineGraph* pCurrentGraph = NULL;

    GT_IF_WITH_ASSERT(NULL != m_pCustomPlot && NULL != pCurrentGraphItem)
    {
        // init new line graph and add it to plot
        pCurrentGraph = new acSingleLineGraph(m_pCustomPlot->xAxis, m_pCustomPlot->yAxis, pCurrentGraphItem->ShouldGraphBeDisabled());

        m_pCustomPlot->addPlottable(pCurrentGraph);

        pCurrentGraph->setSelectable(true);

        // Get the graph color:
        graphColor = pCurrentGraphItem->GetColor();

        // trim the spaces from the name
        pCurrentGraph->setName(pCurrentGraphItem->GetGraphName().trimmed());

        // set graph shown as default
        pCurrentGraph->SetHidden(false);
        pCurrentGraph->SetColor(graphColor);

        pCurrentGraph->SetDescription(pCurrentGraphItem->GetDescription());

        // add the new line graph to the TimeLineGraph graphs list
        m_pGraphsVec.append(pCurrentGraph);

    }

    return pCurrentGraph;
}

void acMultiLinePlot::AddDataToTimeLineGraph(const double key, const QVector<double>& valVec,
                                             const double xRangeStart, const double xRangeEnd,
                                             bool removeOld, bool allowReplot)
{
    // check num of data values == num og single graphs
    int numOfVlaues = valVec.count();
    int maxValue = 0;
    int minNegValue = 0;

    if (numOfVlaues == m_pGraphsVec.count() &&
        numOfVlaues > 0)
    {
        double shownGraphsCumulativeValue = 0; // cumulative value of shown graphs
        double cumulativeValue = 0;            // cumulative value of all graphs
        int totalGraphIndex = -1;   // index of total graph

        if (m_isLastGraphTotal)
        {
            totalGraphIndex = valVec.count() - 1;
        }

        for (int i = 0; i < valVec.count(); i++)
        {
            if (m_isCumulative)
            {
                if (i != totalGraphIndex)
                {
                    double value = valVec[i];

                    // if the graph is not shown don't add its value to the total sum
                    if (!m_pGraphsVec[i]->ShouldBeDisabled())
                    {
                        if (!m_pGraphsVec[i]->IsHidden())
                        {
                            // add graph value to cumulative value of all graphs and to shown graphs
                            // don't add the hidden graph value to the shownGraphscumulativeValue
                            shownGraphsCumulativeValue += valVec[i];
                            value = shownGraphsCumulativeValue;
                            maxValue = value > maxValue ? value : maxValue;
                        }

                        // don't add the disabled graphs value to the cumulative value
                        cumulativeValue += valVec[i];
                    }

                    m_pGraphsVec[i]->AddDataToVector(key, value);
                }
                else
                {
                    // for total graph - get the cumulative value of all graphs
                    cumulativeValue += valVec[i];
                    m_pGraphsVec[i]->AddDataToVector(key, cumulativeValue);
                }
            }
            else // non-cumulative graph
            {
                maxValue = valVec[i] > maxValue ? valVec[i] : maxValue;

                // set the min value only if it is negative - otherwise the lower range value will be set to 0
                if (valVec[i] < 0)
                {
                    minNegValue = minNegValue < valVec[i] ? minNegValue : valVec[i];
                }

                m_pGraphsVec[i]->AddDataToVector(key, valVec[i]);
            }

            GT_UNREFERENCED_PARAMETER(removeOld);
            /* if (removeOld)
             {
                 m_pGraphs[i]->removeDataBefore(key - graphXRange);
             }*/
        }

        m_pCustomPlot->xAxis->setRange(xRangeStart, xRangeEnd);

        // reset YAxis range if needed
        ResetGraphYRange(ceil(maxValue), floor(minNegValue));

        if (allowReplot)
        {
            Replot();
        }
    }
}

void acMultiLinePlot::HideSingleLineGraphFromCumulative(int selectedGraphIndex, bool hideGrpah)
{
    int prevGraphIndex = -1;
    acVectorLineGraph* tmpGraph = dynamic_cast<acVectorLineGraph*>(m_pCustomPlot->graph(selectedGraphIndex));
    GT_IF_WITH_ASSERT(NULL != tmpGraph)
    {
        acDataVector* dataVec = tmpGraph->VectorData();
        acDataVector* prevDataVec = NULL;

        //step 1: get the index of the shown graph that comes right below the one we like to remove/add
        for (int i = selectedGraphIndex - 1; i >= 0; i--)
        {
            if (!m_pGraphsVec[i]->IsHidden())
            {
                prevGraphIndex = i;
                break;
            }
        }

        // step 2: if it is not the first graph shown (first on the bottom of the plot)
        // get the previous (the graphs below it) y values
        if (selectedGraphIndex > 0 && prevGraphIndex > -1)
        {
            tmpGraph = dynamic_cast<acVectorLineGraph*>(m_pCustomPlot->graph(prevGraphIndex));
            GT_IF_WITH_ASSERT(NULL != tmpGraph)
            {
                prevDataVec = tmpGraph->VectorData();
            }
            else
            {
                prevGraphIndex = -1;
            }
        }

        // step 3: get the removed graph (single line graph) y values
        QVector<float> removedGraphVals;
        unsigned int vecSize = dataVec->size();

        // if it is the first graph (on the bottom of the plot) - get its value
        if (selectedGraphIndex == 0 || prevGraphIndex == -1)
        {
            for (unsigned int i = 0; i < vecSize; i++)
            {
                removedGraphVals << ((*dataVec)[i]).Value();
            }
        }
        else
        {
            // if not we need to reduce the graphs below it from its value to get its real value
            // (as in cumulative graph the values are also cumulative)
            // for showing (not hiding) the graph we the the oposide thing (add not reduce)
            GT_IF_WITH_ASSERT(NULL != prevDataVec && NULL != dataVec)
            {
                for (unsigned int i = 0; i < vecSize; i++)
                {
                    // if not first - get its value minus/plus the value of shown graph below
                    if (hideGrpah)
                    {
                        removedGraphVals << ((*dataVec)[i]).Value() - ((*prevDataVec)[i]).Value();
                        ((*dataVec)[i]).SetValue(((*dataVec)[i]).Value() - ((*prevDataVec)[i]).Value());
                    }
                    else // show graph
                    {
                        removedGraphVals << ((*dataVec)[i]).Value();
                        ((*dataVec)[i]).SetValue(((*dataVec)[i]).Value() + ((*prevDataVec)[i]).Value());
                    }
                }
            }
        }

        // step 4: update all graphs data. update graphs values that above the removed/added graph
        // when the graph hidden - we need to reduce its y values from all graphs above it.
        // when the graph is added - we need to add its y values from all graphs above it.
        int numGraphs = m_pCustomPlot->graphCount();

        for (int graphIndex = selectedGraphIndex + 1; graphIndex < numGraphs; graphIndex++)
        {
            // if the graph above not hidden
            if (!m_pGraphsVec[graphIndex]->IsHidden())
            {
                tmpGraph = dynamic_cast<acVectorLineGraph*>(m_pCustomPlot->graph(graphIndex));

                GT_IF_WITH_ASSERT(NULL != tmpGraph)
                {
                    dataVec = tmpGraph->VectorData();

                    // all graphs data vector is in the same size (same keys)
                    GT_IF_WITH_ASSERT(NULL != dataVec && vecSize == dataVec->size())
                    {
                        for (unsigned int index = 0; index < vecSize; index++)
                        {
                            if (hideGrpah)
                            {
                                ((*dataVec)[index]).SetValue(((*dataVec)[index]).Value() - removedGraphVals[index]);
                            }
                            else
                            {
                                ((*dataVec)[index]).SetValue(((*dataVec)[index]).Value() + removedGraphVals[index]);
                            }
                        }
                    }
                }
            }
        }

        // step 5: order fill
        OrderGraphChannelFill();
    }
}

void acMultiLinePlot::OrderGraphChannelFill()
{
    int numGraphs = m_pGraphsVec.size();
    int lastgraphshown = -1;

    for (int i = 0; i < numGraphs; i++)
    {
        if (m_pGraphsVec[i]->IsHidden())
        {
            continue;
        }
        else
        {
            m_pGraphsVec[i]->setAntialiasedFill(false);

            if (lastgraphshown == -1)
            {
                m_pGraphsVec[i]->setChannelFillGraph(0);
            }
            else
            {
                m_pGraphsVec[i]->setChannelFillGraph(m_pGraphsVec[lastgraphshown]);
            }

            lastgraphshown = i;
        }
    }

    //m_pCustomPlot->replot();
}

double acMultiLinePlot::getGraphValueByKey(acSingleLineGraph* graph, const double& key) const
{
    double retVal = -1;
    int keyIndex = 0;

    if (graph->GetNearestIndexToKey(key, -1, keyIndex))
    {
        retVal = graph->VectorData()->at(keyIndex).Value();
    }

    return retVal;
}

void acMultiLinePlot::SetInfoTableBySpecificTimePoint(const double& timeKey)
{
    QVector<double> valueVec;

    int numOfGraphs = m_pGraphsVec.count();

    if (numOfGraphs > 0)
    {
        double value, prevValue = 0, graphValue;

        if (m_isCumulative)
        {
            // in cumulative graph - reduce the button graph value
            for (int i = 0; i < numOfGraphs; i++)
            {
                value = getGraphValueByKey(m_pGraphsVec[i], timeKey);
                graphValue = value;

                // if the graph is hidden don't change its value, and don't reduce its value from the graphs above
                // same for disabled graphs
                if (!m_pGraphsVec[i]->IsHidden() &&
                    !m_pGraphsVec[i]->ShouldBeDisabled() &&
                    (!m_isLastGraphTotal || (i != (numOfGraphs - 1))))
                {
                    value -= prevValue;
                    prevValue = graphValue;
                }

                valueVec.push_front(value);
            }
        }
        else
        {
            // regular line graph
            for (int i = 0; i < numOfGraphs; i++)
            {
                valueVec.push_front(getGraphValueByKey(m_pGraphsVec[i], timeKey));
            }
        }

        for (int i = 0; i < numOfGraphs; i++)
        {
            // Check if the value type is int/double and set the string into the legend
            QString tmpStr = (m_valuesType == GRAPHVALUESTYPE_DOUBLE) ? QString::number(valueVec[i], 'f', 2) : QString::number((int)valueVec[i]);
            m_pPlotInfoTable->setItemText(i, AC_PLOT_LEGEND_VALUE_COL_INDEX, tmpStr);
        }
    }
}

void acMultiLinePlot::GetGraphsNames(QVector<QString>& names) const
{
    foreach (acSingleLineGraph* graph, m_pGraphsVec)
    {
        names << graph->name();
    }
}

int acMultiLinePlot::GetGraphIndexByName(const QString& name) const
{
    int retVal = -1;
    GT_IF_WITH_ASSERT(!m_pGraphsVec.isEmpty())
    {
        for (int i = 0; i < m_pGraphsVec.size(); i++)
        {
            QString nn = m_pGraphsVec[i]->name();

            if (nn == name)
            {
                retVal = i;
                break;
            }
        }
    }
    return retVal;
}

void acMultiLinePlot::HideGraph(const QString& name, bool hide)
{
    int index = GetGraphIndexByName(name);

    if (index != -1 && index >= 0 && index < m_pGraphsVec.size())
    {
        HideGraph(index, hide);
    }
}

void acMultiLinePlot::HideGraph(int i, bool hide)
{
    m_pGraphsVec[i]->SetHidden(hide);

    if (m_isCumulative)
    {
        HideSingleLineGraphFromCumulative(i, hide);
    }

}

void acMultiLinePlot::InitPlotInfoTable()
{
    m_pPlotInfoTable = new acListCtrl(NULL, AC_PLOT_DEFAULT_ROW_HEIGHT, true);
    m_pPlotInfoTable->SetSelectionMode(QAbstractItemView::SingleSelection);

    // remove delete action from legend context menu
    m_pPlotInfoTable->setEnableRowDeletion(false);

    bool rcConnect = connect(m_pPlotInfoTable, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(OnLegendTableItemActivate(QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    QStringList strList;
    strList << "Counter";
    strList << "Value " + m_unitsStr;

    m_pPlotInfoTable->setColumnCount(2);
    m_pPlotInfoTable->initHeaders(strList, false);

    ResetPlotInfoTable();
}

void acMultiLinePlot::DisableCountersThatShuldBeDisabled()
{
    unsigned int count = m_pGraphsVec.count();

    for (unsigned int graphIndex = 0; graphIndex < count; graphIndex++)
    {
        // if child graph un-check its item in the legend
        if (m_pGraphsVec[graphIndex]->ShouldBeDisabled())
        {
            int row = count - 1 - graphIndex;
            GT_IF_WITH_ASSERT(row < m_pPlotInfoTable->rowCount())
            {
                // set item un-checked
                QTableWidgetItem* pItem = m_pPlotInfoTable->item(row, LEGEND_CHECKBOX_COLUMN);
                pItem->setCheckState(Qt::Unchecked);

                // hide disabled graph
                HideGraph(graphIndex, true);

                // set disabled
                m_pPlotInfoTable->SetRowEnabled(row, false);
            }
        }
    }
}

void acMultiLinePlot::ResetPlotInfoTable()
{
    GT_IF_WITH_ASSERT(NULL != m_pCustomPlot)
    {
        int numGrpahs = m_pCustomPlot->graphCount();
        int graphIndex = numGrpahs - 1;
        QStringList strList;
        GT_IF_WITH_ASSERT(NULL != m_pPlotInfoTable)
        {
            m_pPlotInfoTable->clearList();

            disconnect(m_pPlotInfoTable, SIGNAL(itemSelectionChanged()), this, SLOT(OnInfoTableSelectedItemChanged()));
            disconnect(m_pPlotInfoTable, SIGNAL(cellChanged(int, int)), this, SLOT(OnCellClicked(int, int)));

            for (int i = 0; i < numGrpahs; i++)
            {
                strList.clear();
                // legend rows order is reverted to graphs order
                strList << m_pGraphsVec[graphIndex]->name();
                strList << "";

                m_pPlotInfoTable->addRow(strList, NULL, true, Qt::Checked);

                // set legend row description
                QTableWidgetItem* pRowItem = m_pPlotInfoTable->item(i, 0);
                QString tooltip;

                acWrapAndBuildFormattedTooltip(m_pGraphsVec[graphIndex]->name(), m_pGraphsVec[graphIndex]->GetGraphDescription(), tooltip);
                pRowItem->setToolTip(tooltip);

                graphIndex--;
            }

            // Add icons for each of the graphs:
            SetRowIcons(numGrpahs);

            // Resize columns:
            ResizeColumns();

            // Add "Double-click to add or remove counters" line:
            m_addRemoveRowNum = m_pPlotInfoTable->rowCount();
            QStringList newItemStrings;
            newItemStrings << AC_STR_CounterSelectionString;
            newItemStrings << "";


            m_pPlotInfoTable->addRow(newItemStrings, NULL);
            QTableWidgetItem* pFirstItemInNewLine = m_pPlotInfoTable->item(m_addRemoveRowNum, 0);
            GT_IF_WITH_ASSERT(pFirstItemInNewLine != NULL)
            {
                // Set it to be gray:
                pFirstItemInNewLine->setTextColor(Qt::black);
                m_pPlotInfoTable->setSpan(m_addRemoveRowNum, 0, 1, 2);
            }

            if (m_isCumulative)
            {
                // disable child counters
                DisableCountersThatShuldBeDisabled();
            }

            connect(m_pPlotInfoTable, SIGNAL(itemSelectionChanged()), this, SLOT(OnInfoTableSelectedItemChanged()), Qt::UniqueConnection);
            connect(m_pPlotInfoTable, SIGNAL(cellChanged(int, int)), this, SLOT(OnCellClicked(int, int)), Qt::UniqueConnection);
        }
    }
}

void acMultiLinePlot::HideGraphsLegend(int i, bool hide)
{
    if (hide)
    {
        m_pPlotInfoTable->hideRow(i);
    }
    else
    {
        m_pPlotInfoTable->showRow(i);
    }
}

void acMultiLinePlot::ChangeGraphRangeByMidPoint(double userMidIndex, double userRange)
{
    bool foundRange;
    QCPRange range = m_pGraphsVec[0]->GetKeyRange(foundRange);

    if (foundRange)
    {
        // get mid point
        double low = range.lower;
        double up = range.upper;

        if (userMidIndex > 0 &&
            userMidIndex > low &&
            userMidIndex < up)
        {
            // if requested range is bigger then all range - fix it
            if (userRange > up - low)
            {
                userRange = up - low;
            }

            double userStartrPoint = userMidIndex - userRange / 2;

            // if lower point is out of range
            if (userStartrPoint < low)
            {
                userStartrPoint = low;
            }

            double userEnfPoint = userStartrPoint + userRange;

            // if upper point is out of range
            if (userEnfPoint > up)
            {
                userEnfPoint = up;
                //fix low point
                userStartrPoint = userEnfPoint - userRange;
            }

            m_pCustomPlot->xAxis->setRange(userStartrPoint, userEnfPoint);

            //m_pCustomPlot->replot();
        }
    }
}

void acMultiLinePlot::ChangeGraphRangeByBothPoints(double startPoint, double endPoint)
{
    // *** this function does not do replot to the graph.
    // *** is case replot needed, call Replot()
    m_pCustomPlot->xAxis->setRange(startPoint, endPoint);
}

void acMultiLinePlot::OnLegendTableItemActivate(QTableWidgetItem* pItem)
{
    GT_IF_WITH_ASSERT((m_pPlotInfoTable != NULL) && (pItem != NULL))
    {
        // If the item in the last row:
        if (pItem->row() == m_pPlotInfoTable->rowCount() - 1)
        {
            emit AddRemoveActivated();
        }
    }
}

void acMultiLinePlot::ResetGraphYRange(int maxBarVal, int minBarNegValue)
{
    // change yAxis upper bound when exceeding 90% of upper
    GT_IF_WITH_ASSERT(NULL != m_pCustomPlot)
    {
        double origUpperVal = m_pCustomPlot->yAxis->range().upper;
        double axisTopRange = origUpperVal * 0.9;
        int newUpperRange = 0;

        // calculate new upper range
        if (maxBarVal > axisTopRange)
        {
            int divVal = 100;

            if (maxBarVal < 20)
            {
                divVal = 5;
            }
            else if (maxBarVal < 100)
            {
                divVal = 20;
            }
            else if (maxBarVal > 100 && maxBarVal < 1000)
            {
                divVal = 50;
            }

            // find the next step grater then the max
            while (newUpperRange <= maxBarVal)
            {
                newUpperRange += divVal;
            }
        }

        double origLowerVal = m_pCustomPlot->yAxis->range().lower;
        double axisBottomRange = origLowerVal * 0.9;
        int newLowerRange = 0;

        // calculate new lower range
        if (minBarNegValue < 0 && minBarNegValue < axisBottomRange)
        {
            int divVal = 100;

            if (minBarNegValue > -100)
            {
                divVal = 20;
            }
            else if (minBarNegValue < -100 && maxBarVal > -1000)
            {
                divVal = 50;
            }

            // find the next step smaller then the min
            while (newLowerRange >= minBarNegValue)
            {
                newLowerRange -= divVal;
            }
        }

        // change only if current upper value is less then new calculated value, or,
        // if current lower value is less then new calculated value
        if (newUpperRange > origUpperVal || newLowerRange < origLowerVal)
        {
            // set to y axis to new range
            newUpperRange = newUpperRange > origUpperVal ? newUpperRange : origUpperVal;
            newLowerRange = newLowerRange < origLowerVal ? newLowerRange : origLowerVal;

            m_pCustomPlot->yAxis->setRange(newLowerRange, newUpperRange);

            // fix tick steps
            int tickNum = m_pCustomPlot->yAxis->autoTickCount();
            int range = newUpperRange - newLowerRange;

            if ((range % tickNum) == 0)
            {
                m_pCustomPlot->yAxis->setTickStep(range / tickNum);
                m_pCustomPlot->yAxis->setAutoTickStep(false);
            }
        }
    }
}

QCPRange acMultiLinePlot::GetGraphRange() const
{
    QCPRange range;

    if (NULL != m_pCustomPlot)
    {
        range = m_pCustomPlot->xAxis->range();
    }

    return range;
}

void acMultiLinePlot::SetRowIcons(int numGrpahs)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pPlotInfoTable != NULL)
    {
        int graphIndex = numGrpahs - 1;

        for (int i = 0; i < numGrpahs; i++)
        {
            QTableWidgetItem* pItem = m_pPlotInfoTable->item(i, 0);
            GT_IF_WITH_ASSERT(pItem != NULL)
            {
                // Draw an icon:
                QPixmap* pIconPixmap = new QPixmap(10, 10);
                QPainter* pIconPainter = new QPainter(pIconPixmap);
                pIconPainter->fillRect(0, 0, 10, 10, Qt::white);
                pIconPainter->fillRect(0, 0, 10, 10, m_pGraphsVec[graphIndex]->GetColor());
                graphIndex--;
                pItem->setIcon(QIcon(*pIconPixmap));
            }
        }
    }
}

void acMultiLinePlot::OnCellClicked(int row, int column)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pPlotInfoTable != NULL)
    {
        if (m_pGraphsVec.size() > row &&
            row != m_pPlotInfoTable->rowCount() - 1 &&
            column == 0)
        {
            disconnect(m_pPlotInfoTable, SIGNAL(cellChanged(int, int)), this, SLOT(OnCellClicked(int, int)));

            // graph order is reverted to legend order
            int graphIndex = m_pGraphsVec.count() - 1 - row;

            // Get the clicked item from table:
            QTableWidgetItem* pItem = m_pPlotInfoTable->item(row, 0);
            GT_IF_WITH_ASSERT(pItem != NULL)
            {
                // Draw an icon:
                QPixmap* pIconPixmap = new QPixmap(10, 10);
                QPainter* pIconPainter = new QPainter(pIconPixmap);
                pIconPainter->fillRect(0, 0, 10, 10, Qt::white);

                // Draw an empty / filled rectangle according to check state:
                bool isChecked = (pItem->checkState() == Qt::Checked);

                if (isChecked)
                {
                    pIconPainter->fillRect(0, 0, 10, 10, m_pGraphsVec[graphIndex]->GetColor());

                    HideGraph(graphIndex, false);
                }
                else
                {
                    pIconPainter->setPen(m_pGraphsVec[graphIndex]->GetColor());
                    pIconPainter->drawRect(0, 0, 9, 9);

                    HideGraph(graphIndex, true);

                    if (m_pPlotInfoTable->selectedItems().count() > 0 &&
                        (m_pPlotInfoTable->selectedItems())[0] == pItem)
                    {
                        //deselect the item
                        SetSelectedColorsForGraphs(m_pPlotInfoTable->rowCount() - 1);

                        // select the last row (add/remove) - will automatically deselect the clciked row because of single selection policy
                        //m_pPlotInfoTable->selectRow(m_pPlotInfoTable->rowCount() - 1);
                    }
                }

                // Set the icon pixmap according to its check state:
                pItem->setIcon(QIcon(*pIconPixmap));

                Replot();
            }

            connect(m_pPlotInfoTable, SIGNAL(cellChanged(int, int)), this, SLOT(OnCellClicked(int, int)), Qt::UniqueConnection);
        }

        // if (m_pGraphsVec.size() == row) its add/remove counters row
    }
}

void acMultiLinePlot::OnGraphSelectionChanged()
{
    GT_IF_WITH_ASSERT(NULL != m_pCustomPlot)
    {
        for (int i = 0; i < m_pCustomPlot->graphCount(); ++i)
        {
            QCPGraph* graph = m_pCustomPlot->graph(i);

            GT_IF_WITH_ASSERT(NULL != graph)
            {
                if (graph->selected())
                {
                    if (NULL != m_pPlotInfoTable)
                    {
                        m_pPlotInfoTable->selectRow(m_pCustomPlot->graphCount() - 1 - i);
                        m_pPlotInfoTable->setFocus(Qt::OtherFocusReason);
                    }
                }
            }
        }
    }
}

void acMultiLinePlot::OnInfoTableSelectedItemChanged()
{
    QList<QTableWidgetItem*> selectedItems = m_pPlotInfoTable->selectedItems();

    if (selectedItems.size() > 0)
    {
        // get only one selected row
        int rowIndex = selectedItems[0]->row();

        SetSelectedColorsForGraphs(rowIndex);

        // needed for offline session
        Replot();
    }

}

void acMultiLinePlot::HideXAxisLabels(bool hide)
{
    if (NULL != m_pCustomPlot)
    {
        m_pCustomPlot->xAxis->setTickLabels(!hide);
    }
}

void acMultiLinePlot::GetGraphsValues(double key, QMap<QString, double>& names_values)
{
    for (QVector<acSingleLineGraph*>::iterator lineGraph = m_pGraphsVec.begin(); lineGraph != m_pGraphsVec.end(); ++lineGraph)
    {
        QString name = (*lineGraph)->name();

        if (!name.isEmpty())
        {
            QCPDataMap* pDataMap = (*lineGraph)->data();

            if (pDataMap->count(key) != 0)
            {
                double value = (*pDataMap)[key].value;

                if (0 < value)
                {
                    names_values[name] = value;
                }
            }
        }
    }
}


/// when hovering over custom plot the nearest real data absciss is emitted
void acMultiLinePlot::OnPlotHovered(QMouseEvent* pMouseEvent)
{
    (void)pMouseEvent;
    QPoint mousePos = pMouseEvent->pos();
    GT_IF_WITH_ASSERT(NULL != m_pCustomPlot)
    {
        QCPAxisRect*  pPlotRect = m_pCustomPlot->axisRect();
        GT_IF_WITH_ASSERT(NULL != pPlotRect)
        {
            if (pPlotRect->rect().contains(mousePos))
            {
                if (!m_pGraphsVec.empty())
                {
                    if (!m_pGraphsVec[0]->VectorData()->empty())
                    {
                        double mouseAsKey = m_pCustomPlot->xAxis->pixelToCoord(mousePos.x());
                        int mouseAsKeyIndex;

                        if (m_pGraphsVec[0]->GetNearestIndexToKey(mouseAsKey, -1, mouseAsKeyIndex))
                        {
                            double realKey = m_pGraphsVec[0]->VectorData()->at(mouseAsKeyIndex).Key();
                            int realKeyPixel = m_pCustomPlot->xAxis->coordToPixel(realKey);
                            emit TrackingXAxis(realKey, realKeyPixel);
                        }
                    }
                }
            }
            else
            {
                emit TrackingXAxis(-1, -1);
            }
        }
    }

}

void acMultiLinePlot::SetPlotToolTip(const QString& toolTipText, int trackingLineAbscissa)
{
    if (toolTipText.isEmpty())
    {
        m_pValuesTextLabel->setVisible(false);
    }
    else
    {
        m_pValuesTextLabel->setText(toolTipText);
        m_pValuesTextLabel->adjustSize();
        m_pValuesTextLabel->setVisible(true);
        int toolTipX = trackingLineAbscissa;
        int toolTipWidth = m_pValuesTextLabel->width();
        int toolTipHeight = m_pValuesTextLabel->height();

        if ((trackingLineAbscissa + m_pValuesTextLabel->width() > m_pCustomPlot->axisRect()->right()))
        {
            toolTipX = trackingLineAbscissa - toolTipWidth + 1;
        }

        m_pValuesTextLabel->setGeometry(toolTipX, m_pCustomPlot->axisRect()->top(), toolTipWidth, toolTipHeight);
    }
}

// ---------------------------------------------------------------------------
// Name:        acMultiLinePlot::InitTrackingToolTip
// Description: sets the tool tip label text format, style sheet, visibility
//              and other attributes
// Author:      Yuri Rshtunique
// Date:        October 2014
// ---------------------------------------------------------------------------
void acMultiLinePlot::InitTrackingToolTip()
{
    m_pValuesTextLabel = new QLabel(m_pCustomPlot);
    m_pValuesTextLabel->setTextFormat(Qt::RichText);
    m_pValuesTextLabel->setStyleSheet(m_sTooltipHTMLStyle);
    m_pValuesTextLabel->setVisible(false);
    m_pValuesTextLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
}

int acMultiLinePlot::UpdateTrackingTime(int oldKey)
{
    double newKey = -1.0;

    if (!m_pGraphsVec.empty())
    {
        QCPDataMap* pDataMap = m_pGraphsVec[0]->data();

        if (!pDataMap->isEmpty())
        {
            QMap<double, QCPData>::iterator ub = pDataMap->find(double(oldKey));

            if (ub != pDataMap->end() && ++ub != pDataMap->end())
            {
                newKey = ub.key();
            }
        }
    }

    return int(newKey);
}

void acMultiLinePlot::Replot()
{
    if (NULL != m_pCustomPlot)
    {
        m_pCustomPlot->replot();

        if (NULL != m_pValuesTextLabel && m_pValuesTextLabel->isVisible())
        {
            m_pValuesTextLabel->update();
        }
    }
}

void acMultiLinePlot::EnableInfoTableAddRemoveCounters(bool enable)
{
    if (m_pPlotInfoTable != NULL && m_pPlotInfoTable->rowCount() > 0)
    {
        QColor color;

        if (enable)
        {
            connect(m_pPlotInfoTable, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(OnLegendTableItemActivate(QTableWidgetItem*)));
            color = Qt::black;
        }
        else
        {
            disconnect(m_pPlotInfoTable, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(OnLegendTableItemActivate(QTableWidgetItem*)));
            color = Qt::gray;
        }

        QTableWidgetItem* pAddRemoveLine = m_pPlotInfoTable->item(m_addRemoveRowNum, 0);
        GT_IF_WITH_ASSERT(pAddRemoveLine != NULL)
        {
            // Set it to be gray:
            pAddRemoveLine->setTextColor(color);
        }
    }
}

void acMultiLinePlot::ResizeColumns()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pPlotInfoTable != NULL)
    {
        m_pPlotInfoTable->resizeColumnToContents(0);
    }
}

bool acMultiLinePlot::GetInfoTableSelectedGraphValue(QString& graphName, QString& valueStr) const
{
    bool rowSelected = false;

    GT_IF_WITH_ASSERT(NULL != m_pPlotInfoTable)
    {
        QList<QTableWidgetItem*> selectedList = m_pPlotInfoTable->selectedItems();

        if (selectedList.size() > 0)
        {
            int rowNum = selectedList[0]->row();

            // sanity check - last row is the add/remove counters button and not a graph
            if (rowNum < m_pPlotInfoTable->rowCount() - 1)
            {
                rowSelected = true;

                gtString name;
                m_pPlotInfoTable->getItemText(rowNum, AC_PLOT_LEGEND_NAME_COL_INDEX, name);

                QString tmpStr(QString::fromWCharArray(name.asCharArray(), name.length()));
                graphName = tmpStr;

                m_pPlotInfoTable->getItemText(rowNum, AC_PLOT_LEGEND_VALUE_COL_INDEX, valueStr);
            }
        }
        else
        {
            //no graph was selected
            // check at least 1 graph in the plot (except the add/remove row)
            if (m_pPlotInfoTable->rowCount() > 1)
            {
                gtString name;
                m_pPlotInfoTable->getItemText(0, AC_PLOT_LEGEND_NAME_COL_INDEX, name);

                QString tmpStr(QString::fromWCharArray(name.asCharArray(), name.length()));
                graphName = tmpStr;

                m_pPlotInfoTable->getItemText(0, AC_PLOT_LEGEND_VALUE_COL_INDEX, valueStr);
            }
        }
    }

    return rowSelected;
}

bool acMultiLinePlot::GetInfoTableValueByGraphName(gtString graphName, double& value) const
{
    bool retVal = false;
    value = -1;
    GT_IF_WITH_ASSERT(NULL != m_pPlotInfoTable)
    {
        int rowNum = m_pPlotInfoTable->rowCount();

        for (int i = 0; i < rowNum; i++)
        {
            gtString name;
            m_pPlotInfoTable->getItemText(i, AC_PLOT_LEGEND_NAME_COL_INDEX, name);

            if (graphName.compare(name))
            {
                gtString valStr;
                m_pPlotInfoTable->getItemText(i, AC_PLOT_LEGEND_VALUE_COL_INDEX, valStr);
                value = atof(valStr.asUTF8CharArray());
                retVal = true;
                break;
            }
        }
    }
    return retVal;
}

void acMultiLinePlot::SetSelectedColorsForGraphs(int selectedRowIndex)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pCustomPlot != NULL)
    {
        // in cumulative graphs with total data - do not do anything on total row selection in the legend
        if (!(m_isCumulative && m_isLastGraphTotal && (selectedRowIndex < 0)))
        {
            int numOfGraphs = m_pGraphsVec.count();

            QCPGraph* pCurrentGraph = NULL;
            int selectedGraphIndex = numOfGraphs - 1 - selectedRowIndex;

            for (int i = 0; i < numOfGraphs - 1; i++)
            {
                QColor graphColor = m_pGraphsVec[i]->GetColor();
                pCurrentGraph = m_pCustomPlot->graph(i);

                GT_IF_WITH_ASSERT(pCurrentGraph != NULL)
                {
                    // Set the selected flag:
                    pCurrentGraph->setSelected(i == selectedGraphIndex);

                    if (m_isCumulative)
                    {
                        QBrush brush(graphColor);
                        QPen pen(graphColor);

                        if ((i != selectedGraphIndex) && (selectedGraphIndex >= 0) && (selectedRowIndex > 0))
                        {
                            brush.setStyle(Qt::SolidPattern);
                            QColor alphaColor = Qt::black;
                            alphaColor.setAlpha(5);
                            // QColor alphaColor(0, 170, 181, 20);
                            brush.setColor(alphaColor);
                            pen.setColor(graphColor);
                        }

                        pCurrentGraph->setPen(pen);
                        pCurrentGraph->setBrush(brush);
                    }
                    else
                    {
                        if ((i != selectedGraphIndex) && (selectedGraphIndex >= 0))
                        {
                            graphColor.setAlpha(graphColor.alpha() * 0.4);
                        }

                        pCurrentGraph->setPen(graphColor);
                    }
                }
            }
        }
    }
}

bool acMultiLinePlot::IsGraphDisabledAtIndex(int graphIndex)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(graphIndex < m_pGraphsVec.size())
    {
        ret = m_pGraphsVec[graphIndex]->ShouldBeDisabled();
    }
    return ret;
}

void acMultiLinePlot::GetAllGraphsHiddenState(QVector<bool>& hiddenStateVec)
{
    unsigned int count = m_pGraphsVec.count();

    // for each graph set the vector with its m_isHidden value
    for (unsigned int i = 0; i < count; i++)
    {
        hiddenStateVec.append(m_pGraphsVec[i]->IsHidden());
    }
}