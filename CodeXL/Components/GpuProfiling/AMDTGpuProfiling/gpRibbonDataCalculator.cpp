//------------------------------ gpRibbonDataCalculator.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>

// std
#include <algorithm>

// infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineGrid.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

// Local
#include <AMDTGpuProfiling/gpRibbonDataCalculator.h>
#include <AMDTGpuProfiling/gpTraceDataContainer.h>


#define GP_DEFAULT_BUCKET_NUMBER 2000


gpRibbonDataCalculator::gpRibbonDataCalculator(gpTraceDataContainer* pSessionData, acTimeline* pTimeLine):
    m_pTimeLine(pTimeLine),
    m_pData(pSessionData),
    m_wasConcurrencyCalculated(false)
{
    if (pTimeLine != nullptr)
    {
        m_pTimeLineGrid = pTimeLine->grid();
    }
}

void gpRibbonDataCalculator::CalculateCallsData()
{
    if (m_apiCallsSortedByTime.isEmpty())
    {
        GT_IF_WITH_ASSERT(m_pData != nullptr && m_pTimeLineGrid != nullptr && m_pTimeLine != nullptr)
        {
            // the apiID will be used so we'll have the same color as the time line graph
            int numThreads = m_pData->ThreadsCount();

            for (int nThread = 0; nThread < numThreads; nThread++)
            {
                osThreadId currentThread = m_pData->ThreadID(nThread);
                int numApi = m_pData->ThreadAPICount(currentThread);

                for (int nApiCall = 0; nApiCall < numApi; nApiCall++)
                {
                    ProfileSessionDataItem* pCurrentItem = m_pData->APIItem(currentThread, nApiCall);

                    GT_IF_WITH_ASSERT(pCurrentItem != nullptr)
                    {
                        double startTime = (pCurrentItem->StartTime() - m_pTimeLine->startTime());
                        quint64 duration = pCurrentItem->EndTime() - pCurrentItem->StartTime();

                        gpRibbonCallsData newData;
                        newData.m_startTime = startTime;
                        newData.m_duration = duration;
                        m_apiCallsSortedByTime.push_back(newData);
                    }
                }
            }

            // sort the data since we get the data by thread and not by time
            qSort(m_apiCallsSortedByTime.begin(), m_apiCallsSortedByTime.end());
        }
    }

    if (m_drawCallsSortedByTime.isEmpty())
    {
        GT_IF_WITH_ASSERT(m_pData != nullptr && m_pTimeLineGrid != nullptr && m_pTimeLine != nullptr)
        {
            int numThreads = m_pData->ThreadsCount();

            for (int nThread = 0; nThread < numThreads; nThread++)
            {
                osThreadId currentThread = m_pData->ThreadID(nThread);
                int numApi = m_pData->ThreadAPICount(currentThread);

                for (int nApiCall = 0; nApiCall < numApi; nApiCall++)
                {
                    ProfileSessionDataItem* pCurrentItem = m_pData->APIItem(currentThread, nApiCall);

                    GT_IF_WITH_ASSERT(pCurrentItem != nullptr)
                    {
                        eAPIType apiType;
                        bool rc = pCurrentItem->GetDX12APIType(apiType);

                        // add to the draw bucket
                        if ((pCurrentItem->ItemType().m_itemMainType == ProfileSessionDataItem::DX12_API_PROFILE_ITEM) && rc && (apiType == kAPIType_DrawCommand))
                        {
                            double startTime = (pCurrentItem->StartTime() - m_pTimeLine->startTime());
                            quint64 duration = (pCurrentItem->EndTime() - pCurrentItem->StartTime());

                            gpRibbonCallsData newData;
                            newData.m_startTime = startTime;
                            newData.m_duration = duration;
                            m_drawCallsSortedByTime.push_back(newData);
                        }
                    }
                }
            }

            // sort the data since we get the data by thread and not by time
            qSort(m_drawCallsSortedByTime.begin(), m_drawCallsSortedByTime.end());
        }
    }
}

void gpRibbonDataCalculator::CalculateData()
{
    // Calculate the API and draw calls data
    CalculateCallsData();
}

