//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file generate occupancy charts
//==============================================================================

#include <iomanip>
#include <fstream>
#include <cmath>
#include <map>
#include "OccupancyChart.h"
#include "OccupancyUtils.h"
#include "../Common/jqPlotChart.h"
#include "../Common/FileUtils.h"
#include "../Common/Logger.h"
#include "../CLCommon/CLCUInfoBase.h"

using namespace std;

#define CHART_WIDTH "100%"
#define TABLE_WIDTH "100%"
#define TABLE_HEIGHT "300px"

static const unsigned int LDS_TABLE_NBR_BINS = 320;
static const size_t BYTE_2_KBYTE_CONVERSION = 1024;

void GenerateVGPRLimitedWFTables(CLCUInfoBase* cuDevice, jqPlotChart& chart, const OccupancyUtils::OccupancyParams& params)
{
    cuDevice->SetCUParam(CU_PARAMS_LDS_USED, params.m_nUsedLDS);

    if (params.m_gen >= GDT_HW_GENERATION_SOUTHERNISLAND && params.m_gen < GDT_HW_GENERATION_LAST)
    {
        cuDevice->SetCUParam(CU_PARAMS_SCALAR_GPRS_USED, params.m_nUsedSGPRS);
    }

    int stride = 4;
    size_t prevNumWaves = 0;
    size_t nextNumWaves = 0;

    // get the value for 0 VGPRs to be used in the first iteration below
    cuDevice->SetCUParam(CU_PARAMS_VECTOR_GPRS_USED, 0);
    cuDevice->ComputeNumActiveWaves(params.m_nWorkgroupSize, nextNumWaves);

    // We have a minimum of 2 wavefronts required on the compute unit, so just show
    // the number of VGPR that allows up to 2 wavefronts.  Extra VGPR on the x-axis
    // is not necessary.
    for (unsigned int i = 0; i <= params.m_nMaxVGPRS; i++)
    {
        size_t nUsedVGPRs = i == 0 ? 1 : i;

        // except for the first iteration, the number of waves for the current iteration was
        // calculated during the previous iteration (when checking boundaries). For the first
        // iteration, is was calculated above, before the for loop.
        size_t curNumWaves = nextNumWaves;

        // get the value for the next iteration (used in the current iteration for boundary checking)
        cuDevice->SetCUParam(CU_PARAMS_VECTOR_GPRS_USED, nUsedVGPRs + 1);
        cuDevice->ComputeNumActiveWaves(params.m_nWorkgroupSize, nextNumWaves);

        // check to see if the current item is a boundary -- if so always include the
        // point, regardless of stride
        bool isBoundary = (prevNumWaves != curNumWaves) || (nextNumWaves != curNumWaves);

        if (i % stride == 0 || isBoundary)
        {
            chart.AddData(jqPlotChartData((float)nUsedVGPRs, (float)curNumWaves));
        }

        prevNumWaves = curNumWaves;
    }
}

