//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acVectorLineGraph.cpp
///
//==================================================================================

//------------------------------ acVectorLineGraph.cpp ------------------------------

#include <math.h>

// framework
#include <AMDTBaseTools/Include/gtAssert.h>

// Local
#include <AMDTApplicationComponents/Include/acVectorLineGraph.h>

#define DRAW_EVERY_NTH_PIXEL 2
#define DRAW_MAXIMUM_POINTS 200
#define DRAW_MINIMAL_BUFFER_INCREASE 1

//--------------------------------------------------------
acVectorLineGraph::acVectorLineGraph(QCPAxis* keyAxis, QCPAxis* valueAxis) : QCPGraph(keyAxis, valueAxis), m_keyInterval(0), m_lastUsedUpperIndex(0)
{
    m_vectorData = new acDataVector;
    m_vectorData->reserve(10000);
    m_drawWholeSpan = false;
    m_lastAxisSpan = 0;
}

//--------------------------------------------------------
acVectorLineGraph::~acVectorLineGraph()
{
    delete m_vectorData;
}

//--------------------------------------------------------
acVectorLineGraph* acVectorLineGraph::AddGraph(QCustomPlot* pPlot, QCPAxis* pKeyAxis, QCPAxis* pValueAxis)
{
    acVectorLineGraph* pRetGraph = NULL;

    if (NULL != pPlot)
    {
        // If key axis or value axis not supplied, use the custom plot axis
        if (!pKeyAxis)
        {
            pKeyAxis = pPlot->xAxis;
        }

        if (!pValueAxis)
        {
            pValueAxis = pPlot->yAxis;
        }

        if (NULL != pKeyAxis && NULL != pValueAxis)
        {
            // validate the axis are valid with the same custom plat parent, if not it will cause a crash
            if (pKeyAxis->parentPlot() == pPlot && pValueAxis->parentPlot() == pPlot)
            {
                // create the graph and add it
                acVectorLineGraph* newGraph = new acVectorLineGraph(pKeyAxis, pValueAxis);

                if (pPlot->addPlottable(newGraph))
                {
                    newGraph->setName("Graph " + QString::number(pPlot->graphCount()));
                    pRetGraph = newGraph;
                }
                else
                {
                    // if failed to add it delete the graph
                    delete newGraph;
                }
            }
            else
            {
                qDebug() << Q_FUNC_INFO << "passed keyAxis or valueAxis doesn't have this QCustomPlot as parent";
            }
        }
        else
        {
            qDebug() << Q_FUNC_INFO << "can't use default QCustomPlot xAxis or yAxis, because at least one is invalid (has been deleted)";
        }
    }

    return pRetGraph;
}

//--------------------------------------------------------
bool acVectorLineGraph::AddDataToVector(double key, double value)
{
    bool addData = false;

    // If there are no keys add the key:
    if (m_vectorData->size() == 0)
    {
        addData = true;
    }
    else
    {
        if (m_vectorData->at(m_vectorData->size() - 1).m_key < key)
        {
            addData = true;
        }
    }

    if (addData)
    {
        float fKey = static_cast<float>(key);
        float fValue = static_cast<float>(value);
        acVectorDataElement newData(fKey, fValue);
        m_vectorData->push_back(newData);

        CalcKeyInterval();
    }

    return addData;
}

//--------------------------------------------------------
bool acVectorLineGraph::SetVectorData(const QVector<double>& key, const QVector<double>& value)
{
    int continueCheck = true;
    int lastValidData = 0;

    // validate that there are keys and all keys are in ascending order and that there is the same number of keys and values
    int keysSize = key.size();

    if (keysSize == value.size() && keysSize > 0)
    {
        // validate that all keys are in ascending order
        float currentKey = static_cast<float>(key[0]);

        for (int nKey = 1; nKey < keysSize && continueCheck; nKey++)
        {
            float checkedKey = static_cast<float>(key[nKey]);

            if (checkedKey < currentKey)
            {
                continueCheck = false;
            }
            else
            {
                currentKey = checkedKey;
                lastValidData = nKey;
            }
        }

        // Clear all old data
        m_vectorData->clear();

        for (int nKey = 0; nKey <= lastValidData; nKey++)
        {
            acVectorDataElement newData(key[nKey], value[nKey]);
            m_vectorData->push_back(newData);
        }

        CalcKeyInterval();
    }

    return continueCheck;
}