void gpRibbonDataCalculator::GetGPUOps(QVector<gpRibbonCallsData>& dataVector)
{
    GT_IF_WITH_ASSERT(m_pData != nullptr && m_pTimeLineGrid != nullptr && m_pTimeLine != nullptr)
    {
        int numQueues = m_pData->DX12QueuesCount();

        for (int nQueue = 0; nQueue < numQueues; nQueue++)
        {
            QString queueName = m_pData->QueueName(nQueue);
            int numGPUOps = m_pData->QueueItemsCount(queueName);

            for (int nGPUOp = 0; nGPUOp < numGPUOps; nGPUOp++)
            {
                ProfileSessionDataItem* pCurrentItem = m_pData->QueueItem(queueName, nGPUOp);
                GT_IF_WITH_ASSERT(pCurrentItem != nullptr)
                {
                    if (pCurrentItem->ItemType().m_itemMainType == ProfileSessionDataItem::DX12_GPU_PROFILE_ITEM)
                    {
                        gpRibbonCallsData newData;
                        newData.m_startTime = pCurrentItem->StartTime() - m_pTimeLine->startTime();
                        newData.m_duration = pCurrentItem->EndTime() - pCurrentItem->StartTime();
                        dataVector.push_back(newData);
                    }
                }
            }
        }
    }
}