void GenerateSGPRLimitedWFTables(CLCUInfoBase* cuDevice, jqPlotChart& chart, const OccupancyUtils::OccupancyParams& params)
{
    cuDevice->SetCUParam(CU_PARAMS_LDS_USED, params.m_nUsedLDS);
    cuDevice->SetCUParam(CU_PARAMS_VECTOR_GPRS_USED, params.m_nUsedVGPRS);

    int stride = 2;
    size_t prevNumWaves = 0;
    size_t nextNumWaves = 0;

    // get the value for 0 SGPRs to be used in the first iteration below
    cuDevice->SetCUParam(CU_PARAMS_SCALAR_GPRS_USED, 0);
    cuDevice->ComputeNumActiveWaves(params.m_nWorkgroupSize, nextNumWaves);

    // We have a minimum of 2 wavefronts required on the compute unit, so just show
    // the number of SGPR that allows up to 2 wavefronts.  Extra SGPR on the x-axis
    // is not necessary.
    for (unsigned int i = 0; i <= params.m_nMaxSGPRS; i++)
    {
        size_t nUsedSGPRs = i == 0 ? 1 : i;

        // except for the first iteration, the number of waves for the current iteration was
        // calculated during the previous iteration (when checking boundaries). For the first
        // iteration, is was calculated above, before the for loop.
        size_t curNumWaves = nextNumWaves;

        // get the value for the next iteration (used in the current iteration for boundary checking)
        cuDevice->SetCUParam(CU_PARAMS_SCALAR_GPRS_USED, nUsedSGPRs + 1);
        cuDevice->ComputeNumActiveWaves(params.m_nWorkgroupSize, nextNumWaves);

        // check to see if the current item is a boundary -- if so always include the
        // point, regardless of stride
        bool isBoundary = (prevNumWaves != curNumWaves) || (nextNumWaves != curNumWaves);

        if (i % stride == 0 || isBoundary)
        {
            chart.AddData(jqPlotChartData((float)nUsedSGPRs, (float)curNumWaves));
        }

        prevNumWaves = curNumWaves;
    }
}


void GenerateWGLimitedWFTable(CLCUInfoBase* cuDevice, jqPlotChart& chart, const OccupancyUtils::OccupancyParams& params)
{
    cuDevice->SetCUParam(CU_PARAMS_LDS_USED, params.m_nUsedLDS);
    cuDevice->SetCUParam(CU_PARAMS_VECTOR_GPRS_USED, params.m_nUsedVGPRS);

    if (params.m_gen >= GDT_HW_GENERATION_SOUTHERNISLAND && params.m_gen < GDT_HW_GENERATION_LAST)
    {
        cuDevice->SetCUParam(CU_PARAMS_SCALAR_GPRS_USED, params.m_nUsedSGPRS);
    }

    for (unsigned int i = 1; i <= params.m_nMaxWGSize; ++i)
    {
        size_t activeWave;
        cuDevice->ComputeNumActiveWaves(i, activeWave);
        chart.AddData(jqPlotChartData((float)i, (float)activeWave));
    }
}

void GenerateLDSLimitedWFTable(CLCUInfoBase* cuDevice, jqPlotChart& chart, const OccupancyUtils::OccupancyParams& params)
{
    cuDevice->SetCUParam(CU_PARAMS_VECTOR_GPRS_USED, params.m_nUsedVGPRS);
    unsigned int stride = 1024;

    if (params.m_gen >= GDT_HW_GENERATION_SOUTHERNISLAND && params.m_gen < GDT_HW_GENERATION_LAST)
    {
        cuDevice->SetCUParam(CU_PARAMS_SCALAR_GPRS_USED, params.m_nUsedSGPRS);
    }

    for (unsigned int i = 0; i <= params.m_nMaxLDS; i += stride)
    {
        cuDevice->SetCUParam(CU_PARAMS_LDS_USED, i);
        size_t activeWave;
        cuDevice->ComputeNumActiveWaves(params.m_nWorkgroupSize, activeWave);
        chart.AddData(jqPlotChartData(((float)i / BYTE_2_KBYTE_CONVERSION), (float)activeWave));
    }
}

