//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class generate jqPlot chart, jqPlot is a 3rd party html5
///        GUI library build on top of jQuery. MIT and GPL2 licenses.
//==============================================================================

#include <sstream>
#include "jqPlotChart.h"
#include "OSUtils.h"
#include "FileUtils.h"

using namespace std;

#define JQUERY_JS "jquery.min.js"
#define JPLOT_JS "jquery.jqplot.min.js"
#define JPLOT_CANVAS_AXIS_LABEL_RENDERER_JS "jqplot.canvasAxisLabelRenderer.min.js"
#define JPLOT_CANVAS_TEXT_RENDERER_JS "jqplot.canvasTextRenderer.min.js"
#define JPLOT_HIGHLIGHTER_JS "jqplot.highlighter.min.js"
#define JPLOT_CSS "jquery.jqplot.min.css"
#define EXCANVAS "excanvas.min.js"

jqPlotChart::jqPlotChart(void)
{
    m_strXAxisName = "Unnamed";
    m_strYAxisName = "Unnamed";
    m_strChartName = "Unnamed";
    m_strChartID = "diagram0";

    m_bSetFont = false;
    m_strLabelFont = "Arial";
    m_strLabelFontSize = "12px";
    m_strTooltipFontSize = "12px";
    m_strTitleColor.clear();

    // First arg = x value, Second arg = y value
    m_strTooltipFormatString = "x = %s, y = %s";

    m_uiMaxXAxis = 100;
    m_uiMinXAxis = 0;
    m_uiMaxYAxis = 100;
    m_uiMinYAxis = 0;

    m_strTickYFormatString = m_strTickXFormatString = "%i";
}

jqPlotChart::~jqPlotChart(void)
{
}

bool jqPlotChart::CopyJqPlotFile(std::string destPath, std::string fileName)
{
    std::string destFilePath = destPath + fileName;

    // remove previously-copied file (it is probably readonly and then the copy would fail)
    if (FileUtils::FileExist(destFilePath))
    {
        remove(destFilePath.c_str());
    }

    string scriptSourcePath = FileUtils::GetExePath() + "/jqPlot/" + fileName;

    return OSUtils::Instance()->osCopyFile(scriptSourcePath.c_str(), destFilePath.c_str());
}

bool jqPlotChart::CopyScripts(const std::string& strOutputPath)
{

    bool bRet = true;
    bRet &= CopyJqPlotFile(strOutputPath, JPLOT_JS);
    bRet &= CopyJqPlotFile(strOutputPath, JQUERY_JS);
    bRet &= CopyJqPlotFile(strOutputPath, JPLOT_CANVAS_AXIS_LABEL_RENDERER_JS);
    bRet &= CopyJqPlotFile(strOutputPath, JPLOT_CANVAS_TEXT_RENDERER_JS);
    bRet &= CopyJqPlotFile(strOutputPath, JPLOT_HIGHLIGHTER_JS);
    bRet &= CopyJqPlotFile(strOutputPath, JPLOT_CSS);
    bRet &= CopyJqPlotFile(strOutputPath, EXCANVAS);

    return bRet;
}

void jqPlotChart::WriteScriptDesc(ostream& os)
{
    // include css
    os << "<link rel=\"stylesheet\" type=\"text/css\" href=\"./jquery.jqplot.min.css\" />" << endl;

    // include jQuery
    os << "<script language=\"javascript\" type=\"text/javascript\" src=\"" << "./jquery.min.js\"></script>" << endl;

    // include jqPlot
    os << "<script language=\"javascript\" type=\"text/javascript\" src=\"" << "./jquery.jqplot.min.js\"></script>" << endl;
    os << "<script language=\"javascript\" type=\"text/javascript\" src=\"" << "./jqplot.highlighter.min.js\"></script>" << endl;
    os << "<script language=\"javascript\" type=\"text/javascript\" src=\"" << "./jqplot.canvasAxisLabelRenderer.min.js\"></script>" << endl;
    os << "<script language=\"javascript\" type=\"text/javascript\" src=\"" << "./jqplot.canvasTextRenderer.min.js\"></script>" << endl;
}

