//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acVectorLineGraph.h
///
//==================================================================================

//------------------------------ acVectorLineGraph.h ------------------------------

#ifndef __ACVECTORLINEGRAPH_H
#define __ACVECTORLINEGRAPH_H

#include <qcustomplot.h>

#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

class AC_API acVectorDataElement
{
public:
    acVectorDataElement(float key, float value)
    {
        m_key = key;
        m_value = value;
    }

    /// get data element key
    /// \returns the data element key
    float Key() { return m_key; }

    /// sets data element key
    /// \param key is the data element key to be set
    void SetKey(const float key) { m_key = key; }

    /// get data element value
    /// \returns the data element value
    float Value() { return m_value; }

    /// sets data element value
    /// \param value is the data element value to be set
    void SetValue(const float value) { m_value = value; }

    float m_key;
    float m_value;
};

typedef gtVector<acVectorDataElement> acDataVector;

class AC_API acVectorLineGraphSegment
{
public:
    acVectorLineGraphSegment(int segmentEndIndex, QPen& segmentPen, QBrush& segmentBrush) : m_endIndex(segmentEndIndex), m_pen(segmentPen), m_brush(segmentBrush) {};
    ~acVectorLineGraphSegment() {};

    int    m_endIndex;
    QPen   m_pen;
    QBrush m_brush;
};

class AC_API acVectorLineGraph : public QCPGraph
{
    Q_OBJECT
public:
    acVectorLineGraph(QCPAxis* keyAxis, QCPAxis* valueAxis);
    virtual ~acVectorLineGraph();

    // Add a graph of acVectorLineGraph type to the custom plot based on the QCustomPlot::addGraph
    static acVectorLineGraph* AddGraph(QCustomPlot* pPlot, QCPAxis* pKeyAxis = 0, QCPAxis* pValueAxis = 0);

    // Add data to the graph
    // AddDataToVector assumes that the key is in ascending order and must be larger then the last key in the vector
    // if not it will not add and return false
    bool AddDataToVector(double key, double value);

    // Set the data in one vector
    // SetVectorData assumes that the keys are in ascending order, if not it will not add the key not in order
    // and return false
    bool SetVectorData(const QVector<double>& key, const QVector<double>& value);

    // re implement public virtual functions
    virtual void clearData();

    // void clear the segments vector so new segments can be added
    void ClearSegments() { m_segmentsVector.clear(); }

    // Add a segment with its pen and brush. however all segments will refer to the same fill graph
    // the segments must be added in ascending order, if not the last segment added not following this rule will be ignored
    // the segment will be from the end of previous segment (or start in first segment) to the end index of the segment
    void AddSegment(int segmentEndIndex, QPen segmentPen, QBrush segmentBrush);

    // Access to the data
    acDataVector* VectorData() { return m_vectorData; }

    // Get nearest index to a specific Key if key is out of bound then either index 0 or size-1 index will return
    // if no data exists or interval not defined it will fail and return false.
    // negative hintIndex means look normal, otherwise use that index as a hint for the searchKey
    // solve shifting issues
    bool GetNearestIndexToKey(float searchKey, int hintIndex, int& nearestIndex);

    /// reimplemented virtual method - because this class holds data vector instead of data map
    /// inherits documentation from base class
    virtual double selectTest(const QPointF& pos, bool onlySelectable, QVariant* details = 0) const;

    // Get the key interval
    double KeyInterval() { return m_keyInterval; }

protected:
    // set the key interval based on the data
    void CalcKeyInterval();

    // re implement protected virtual functions
    virtual void draw(QCPPainter* painter);

    // Get visible bound indexes
    void GetVisibleDataIndexRange(int& lowerIndex, int& upperIndex);

    // Get the drawn indexes based on the number of pixels and max number of points set
    void GetDrawnIndexes(int& lowerIndex, int& upperIndex);

    // Get the indexes of a partial drawn span
    void GetPartialSpanIndexes(int& lowerIndex, int& upperIndex, int& numPoints);

    // Recalc points values for a specific range of new points
    void RecalcEntireSpanPoints(int numPoints);

    // Check adding point to the whole span drawing
    void CheckAddPointForWholeSpanIndexes(int numPoints);

    // calc whole span number of points based on calculated legal number of points:
    int MaxWholeSpanNumberOfPoints(int numPoints);

    // convert from the key index to the index in the drawn indexes vector
    bool ConvertIndexToDrawnIndex(int& keyIndex, int& drawinVectorIndex);

    // Draw the indexes
    void DrawIndexes(QCPPainter* painter, int startDrawnVectorIndex, int endDrawnVectorIndex);

    // Create the screen coordinates of the points to be drawn
    void GetScreenPoints(QVector<QPointF>& pointsVector, int startDrawnVectorIndex, int endDrawnVectorIndex) const;

    // Draw line taking special care when selected
    void DrawLinePlot(QCPPainter* painter, QVector<QPointF>& pointsVector);

    // Draw Fill if there is a mChannelFillGraph using the vector data
    void DrawVectorFill(QCPPainter* painter, QVector<QPointF>& lineData, int startDrawnVectorIndex, int endDrawnVectorIndex);

    /// reimplemented virtual method - because this class work on data vector instead of data map
    /// inherits documentation from base class
    /// \param pixelPoint - clicked/hovered point
    /// \returns destination from point
    double pointDistance(const QPointF& pixelPoint) const;

    // Get the allowed number of points to draw based on an initial value:
    int GetAllowedPointsToDraw(int& initialNumberPoints, int& lowerIndex, int& upperIndex) const;

    // Get the lowest draw index that is lower then the given index
    int GetPrecedingDrawIndex(int& vectorIndex);

protected:
    // The data as vector (replacing the map in the original QCPGraph)
    acDataVector* m_vectorData;

    // the average key interval used for estimation of position
    float m_keyInterval;

    // displayed indexed range
    int m_lastUsedUpperIndex;
    int m_lastAxisSpan;

    // displayed indexes
    gtVector<int> m_drawnIndexes;

    // Information needed to draw whole span
    // when reaching this number of indexes compression needs to be done
    int m_maxIndexesBeforeCompression;
    float m_keyIntervalForAddingPoint;
    bool m_drawWholeSpan;

    // Segments handling
    gtVector<acVectorLineGraphSegment> m_segmentsVector;
};
#endif // __ACVECTORLINEGRAPH_H