void CreateInfoTable(const OccupancyUtils::OccupancyParams& params, ostream& os, const string& strLimitingFactor)
{
    string strDeviceName = params.m_strDeviceName;
    size_t numCU = params.m_nNbrComputeUnits;
    size_t maxWavePerCU = params.m_nMaxWavesPerCU;
    size_t waveSize = params.m_nWavefrontSize;

    string strKernelName = params.m_strKernelName;
    size_t usedLDS = params.m_nUsedLDS;
    size_t flattenedGroupWorkSize = params.m_nWorkgroupSize;
    size_t flattenedGlobalWorkSize = params.m_nGlobalWorkSize;
    size_t numWavePerGroup = params.m_nWavesPerWG;

    size_t maxLDS = params.m_nMaxLDS;
    size_t maxGroupWorkSize = params.m_nMaxWGSize;
    size_t maxGlobalWorkSize = params.m_nMaxGlobalWorkSize;
    size_t maxWavePerGroup = params.m_nMaxWavesPerWG;

    size_t numWaveLimitedByLDS = params.m_nLDSLimitedWaveCount;
    size_t numWaveLimitedByWGS = params.m_nWGLimitedWaveCount;

    os << "<table style=\"width:100%;height:" TABLE_HEIGHT ";\">" << endl;
    os << "<th>Variable</th>	<th>Value</th>	<th>Device Limit</th>" << endl;
    os << "<tr><td colspan=\"3\" class=\"title_row\">Device Info</td></tr>" << endl;
    os << "<tr><td>Device name</td>	<td>" << strDeviceName << "</td>	<td></td></tr>" << endl;
    os << "<tr><td>Number of compute units</td>	<td>" << numCU << "</td>	<td></td></tr>" << endl;
    os << "<tr><td>Max number of waves per compute unit</td>	<td>" << maxWavePerCU << "</td>	<td></td></tr>" << endl;

    os << "<tr><td>Max number of work-groups per compute unit</td>	<td>" << params.m_nMaxWGPerCU << "</td>	<td></td></tr>" << endl;
    os << "<tr><td>Wavefront size</td>	<td>" << waveSize << "</td>	<td></td></tr>" << endl;

    os << "<tr><td colspan = \"3\" class=\"title_row\">Kernel Info</td></tr>" << endl;
    os << "<tr><td>Kernel name</td>	<td>" << strKernelName << "</td>	<td></td></tr>" << endl;
    os << "<tr><td>Vector GPR usage per work-item</td>	<td>" << params.m_nUsedVGPRS << "</td>	<td>" << params.m_nMaxVGPRS << "</td></tr>" << endl;

    if (params.m_gen >= GDT_HW_GENERATION_SOUTHERNISLAND && params.m_gen < GDT_HW_GENERATION_LAST)
    {
        os << "<tr><td>Scalar GPR usage per work-item</td>	<td>" << params.m_nUsedSGPRS << "</td>	<td>" << params.m_nMaxSGPRS << "</td></tr>" << endl;
    }

    os << "<tr><td>LDS usage per work-group</td>	<td>" << usedLDS << "</td>	<td>" << maxLDS << "</td></tr>" << endl;
    os << "<tr><td>Flattened work-group size</td>	<td>" << flattenedGroupWorkSize << "</td>	<td>" << maxGroupWorkSize << "</td></tr>" << endl;
    os << "<tr><td>Flattened global work size</td>	<td>" << flattenedGlobalWorkSize << "</td>	<td>" << maxGlobalWorkSize << "</td></tr>" << endl;
    os << "<tr><td>Number of waves per work-group</td>	<td>" << numWavePerGroup << "</td>	<td>" << maxWavePerGroup << "</td></tr>" << endl;

    os << "<tr><td colspan = \"3\" class=\"title_row\">Kernel Occupancy</td></tr>" << endl;
    os << "<tr><td>Number of waves limited by Vector GPR and Work-group size</td>	<td>" << params.m_nVGPRLimitedWaveCount << "</td>	<td>" << maxWavePerCU << "</td></tr>" << endl;

    if (params.m_gen >= GDT_HW_GENERATION_SOUTHERNISLAND && params.m_gen < GDT_HW_GENERATION_LAST)
    {
        os << "<tr><td>Number of waves limited by Scalar GPR and Work-group size</td>	<td>" << params.m_nSGPRLimitedWaveCount << "</td>	<td>" << maxWavePerCU << "</td></tr>" << endl;
    }

    os << "<tr><td>Number of waves limited by LDS and Work-group size</td>	<td>" << numWaveLimitedByLDS << "</td>	<td>" << maxWavePerCU << "</td></tr>" << endl;
    os << "<tr><td>Number of waves limited by Work-group size</td>	<td>" << numWaveLimitedByWGS << "</td>	<td>" << maxWavePerCU << "</td></tr>" << endl;
    os << "<tr class=\"hightlight_row\"><td>Limiting factor(s)</b></td>	<td>" << strLimitingFactor << "</td>	<td></td></tr>" << endl;
    os.precision(4);
    os << "<tr><td>Estimated occupancy</td>	<td>" << params.m_fOccupancy << "%</td>	<td></td></tr>" << endl;
    os << "</table>" << endl;
}