void jqPlotChart::WriteToStream(ostream& os)
{
    // generate table
    os << "   $.jqplot.config.enablePlugins = true;" << endl;

    os << "   var data_" << m_strChartID << " = [" << endl;

    bool bAnySelected = false;
    stringstream ss;
    ss << "   var data_" << m_strChartID << "_selected = [" << endl;

    bool bFirstItem = true;
    bool bFirstItemSelected = true;

    for (size_t i = 0; i != m_data.size(); ++i)
    {
        if (!m_data[i].bSelected)
        {
            if (!bFirstItem)
            {
                os << "," << endl;
            }

            os << "   [" << m_data[i].x << ", " << m_data[i].y << "]";
            bFirstItem = false;
        }
        else
        {
            if (!bFirstItemSelected)
            {
                ss << "," << endl;
            }

            // need to create two data sets
            bAnySelected = true;
            ss << "   [" << m_data[i].x << ", " << m_data[i].y << "]";
            bFirstItemSelected = false;
        }
    }

    os << endl << "   ];" << endl;
    ss << endl << "   ];" << endl;

    if (!bAnySelected)
    {
        os << "   _" << m_strChartID << " = $.jqplot('" << m_strChartID << "', [data_" << m_strChartID << "], {" << endl;
    }
    else
    {
        os << ss.str();
        os << "   _" << m_strChartID << " = $.jqplot('" << m_strChartID << "', [data_" << m_strChartID << ", data_" << m_strChartID << "_selected], {" << endl;
    }

    os << "      title: {" << endl;
    os << "      text: '" << m_strChartName << "'" << endl;

    if (m_bSetFont)
    {
        os << "      ,fontFamily: '" << m_strLabelFont << "'," << endl;
        os << "      fontSize: '" << m_strLabelFontSize << "'" << endl;
    }

    if (!m_strTitleColor.empty())
    {
        os << "      ,color: '" << m_strTitleColor << "'" << endl;
    }

    os << "      }," << endl;

    if (bAnySelected)
    {
        os << "      series:[" << endl;
        os << "         {" << endl;
        os << "            // Normal markers" << endl;
        os << "            markerOptions: { size:5, lineWidth: 1 }" << endl;
        os << "         }," << endl;
        os << "         {" << endl;
        os << "            // Selected markers" << endl;
        os << "            markerOptions: { style:'filledSquare', lineWidth: 1 }" << endl;
        os << "         }" << endl;
        os << "      ]," << endl;
    }

    os << "      axes: {" << endl;
    os << "         xaxis: {" << endl;
    os << "            min: " << m_uiMinXAxis << "," << endl;
    os << "            max: " << m_uiMaxXAxis << "," << endl;
    os << "            label: \"" << m_strXAxisName << "\"," << endl;
    os << "            labelRenderer: $.jqplot.CanvasAxisLabelRenderer," << endl;

    if (m_bSetFont)
    {
        os << "            labelOptions: {" << endl;
        os << "              fontFamily: '" << m_strLabelFont << "'," << endl;
        os << "              fontSize: '" << m_strLabelFontSize << "'" << endl;
        os << "            }," << endl;
    }

    if (m_ticksX.empty())
    {
        os << "            numberTicks: 4," << endl;
    }
    else
    {
        os << "            ticks: [";

        for (size_t i = 0; i < m_ticksX.size(); ++i)
        {
            if (i != 0)
            {
                os << ',';
            }

            os << m_ticksX[i];
        }

        os << "]," << endl;
    }

    os << "            tickOptions:" << endl;
    os << "            {" << endl;
    os << "               formatString: '" << m_strTickXFormatString << "'" << endl;
    os << "            }" << endl;
    os << "         }," << endl;
    os << "         yaxis: {" << endl;
    os << "            min: " << m_uiMinYAxis << "," << endl;
    os << "            max: " << m_uiMaxYAxis << "," << endl;
    os << "            label: \"" << m_strYAxisName << "\"," << endl;
    os << "            labelRenderer: $.jqplot.CanvasAxisLabelRenderer," << endl;

    if (m_bSetFont)
    {
        os << "            labelOptions: {" << endl;
        os << "              fontFamily: '" << m_strLabelFont << "'," << endl;
        os << "              fontSize: '" << m_strLabelFontSize << "'" << endl;
        os << "            }" << endl;
    }

    if (!m_ticksY.empty())
    {
        os << "            ticks: [";

        for (size_t i = 0; i < m_ticksY.size(); ++i)
        {
            if (i != 0)
            {
                os << ',';
            }

            os << m_ticksY[i];
        }

        os << "]," << endl;
    }

    os << "            tickOptions:" << endl;
    os << "            {" << endl;
    os << "               formatString: '" << m_strTickYFormatString << "'" << endl;
    os << "            }" << endl;

    os << "         }" << endl;
    os << "      }," << endl;
    os << "      highlighter: { " << endl;
    //os << "         bringSeriesToFront: true, " << endl;
    os << "         tooltipLocation: 'e', " << endl;
    os << "         tooltipOffset: 0, " << endl;
    os << "         formatString: '<div class=\"jqplot-highlighter\"><span style=\"font-size:" << m_strTooltipFontSize << "\">" << m_strTooltipFormatString << "</span></div>' " << endl;
    os << "      }" << endl;
    os << "   });" << endl;
}
