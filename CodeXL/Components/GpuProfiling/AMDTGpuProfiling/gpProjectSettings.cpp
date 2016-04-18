//------------------------------ gpProjectSettings.cpp ------------------------------

#include <qtIgnoreCompilerWarnings.h>

// Qt:
#include <QDomDocument>

// Infra:
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtASCIIStringTokenizer.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>

// AMDTGraphicsServerInterface:
#include <AMDTGraphicsServerInterface/Include/AMDTGraphicsServerInterface.h>

// Local:
#include <AMDTGpuProfiling/gpProjectSettings.h>
#include <AMDTGpuProfiling/gpExecutionMode.h>
#include <AMDTGpuProfiling/ProfileManager.h>


gpProjectSettings::gpProjectSettings() : m_shouldConnectAutomatically(1), m_connection(egpFirstDX12Connection), m_serverConnectionPort(GP_DEFAULT_PORT),
    m_processNumber("1"), m_processName("")
{

}

gpProjectSettings::~gpProjectSettings()
{

}

void gpProjectSettings::Initialize()
{
    // Get the counters lists from the server
    GetDeviceCountersInformation();

    // Load the preset lists
    LoadPresetCountersLists();
}

void gpProjectSettings::GetDeviceCountersInformation()
{
    gpExecutionMode* pModeManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();

    GT_IF_WITH_ASSERT(pModeManager != nullptr)
    {
        GraphicsServerCommunication* pServerComm = pModeManager->GetGraphicsServerComminucation();
        // Uri, 14/12/2015 - this should not be an assertion, or the initialization order should change.
        // When initializing the GPU Profiling plugin:
        //      ProfileManager::SetupGPUProfiling() ->
        //      gpExecutionMode::Initialize() (was previously code in the constructor) ->
        //      gpProjectSettings::Initialize() (was previously code in the constructor) ->
        //      This location
        // we try to get gpExecutionMode::m_pGraphicsServerCommunication.
        //
        // That member is only ever initialized / created in gpExecutionMode::OnStartFrameAnalysis()
        // Which obviously happens MUCH later than when the execution mode is created.
        //
        // There was a similar cross-dependency between gpProjectSettings and ProfileManager::m_pFrameAnalysisMode,
        // Which I have already fixed in changelist 552110.
        //
        // This looping dependency ALWAYS triggers an assertion and causes the counters map to never get initialized!
#pragma message("FIXME!")

        // GT_IF_WITH_ASSERT(pServerComm != nullptr)
        if (nullptr != pServerComm)
        {
            gtASCIIString countersStr;
            bool rc = pServerComm->GetCounters(countersStr);
            GT_IF_WITH_ASSERT(rc)
            {
                QDomDocument countersDoc;
                countersDoc.setContent(acGTASCIIStringToQString(countersStr));
                // get all the counters nodes
                QDomNodeList counters = countersDoc.elementsByTagName(GPU_STR_perfXMLCounterNode);
                int numCounters = counters.size();

                for (int nCounter = 0; nCounter < numCounters; nCounter++)
                {
                    QDomNode currentCounter = counters.item(nCounter);
                    QDomElement nameElement = currentCounter.firstChildElement(GPU_STR_perfXMLNameNode);
                    QDomElement descElement = currentCounter.firstChildElement(GPU_STR_perfXMLDescriptionNode);
                    QDomElement dataTypeElement = currentCounter.firstChildElement(GPU_STR_perfXMLDatatypeNode);
                    QDomElement usageElement = currentCounter.firstChildElement(GPU_STR_perfXMLUsageNode);
                    GT_IF_WITH_ASSERT(!nameElement.isNull() && !descElement.isNull() && !dataTypeElement.isNull() && !usageElement.isNull())
                    {
                        gpPerfCounter* pCounter = new gpPerfCounter;
                        pCounter->m_id = nCounter;

                        pCounter->m_name = nameElement.text();
                        pCounter->m_description = descElement.text();
                        pCounter->m_dataType = dataTypeElement.text();
                        pCounter->m_usage = usageElement.text();

                        // get the parent node from the desc it is in the format #parent#desc
                        int parentEnd = pCounter->m_description.lastIndexOf('#');
                        GT_IF_WITH_ASSERT(parentEnd != -1 && pCounter->m_description[0] == '#')
                        {
                            pCounter->m_parent = pCounter->m_description.mid(1, parentEnd - 1);
                            pCounter->m_description = pCounter->m_description.mid(parentEnd + 1);
                        }
                        m_countersMap[pCounter->m_name] = pCounter;
                    }
                }
            }
        }
    }
}

void gpProjectSettings::LoadPresetCountersLists()
{
    osFilePath presetPath(osFilePath::OS_CODEXL_BINARIES_PATH);
    presetPath.appendSubDirectory(OS_STR_CodeXLDataDirName);
    presetPath.setFileName(GPU_STR_presetFileName);
    presetPath.setFileExtension(GPU_STR_presetFileExt);
    osFile presetFile(presetPath);

    bool rc = presetFile.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_READ);
    GT_IF_WITH_ASSERT(rc)
    {
        gtASCIIString presetFileAsString;
        presetFile.readIntoString(presetFileAsString);
        gtASCIIStringTokenizer fileAsLines(presetFileAsString, "\n");

        // The format of each preset is: preset name:counter name, counter name...
        gtASCIIString presetAsString;

        while (fileAsLines.getNextToken(presetAsString))
        {
            int endOfpresetName = presetAsString.find(':');
            GT_IF_WITH_ASSERT(endOfpresetName != -1)
            {
                gtASCIIString setName;
                gtASCIIString setCounters;
                presetAsString.getSubString(0, endOfpresetName - 1, setName);
                presetAsString.getSubString(endOfpresetName + 1, presetAsString.length(), setCounters);
                QStringList countresList = acGTASCIIStringToQString(setCounters).split(',');

                m_presetCountersLists[acGTASCIIStringToQString(setName)] = countresList;
            }
        }

        presetFile.close();
    }
}

void gpProjectSettings::SavePresetCountersLists()
{
    osFilePath presetPath(osFilePath::OS_CODEXL_BINARIES_PATH);
    presetPath.appendSubDirectory(OS_STR_CodeXLDataDirName);
    presetPath.setFileName(GPU_STR_presetFileName);
    presetPath.setFileExtension(GPU_STR_presetFileExt);
    osFile presetFile(presetPath);

    bool rc = presetFile.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
    GT_IF_WITH_ASSERT(rc)
    {
        std::map<QString, QStringList>::iterator countersIt = m_presetCountersLists.begin();

        while (countersIt != m_presetCountersLists.end())
        {
            gtString outputLine;
            outputLine.appendFormattedString(L"%ls:", acQStringToGTString((*countersIt).first).asCharArray());
            QString joinedString = (*countersIt).second.join(',');
            outputLine.appendFormattedString(L"%ls\n", acQStringToGTString(joinedString).asCharArray());

            presetFile.writeString(outputLine);
            countersIt++;
        }

        presetFile.close();
    }
}

void gpProjectSettings::AddSelectedCounter(int counterId)
{
    if (gtFind(m_sessionSelectedCounters.begin(), m_sessionSelectedCounters.end(), counterId) == m_sessionSelectedCounters.end())
    {
        m_sessionSelectedCounters.push_back(counterId);
    }
}