void gpRibbonDataCalculator::CalculateCPUConcurrency()
{
    if (!m_wasConcurrencyCalculated)
    {
        if (m_cpuMaxThreadConcurrency.isEmpty() && m_cpuAverageThreadConcurrency.isEmpty() && m_cpuTotalThreadConcurrency.isEmpty())
        {
            // Each bucket has a vector of thread segment data
            QVector<QVector<gpRibbonSegmentData>> threadSegments;

            GT_IF_WITH_ASSERT(m_pData != nullptr && m_pTimeLineGrid != nullptr && m_pTimeLine != nullptr && GP_DEFAULT_BUCKET_NUMBER > 0)
            {
                double rangePerBucket = m_pTimeLineGrid->fullRange() * 1.0 / GP_DEFAULT_BUCKET_NUMBER;
                // Sanity check:
                GT_IF_WITH_ASSERT(rangePerBucket > 0)
                {
                    threadSegments.resize(GP_DEFAULT_BUCKET_NUMBER);
                    m_cpuMaxThreadConcurrency.resize(GP_DEFAULT_BUCKET_NUMBER);
                    m_cpuAverageThreadConcurrency.resize(GP_DEFAULT_BUCKET_NUMBER);
                    m_cpuTotalThreadConcurrency.resize(GP_DEFAULT_BUCKET_NUMBER);
                    // Count the entries per bucket
                    int numThreads = m_pData->ThreadsCount();

                    for (int nThread = 0; nThread < numThreads; nThread++)
                    {
                        osThreadId currentThread = m_pData->ThreadID(nThread);
                        QString threadIdAsStr = QString("%1").arg(currentThread);
                        acTimelineBranch* threadTimeLine = m_pTimeLine->getBranchFromText(threadIdAsStr, true);

                        bool threadIsVisible = false;

                        if (threadTimeLine != nullptr)
                        {
                            threadIsVisible = threadTimeLine->IsVisible();
                        }

                        if (threadIsVisible)
                        {
                            int numApi = m_pData->ThreadAPICount(currentThread);

                            for (int nApiCall = 0; nApiCall < numApi; nApiCall++)
                            {
                                ProfileSessionDataItem* pCurrentItem = m_pData->APIItem(currentThread, nApiCall);

                                GT_IF_WITH_ASSERT(pCurrentItem != nullptr)
                                {
                                    // Add the item segments to each of the buckets it is part of
                                    quint64 bucketStart = (int)((pCurrentItem->StartTime() - m_pTimeLine->startTime()) * 1.0 / rangePerBucket);
                                    quint64 bucketEnd = (int)((pCurrentItem->EndTime() - m_pTimeLine->startTime()) * 1.0 / rangePerBucket);

                                    for (int nBucket = bucketStart; nBucket <= bucketEnd && nBucket < GP_DEFAULT_BUCKET_NUMBER; nBucket++)
                                    {
                                        // get the segment in the current bucket
                                        double segmentStart = (nBucket == bucketStart ? pCurrentItem->StartTime() - m_pTimeLine->startTime() : nBucket * rangePerBucket);
                                        double segmentEnd = (nBucket == bucketEnd ? (pCurrentItem->EndTime() - m_pTimeLine->startTime()) : (nBucket + 1) * rangePerBucket);

                                        // add the segment with the bucket with the thread it belongs to
                                        gpRibbonSegmentData segmentData;
                                        segmentData.m_startTime = segmentStart;
                                        segmentData.m_endTime = segmentEnd;
                                        segmentData.m_threadID = currentThread;

                                        GT_IF_WITH_ASSERT(nBucket >= 0)
                                        {
                                            threadSegments[nBucket].push_back(segmentData);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // for each bucket pass on all the start and end times, sort them and create sub segments. for each sub segments check each each time segment if it overlaps (which
                // means the thread is active), and to which thread it belongs to. If a thread appears to be active more then once in a time slice then it means there is a problem
                // count the number of threads
                for (int nBucket = 0; nBucket < GP_DEFAULT_BUCKET_NUMBER; nBucket++)
                {
                    QVector<double> timeSlices;
                    int numSegments = threadSegments[nBucket].size();

                    for (int nSegment = 0; nSegment < numSegments; nSegment++)
                    {
                        gpRibbonSegmentData currentSegment = threadSegments[nBucket][nSegment];

                        // push the start and end time if not in the vector
                        if (timeSlices.indexOf(currentSegment.m_startTime) == -1)
                        {
                            timeSlices.push_back(currentSegment.m_startTime);
                        }

                        if (timeSlices.indexOf(currentSegment.m_endTime) == -1)
                        {
                            timeSlices.push_back(currentSegment.m_endTime);
                        }
                    }

                    // sort the vector
                    qSort(timeSlices);

                    // start passing each time slice and see to which thread it belongs
                    int numTimeSlices = timeSlices.size();
                    int maxThreadUsed = 0;
                    double averageThreads = 0.0;
                    QVector<osThreadId> threadsUsed;

                    for (int nSlice = 0; nSlice < numTimeSlices - 1; nSlice++)
                    {
                        threadsUsed.clear();
                        QVector<osThreadId> overlapThreadsUsed;

                        for (int nSegment = 0; nSegment < numSegments; nSegment++)
                        {
                            // if the slice overlaps the segment add the thread
                            osThreadId currentThreadId = threadSegments[nBucket][nSegment].m_threadID;

                            if (timeSlices[nSlice] >= threadSegments[nBucket][nSegment].m_startTime && timeSlices[nSlice + 1] <= threadSegments[nBucket][nSegment].m_endTime)
                            {
                                if (overlapThreadsUsed.indexOf(currentThreadId) == -1)
                                {
                                    overlapThreadsUsed.push_back(currentThreadId);
                                }
                                else
                                {
                                    // there can't be two segments for the same thread for the same time slice
                                    GT_ASSERT(false);
                                }
                            }

                            // count threads
                            if (threadsUsed.indexOf(currentThreadId) == -1)
                            {
                                threadsUsed.push_back(currentThreadId);

                            }
                        }

                        // check max threads used for the current time slice
                        if (overlapThreadsUsed.size() > maxThreadUsed)
                        {
                            maxThreadUsed = overlapThreadsUsed.size();
                        }

                        // calculate the average value for this bucket
                        double sliceSize = timeSlices[nSlice + 1] - timeSlices[nSlice];
                        averageThreads += overlapThreadsUsed.size() * sliceSize / rangePerBucket;
                    }

                    m_cpuMaxThreadConcurrency[nBucket] = maxThreadUsed;
                    m_cpuAverageThreadConcurrency[nBucket] = averageThreads;
                    m_cpuTotalThreadConcurrency[nBucket] = threadsUsed.size();
                }
            }
        }

        m_wasConcurrencyCalculated = true;
    }
}

void gpRibbonDataCalculator::GetTopCalls(QVector<gpRibbonCallsData>& dataVector, QVector<double>& callsTime, QVector<double>& callsDuration, int numTopCalls)
{
    QList<gpRibbonCallsData> topCallsList;

    int numData = dataVector.size();

    // Get the top numTopCalls calls
    for (int nData = 0; nData < numData; nData++)
    {
        double duration = dataVector[nData].m_duration;

        double minValue = 0.0;

        // check if the items should be inserted: if it is larger than smallest value or we still don't have numTopCalls values
        if (duration > minValue || topCallsList.size() < numTopCalls)
        {
            // find the position in the list where the item should be inserted
            QList<gpRibbonCallsData>::iterator iter = topCallsList.begin();

            while (iter != topCallsList.end())
            {
                if ((*iter).m_duration > duration)
                {
                    break;
                }

                iter++;
            }

            // create a new data item based on the data that should be inserted
            gpRibbonCallsData longerDrawCall;
            longerDrawCall.m_duration = duration;
            longerDrawCall.m_startTime = dataVector[nData].m_startTime;
            topCallsList.insert(iter, longerDrawCall);

            // if we already have a list with the maximum size we then need to remove the first smallest item
            if (topCallsList.size() > numTopCalls)
            {
                topCallsList.removeFirst();
            }

            // store the value of the smallest item in the list to know if we should start looking in the next item
            minValue = topCallsList.begin()->m_duration;
        }
    }

    // sort the list
    qSort(topCallsList.begin(), topCallsList.end());

    // create the output vectors
    QList<gpRibbonCallsData>::iterator listIt = topCallsList.begin();

    while (listIt != topCallsList.end())
    {
        callsTime.push_back(listIt->m_startTime);
        callsDuration.push_back(listIt->m_duration);
        listIt++;
    }
}

void gpRibbonDataCalculator::CallsToBuckets(QVector<gpRibbonCallsData>& dataVector, QVector<double>& bucketsData, eCalculatorBucketMode bucketMode)
{
    bucketsData.clear();
    bucketsData.resize(GP_DEFAULT_BUCKET_NUMBER);

    GT_IF_WITH_ASSERT(GP_DEFAULT_BUCKET_NUMBER > 0)
    {
        double rangePerBucket = m_pTimeLineGrid->fullRange() * 1.0 / GP_DEFAULT_BUCKET_NUMBER;

        GT_IF_WITH_ASSERT(rangePerBucket > 0)
        {
            int numData = dataVector.size();

            for (int nData = 0; nData < numData; nData++)
            {
                quint64 bucket = (int)(dataVector[nData].m_startTime * 1.0 / rangePerBucket);
                GT_IF_WITH_ASSERT(bucket < GP_DEFAULT_BUCKET_NUMBER && bucket >= 0)
                {
                    if (eBucketCountMode == bucketMode)
                    {
                        // add to the correct bucket
                        bucketsData[bucket]++;
                    }
                    else if (eBucketMaxValueMode == bucketMode)
                    {
                        if (dataVector[nData].m_duration > bucketsData[bucket])
                        {
                            bucketsData[bucket] = dataVector[nData].m_duration;
                        }
                    }
                }
            }
        }
    }
}

void gpRibbonDataCalculator::ConcurrencyToBuckets(QVector<gpRibbonCallsData>& dataVector, QVector<double>& bucketsData, eConcurrencyBucketMode bucketMode)
{
    bucketsData.clear();
    bucketsData.resize(GP_DEFAULT_BUCKET_NUMBER);

    GT_IF_WITH_ASSERT(GP_DEFAULT_BUCKET_NUMBER > 0)
    {
        double rangePerBucket = m_pTimeLineGrid->fullRange() * 1.0 / GP_DEFAULT_BUCKET_NUMBER;

        GT_IF_WITH_ASSERT(rangePerBucket > 0)
        {
            int numData = dataVector.size();

            for (int nData = 0; nData < numData - 1; nData++)
            {
                quint64 bucket = (int)(dataVector[nData].m_startTime * 1.0 / rangePerBucket);
                GT_IF_WITH_ASSERT(bucket < GP_DEFAULT_BUCKET_NUMBER && bucket >= 0)
                {
                    if (eConcurrencyBucketMaxMode == bucketMode)
                    {
                        if (dataVector[nData].m_duration > bucketsData[bucket])
                        {
                            bucketsData[bucket] = dataVector[nData].m_duration;
                        }
                    }
                    else if (eConcurrencyBucketAvgMode == bucketMode)
                    {
                        double sliceSize = dataVector[nData + 1].m_startTime - dataVector[nData].m_startTime;
                        bucketsData[bucket] += dataVector[nData].m_duration * sliceSize / rangePerBucket;
                    }
                    else if (eConcurrencyBucketTotalMode == bucketMode)
                    {
                        bucketsData[bucket] = dataVector[nData].m_duration;
                    }
                }
            }
        }
    }
}

void gpRibbonDataCalculator::GetPresentData(QVector<double>& presentData)
{
    presentData.clear();
    presentData.resize(eNumPresent);
    presentData[ePresentCPU] = -1;
    presentData[ePresentGPU] = DBL_MAX;

    // find the first GPU op
    double firstGPUTime = DBL_MAX;
    GT_IF_WITH_ASSERT(m_pData != nullptr && m_pTimeLine != nullptr)
    {
        int numQueues = m_pData->DX12QueuesCount();

        for (int nQueue = 0; nQueue < numQueues; nQueue++)
        {
            QString queueName = m_pData->QueueName(nQueue);
            int numGPUOps = m_pData->QueueItemsCount(queueName);

            for (int nGPUOp = 0; nGPUOp < numGPUOps; nGPUOp++)
            {
                ProfileSessionDataItem* pCurrentItem = m_pData->QueueItem(queueName, nGPUOp);
                GT_IF_WITH_ASSERT(pCurrentItem != nullptr)
                {
                    if (pCurrentItem->StartTime() - m_pTimeLine->startTime() < firstGPUTime)
                    {
                        firstGPUTime = pCurrentItem->StartTime() - m_pTimeLine->startTime();
                    }
                }
            }
        }
    }
    presentData[ePresentGPU] = firstGPUTime;

    // first CPU present item
    int numThreads = m_pData->ThreadsCount();

    for (int nThread = 0; nThread < numThreads; nThread++)
    {
        osThreadId currentThread = m_pData->ThreadID(nThread);
        int numApi = m_pData->ThreadAPICount(currentThread);

        for (int nApiCall = 0; nApiCall < numApi; nApiCall++)
        {
            ProfileSessionDataItem* pCurrentItem = m_pData->APIItem(currentThread, nApiCall);

            GT_IF_WITH_ASSERT(pCurrentItem != nullptr)
            {
                eAPIType apiType;
                bool rc = pCurrentItem->GetDX12APIType(apiType);

                if (rc && pCurrentItem->ItemType().m_itemSubType == FuncId_IDXGISwapChain_Present)
                {
                    presentData[ePresentCPU] = pCurrentItem->EndTime() - m_pTimeLine->startTime();
                    break;
                }
            }
        }
    }
}

int gpRibbonDataCalculator::GetTotalApiCalls()
{
    int retVal = 0;

    GT_IF_WITH_ASSERT(m_pData != nullptr)
    {
        // add the GPU API counts in the GPU queues
        int numQueues = m_pData->DX12QueuesCount();

        for (int nQueue = 0; nQueue < numQueues; nQueue++)
        {
            QString queueName = m_pData->QueueName(nQueue);
            int numGPUOps = m_pData->QueueItemsCount(queueName);

            retVal += numGPUOps;
        }

        // add the CPU API counts in the CPU threads
        int numThreads = m_pData->ThreadsCount();

        for (int nThread = 0; nThread < numThreads; nThread++)
        {
            osThreadId currentThread = m_pData->ThreadID(nThread);
            int numApi = m_pData->ThreadAPICount(currentThread);

            retVal += numApi;
        }
    }

    return retVal;
}