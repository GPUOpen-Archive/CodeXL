//    
//  Workfile: XltParserEnv.h
//
//  Description:
//      XltParserEnv class definition.
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

#ifndef DX10ASMENV_H
#define DX10ASMENV_H

#include "DX10AsmBuffer.h"
#include "DX10AsmInterface.h"
#include "DX10AsmPacker.h"
#include <list>

#define qLittleEndian 1
//#include "pele_registers.h"

#define TMP_BUFFER_SIZE 1024

void empty_printf( char* pszFormat, ... );

#define DBG_PRINT empty_printf

namespace xlt
{

struct Clause {
  DX10AsmBuffer buf;
  int byteAddr;
  int cnt;
  int type;
};

#define CLAUSE_TYPE_ALU     0
#define CLAUSE_TYPE_FETCH   1

//----------------------------------------------------------------------------
// Class name: DX10AsmEnv
//
// Class Description:
//     Singleton class that provides environment for parser.
//----------------------------------------------------------------------------
class DX10AsmEnv
{
public:
    static XLT_BOOL Create(XLT_CALLBACKS *XltCallBacks);
    static DX10AsmEnv& singleton(void);

    void init(void);

    void writeWord( unsigned long word ) {
      DBG_PRINT( "WORD: 0x%08X\n", word );
      m_program << word;
    }

    void writeWord( unsigned long nOffset, unsigned long word )
    {
        m_program.write( nOffset, word );
    }

    unsigned long getBufferOffset(void)
    {
        return m_program.tellg();
    }

    void buildProgram(void)
    {
        // Patch program etc.
        unsigned long progSize = m_program.size() / sizeof(unsigned int);
        unsigned long* ptr = (unsigned long*)m_program.ptr();
        ptr[1] = progSize;
    }
    void destroy(void);

    ////////////////////////////////////////////////////////
    // Buffering methods.
    char* queryBuffer(void) const { return static_cast<char*>(m_program.ptr()); }
    int   queryBufferSize(void) const { return m_program.size(); }

    char* strdup( const char* pszString );

    ////////////////////////////////////////////////////////
    // Configuration methods

    enum XltMode { STRING = 0, BINARY };

    void xltMode( XltMode eMode )
        { m_eMode = eMode; }

    XltMode xltMode(void)
        { return m_eMode; }

    bool isLineMode(void) const;

    void setCallbacks( LPXLT_CALLBACKS xltCallbacks )
        { m_xltCallbacks = xltCallbacks; }

    LPXLT_CALLBACKS getCallbacks(void) const
        { return m_xltCallbacks; }

    xlt::DX10AsmPacker& GetPacker() { return m_packer; }

    bool IsTestAsmMode(void) const {return m_xltCallbacks->flags == 2;}

private:
    // ctor private because we are a singleton.
    DX10AsmEnv(void);

    void * operator new(size_t nBytes, XLT_CALLBACKS *XltCallBacks);
    void operator delete(void *Ptr, XLT_CALLBACKS *XltCallBacks);
    void operator delete(void *Ptr);

    XltMode m_eMode;

    DX10AsmBuffer m_program;
    DX10AsmPacker m_packer;

    char m_szTmpBuffer[ TMP_BUFFER_SIZE ];

    LPXLT_CALLBACKS m_xltCallbacks;
};

} // namespace xlt

#endif // DX10ASMENV_H

