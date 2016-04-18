//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class generates sequence IDs for called APIs
//==============================================================================

#ifndef _SEQID_GENERATOR_H_
#define _SEQID_GENERATOR_H_

#include <map>

#include <AMDTOSWrappers/Include/osThread.h>

#include "OSUtils.h"
#include "TSingleton.h"

//------------------------------------------------------------------------------------
/// Sequence ID generator, Singleton class
//------------------------------------------------------------------------------------
class SeqIDGenerator : public TSingleton<SeqIDGenerator>
{
    friend class TSingleton<SeqIDGenerator>;
public:
    /// Enables or disables generation of a sequence id for all subsequent apis
    /// \param bEnabled flag indicating whether the request is to enable or disable sequence ID generation
    void EnableGenerator(bool bEnabled)
    {
        osThreadId tid = osGetUniqueCurrentThreadId();
        EnabledMapType::iterator it = m_enabledMap.find(tid);

        if (it == m_enabledMap.end())
        {
            m_enabledMap.insert(EnabledPairType(tid, bEnabled));
        }
        else
        {
            it->second = bEnabled;
        }
    }

    /// Generate new sequence ID
    /// \param[out] pTID thread id
    /// \param[out] pSeqID sequence id
    void GenerateID(osThreadId* pTID = NULL, unsigned int* pSeqID = NULL)
    {
        osThreadId tid = osGetUniqueCurrentThreadId();

        if (pTID != NULL)
        {
            *pTID = tid;
        }

        EnabledMapType::iterator enabled_it = m_enabledMap.find(tid);

        if (enabled_it != m_enabledMap.end() && !enabled_it->second)
        {
            return;
        }

        SequenceMapType::iterator it = m_seqMap.find(tid);

        if (it == m_seqMap.end())
        {
            m_seqMap.insert(SequencePairType(tid, 0));

            if (pSeqID != NULL)
            {
                *pSeqID = 0;
            }
        }
        else
        {
            it->second++;

            if (pSeqID != NULL)
            {
                *pSeqID = it->second;
            }
        }
    }

private:

    /// private Constructor
    SeqIDGenerator()
    {
    }

    /// disable copy constructor
    SeqIDGenerator(const SeqIDGenerator& obj);
    /// disable assignment op
    SeqIDGenerator& operator = (const SeqIDGenerator& obj);

private:
    typedef std::map<osThreadId, unsigned int> SequenceMapType; ///< typedef of sequence map type
    typedef std::pair<osThreadId, unsigned int> SequencePairType; ///< typedef of sequence Pair type
    SequenceMapType m_seqMap; ///< map of thread id to current sequence id for that thread

    typedef std::map<osThreadId, bool> EnabledMapType; ///< typedef of enabled threads map type
    typedef std::pair<osThreadId, bool> EnabledPairType; ///< typedef of enabled threads Pair type
    EnabledMapType m_enabledMap;     ///< map of thread id to enabled state for that thread
};

#endif //_SEQID_GENERATOR_H_
