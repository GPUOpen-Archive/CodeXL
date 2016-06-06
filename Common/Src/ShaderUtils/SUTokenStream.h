//=====================================================================
// Copyright 2010-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SUTokenStream.h
/// \brief  Token streams handle the reading or writing of streams of
///         tokens to or from memory.
///
//=====================================================================
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/SUTokenStream.h#7 $
// Last checkin:   $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569612 $
//=====================================================================

#ifndef _SU_TOKEN_STREAM_H_
#define _SU_TOKEN_STREAM_H_

#include <cassert>
#include <cstring>
#include "SUCommon.h"

namespace ShaderUtils
{
/// SUTokenStream provides base functionality for CInputTokenSteam and SUOutputTokenStream.
/// Token streams handle the reading or writing of streams of tokens to or from memory.
/// It is an abstract base class that can not be instantiated.
template <typename T> class SUTokenStream
{
protected:
    /// Constructor for a SUTokenStream.
    /// \param[in] nInitialTokenCount    Initial number of tokens to size the stream to. Default is zero.
    SUTokenStream(const size_t nInitialTokenCount = 0);

    /// Destructor.
    virtual ~SUTokenStream() {;};

public:
    /// Retrieve a pointer to the stream data.
    /// \return Pointer to the stream data.
    virtual const T* GetStream() const = 0;

    /// Retrieve a pointer to the current position within the stream.
    /// \return Pointer to the current position within the stream
    virtual const T* GetStreamPos() const = 0;

    /// Retrieve the number of tokens in the stream.
    /// \return The number of tokens in the stream.
    size_t GetTokenCount() const { return m_nTokenCount; }

protected:
    /// The number of tokens in the stream.
    size_t m_nTokenCount;
};


template <typename T> class SUInputTokenStream;

/// SUOutputTokenStream provides functionality for writing a stream of
/// tokens to memory, re-allocating memory as required.
template <typename T> class SUOutputTokenStream : public SUTokenStream<T>
{
public:
    /// Constructor.
    /// \param[in] nInitialTokenCount   Initial number of tokens to size the stream to.
    SUOutputTokenStream(const size_t nInitialTokenCount);

    /// Destructor.
    virtual ~SUOutputTokenStream();

    virtual const T* GetStream() const { return m_ptStream; }
    virtual const T* GetStreamPos() const { return m_ptStreamPos; }

    /// Write a token to the stream.
    /// \param[in] tToken The token to write.
    /// \return    True if successful, otherwise false.
    bool WriteToken(const T tToken);

    /// Write a token to the stream at a specific offset.
    /// If the offset is beyond the tokens currently written then this write fails.
    /// The write overwrites the token at the specified offset.
    /// \param[in] nOffset The offset at which to write.
    /// \param[in] tToken  The token to write.
    /// \return    True if successful, otherwise false.
    bool WriteTokenAt(const size_t nOffset,
                      const T tToken);

    /// Write an array of tokens to the stream.
    /// \param[in] ptTokens      The tokens to write.
    /// \param[in] nNumTokens    The number of tokens to write.
    /// \return    True if successful, otherwise false.
    bool WriteTokens(const T* const ptTokens,
                     const size_t nNumTokens);

    /// Write an data to the stream.
    /// The number of bytes to write must be a multiple of sizeof(T).
    /// \param[in] pData    The data to write.
    /// \param[in] nBytes   The number of bytes of data to write.
    /// \return    True if successful, otherwise false.
    bool WriteData(const void* const pData,
                   const size_t nBytes);

    /// Read tokens from the input stream and write them to this stream.
    /// \param[in] src          The input stream from which to read.
    /// \param[in] nNumTokens   The number of tokens to copy/
    /// \return    True if successful, otherwise false.
    bool CopyTokens(SUInputTokenStream<T>& src,
                    const size_t nNumTokens);

    /// Get the number of tokens written.
    /// \return The number of tokens written.
    size_t GetTokensWritten() const { return m_nTokensWritten; }

protected:
    /// Resize the stream to hold the specified number of tokens.
    /// The stream can not be resized to less than the number of tokens written.
    /// \param[in] nTokenCount The number to tokens to resize to.
    /// \return    True is successful, otherwise false.
    bool Resize(const size_t nTokenCount);

    static size_t NextPowerOfTwo(size_t nValue);

    T* m_ptStream;         ///< Pointer to the buffer holding the stream data.
    T* m_ptStreamPos;      ///< Pointer to the next position at which to write a token.

    size_t m_nTokensWritten;  ///< The number of tokens written.
};


/// SUInputTokenStream provides functionality for reading a stream of tokens from memory.
template <typename T> class SUInputTokenStream : public SUTokenStream<T>
{
public:
    /// Constructor.
    /// \param[in] ptStream       The input buffer from which to read tokens.
    /// \param[in] nTokenCount    The number of tokens in the input buffer.
    SUInputTokenStream(const T* const tStream,
                       const size_t nTokenCount);

    virtual const T* GetStream() const { return m_ptStream; }
    virtual const T* GetStreamPos() const { return m_ptStreamPos; }

    /// Peek at the next token in the input stream, i.e. read it without moving the stream position.
    /// \param[out]   tToken  The next token.
    /// \return       True if successful, otherwise false.
    bool PeekToken(T& tToken);

    /// Read the next token in the input stream.
    /// \param[out]   tToken  The next token.
    /// \return       True if successful, otherwise false.
    bool ReadToken(T& tToken);

    /// Read the specified number of tokens from the input stream.
    /// \param[out]   ptTokens  The next tokens.
    /// \param[in]    nTokens   The number of tokens to read.
    /// \return       True if successful, otherwise false.
    bool ReadTokens(T* const ptTokens, const size_t nTokens);

    /// Skip past the specified number of tokens in the input stream.
    /// \param[in]    nTokens   The number of tokens to skip.
    /// \return       True if successful, otherwise false.
    bool SkipTokens(const size_t nTokens);

    /// Friend function in SUOutputTokenStream responsible for reading tokens from the input
    /// stream and writing them to the output stream.
    /// \param[in] src      The input stream from which to read.
    /// \param[in] nTokens The number of tokens to copy/
    /// \return    True if successful, otherwise false.
    friend bool SUOutputTokenStream<T>::CopyTokens(SUInputTokenStream<T>& src,
                                                   const size_t nTokens);

    /// Get the number of tokens read.
    /// \return The number of tokens read.
    int GetTokensRead() const { return m_nTokensRead; }

protected:
    const T* m_ptStream;           ///< Pointer to the buffer holding the stream data.
    const T* m_ptStreamPos;        ///< Pointer to the next position at which to write a token.
    size_t m_nTokensRead;          ///< The number of tokens read.
};



template <typename T> SUTokenStream<T>::SUTokenStream(const size_t nTokenCount)
    : m_nTokenCount(nTokenCount)
{
}


template <typename T> SUOutputTokenStream<T>::SUOutputTokenStream(const size_t nInitialTokenCount)
    : SUTokenStream<T>(),
      m_ptStream(NULL),
      m_ptStreamPos(NULL),
      m_nTokensWritten(0)
{
    SU_Verify(Resize(nInitialTokenCount));
}

template <typename T> SUOutputTokenStream<T>::~SUOutputTokenStream()
{
    SU_SAFE_FREE(m_ptStream);
    m_ptStreamPos = NULL;
}


template <typename T> bool SUOutputTokenStream<T>::Resize(const size_t nTokenCount)
{
    SU_Assert(nTokenCount >= m_nTokensWritten);

    if (nTokenCount < m_nTokensWritten)
    {
        return false;
    }

    T* ptNewStream = new(std::nothrow) T[nTokenCount];
    SU_Assert(ptNewStream != NULL);

    if (ptNewStream == NULL)
    {
        return false;
    }

    if (m_ptStream)
    {
        memcpy(ptNewStream, m_ptStream, m_nTokensWritten * sizeof(T));

        delete[] m_ptStream;
    }

    m_ptStream = ptNewStream;
    m_ptStreamPos = m_ptStream + m_nTokensWritten;
    this->m_nTokenCount = nTokenCount;

    return true;
}


template <typename T> bool SUOutputTokenStream<T>::WriteToken(const T tToken)
{
    if (m_nTokensWritten >= this->GetTokenCount())
    {
        size_t nNewTokenCount = this->GetTokenCount() * 2;

        if (nNewTokenCount == 0)
        {
            nNewTokenCount = 100;
        }

        if (!Resize(nNewTokenCount))
        {
            return false;
        }
    }

    SU_Assert(m_ptStreamPos);
    SU_Assert(this->m_nTokenCount);
    SU_Assert(m_nTokensWritten < this->GetTokenCount());

    if (m_ptStreamPos == NULL || m_nTokensWritten >= this->GetTokenCount())
    {
        return false;
    }

    *m_ptStreamPos++ = tToken;
    m_nTokensWritten++;

    return true;
}


template <typename T> bool SUOutputTokenStream<T>::WriteTokenAt(const size_t nOffset,
                                                                const T tToken)
{
    SU_Assert(m_ptStreamPos);
    SU_Assert(m_nTokensWritten);
    SU_Assert(nOffset < m_nTokensWritten);

    if (m_ptStreamPos == NULL || nOffset >= m_nTokensWritten)
    {
        return false;
    }

    m_ptStream[nOffset] = tToken;

    return true;
}


template <typename T> size_t SUOutputTokenStream<T>::NextPowerOfTwo(size_t nValue)
{
    assert(nValue < ((size_t)1 << ((sizeof(size_t) * 8) - 1)));

    // special case zero
    if (nValue == 0)
    {
        return 1;
    }

    nValue--;
    nValue = (nValue >> 1)  | nValue;
    nValue = (nValue >> 2)  | nValue;
    nValue = (nValue >> 4)  | nValue;
    nValue = (nValue >> 8)  | nValue;
    nValue = (nValue >> 16) | nValue;
#ifdef _WIN64
    nValue = (nValue >> 32) | nValue;   // needed for 64 bit values
#endif
    nValue++;

    // if original nValue > (1<<(bits_in_size_t - 1)) then return will be 0
    return nValue;
}


template <typename T> bool SUOutputTokenStream<T>::WriteTokens(const T* const ptTokens,
                                                               const size_t nNumTokens)
{
    SU_Assert(nNumTokens > 0);

    if (nNumTokens <= 0)
    {
        return false;
    }

    size_t nTokensNeeded = m_nTokensWritten + nNumTokens;

    if (nTokensNeeded >= this->GetTokenCount())
    {
        // Resize to power-of-two greater than nTokensNeeded
        size_t nNewTokenCount = NextPowerOfTwo(nTokensNeeded);

        if (!Resize(nNewTokenCount))
        {
            return false;
        }
    }

    SU_Assert(m_ptStreamPos != NULL);
    SU_Assert(this->GetTokenCount() > 0);
    SU_Assert(nTokensNeeded <= this->GetTokenCount());
    SU_Assert(ptTokens != NULL);

    if (m_ptStreamPos == NULL || ptTokens == NULL || nTokensNeeded > this->GetTokenCount())
    {
        return false;
    }

    memcpy(m_ptStreamPos, ptTokens, nNumTokens * sizeof(T));
    m_ptStreamPos += nNumTokens;
    m_nTokensWritten += nNumTokens;

    return true;
}


template <typename T> bool SUOutputTokenStream<T>::WriteData(const void* const pData,
                                                             const size_t nNumBytes)
{
    SU_Assert((nNumBytes % sizeof(T)) == 0);

    if ((nNumBytes % sizeof(T)) == 0)
    {
        return WriteTokens((T*)pData, nNumBytes / sizeof(T));
    }

    return false;
}


template <typename T> bool SUOutputTokenStream<T>::CopyTokens(SUInputTokenStream<T>& src,
                                                              const size_t nNumTokens)
{
    SU_Assert(m_ptStreamPos);
    SU_Assert(this->GetTokenCount());
    SU_Assert((m_nTokensWritten + nNumTokens) <= this->GetTokenCount());

    if (m_ptStreamPos == NULL || (m_nTokensWritten + nNumTokens) > this->GetTokenCount())
    {
        return false;
    }

    SU_Assert(src.m_ptStreamPos);
    SU_Assert(src.m_nTokenCount);
    SU_Assert((src.m_nTokensRead + nNumTokens) <= src.m_nTokenCount);

    if (src.m_ptStreamPos == NULL || (src.m_nTokensRead + nNumTokens) > src.m_nTokenCount)
    {
        return false;
    }

    memcpy(m_ptStreamPos, src.m_ptStreamPos, nNumTokens * sizeof(T));

    m_ptStreamPos += nNumTokens;
    m_nTokensWritten += nNumTokens;

    src.m_ptStreamPos += nNumTokens;
    src.m_nTokensRead += nNumTokens;

    return true;
}


template <typename T> SUInputTokenStream<T>::SUInputTokenStream(const T* const ptStream,
                                                                const size_t nTokenCount)
    : SUTokenStream<T>(nTokenCount),
      m_ptStream(ptStream),
      m_ptStreamPos(ptStream),
      m_nTokensRead(0)
{
    SU_Assert(this->m_nTokenCount > 0);
}


template <typename T> bool SUInputTokenStream<T>::PeekToken(T& tToken)
{
    SU_Assert(m_ptStreamPos);
    SU_Assert(this->m_nTokenCount);

    if (m_ptStreamPos == NULL || m_nTokensRead >= this->GetTokenCount())
    {
        return false;
    }

    tToken = *m_ptStreamPos;

    return true;
}


template <typename T> bool SUInputTokenStream<T>::ReadToken(T& tToken)
{
    SU_Assert(m_ptStreamPos);
    SU_Assert(this->GetTokenCount());

    if (m_ptStreamPos == NULL || m_nTokensRead >= this->GetTokenCount())
    {
        return false;
    }

    tToken = *m_ptStreamPos++;
    m_nTokensRead++;

    return true;
}


template <typename T> bool SUInputTokenStream<T>::ReadTokens(T* const ptTokens,
                                                             const size_t nNumTokens)
{
    SU_Assert(m_ptStreamPos);
    SU_Assert(this->GetTokenCount());
    SU_Assert((m_nTokensRead + nNumTokens) <= this->GetTokenCount());
    SU_Assert(ptTokens);

    if (m_ptStreamPos == NULL || ptTokens == NULL || (m_nTokensRead + nNumTokens) > this->GetTokenCount())
    {
        return false;
    }

    memcpy(ptTokens, m_ptStreamPos, nNumTokens * sizeof(T));
    m_ptStreamPos += nNumTokens;
    m_nTokensRead += nNumTokens;

    return true;
}


template <typename T> bool SUInputTokenStream<T>::SkipTokens(const size_t nNumTokens)
{
    SU_Assert(m_ptStreamPos);
    SU_Assert(this->GetTokenCount());
    SU_Assert((m_nTokensRead + this->nTokens) <= this->GetTokenCount());

    if (m_ptStreamPos == NULL || (m_nTokensRead + nNumTokens) > this->GetTokenCount())
    {
        return false;
    }

    m_ptStreamPos += nNumTokens;
    m_nTokensRead += nNumTokens;

    return true;
}


} // namespace ShaderUtils

#endif //_SU_TOKEN_STREAM_H_
