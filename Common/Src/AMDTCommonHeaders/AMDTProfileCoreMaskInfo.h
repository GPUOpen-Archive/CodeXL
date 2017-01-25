//=============================================================
// (c) 2017 Advanced Micro Devices, Inc.
//
/// \author CodeXL Developer Tools
/// \version $Revision: $
/// \brief Common Data Types used by codexl cpu and power profilers
//
//=============================================================

#ifndef _AMDTPROFILECOREMASKINFO_H_
#define _AMDTPROFILECOREMASKINFO_H_

#pragma once

// Base headers
#include <AMDTCommonHeaders/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

//
//  Structs
//

struct AMDTProfileCoreMaskInfo
{
    AMDTProfileCoreMaskInfo() {}
    ~AMDTProfileCoreMaskInfo() {}
    //AMDTProfileCoreMaskInfo(const AMDTProfileCoreMaskInfo& other) = delete;

    void AddCoreId(gtUInt32 coreId)
    {
        m_coresList.push_back(coreId);

        gtUInt32 idx = static_cast<gtUInt32>(coreId / 64);
        m_coreMask[idx] |= static_cast<gtUInt64>(1) << (coreId % 64);

        m_maxCoreId = (coreId > m_maxCoreId) ? coreId : m_maxCoreId;
        m_coreMaskSize = static_cast<gtUInt32>((m_maxCoreId + 64) / 64);
    }

    void AddCores(gtUInt32 nbrCores)
    {
        for (gtUInt32 coreId = 0; coreId < nbrCores; coreId++)
        {
            m_coresList.push_back(coreId);

            gtUInt32 idx = static_cast<gtUInt32>(coreId / 64);
            m_coreMask[idx] |= static_cast<gtUInt64>(1) << (coreId % 64);
        }

        m_maxCoreId = nbrCores - 1;
        m_coreMaskSize = static_cast<gtUInt32>((m_maxCoreId + 64) / 64);
    }

    void AddCoresList(gtVector<gtUInt32>& coresList)
    {
        for (auto& coreId : coresList)
        {
            m_coresList.push_back(coreId);

            gtUInt32 idx = static_cast<gtUInt32>(coreId / 64);
            m_coreMask[idx] |= static_cast<gtUInt64>(1) << (coreId % 64);
            m_maxCoreId = (coreId > m_maxCoreId) ? coreId : m_maxCoreId;
        }

        m_coreMaskSize = static_cast<gtUInt32>((m_maxCoreId + 64) / 64);
    }

    void AddCoreMask(gtUInt64* pCoreMask, gtUInt32 coreMaskSize)
    {
       for (gtUInt32 idx = 0; idx < coreMaskSize; idx++)
        {
            m_coreMask[idx] |= pCoreMask[idx];
            gtUInt64 coreMask = pCoreMask[idx];
            gtUInt32 coreId = 0;
            
            while (coreMask)
            {
                if (coreMask & 0x1)
                {
                    coreId += (idx * 64);
                    m_coresList.push_back(coreId);
                    m_maxCoreId = (coreId > m_maxCoreId) ? coreId : m_maxCoreId;
                }

                coreMask = coreMask >> 1;
                coreId++;
            }
        }

        m_coreMaskSize = static_cast<gtUInt32>((m_maxCoreId + 64) / 64);
    }

    // !! Incomplete !!
    void AddCoreMaskStr(gtString coreMaskStr)
    {
        if (m_coreMaskStr.isEmpty())
        {
            m_coreMaskStr = coreMaskStr;
        }
        else
        {
            m_coreMaskStr += L",";
            m_coreMaskStr += coreMaskStr;
        }

        // Update m_coreBitMaskStr and m_coresList
    }

    bool operator<=(const AMDTProfileCoreMaskInfo& other) const
    {
        bool ret = false;

        if (other.m_coreMaskSize <= m_coreMaskSize)
        {
            for (gtUInt32 idx = 0; idx < other.m_coreMaskSize; idx++)
            {
                ret = true;

                if (other.m_coreMask[idx] > m_coreMask[idx])
                {
                    ret = false;
                    break;
                }
            }
        }

        return ret;
    }

    gtUInt32 GetNumberOfCores() const { return m_coresList.size(); }
    gtVector<gtUInt32> GetCoresList() const { return m_coresList; }
    void GetCoreMask(gtUInt64*& ppCoreMask, gtUInt32& coreMaskSize)
    {
        ppCoreMask = m_coreMask;
        coreMaskSize = m_coreMaskSize;
    }

    void Clear()
    {
        m_coreMaskStr.makeEmpty();
        m_coresList.clear();
    }

    gtString            m_coreMaskStr;  // comman seperated coremask
    gtUInt64            m_coreMask[4] = { 0 };
    gtUInt32            m_coreMaskSize = 0;
    gtUInt32            m_maxCoreId = 0;
    gtVector<gtUInt32>  m_coresList;    // may have duplicates?
};

#endif // _AMDTPROFILECOREMASKINFO_H_