void AddDisableSelectionCode(ostream& os)
{
    const std::string constUserSelectNoneString("-user-select: none;");
    os << " ";
    os << "style=";

    os << "'";

    os << "-ms" << constUserSelectNoneString;
    os << "-moz" << constUserSelectNoneString;
    os << "-webkit" << constUserSelectNoneString;

    os << "'";
}

bool GenerateOccupancyChart(const OccupancyUtils::OccupancyParams& params, const string& strFileName, std::string& strError)
{
    if (strFileName.empty())
    {
        strError = "No output file specified";
        return false;
    }

    ofstream fout;
    fout.open(strFileName.c_str());

    if (fout.fail())
    {
        strError = "Unable to open output file: " + strFileName;
        return false;
    }

    string outputPath;

    if (!FileUtils::GetWorkingDirectory(strFileName, outputPath))
    {
        strError = "Unable to determine ouput directory";
        return false;
    }

    if (!jqPlotChart::CopyScripts(outputPath))
    {
        strError = "Unable to copy required files to output directory";
        return false;
    }

    size_t nMaxNumWavefronts = params.m_nMaxWavesPerCU;


    CLCUInfoBase* cuDevice = NULL;

    if (params.m_gen >= GDT_HW_GENERATION_VOLCANICISLAND && params.m_gen < GDT_HW_GENERATION_LAST)
    {
        cuDevice = new CLCUInfoVI();
    }
    else if (params.m_gen == GDT_HW_GENERATION_SOUTHERNISLAND || params.m_gen == GDT_HW_GENERATION_SEAISLAND)
    {
        cuDevice = new CLCUInfoSI();
    }
    else
    {
        return false;
    }

    cuDevice->SetCUParam(CU_PARAMS_KERNEL_NAME,        params.m_strKernelName);
    cuDevice->SetCUParam(CU_PARAMS_DEVICE_NAME,        params.m_strDeviceName);
    cuDevice->SetCUParam(CU_PARAMS_VECTOR_GPRS_MAX,    params.m_nMaxVGPRS);
    cuDevice->SetCUParam(CU_PARAMS_SCALAR_GPRS_MAX,    params.m_nMaxSGPRS);
    cuDevice->SetCUParam(CU_PARAMS_LDS_MAX,            params.m_nMaxLDS);
    cuDevice->SetCUParam(CU_PARAMS_VECTOR_GPRS_USED,   params.m_nUsedVGPRS);
    cuDevice->SetCUParam(CU_PARAMS_SCALAR_GPRS_USED,   params.m_nUsedSGPRS);
    cuDevice->SetCUParam(CU_PARAMS_LDS_USED,           params.m_nUsedLDS);
    cuDevice->SetCUParam(CU_PARAMS_WG_SIZE_MAX,        params.m_nMaxWGSize);
    cuDevice->SetCUParam(CU_PARAMS_GLOBAL_SIZE_MAX,    params.m_nMaxGlobalWorkSize);

    size_t nActiveWaves;
    cuDevice->ComputeNumActiveWaves(params.m_nWorkgroupSize, nActiveWaves);

    jqPlotChart chart1;
    jqPlotChart chart2;
    jqPlotChart chart3;

    chart1.m_strChartID = "chartVGPR";
    chart1.m_strChartName = "Number of waves limited by VGPRs";
    chart1.m_strXAxisName = "Number of VGPRs";
    chart1.m_strYAxisName = "Number of active wavefronts";
    chart1.m_strTooltipFormatString = "VGPR = %s<br/>Wave = %s";

    vector<float> ticksVGPR;
    ticksVGPR.push_back(0);
    ticksVGPR.push_back(64);
    ticksVGPR.push_back(128);
    ticksVGPR.push_back(192);
    ticksVGPR.push_back(256);
    chart1.SetXAxisTicks(ticksVGPR);

    float fPad = 1.2f;

    chart1.m_uiMaxXAxis = (unsigned int)((float)params.m_nMaxVGPRS * fPad);
    chart1.m_uiMinXAxis = 0;
    chart1.m_uiMaxYAxis = (unsigned int)(fPad * (float)params.m_nMaxWavesPerCU);
    chart1.m_uiMinYAxis = 0;
    chart1.AddData(jqPlotChartData((float)params.m_nUsedVGPRS, (float)nActiveWaves, true));

    jqPlotChart chartSGPR;

    if (params.m_gen >= GDT_HW_GENERATION_SOUTHERNISLAND && params.m_gen < GDT_HW_GENERATION_LAST)
    {
        chartSGPR.m_strChartID = "chartSGPR";
        chartSGPR.m_strChartName = "Number of waves limited by SGPRs";
        chartSGPR.m_strXAxisName = "Number of SGPRs";
        chartSGPR.m_strYAxisName = "Number of active wavefronts";
        chartSGPR.m_strTooltipFormatString = "SGPR = %s<br/>Wave = %s";

        vector<float> ticksSGPR;
        ticksSGPR.push_back(0);
        ticksSGPR.push_back(26);
        ticksSGPR.push_back(52);
        ticksSGPR.push_back(78);
        ticksSGPR.push_back(102);
        chartSGPR.SetXAxisTicks(ticksSGPR);

        chartSGPR.m_uiMaxXAxis = (unsigned int)((float)params.m_nMaxSGPRS * fPad);
        chartSGPR.m_uiMinXAxis = 0;
        chartSGPR.m_uiMaxYAxis = (unsigned int)(fPad * (float)params.m_nMaxWavesPerCU);
        chartSGPR.m_uiMinYAxis = 0;
        chartSGPR.AddData(jqPlotChartData((float)params.m_nUsedSGPRS, (float)nActiveWaves, true));
    }

    // Chart for LDS
    chart2.m_strChartID = "chartLDS";
    chart2.m_strChartName = "Number of waves limited by LDS";
    chart2.m_strXAxisName = "LDS Size ";
    chart2.m_strYAxisName = "Number of active wavefronts";
    chart2.m_strTooltipFormatString = "LDS = %s <br/>Wave = %s";
    chart2.m_uiMaxXAxis = (unsigned int)(fPad * params.m_nMaxLDS / BYTE_2_KBYTE_CONVERSION);
    chart2.m_uiMinXAxis = 0;
    chart2.m_uiMaxYAxis = (unsigned int)(fPad * (float)nMaxNumWavefronts);
    chart2.m_uiMinYAxis = 0;
    chart2.AddData(jqPlotChartData(((float)params.m_nUsedLDS / BYTE_2_KBYTE_CONVERSION), (float)nActiveWaves, true));

    vector<float> ticksLDS;
    ticksLDS.push_back(0.0f);
    ticksLDS.push_back(8.0f);
    ticksLDS.push_back(16.0f);
    ticksLDS.push_back(24.0f);
    ticksLDS.push_back(32.0f);
    chart2.m_strTickXFormatString = "%4.1fk";
    chart2.SetXAxisTicks(ticksLDS);

    // Chart for Workgroup size
    chart3.m_strChartID = "chartWGS";
    chart3.m_strChartName = "Number of waves limited by Work-group size";
    chart3.m_strXAxisName = "Work-group size";
    chart3.m_strYAxisName = "Number of active wavefronts";
    chart3.m_strTooltipFormatString = "Work-group size = %s<br/>Wave = %s";
    chart3.m_uiMaxXAxis = (unsigned int)(fPad * params.m_nMaxWGSize);
    chart3.m_uiMinXAxis = 0;
    chart3.m_uiMaxYAxis = (unsigned int)(fPad * (float)nMaxNumWavefronts);
    chart3.m_uiMinYAxis = 0;
    chart3.AddData(jqPlotChartData((float)params.m_nWorkgroupSize, (float)nActiveWaves, true));

    vector<float> ticksWG;
    ticksWG.push_back(0);
    ticksWG.push_back(64);
    ticksWG.push_back(128);
    ticksWG.push_back(192);
    ticksWG.push_back(256);
    chart3.SetXAxisTicks(ticksWG);

    string strLimitingFactor;
    strLimitingFactor.clear();

    if (params.m_fOccupancy < 100.0f)
    {
        size_t numLimitingVal = min(min(params.m_nLDSLimitedWaveCount, params.m_nVGPRLimitedWaveCount), params.m_nWGLimitedWaveCount);

        if (params.m_gen >= GDT_HW_GENERATION_SOUTHERNISLAND && params.m_gen < GDT_HW_GENERATION_LAST)
        {
            numLimitingVal = min((size_t)params.m_nSGPRLimitedWaveCount, numLimitingVal);

            if (params.m_nUsedSGPRS != 0 && params.m_nSGPRLimitedWaveCount == numLimitingVal)
            {
                strLimitingFactor += "SGPR";
                chartSGPR.m_strTitleColor = "#FF0000";
            }
        }

        if (params.m_nUsedVGPRS != 0 && params.m_nVGPRLimitedWaveCount == numLimitingVal)
        {
            if (!strLimitingFactor.empty())
            {
                strLimitingFactor += ", ";
            }

            strLimitingFactor += "VGPR";
            chart1.m_strTitleColor = "#FF0000";
        }

        if (params.m_nUsedLDS != 0 && params.m_nLDSLimitedWaveCount == numLimitingVal)
        {
            if (!strLimitingFactor.empty())
            {
                strLimitingFactor += ", ";
            }

            strLimitingFactor += "LDS";
            chart2.m_strTitleColor = "#FF0000";
        }

        if (params.m_nWGLimitedWaveCount == numLimitingVal)
        {
            if (!strLimitingFactor.empty())
            {
                strLimitingFactor += ", ";
            }

            strLimitingFactor += "Work-group size";
            chart3.m_strTitleColor = "#FF0000";
        }
    }
    else
    {
        strLimitingFactor = "None";
    }

    GenerateVGPRLimitedWFTables(cuDevice, chart1, params);
    GenerateLDSLimitedWFTable(cuDevice, chart2, params);
    GenerateWGLimitedWFTable(cuDevice, chart3, params);

    if (params.m_gen >= GDT_HW_GENERATION_SOUTHERNISLAND && params.m_gen < GDT_HW_GENERATION_LAST)
    {
        GenerateSGPRLimitedWFTables(cuDevice, chartSGPR, params);
    }

    // IE9 requires this tag to support convas
    fout << "<!DOCTYPE html>" << endl;
    fout << "<!-- saved from url=(0014)about:internet -->" << endl;
    fout << "<!--[if lt IE 9]><script language=\"javascript\" type=\"text/javascript\" src=\"excanvas.min.js\"></script><![endif]-->" << endl;
    fout << "<html lang=\"en\">" << endl;
    fout << "<head>" << endl;
    fout << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">" << endl;
    fout << "<title>Kernel Occupancy</title>" << endl;

    // insert style

    fout << "<style type=\"text/css\">" << endl;
    fout << "th {" << endl;
    fout << "   font-size:1em;" << endl;
    fout << "   text-align:left;" << endl;
    fout << "   padding-top:5px;" << endl;
    fout << "   padding-bottom:4px;" << endl;
    fout << "   background-color:#969696;" << endl;
    fout << "   color:#fff;" << endl;
    fout << "}" << endl;
    fout << "td, th {" << endl;
    fout << "   font-size:0.85em;" << endl;
    fout << "   border:1px solid #969696;" << endl;
    fout << "   padding:3px 7px 2px 7px;" << endl;
    fout << "}" << endl;
    fout << "table {" << endl;
    fout << "   font-family:\"Trebuchet MS\", Arial, Helvetica, sans-serif;" << endl;
    fout << "   width:" TABLE_WIDTH ";" << endl;
    fout << "   border-collapse:collapse;" << endl;
    fout << "   margin-left: auto;" << endl;
    fout << "   margin-right: auto;" << endl;
    fout << "}" << endl;

    fout << ".title_row" << endl;
    fout << "{" << endl;
    fout << "   background-color:#C8BBBE;" << endl;
    fout << "   color:#fff;" << endl;
    fout << "}" << endl;

    fout << ".hightlight_row" << endl;
    fout << "{" << endl;
    fout << "   color:#FF0000;" << endl;
    fout << "   font-weight:bold;" << endl;
    fout << "}" << endl;
    fout << "</style>" << endl;

    jqPlotChart::WriteScriptDesc(fout);

    fout << "<script class=\"code\" language=\"javascript\" type=\"text/javascript\">" << endl;
    fout << "$(document).ready(function(){" << endl;

    // insert table here
    // Chart for GPR
    chart1.WriteToStream(fout);

    if (params.m_gen >= GDT_HW_GENERATION_SOUTHERNISLAND && params.m_gen < GDT_HW_GENERATION_LAST)
    {
        chartSGPR.WriteToStream(fout);
    }

    chart2.WriteToStream(fout);
    chart3.WriteToStream(fout);


    fout << "});" << endl;

    fout << "</script>" << endl;

    fout << "</head>" << endl;


    // document body
    fout << "<body";
    AddDisableSelectionCode(fout);
    fout << ">" << endl;

    // create table
    fout << "<table>" << endl;

    int nCells = (params.m_gen >= GDT_HW_GENERATION_SOUTHERNISLAND  && params.m_gen < GDT_HW_GENERATION_LAST) ? 4 : 3;
    fout << "<tr>" << endl;

    // row 0, cell 0
    fout << "<td>" << endl;
    fout << "<div id=\"chartWGS\" style=\"width:" CHART_WIDTH ";height:" TABLE_HEIGHT ";\"></div>" << endl;
    fout << "</td>" << endl;

    // row 0, cell 1
    fout << "<td>" << endl;
    fout << "<div id=\"chartVGPR\" style=\"width:" CHART_WIDTH ";height:" TABLE_HEIGHT ";\"></div>" << endl;
    fout << "</td>" << endl;

    if (params.m_gen >= GDT_HW_GENERATION_SOUTHERNISLAND && params.m_gen < GDT_HW_GENERATION_LAST)
    {
        fout << "<td>" << endl;
        fout << "<div id=\"chartSGPR\" style=\"width:" CHART_WIDTH ";height:" TABLE_HEIGHT ";\"></div>" << endl;
        fout << "</td>" << endl;
    }

    // row 0, cell 2
    fout << "<td>" << endl;
    fout << "<div id=\"chartLDS\" style=\"width:" CHART_WIDTH ";height:" TABLE_HEIGHT ";\"></div>" << endl;
    fout << "</td>" << endl;

    fout << "</tr>" << endl;

    // row 1, cell 1
    fout << "<tr>" << endl;
    fout << "<td colspan=\"" << nCells << "\">" << endl;
    CreateInfoTable(params, fout, strLimitingFactor);
    fout << "</td>" << endl;

    fout << "</tr>" << endl;

    fout << "</table>" << endl;

    fout << "</body>" << endl;
    fout << "</html>" << endl;

    fout.close();

    strError.clear();

    SAFE_DELETE(cuDevice);

    return true;
}

