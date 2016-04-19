//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class generate jqPlot chart, jqPlot is a 3rd party html5
///        GUI library build on top of jQuery. MIT and GPL2 licenses.
//==============================================================================

#ifndef _JQ_PLOT_CHART_H_
#define _JQ_PLOT_CHART_H_

#include <ostream>
#include <string>
#include <vector>

//------------------------------------------------------------------------------------
/// jqPlotChart data
//------------------------------------------------------------------------------------
struct jqPlotChartData
{
    float x;          ///< x value
    float y;          ///< y value
    bool bSelected;   ///< Data selected

    /// Constructor
    /// \param _x x value
    /// \param _y y value
    /// \param _bSelected data selected
    jqPlotChartData(float _x, float _y, bool _bSelected = false) : x(_x), y(_y), bSelected(_bSelected) {}
};

//------------------------------------------------------------------------------------
/// jqPlotChart class
//------------------------------------------------------------------------------------
class jqPlotChart
{
public:
    /// Constructor
    jqPlotChart(void);

    /// Destructor
    ~jqPlotChart(void);

    /// Write chart javascript code to output stream
    /// \param os Output stream
    void WriteToStream(std::ostream& os);

    /// Add data
    /// \param data chart data
    void AddData(jqPlotChartData data)
    {
        m_data.push_back(data);
    }

    /// Set X Axis ticks
    /// \param ticks input ticks
    void SetXAxisTicks(const std::vector<float>& ticks)
    {
        SetAxisTicks(ticks, m_ticksX);
    }

    /// Set Y Axis ticks
    /// \param ticks input ticks
    void SetYAxisTicks(const std::vector<float>& ticks)
    {
        SetAxisTicks(ticks, m_ticksY);
    }

    /// Write script includes
    /// \param os Output stream
    static void WriteScriptDesc(std::ostream& os);

    /// Copy jQuery, jqPlot script files to output path
    /// \param strOutputPath Output path
    static bool CopyScripts(const std::string& strOutputPath);

    std::string m_strXAxisName;                     ///< X Axis name
    std::string m_strYAxisName;                     ///< Y Axis name
    std::string m_strChartName;                     ///< Chart title
    std::string m_strChartID;                       ///< Chart ID
    bool        m_bSetFont;                         ///< Set font
    std::string m_strLabelFont;                     ///< Font
    std::string m_strLabelFontSize;                 ///< Font size
    std::string m_strTooltipFormatString;           ///< Tooltip format string
    std::string m_strTooltipFontSize;               ///< Font size
    std::string m_strTitleColor;                    ///< Title color

    std::string m_strTickXFormatString;             ///< X Axis tick format string
    std::string m_strTickYFormatString;             ///< Y Axis tick format string
    unsigned int m_uiMaxXAxis;                      ///< Max X Axis value
    unsigned int m_uiMinXAxis;                      ///< Min X Axis value

    unsigned int m_uiMaxYAxis;                      ///< Max Y Axis value
    unsigned int m_uiMinYAxis;                      ///< Min Y Axis value

private:
    /// Disable copy constructor
    jqPlotChart(const jqPlotChart& obj);

    /// Disable assignment operator
    jqPlotChart& operator = (const jqPlotChart& obj);

    /// Helper function that copy input ticks to destination ticks array
    /// \param ticks Input ticks
    /// \param dest destination ticks
    void SetAxisTicks(const std::vector<float>& ticks, std::vector<float>& dest)
    {
        dest.resize(ticks.size());
        std::copy(ticks.begin(), ticks.end(), dest.begin());
    }

    /// Copy the specified file from the script source path to the desination path, first removing the file in the destination if it already exists
    /// \param destPath the destination path of the file
    /// \param fileName the name of the file to copy
    /// \return true if file could be copied, false otherwise
    static bool CopyJqPlotFile(std::string destPath, std::string fileName);

    std::vector<jqPlotChartData> m_data;            ///< Data points

    std::vector<float> m_ticksX;                    ///< X Axis ticks
    std::vector<float> m_ticksY;                    ///< Y Axis ticks
};

#endif //_JQ_PLOT_CHART_H_
