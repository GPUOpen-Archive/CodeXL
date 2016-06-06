//    
//  Workfile: DX10AsmBuffer.h
//
//  Description:
//      DX10AsmBuffer class definition.
//
//  Trade secret of ATI Technologies, Inc.
//  Copyright 2002, ATI Technologies, Inc., (unpublished)
//
//  All rights reserved.  This notice is intended as a precaution against
//  inadvertent publication and does not imply publication or any waiver
//  of confidentiality.  The year included in the foregoing notice is the
//  year of creation of the work.
//
//

#ifndef DX10ASMBUFFER_H
#define DX10ASMBUFFER_H

namespace xlt
{

class DX10AsmBuffer
{
public:
    DX10AsmBuffer(void);
    DX10AsmBuffer( const DX10AsmBuffer& binary );  // copy ctor
   ~DX10AsmBuffer(void);

    DX10AsmBuffer& seekg( unsigned int nOffset );
    int tellg(void);

    //DX10AsmBuffer& operator += ( const std::string& rhs );
    DX10AsmBuffer& operator += ( const char* rhs );
    DX10AsmBuffer& operator += ( const char rhs );
    DX10AsmBuffer& operator += ( const DX10AsmBuffer& r );
    DX10AsmBuffer& operator = ( const DX10AsmBuffer& rhs );
    DX10AsmBuffer& operator<<( const char* rhs );
    DX10AsmBuffer& operator<<( const char rhs );
    DX10AsmBuffer& operator<<( const float rhs );
    DX10AsmBuffer& operator<<( const int rhs );
    DX10AsmBuffer& operator<<( const unsigned int rhs );
    DX10AsmBuffer& operator<<( const unsigned long rhs );
    DX10AsmBuffer& operator<<( const bool rhs );

    DX10AsmBuffer& operator>>( char* rhs );
    DX10AsmBuffer& operator>>( char& rhs );
    DX10AsmBuffer& operator>>( float& rhs );
    DX10AsmBuffer& operator>>( int& rhs );
    DX10AsmBuffer& operator>>( unsigned int& rhs );
    DX10AsmBuffer& operator>>( unsigned long& rhs );
    DX10AsmBuffer& operator>>( bool& rhs );

    DX10AsmBuffer& write( const DX10AsmBuffer& buf, int nTargetOffset );
    DX10AsmBuffer& write( int nOffset, unsigned long dwWord );

    operator char*() { return ptr(); }

    DX10AsmBuffer& eol(void);            // translator specific method
    DX10AsmBuffer& finish(void);         // translator specific method
    DX10AsmBuffer& indent( int level );

    char* attach( char* pBuffer, int nSize );
    char* alloc( unsigned int nSize );
    bool  realloc( int nSize = 0 );

    bool save( const char* szFilename );
    bool load( const char* szFilename );

    int read( char* pBuffer, int nLength );
    int write( const char* pBuffer, int nLength );    

    bool equal( const DX10AsmBuffer& rhs ) const;
    bool unequal( const DX10AsmBuffer& rhs ) const;    

    bool copy( const char* p, unsigned int s );

    unsigned int size(void) const { return m_nSize; }

    void reset(void);

    void init(void);
    void destroy(void);

    unsigned long peekWord(void);
    // the following member should be private.
    char* ptr(void) const { return m_pBuffer; }
private:
   
    unsigned int allocSize(void) const { return m_nMemorySize; }

    void  free(void);

    char* m_pBuffer;
    unsigned int m_nSize;       // how much of the memory is filled with data.
    unsigned int m_nMemorySize;
    unsigned int m_nOffset;

    unsigned long m_nMemoryAllocIncrements;
};

inline const DX10AsmBuffer operator + ( const DX10AsmBuffer& lhs, const DX10AsmBuffer& rhs)
    { return DX10AsmBuffer(lhs) += rhs; }

/*
inline const DX10AsmBuffer operator + ( const DX10AsmBuffer& lhs, const std::string& rhs)
    { return DX10AsmBuffer(lhs) += rhs; }
*/

inline const bool operator == ( const DX10AsmBuffer& lhs, const DX10AsmBuffer& rhs )
    { return lhs.equal(rhs); }

inline const bool operator != ( const DX10AsmBuffer& lhs, const DX10AsmBuffer& rhs )
    { return lhs.unequal(rhs); }

/*
inline std::string& operator<< ( std::string& lhs, const DX10AsmBuffer& rhs )
    { const_cast<DX10AsmBuffer&>(rhs).operator>>(lhs); return lhs; }
*/

inline char* operator<< ( char* lhs, const DX10AsmBuffer& rhs )
    { const_cast<DX10AsmBuffer&>(rhs).operator>>(lhs); return lhs; }

inline char operator<< ( char& lhs, const DX10AsmBuffer& rhs )
    { const_cast<DX10AsmBuffer&>(rhs).operator>>(lhs); return lhs; }

inline int& operator<< ( int& lhs, const DX10AsmBuffer& rhs )
    { const_cast<DX10AsmBuffer&>(rhs).operator>>(lhs); return lhs; }

inline unsigned int& operator<< ( unsigned int& lhs, const DX10AsmBuffer& rhs )
    { const_cast<DX10AsmBuffer&>(rhs).operator>>(lhs); return lhs; }

inline float& operator<< ( float& lhs, const DX10AsmBuffer& rhs )
    { const_cast<DX10AsmBuffer&>(rhs).operator>>(lhs); return lhs; }

inline unsigned long& operator<< ( unsigned long& lhs, const DX10AsmBuffer& rhs )
    { const_cast<DX10AsmBuffer&>(rhs).operator>>(lhs); return lhs; }

inline bool& operator<< ( bool& lhs, const DX10AsmBuffer& rhs )
    { const_cast<DX10AsmBuffer&>(rhs).operator>>(lhs); return lhs; }

} // namespace xlt

#endif // DX10ASMBUFFER_H