//--------------------------------------------------------
void acVectorLineGraph::CalcKeyInterval()
{
    int vectorSize = m_vectorData->size();

    if (vectorSize > 1)
    {
        m_keyInterval = (m_vectorData->at(vectorSize - 1).m_key - m_vectorData->at(0).m_key) / (vectorSize - 1);
    }
    else
    {
        m_keyInterval = 0;
    }
}

//--------------------------------------------------------
void acVectorLineGraph::clearData()
{
    // clear data
    m_vectorData->clear();
    m_keyInterval = 0;

    // clear displayed information
    m_lastUsedUpperIndex = 0;
    m_drawnIndexes.clear();

}

//--------------------------------------------------------
void acVectorLineGraph::draw(QCPPainter* painter)
{
    bool shouldDraw = true;

    // validations that there is something to draw at all
    if (!mKeyAxis || !mValueAxis)
    {
        qDebug() << Q_FUNC_INFO << "invalid key or value axis";
        shouldDraw = false;
    }

    if (mKeyAxis.data()->range().size() <= 0 || m_vectorData->empty())
    {
        shouldDraw = false;
    }

    if (mLineStyle == lsNone && mScatterStyle.isNone())
    {
        shouldDraw = false;
    }

    if (0 == m_keyInterval)
    {
        shouldDraw = false;
    }

    if (shouldDraw)
    {
        int lowerIndex, upperIndex;
        GetVisibleDataIndexRange(lowerIndex, upperIndex);
        GetDrawnIndexes(lowerIndex, upperIndex);

        if (m_segmentsVector.empty())
        {
            DrawIndexes(painter, 0, m_drawnIndexes.size());
        }
        else
        {
            for (int nSegment = 0; nSegment < (int)m_segmentsVector.size(); nSegment++)
            {
                // set the pen and brush to the one of the segment
                setPen(m_segmentsVector[nSegment].m_pen);
                setBrush(m_segmentsVector[nSegment].m_brush);

                // set the range
                int startIndex = (0 == nSegment ? 0 : m_segmentsVector[nSegment - 1].m_endIndex);
                int endIndex = m_segmentsVector[nSegment].m_endIndex;

                GT_IF_WITH_ASSERT(startIndex != -1 && endIndex != -1)
                {
                    // convert the indexes of range to the drawnIndexes vector index
                    int startDrawnIndex;
                    int endDrawnIndex;
                    bool rcStart = ConvertIndexToDrawnIndex(startIndex, startDrawnIndex);
                    bool rcEnd = ConvertIndexToDrawnIndex(endIndex, endDrawnIndex);
                    GT_IF_WITH_ASSERT(rcStart && rcEnd)
                    {
                        DrawIndexes(painter, startDrawnIndex, endDrawnIndex);
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------
void acVectorLineGraph::GetVisibleDataIndexRange(int& lowerIndex, int& upperIndex)
{

    lowerIndex = 0;
    upperIndex = m_vectorData->size() > 0 ? m_vectorData->size() - 1 : 0;
    GetNearestIndexToKey(mKeyAxis.data()->range().lower, -1, lowerIndex);
    GetNearestIndexToKey(mKeyAxis.data()->range().upper, -1, upperIndex);
}

//--------------------------------------------------------
bool acVectorLineGraph::GetNearestIndexToKey(float searchKey, int hintIndex, int& nearestIndex)
{
    bool retVal = false;

    int vectorSize = m_vectorData->size();

    if (vectorSize > 0)
    {
        if (m_keyInterval == 0)
        {
            nearestIndex = 0;
            retVal = true;
        }
        else
        {
            int currentIndex = 0;

            // if there is no hint index (negative value) calculate based on the searchKey alone
            if (hintIndex < 0)
            {
                // initial estimated index based on the interval and should take into account the value of the first key
                currentIndex = (searchKey - m_vectorData->at(0).m_key) / m_keyInterval;
            }
            else if (hintIndex < vectorSize)
            {
                currentIndex = hintIndex + (searchKey - m_vectorData->at(hintIndex).m_key) / m_keyInterval;
            }
            else
            {
                GT_ASSERT(false);
            }

            // make sure first index is in the range
            if (currentIndex > vectorSize - 1)
            {
                currentIndex = vectorSize - 1;
            }
            else if (currentIndex < 0)
            {
                currentIndex = 0;
            }

            // search direction based if the key at the index is smaller or larger
            bool searchDirectionUpward = true;

            if (m_vectorData->at(currentIndex).m_key > searchKey)
            {
                searchDirectionUpward = false;
            }

            bool continueSearch = true;
            int newIndex = currentIndex;
            float currentKeyVal;
            float newKeyVal;

            while (continueSearch)
            {
                newIndex = currentIndex + (searchDirectionUpward ? 1 : -1);

                if (newIndex >= 0 && newIndex < vectorSize)
                {
                    currentKeyVal = m_vectorData->at(currentIndex).m_key;
                    newKeyVal = m_vectorData->at(newIndex).m_key;

                    // check if the key is between two values then find the nearest one out of the two and stop
                    if (searchDirectionUpward)
                    {
                        if (currentKeyVal <= searchKey && newKeyVal >= searchKey)
                        {
                            continueSearch = false;
                        }
                    }
                    else
                    {
                        if (newKeyVal <= searchKey && currentKeyVal >= searchKey)
                        {
                            continueSearch = false;
                        }
                    }

                    // if need continue the search
                    if (continueSearch)
                    {
                        currentIndex = newIndex;
                    }
                    else
                    {
                        // need to check if we are closer to the new index
                        if (fabs(searchKey - currentKeyVal) > fabs(searchKey - newKeyVal))
                        {
                            currentIndex = newIndex;
                        }
                    }
                }
                else
                {
                    continueSearch = false;
                }
            }

            nearestIndex = currentIndex;

            retVal = true;
        }
    }

    return retVal;
}

//--------------------------------------------------------
void acVectorLineGraph::GetDrawnIndexes(int& lowerIndex, int& upperIndex)
{
    // assume it is the number of samples
    int numPoints = upperIndex - lowerIndex;
    numPoints = GetAllowedPointsToDraw(numPoints, lowerIndex, upperIndex);

    if (numPoints > 0)
    {
        // At this stage do not set min number of points since it means drawing in the negative direction so 0+points is valid

        // Check for the special case of displaying the entire span without enough points to display all data
        if ((numPoints < (int)m_vectorData->size() - 1) && (0 == lowerIndex) && (upperIndex == (int)(m_vectorData->size() - 1)))
        {
            // if this is the first time to draw whole span clear old indexes since they are useless
            if (!m_drawWholeSpan || m_drawnIndexes.size() == 0)
            {
                RecalcEntireSpanPoints(numPoints);
                m_drawWholeSpan = true;
            }
            else
            {
                CheckAddPointForWholeSpanIndexes(numPoints);
            }
        }
        else
        {
            m_drawWholeSpan = false;
            GetPartialSpanIndexes(lowerIndex, upperIndex, numPoints);
        }
    }
}

int acVectorLineGraph::GetAllowedPointsToDraw(int& initialNumberPoints, int& lowerIndex, int& upperIndex) const
{
    float lowerKeyVal = m_vectorData->at(lowerIndex).m_key;
    float upperKeyVal = m_vectorData->at(upperIndex).m_key;

    // Set the maximum number of points based on number of pixel and total number of points and actual samples in the area drawn
    int retVal = initialNumberPoints;
    int rectWidth = mKeyAxis->axisRect()->width();

    // nothing to do if the rect has negative width (it means it is not really shown due to sizing in the layout)
    if (rectWidth > 0)
    {
        retVal = (int)fmin(retVal, rectWidth);
        // set minimum points based on an predefined values
        QCPAxis* pKeyAxis = mKeyAxis.data();
        int keyPixelSpan = 1;

        if (NULL != pKeyAxis)
        {
            keyPixelSpan = qAbs(pKeyAxis->coordToPixel(lowerKeyVal) - pKeyAxis->coordToPixel(upperKeyVal));
        }

        retVal = (int)fmin(retVal, keyPixelSpan * 1.0 / DRAW_EVERY_NTH_PIXEL);
        retVal = (int)fmin(retVal, DRAW_MAXIMUM_POINTS);
    }
    else
    {
        retVal = 0;
    }

    return retVal;
}

//--------------------------------------------------------
void acVectorLineGraph::GetPartialSpanIndexes(int& lowerIndex, int& upperIndex, int& numPoints)
{
    bool isSameRange = (mKeyAxis.data()->range().size() == m_lastAxisSpan);

    float lowerKeyVal = m_vectorData->at(lowerIndex).m_key;
    float upperKeyVal = m_vectorData->at(upperIndex).m_key;

    int currentKeyIndex = lowerIndex;
    float keyInterval = 0;

    if (numPoints > 0)
    {
        keyInterval = (upperKeyVal - lowerKeyVal) / numPoints;
    }

    // find the "real" first key value that will be the same when moving part of the graph by X samples
    float initialKeyVal = lowerKeyVal - m_vectorData->at(0).m_key;
    float roundKeyVal = floor(initialKeyVal / keyInterval) * keyInterval + m_vectorData->at(0).m_key;
    float currentKey = roundKeyVal;

    int currentDrawnSize = m_drawnIndexes.size();
    int currentIndex = 0;
    int numberOfPointsToShiftVector = 0;

    if (isSameRange)
    {
        // scrolling upward and still there is an overlap
        if (upperIndex >= m_lastUsedUpperIndex && lowerIndex < m_lastUsedUpperIndex)
        {
            int drawnSize = upperIndex - lowerIndex + 1;
            drawnSize = GetAllowedPointsToDraw(drawnSize, lowerIndex, upperIndex);

            if (currentDrawnSize == drawnSize)
            {
                currentKey = m_vectorData->at(m_lastUsedUpperIndex).m_key + keyInterval;

                if (lowerIndex >= m_drawnIndexes[0])
                {
                    // find the lowest key in the drawn vector
                    numberOfPointsToShiftVector = GetPrecedingDrawIndex(lowerIndex);
                    currentIndex = currentDrawnSize - numberOfPointsToShiftVector;

                    // shift points
                    if (numberOfPointsToShiftVector > 0)
                    {
                        for (int nIndex = 0; nIndex < currentIndex && (nIndex + numberOfPointsToShiftVector < currentDrawnSize); nIndex++)
                        {
                            m_drawnIndexes[nIndex] = m_drawnIndexes[nIndex + numberOfPointsToShiftVector];
                        }
                    }
                }
                else
                {
                    // This is the case that the user has dragged the active range bar to the left.
                    // In this is a case which the algorithm does not attempt to optimize because it is a manual user operation that is not expected to occur as often. as the periodic sampling.
                    // Instead the algorithm recalcs the entire draw vector.
                    currentIndex = 0;
                    currentKey = roundKeyVal;
                    m_drawnIndexes.resize(drawnSize + DRAW_MINIMAL_BUFFER_INCREASE);
                }
            }
            else if (currentDrawnSize < drawnSize && currentDrawnSize > 0)
            {
                // check if need to shift because of lower limit
                if (m_drawnIndexes[0] < lowerIndex)
                {
                    // find the first index that is lower then the lower limit
                    int nearestLowIndex = GetPrecedingDrawIndex(lowerIndex);

                    // set the new creation data info before the shifting
                    currentKey = m_vectorData->at(m_drawnIndexes[currentDrawnSize - 1]).m_key;
                    currentIndex = currentDrawnSize - nearestLowIndex;

                    if (nearestLowIndex > 0)
                    {
                        for (int nIndex = nearestLowIndex; nIndex < currentDrawnSize; nIndex++)
                        {
                            m_drawnIndexes[nIndex - nearestLowIndex] = m_drawnIndexes[nIndex];
                        }
                    }
                }
                else
                {
                    // add the new points
                    currentKey = m_vectorData->at(m_lastUsedUpperIndex).m_key + keyInterval;
                    currentIndex = currentDrawnSize;
                }

                // shift one point if
                m_drawnIndexes.resize(drawnSize + DRAW_MINIMAL_BUFFER_INCREASE);
            }
            else
            {
                // make sure there is enough space for items because same span does not mean same number of points (since some might have being added in an empty area)
                m_drawnIndexes.resize(drawnSize + DRAW_MINIMAL_BUFFER_INCREASE);
            }
        }
        else
        {
            // if we scroll downwards it is a manual scroll so it is ok to recalc everything instead of shifting the vector
            m_drawnIndexes.clear();
            m_drawnIndexes.resize(numPoints + DRAW_MINIMAL_BUFFER_INCREASE);
        }
    }
    else
    {
        m_drawnIndexes.clear();
        m_drawnIndexes.resize(numPoints + DRAW_MINIMAL_BUFFER_INCREASE);
    }

    int drawnIndexesSize = m_drawnIndexes.size();

    // set the upper limit to upperKeyVal + keyInterval and not upperKeyVal since because of rounding issues
    // last point can be missed
    while (currentKey <= upperKeyVal + keyInterval && keyInterval > 0 && currentIndex < drawnIndexesSize && currentIndex < DRAW_MAXIMUM_POINTS)
    {
        if (GetNearestIndexToKey(currentKey, -1, currentKeyIndex))
        {
            // make sure the last point of the data is not inserted twice in case there was no rounding issues
            if (currentIndex == 0 || (m_drawnIndexes[currentIndex - 1] != currentKeyIndex))
            {
                m_drawnIndexes[currentIndex++] = currentKeyIndex;
            }

            //m_drawnIndexes.push_back(currentKeyIndex);
            currentKey += keyInterval;
        }
        else
        {
            // not found exit
            currentKey = upperKeyVal + keyInterval;
        }
    }

    // remove 0 at the end that might have occurred during shifting when adding points and last point does not have a
    // value to add because of adding behind the interval
    while (m_drawnIndexes[currentIndex - 1] == 0)
    {
        currentIndex--;
    }

    // remove the extra points:
    m_drawnIndexes.resize(currentIndex);

    // update used limits
    m_lastUsedUpperIndex = upperIndex;

    m_lastAxisSpan = mKeyAxis.data()->range().size();
}

//--------------------------------------------------------
int acVectorLineGraph::GetPrecedingDrawIndex(int& vectorIndex)
{
    // 0 is the lowest index so it is the default initial value of the return draw index.
    int retIndex = 0;
    int currentDrawnSize = m_drawnIndexes.size();
    // find the first index that is lower then the lower limit

    for (int nIndex = 1; nIndex < currentDrawnSize; nIndex++)
    {
        if (m_drawnIndexes[nIndex] >= vectorIndex)
        {
            retIndex = nIndex;
            break;

        }
    }

    // shift the indexes but first check if we are not shifting to much in case
    if (retIndex > 0 && m_drawnIndexes[retIndex] > vectorIndex)
    {
        retIndex--;
    }

    return retIndex;
}

//--------------------------------------------------------
void acVectorLineGraph::RecalcEntireSpanPoints(int numPoints)
{
    int finalNumberOfPoints = MaxWholeSpanNumberOfPoints(numPoints);
    m_maxIndexesBeforeCompression = finalNumberOfPoints * 2;

    // Calculate indexes based on the number of points
    int numDataPoints = m_vectorData->size();
    float currentKey = m_vectorData->at(0).m_key;
    float upperKeyVal = m_vectorData->at(numDataPoints - 1).m_key;
    m_keyIntervalForAddingPoint = (upperKeyVal - currentKey) / finalNumberOfPoints;
    int currentIndex = 0;
    int currentKeyIndex = 0;

    m_drawnIndexes.resize(finalNumberOfPoints + 5);

    while (currentKey <= upperKeyVal && m_keyIntervalForAddingPoint > 0)
    {
        if (GetNearestIndexToKey(currentKey, -1, currentKeyIndex))
        {
            m_drawnIndexes[currentIndex++] = currentKeyIndex;
            //m_drawnIndexes.push_back(currentKeyIndex);
            currentKey += m_keyIntervalForAddingPoint;
        }
        else
        {
            // not found exit
            currentKey = upperKeyVal + m_keyIntervalForAddingPoint;
        }
    }

    m_drawnIndexes.resize(currentIndex);

}

//--------------------------------------------------------
void acVectorLineGraph::CheckAddPointForWholeSpanIndexes(int numPoints)
{
    // check he difference between last point in index vector and the real last point
    // if it is larger then found key span:
    // if the number of points already in indexes list is smaller then m_maxIndexesBeforeCompression add it
    // else recalc everything (compress)

    // if the maximum points is not the same in the recalc it is because of resize event and everything needs to be calculated again
    int currentNumPoints = MaxWholeSpanNumberOfPoints(numPoints);

    if (currentNumPoints * 2 != m_maxIndexesBeforeCompression)
    {
        RecalcEntireSpanPoints(numPoints);
    }
    else
    {
        int numDataPoints = m_vectorData->size();
        int numDrawnPoints = m_drawnIndexes.size();
        int lastPointIndex = m_drawnIndexes[numDrawnPoints - 1];

        GT_IF_WITH_ASSERT(lastPointIndex < numDataPoints && numDataPoints > 0)
        {
            float deltaKey = m_vectorData->at(numDataPoints - 1).m_key - m_vectorData->at(lastPointIndex).m_key;

            if (deltaKey >= m_keyIntervalForAddingPoint)
            {
                if (numDrawnPoints <= m_maxIndexesBeforeCompression)
                {
                    m_drawnIndexes.push_back(numDataPoints - 1);
                }
                else
                {
                    RecalcEntireSpanPoints(numPoints);
                }
            }
        }
        else
        {
            // something went wrong, recalc everything assuming there will be the same number of points
            RecalcEntireSpanPoints(m_maxIndexesBeforeCompression / 2);
        }
    }
}

//--------------------------------------------------------
int acVectorLineGraph::MaxWholeSpanNumberOfPoints(int numPoints)
{
    double retVal = numPoints;

    // Find the final number of points based on initial value passed
    if (numPoints > DRAW_MAXIMUM_POINTS / 2)
    {
        retVal = DRAW_MAXIMUM_POINTS / 2;
    }

    return retVal;

}

//--------------------------------------------------------
bool acVectorLineGraph::ConvertIndexToDrawnIndex(int& keyIndex, int& drawnVectorIndex)
{
    bool retVal = false;
    drawnVectorIndex = -1;

    if (m_drawnIndexes.size() > 0)
    {
        std::vector<int>::iterator it = std::lower_bound(m_drawnIndexes.begin(), m_drawnIndexes.end(), keyIndex);

        if (it != m_drawnIndexes.end())
        {
            retVal = true;
            // we need the index of the item, not the value of it:
            drawnVectorIndex = it - m_drawnIndexes.begin();
        }
        else
        {
            if (keyIndex > m_drawnIndexes[m_drawnIndexes.size() - 1])
            {
                retVal = true;
                drawnVectorIndex = m_drawnIndexes.size();
            }

            // not checking lower index since it is always an error
        }
    }

    return retVal;
}

//--------------------------------------------------------
void acVectorLineGraph::DrawIndexes(QCPPainter* painter, int startDrawnVectorIndex, int endDrawnVectorIndex)
{
    // First convert the key/value to pixels info
    QVector<QPointF> pointsVector;

    // Get the points of the current graph
    GetScreenPoints(pointsVector, startDrawnVectorIndex, endDrawnVectorIndex);

    // Draw a fill if there is a need
    DrawVectorFill(painter, pointsVector, startDrawnVectorIndex, endDrawnVectorIndex);

    // Use the qcustomplot draw line function
    DrawLinePlot(painter, pointsVector);
}

//--------------------------------------------------------
void acVectorLineGraph::GetScreenPoints(QVector<QPointF>& pointsVector, int startDrawnVectorIndex, int endDrawnVectorIndex) const
{
    QCPAxis* pKeyAxis = mKeyAxis.data();
    QCPAxis* pValueAxis = mValueAxis.data();

    int drawnSize = m_drawnIndexes.size();
    int vectorSize = m_vectorData->size();

    // Nothing really to draw if there is 1 point or less
    if (drawnSize > 1 && startDrawnVectorIndex >= 0 && endDrawnVectorIndex <= drawnSize && startDrawnVectorIndex < endDrawnVectorIndex)
    {
        pointsVector.resize(endDrawnVectorIndex - startDrawnVectorIndex + (endDrawnVectorIndex >= drawnSize ? 0 : 1));

        for (int nIndex = startDrawnVectorIndex; nIndex <= endDrawnVectorIndex && nIndex < drawnSize; nIndex++)
        {
            int currentIndex = m_drawnIndexes[nIndex];

            if (currentIndex >= 0 && currentIndex < vectorSize)
            {
                pointsVector[nIndex - startDrawnVectorIndex].setX(pKeyAxis->coordToPixel(m_vectorData->at(currentIndex).m_key));
                pointsVector[nIndex - startDrawnVectorIndex].setY(pValueAxis->coordToPixel(m_vectorData->at(currentIndex).m_value));
            }
        }
    }
}

//--------------------------------------------------------
void acVectorLineGraph::DrawLinePlot(QCPPainter* painter, QVector<QPointF>& pointsVector)
{
    if (selected())
    {
        drawLinePlot(painter, &pointsVector);
        // create a vector parallel to the displayed on, 1 pixel offset and use fill
        int vectorSize = pointsVector.size();

        QVector<QPointF> offsetVector;
        offsetVector.resize(vectorSize);

        // copy the vector in the original direction
        for (int i = 0; i < vectorSize; i++)
        {
            QPointF newPoint = pointsVector.at(i);
            newPoint.setY(newPoint.y() - 1);
            offsetVector[i] = newPoint;
        }

        drawLinePlot(painter, &offsetVector);
    }
    else
    {
        // Use the qcustomplot draw line function
        drawLinePlot(painter, &pointsVector);
    }
}

//--------------------------------------------------------
void acVectorLineGraph::DrawVectorFill(QCPPainter* painter, QVector<QPointF>& pointsVector, int startDrawnVectorIndex, int endDrawnVectorIndex)
{
    bool shouldPaint = true;

    if (mLineStyle == lsImpulse)
    {
        shouldPaint = false;
    }

    if (mainBrush().style() == Qt::NoBrush || mainBrush().color().alpha() == 0)
    {
        shouldPaint = false;
    }

    if (pointsVector.isEmpty())
    {
        shouldPaint = false;
    }

    if (shouldPaint)
    {
        applyFillAntialiasingHint(painter);

        if (mChannelFillGraph)
        {
            // Do the fill if the fill graph is the same as our type
            QCPGraph* pOriginalGraph = mChannelFillGraph;
            acVectorLineGraph* pGraph = dynamic_cast<acVectorLineGraph*>(pOriginalGraph);

            if (NULL != pGraph)
            {
                // Get the points from the fill graph
                QVector<QPointF> fillPointsVector;

                pGraph->GetScreenPoints(fillPointsVector, startDrawnVectorIndex, endDrawnVectorIndex);

                // join the two vectors to one polygon. Currently assume same size (keys exists in both vectors)
                if (fillPointsVector.size() == pointsVector.size())
                {
                    // add it in the reverse order otherwise the polygon will be twisted
                    QVector<QPointF> joinedVector;
                    int vectorSize = pointsVector.size();
                    joinedVector.reserve(vectorSize * 2);

                    for (int i = 0; i < vectorSize; i++)
                    {
                        joinedVector << pointsVector.at(i);
                    }

                    for (int i = vectorSize - 1; i >= 0; --i)
                    {
                        joinedVector << fillPointsVector.at(i);
                    }

                    QPolygonF joinedPolygon(joinedVector);
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(mainBrush());
                    painter->drawPolygon(joinedPolygon);
                }
            }
        }
        else
        {
            // draw base fill under graph, fill goes all the way to the zero-value-line:
            addFillBasePoints(&pointsVector);
            painter->setPen(Qt::NoPen);
            painter->setBrush(mainBrush());
            painter->drawPolygon(QPolygonF(pointsVector));
            removeFillBasePoints(&pointsVector);
        }
    }
}

//--------------------------------------------------------
void acVectorLineGraph::AddSegment(int segmentEndIndex, QPen segmentPen, QBrush segmentBrush)
{
    GT_IF_WITH_ASSERT(segmentEndIndex >= 0 && segmentEndIndex <= (int)m_vectorData->size())
    {
        // the size check is just to make sure there is impact on order of || items
        GT_IF_WITH_ASSERT(m_segmentsVector.empty() || (m_segmentsVector.size() > 0 && (segmentEndIndex > m_segmentsVector[m_segmentsVector.size() - 1].m_endIndex)))
        {
            // first segment just add it
            m_segmentsVector.push_back(acVectorLineGraphSegment(segmentEndIndex, segmentPen, segmentBrush));
        }
    }
}

//--------------------------------------------------------
double acVectorLineGraph::selectTest(const QPointF& pos, bool onlySelectable, QVariant* details) const
{
    Q_UNUSED(details)

    if ((onlySelectable && !mSelectable) || (m_vectorData == NULL) || (m_vectorData->size() == 0))
    {
        return -1;
    }

    if (!mKeyAxis || !mValueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return -1; }

    if (mKeyAxis.data()->axisRect()->rect().contains(pos.toPoint()))
    {
        return pointDistance(pos);
    }
    else
    {
        return -1;
    }
}

//--------------------------------------------------------
double acVectorLineGraph::pointDistance(const QPointF& pixelPoint) const
{
    if (m_vectorData == NULL || m_vectorData->size() == 0)
    {
        qDebug() << Q_FUNC_INFO << "requested point distance on graph" << mName << "without data";
        return 500;
    }

    gtVector<acVectorDataElement>& dataVec = *m_vectorData;

    if (m_vectorData->size() == 1)
    {
        QPointF dataPoint = coordsToPixels(dataVec.at(0).m_key, dataVec.at(0).m_value);
        return QVector2D(dataPoint - pixelPoint).length();
    }

    // line displayed calculate distance to line segments:
    double minDistSqr = std::numeric_limits<double>::max();

    // First convert the key/value to pixels info
    QVector<QPointF> pointsVector;

    // Get the points of the current graph
    GetScreenPoints(pointsVector, 0, m_drawnIndexes.size());
    double currentDistSqr;

    for (int i = 0; i < pointsVector.size() - 1; ++i)
    {
        currentDistSqr = distSqrToLine(pointsVector.at(i), pointsVector.at(i + 1), pixelPoint);

        if (currentDistSqr < minDistSqr)
        {
            minDistSqr = currentDistSqr;
        }
    }

    return sqrt(minDistSqr);
}
