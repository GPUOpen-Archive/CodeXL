///////////////////////////////////////////////////////////////////////////////
// Revision History
///////////////////////////////////////////////////////////////////////////////

V 3.8.5
	Fixed bug in nop/xchg/pause decode.

V 3.8.4
	Fixed bug in frczsd and cvtpd2ps table entries.

V 3.8.3
	Fixed bug in roundsd table entry.
	Fixed bug in Bni byte condition decode.

V 3.8.2
	Reverted handling of DREX.OC0 bit for pmac*, pmad*, com*, pcom*, and pcomu*
	to a "care" from a "don't care".

V 3.8.1
	Fixed bug in roundps/pd/ss/sd implementation

V 3.8.0
	Updated to support v2.0 of the BDAPMU (nda version)

	Removed instructions:
		ptest (0f 7a 20 /0)
		rndz
		floor
		ceil
		rnd

	Added instructions:
		ptest (66 0f 3a 08-0b)
		round

V 3.7.0
	Added support for BNI

V 3.6.0
	Added support for alternate popcnt decode

V 3.5.0
	Added support for MNI

V 3.4.0
	Added additional operand level info
	Added support for multiple derivatives

V 2.7.0
	Fixed bug in the handling of SSE overused prefix bytes and its
	implications on the handling of the REX prefix byte.

	Changed the behavior of the disassembler with respect to overrused 
	prefix bytes (for SSE/MMX).  In the past, prefix bytes were always
	processed the same whether or not they were part of an SSE instruction.
	Now, prefix bytes that are being overused in an SSE/MMX instruction do 
	not set flags that would have been set when being used otherwise.  To 
	look at the overused prefix byte for SSE instructions, you must use 
	HasExtraOpcode()/GetExtraOpcode() and can no longer use 
	HasDataOvrdPrefix()/HasRepPrefix()/HasRepnePrefix().  GetExtraOpcode() 
	has	always been the documented way to do this, the changes only alter 
	undocumented behaviors.

V 2.6.2
	Fixed new address width function to really fix address width bug.

V 2.6.1
	Fixed buffer overflow bug.
	Fixed relative address width bug.

V 2.6.0
	Added support for SEM instructions.

V 2.5.0

	Added HasExtraOpcode() and GetRexPrefix() public routines to 
	allow public access to that information.

	Added code to convert "lock" prefix for clx/stx (rex32 
	instructions) into an extra opcode byte instead of simply 
	removing it from the prefix byte list and discarding it.

	Fixed bug in GetThirdOperandSize() in which the wrong size was 
	being reported for the "imul" instruction in longmode with the 
	REX_64 operand size prefix.

V 2.4.0

	Fixed bug decoding pextrw.

	Added additional modrm validation for fpu instructions.

	Added SizeofNestedTable routine to assist in derived classes 
	which modify disassembler behavior by hooking to the 
	disassembler tables.

V 2.3.0
	End of revision history.

////////////////////////////////////////////////////////////////////////////////
// For Windows distributions
////////////////////////////////////////////////////////////////////////////////

The CDisassembler files:
	disassembler.h
		The prototypes for the CDisassembler object.
	disassembler.tables
		The CDisassembler object's opcode tables and nested
		table routines.  These tables and routines provide 
		insight into the organization of the CDisassembler
		tables to aid in derivation.
	disassembler.lib
	disassemblerd.lib
		The actual disassembler binary libraries for linking
		with your application.
	disassembler64.lib
	disassembler64d.lib
		64-bit versions of the disassembler libraries for
		use with AMD64 OS's (Windows XP 64)
	disassemblerdll.dll
	disassemblerdll.lib
	disassemblerdlld.dll
	disassemblerdlld.lib
		DLL versions of the disassembler library and the
		statically linked interface files (.lib).


The CTypeDisassembler files: the type disassembler object is derived
from the CDisassembler object.  The CTypeDisassembler object hooks to
the CDisassembler tables to provide additional instruction information
(in this case, the instruction type (FPU, MMX, 3DNow, ...)).
	typedisassembler.h
		The prototypes for the CTypeDisassembler object.
	typedisassembler.cpp
		The code for the CTypeDisassembler object, provided as
		an example derivation of the CDisassembler object which
		involves hooking of the CDisassembler tables to provide
		additional instruction information via the CDisassembler
		table hooking mechanism.
	typedisassembler.lib
	typedisassemblerd.lib
		The binary libraries for linking with your application.
	typedisassembler64.lib
	typedisassembler64d.lib
		64-bit versions of the libraries (for building 64-bit apps).


Application files:
	disassemble.exe
		Stand alone implementations of the disassembler (i.e.
		command line versions).
	disassemble64.exe
		64-bit stand alone implementation (for 64-bit OS).

Readme files: A few readme files with the same base name as the entity 
they concern and the extension "readme".
////////////////////////////////////////////////////////////////////////////////

