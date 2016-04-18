//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppMultiLinePowerNonStackedPlot.h
///
//==================================================================================

#ifndef PP_MULTILINEPOWERNONSTACKEDPLOT
#define PP_MULTILINEPOWERNONSTACKEDPLOT

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <AMDTPowerProfiling/src/ppMultiLinePowerStackedPlot.h>

/// this class represent the Power non-stacked graph
class ppMultiLinePowerNonStackedPlot : public ppMultiLnePowerStackedPlot
{
    Q_OBJECT
public:

    ppMultiLinePowerNonStackedPlot(ppSessionController* pSessionController);

protected:
    /// sets the values of APU counter and other (APU value minus the selected counters values) into the values vector
    /// \param valVec the values vector to be set (in/out param)
    /// \apuCounterIndex is the Id of the APU counter
    /// \param cumulativeValue is the sum total value of the selected counters (besides APU)
    virtual void SetApuAndOtherCountersValueToVec(QVector<double>& valVec, int apuCounterIndex, double cumulativeValue);

    /// gets APU values in vector and creates from it data to be added to all graphs data vector
    /// \param graphData - is the data vector of all counters for building the graph. the APU data will be added to it
    /// \param apuValueVec is a vector of values of the APU counter
    virtual void SetTotalApuGraph(QVector<acMultiLinePlotItemData*>& graphData, const QVector<double>& apuValueVec);
};

#endif
