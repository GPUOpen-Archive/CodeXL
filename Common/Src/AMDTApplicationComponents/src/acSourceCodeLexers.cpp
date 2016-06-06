//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acSourceCodeLexers.cpp
///
//==================================================================================

//------------------------------ acSourceCodeLexers.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Local:
#include <AMDTApplicationComponents/Include/acSourceCodeLexers.h>

//----------------------------------------------------------------------------
// keywordlists
// Ada
const char* AdaWordlist1 =
    "abort abstract accept access aliased all array at begin body case "
    "constant declare delay delta digits do else elsif end entry "
    "exception exit for function generic goto if in is limited loop "
    "new null of others out package pragma private procedure protected "
    "raise range record renames requeue return reverse select separate "
    "subtype tagged task terminate then type until use when while with "
    "abs and mod not or rem xor";
// Assembler
const char* AsmWordlist1 =
    "aaa aad aam aas adc add and call cbw clc cld cli cmc cmp cmps "
    "cmpsb cmpsw cwd daa das dec div esc hlt idiv imul in inc int into "
    "iret ja jae jb jbe jc jcxz je jg jge jl jle jmp jna jnae jnb jnbe "
    "jnc jne jng jnge jnl jnle jno jnp jns jnz jo jp jpe jpo js jz "
    "lahf lds lea les lods lodsb lodsw loop loope loopew loopne "
    "loopnew loopnz loopnzw loopw loopz loopzw mov movs movsb movsw "
    "mul neg nop not or out pop popf push pushf rcl rcr ret retf retn "
    "rol ror sahf sal sar sbb scas scasb scasw shl shr stc std sti "
    "stos stosb stosw sub test wait xchg xlat xlatb xor bound enter "
    "ins insb insw leave outs outsb outsw popa pusha pushw arpl lar "
    "lsl sgdt sidt sldt smsw str verr verw clts lgdt lidt lldt lmsw "
    "ltr bsf bsr bt btc btr bts cdq cmpsd cwde insd iretd iretdf "
    "iretf jecxz lfs lgs lodsd loopd looped loopned loopnzd loopzd "
    "lss movsd movsx movzx outsd popad popfd pushad pushd pushfd scasd "
    "seta setae setb setbe setc sete setg setge setl setle setna "
    "setnae setnb setnbe setnc setne setng setnge setnl setnle setno "
    "setnp setns setnz seto setp setpe setpo sets setz shld shrd stosd "
    "bswap cmpxchg invd  invlpg  wbinvd  xadd lock rep repe repne "
    "repnz repz";
const char* AsmWordlist2 =
    "f2xm1 fabs fadd faddp fbld fbstp fchs fclex fcom fcomp fcompp "
    "fdecstp fdisi fdiv fdivp fdivr fdivrp feni ffree fiadd ficom "
    "ficomp fidiv fidivr fild fimul fincstp finit fist fistp fisub "
    "fisubr fld fld1 fldcw fldenv fldenvw fldl2e fldl2t fldlg2 fldln2 "
    "fldpi fldz fmul fmulp fnclex fndisi fneni fninit fnop fnsave "
    "fnsavew fnstcw fnstenv fnstenvw fnstsw fpatan fprem fptan frndint "
    "frstor frstorw fsave fsavew fscale fsqrt fst fstcw fstenv fstenvw "
    "fstp fstsw fsub fsubp fsubr fsubrp ftst fwait fxam fxch fxtract "
    "fyl2x fyl2xp1 fsetpm fcos fldenvd fnsaved fnstenvd fprem1 frstord "
    "fsaved fsin fsincos fstenvd fucom fucomp fucompp";
const char* AsmWordlist3 =
    "ah al ax bh bl bp bx ch cl cr0 cr2 cr3 cs cx dh di dl dr0 dr1 dr2 "
    "dr3 dr6 dr7 ds dx eax ebp ebx ecx edi edx es esi esp fs gs si sp "
    "ss st tr3 tr4 tr5 tr6 tr7";
const char* AsmWordlist4 =
    ".186 .286 .286c .286p .287 .386 .386c .386p .387 .486 .486p .8086 "
    ".8087 .alpha .break .code .const .continue .cref .data .data?  "
    ".dosseg .else .elseif .endif .endw .err .err1 .err2 .errb .errdef "
    ".errdif .errdifi .erre .erridn .erridni .errnb .errndef .errnz "
    ".exit .fardata .fardata? .if .lall .lfcond .list .listall .listif "
    ".listmacro .listmacroall  .model .no87 .nocref .nolist .nolistif "
    ".nolistmacro .radix .repeat .sall .seq .sfcond .stack .startup "
    ".tfcond .type .until .untilcxz .while .xall .xcref .xlist alias "
    "align assume catstr comm comment db dd df dosseg dq dt dup dw "
    "echo else elseif elseif1 elseif2 elseifb elseifdef elseifdif "
    "elseifdifi elseife elseifidn elseifidni elseifnb elseifndef end "
    "endif endm endp ends eq  equ even exitm extern externdef extrn "
    "for forc ge goto group gt high highword if if1 if2 ifb ifdef "
    "ifdif ifdifi ife  ifidn ifidni ifnb ifndef include includelib "
    "instr invoke irp irpc label le length lengthof local low lowword "
    "lroffset lt macro mask mod .msfloat name ne offset opattr option "
    "org %out page popcontext proc proto ptr public purge pushcontext "
    "record repeat rept seg segment short size sizeof sizestr struc "
    "struct substr subtitle subttl textequ this title type typedef "
    "union while width";
const char* AsmWordlist5 =
    "$ ? @b @f addr basic byte c carry? dword far far16 fortran fword "
    "near near16 overflow? parity? pascal qword real4 real8 real10 "
    "sbyte sdword sign? stdcall sword syscall tbyte vararg word zero? "
    "flat near32 far32 abs all assumes at casemap common compact cpu "
    "dotname emulator epilogue error export expr16 expr32 farstack "
    "flat forceframe huge language large listing ljmp loadds m510 "
    "medium memory nearstack nodotname noemulator nokeyword noljmp "
    "nom510 none nonunique nooldmacros nooldstructs noreadonly "
    "noscoped nosignextend nothing notpublic oldmacros oldstructs "
    "os_dos para private prologue radix  readonly req scoped setif2 "
    "smallstack tiny use16 use32 uses";
// C++
#define AC_CPP_RESERVED_WORDS_LIST_FOR_SCINTILLA\
    "asm auto bool break case catch char class const const_cast "\
    "continue default delete do double dynamic_cast else enum explicit "\
    "export extern false float for friend goto if inline int long "\
    "mutable namespace new operator private protected public register "\
    "reinterpret_cast return short signed sizeof static static_cast "\
    "struct switch template this throw true try typedef typeid "\
    "typename union unsigned using virtual void volatile wchar_t "\
    "while"

const char* CppWordlist1 = AC_CPP_RESERVED_WORDS_LIST_FOR_SCINTILLA;

const char* CppWordlist2 = "file";

const char* CppWordlist3 =
    "break continue do for while switch case default void "
    "if else float int bool true false long short double unsigned sizeof namespace using "
    "goto inline volatile public static extern external "
    "a addindex addtogroup anchor arg attention author b brief bug c "
    "class code date def defgroup deprecated dontinclude e em endcode "
    "endhtmlonly endif endlatexonly endlink endverbatim enum example "
    "exception f$ f[ f] file fn hideinitializer htmlinclude "
    "htmlonly if image include ingroup internal invariant interface "
    "latexonly li line link mainpage name namespace nosubgrouping note "
    "overload p page par param post pre ref relates remarks return "
    "retval sa section see showinitializer since skip skipline struct "
    "subsection test throw todo typedef union until var verbatim "
    "verbinclude version warning weakgroup $ @ \"\" & < > # { }";

// GLSL (last version updated GLSL1.5):
const char* GlslWordlist1 =
    // Keywords
    "attribute const uniform varying centroid flat smooth noperspective "

    // Do not add words shared by CPP and GLSL, since we want the word to treat these words as CPP:
    //"break continue do for while switch case default "
    //"if else float int bool true false long short double unsigned sizeof namespace using "
    //"goto inline volatile public static extern external "
    "in out inout invariant discard return mat2 mat3 mat4 mat2x2 mat2x3 mat2x4 mat3x2 mat3x3 mat3x4 mat4x2 mat4x3 mat4x4 "
    "vec2 vec3 vec4 ivec2 ivec3 ivec4 bvec2 bvec3 bvec4 "
    "uint uvec2 uvec3 uvec4 lowp mediump highp precision "
    "sampler1D sampler2D sampler3D samplerCube sampler1DShadow sampler2DShadow samplerCubeShadow "
    "sampler1DArray sampler2DArray sampler1DArrayShadow sampler2DArrayShadow "
    "isampler1D isampler2D isampler3D isamplerCube isampler1DArray isampler2DArray "
    "usampler1D usampler2D usampler3D usamplerCube usampler1DArray usampler2DArray struct "
    "asm class union enum typedef template this packed "
    "noinline interface half fixed lowp mediump highp precision input output "
    "hvec2 hvec3 hvec4 dvec2 dvec3 dvec4 fvec2 fvec3 fvec4 "
    "sampler2DRect sampler3DRect sampler2DRectShadow "
    "active common filter iimage1D iimage1DArray iimage2D iimage2DArray iimage3D iimageBuffer iimageCube cast"
    "image1D image1DArray image1DArrayShadow image1DShadow image2D image2DArray image2DArrayShadow image2DShadow image3D imageBuffer imageCube"
    "output isampler2DRect isamplerBuffer layout partition row_major samplerBuffer superp uimage1D uimage1DArray uimage2D uimage2DArray uimage3D uimageBuffer uimageCube"
    "usampler2DRect usamplerBuffer attribute centroid flat"
    "isampler2DMS isampler2DMSArray noperspective sampler2DMS sampler2DMSArray sizeofcast smooth uniform usampler2DMS usampler2DMSArray varying FALSE";


// not in use
const char* GlslWordlist2 = "";

const char* GlslBuiltInFunctions =
    // BUILT-IN FUNCTIONS
    "abs acos acosh all any asin asinh atan atanh ceil "
    "clamp cos cosh cross degrees dFdx dFdy distance "
    "dot equal exp exp2 faceforward floor fract "
    "ftransform fwidth greaterThan greaterThanEqual inversesqrt isinf isnan length lessThan "
    "lessThanEqual log log2 matrixCompMult max min mix "
    "mod modf noise1 noise2 noise3 noise4 normalize not "
    "notEqual outerProduct pow radians reflect refract round roundEven shadow1D shadow1DLod "
    "shadow1DProj shadow1DProjLod shadow2D shadow2DLod shadow2DProj shadow2DProjLod sign "
    "sin sinh smoothstep sqrt step tan tanh texelFetch texelFetchOffset texture texture1D texture1DLod "
    "texture1DProj texture1DProjLod texture2D texture2DLod texture2DProj texture2DProjLod texture3D "
    "texture3DLod texture3DProj texture3DProjLod textureCube textureCubeLod "
    "textureGrad textureGradOffset textureLod textureLodOffset textureOffset textureProj textureProjGrad textureProjGradOffset"
    "textureProjLod textureProjLodOffset textureSize transpose trunc"
    "EmitVertex EndPrimitive "

    // BUILT-IN VARIABLES
    "gl_BackColor gl_BackLightModelProduct gl_BackLightProduct gl_BackMaterial gl_BackSecondaryColor "
    "gl_ClipPlane gl_ClipDistance gl_ClipVertex gl_Color gl_DepthRange gl_DepthRangeParameters "
    "gl_EyePlaneQ gl_EyePlaneR gl_EyePlaneS gl_EyePlaneT gl_Fog "
    "gl_FogCoord gl_FogFragCoord gl_FogParameters gl_FragColor gl_FragCoord "
    "gl_FragData gl_FragDepth gl_FrontColor gl_FrontFacing gl_FrontLightModelProduct "
    "gl_FrontLightProduct gl_FrontMaterial gl_FrontSecondaryColor gl_InstanceID gl_Layer gl_LightModel gl_LightModelParameters "
    "gl_LightModelProducts gl_LightProducts gl_LightSource gl_LightSourceParameters gl_MaterialParameters gl_MaxClipDistances "
    "gl_MaxClipPlanes gl_MaxCombinedTextureImageUnits gl_MaxDrawBuffers gl_MaxFragmentUniformComponents gl_MaxLights "
    "gl_MaxTextureCoords gl_MaxTextureImageUnits gl_MaxTextureUnits gl_MaxVaryingComponents gl_MaxVaryingFloats gl_MaxVertexAttribs "
    "gl_MaxVertexTextureImageUnits gl_MaxVertexUniformComponents gl_ModelViewMatrix gl_ModelViewMatrixInverse gl_ModelViewMatrixInverseTranspose "
    "gl_ModelViewMatrixTranspose gl_ModelViewProjectionMatrix gl_ModelViewProjectionMatrixInverse gl_ModelViewProjectionMatrixInverseTranspose gl_ModelViewProjectionMatrixTranspose "
    "gl_MultiTexCoord0 gl_MultiTexCoord1 gl_MultiTexCoord2 gl_MultiTexCoord3 gl_MultiTexCoord4 "
    "gl_MultiTexCoord5 gl_MultiTexCoord6 gl_MultiTexCoord7 gl_Normal gl_NormalMatrix "
    "gl_NormalScale gl_ObjectPlaneQ gl_ObjectPlaneR gl_ObjectPlaneS gl_ObjectPlaneT "
    "gl_Point gl_PointCoord gl_PointParameters gl_PointSize gl_Position gl_PrimitiveID gl_ProjectionMatrix "
    "gl_ProjectionMatrixInverse gl_ProjectionMatrixInverseTranspose gl_ProjectionMatrixTranspose gl_SecondaryColor gl_TexCoord "
    "gl_TextureEnvColor gl_TextureMatrix gl_TextureMatrixInverse gl_TextureMatrixInverseTranspose gl_TextureMatrixTranspose gl_Vertex gl_VertexID "
    "gl_FrontColor gl_BackColor gl_FrontSecondaryColor gl_BackSecondaryColor gl_TexCoord gl_FogFragCoord "

    // BUILT-IN CONSTANTS
    "gl_BackColorIn gl_BackSecondaryColorIn gl_ClipVertexIn gl_FogFragCoordIn gl_FrontColorIn gl_FrontSecondaryColorIn gl_PointSizeIn gl_PositionIn gl_PrimitiveIDIn gl_TexCoordIn gl_VerticesIn gl_InstanceIDARB gl_InstanceID";

// CL (OpenCL programming language):
// CL last version updated - OpenCL 2.0
const char* ClTypeKeywords =
    "__attribute__ __constant __generic __global __kernel __kernel_exec __local __private __read_only __read_write __write_only aligned atomic_double atomic_flag atomic_float "
    "atomic_int atomic_intptr_t atomic_long atomic_ptrdiff_t atomic_size_t atomic_uint atomic_uintptr_t atomic_ulong bool char "
    "char16 char2 char3 char4 char8 cl_mem_fence_flags clk_event_t clk_profiling_info const device "
    "double double16 double2 double3 double4 double8 endian even event_t extern "
    "float float16 float2 float3 float4 float8 generic half hi host "
    "image1d_array_t image1d_buffer_t image1d_t image2d_array_depth_t image2d_array_t image2d_depth_t image2d_t image3d_t int int16 "
    "int2 int3 int4 int8 intptr_t kernel_enqueue_flags_t lo long long16 long2 "
    "long3 long4 long8 memory_order memory_scope ndrange_t nosvm odd opencl_unroll_hint packed "
    "pipe ptrdiff_t queue_t read_only read_write reqd_work_group_size reserve_id_t restrict sampler_t short "
    "short16 short2 short3 short4 short8 size_t static typedef uchar uchar16 "
    "uchar2 uchar3 uchar4 uchar8 uint uint16 uint2 uint3 uint4 uint8 "
    "uintptr_t ulong ulong16 ulong2 ulong3 ulong4 ulong8 uniform unsigned ushort "
    "ushort16 ushort2 ushort3 ushort4 ushort8 vec_type_hint void volatile work_group_size_hint write_only ";

const char* ClTypeReservedKeywords =
    "bool16 bool2 bool3 bool4 bool8 complex double16x16 double16x2 double16x3 double16x4 "
    "double16x8 double2x16 double2x2 double2x3 double2x4 double2x8 double3x16 double3x2 double3x3 double3x4 "
    "double3x8 double4x16 double4x2 double4x3 double4x4 double4x8 double8x16 double8x2 double8x3 double8x4 "
    "double8x8 float16x16 float16x2 float16x3 float16x4 float16x8 float2x16 float2x2 float2x3 float2x4 "
    "float2x8 float3x16 float3x2 float3x3 float3x4 float3x8 float4x16 float4x2 float4x3 float4x4 "
    "float4x8 float8x16 float8x2 float8x3 float8x4 float8x8 half16 half2 half3 half4 "
    "half8 imaginary quad quad16 quad2 quad3 quad4 quad8 ";

const char* ClBuiltInFunctions =
    "abs abs_diff acos acosh acospi add_sat all any as_char as_char16 as_char2 as_char3 as_char4 as_char8 as_double as_double16 as_double2 as_double3 as_double4 as_double8 "
    "as_float as_float16 as_float2 as_float3 as_float4 as_float8 as_int as_int16 as_int2 as_int3 as_int4 as_int8 as_long as_long16 as_long2 as_long3 as_long4 as_long8 as_short as_short16 "
    "as_short2 as_short3 as_short4 as_short8 as_uchar as_uchar16 as_uchar2 as_uchar3 as_uchar4 as_uchar8 as_uint as_uint16 as_uint2 as_uint3 as_uint4 as_uint8 as_ulong as_ulong16 as_ulong2 as_ulong3 "
    "as_ulong4 as_ulong8 as_ushort as_ushort16 as_ushort2 as_ushort3 as_ushort4 as_ushort8 asin asinh asinpi async_work_group_copy async_work_group_strided_copy atan atan2 atan2pi atanh atanpi atomic_compare_exchange_strong atomic_compare_exchange_strong_explicit "
    "atomic_compare_exchange_weak atomic_compare_exchange_weak_explicit atomic_exchange atomic_exchange_explicit atomic_fetch_key atomic_fetch_key_explicit atomic_flag_clear atomic_flag_clear_explicit atomic_flag_test_and_set atomic_flag_test_and_set_explicit atomic_load atomic_load_explicit atomic_store atomic_store_explicit atomic_work_item_fence bitselect capture_event_profiling_info cbrt ceil clamp "
    "clz commit_read_pipe commit_write_pipe convert_char convert_char_rte convert_char_rtn convert_char_rtp convert_char_rtz convert_char_sat convert_char_sat_rte convert_char_sat_rtn convert_char_sat_rtp convert_char_sat_rtz convert_char16 convert_char16_rte convert_char16_rtn convert_char16_rtp convert_char16_rtz convert_char16_sat convert_char16_sat_rte "
    "convert_char16_sat_rtn convert_char16_sat_rtp convert_char16_sat_rtz convert_char2 convert_char2_rte convert_char2_rtn convert_char2_rtp convert_char2_rtz convert_char2_sat convert_char2_sat_rte convert_char2_sat_rtn convert_char2_sat_rtp convert_char2_sat_rtz convert_char3 convert_char3_rte convert_char3_rtn convert_char3_rtp convert_char3_rtz convert_char3_sat convert_char3_sat_rte "
    "convert_char3_sat_rtn convert_char3_sat_rtp convert_char3_sat_rtz convert_char4 convert_char4_rte convert_char4_rtn convert_char4_rtp convert_char4_rtz convert_char4_sat convert_char4_sat_rte convert_char4_sat_rtn convert_char4_sat_rtp convert_char4_sat_rtz convert_char8 convert_char8_rte convert_char8_rtn convert_char8_rtp convert_char8_rtz convert_char8_sat convert_char8_sat_rte "
    "convert_char8_sat_rtn convert_char8_sat_rtp convert_char8_sat_rtz convert_float convert_float_rte convert_float_rtn convert_float_rtp convert_float_rtz convert_float_sat convert_float_sat_rte convert_float_sat_rtn convert_float_sat_rtp convert_float_sat_rtz convert_float16 convert_float16_rte convert_float16_rtn convert_float16_rtp convert_float16_rtz convert_float16_sat convert_float16_sat_rte "
    "convert_float16_sat_rtn convert_float16_sat_rtp convert_float16_sat_rtz convert_float2 convert_float2_rte convert_float2_rtn convert_float2_rtp convert_float2_rtz convert_float2_sat convert_float2_sat_rte convert_float2_sat_rtn convert_float2_sat_rtp convert_float2_sat_rtz convert_float3 convert_float3_rte convert_float3_rtn convert_float3_rtp convert_float3_rtz convert_float3_sat convert_float3_sat_rte "
    "convert_float3_sat_rtn convert_float3_sat_rtp convert_float3_sat_rtz convert_float4 convert_float4_rte convert_float4_rtn convert_float4_rtp convert_float4_rtz convert_float4_sat convert_float4_sat_rte convert_float4_sat_rtn convert_float4_sat_rtp convert_float4_sat_rtz convert_float8 convert_float8_rte convert_float8_rtn convert_float8_rtp convert_float8_rtz convert_float8_sat convert_float8_sat_rte "
    "convert_float8_sat_rtn convert_float8_sat_rtp convert_float8_sat_rtz convert_int convert_int_rte convert_int_rtn convert_int_rtp convert_int_rtz convert_int_sat convert_int_sat_rte convert_int_sat_rtn convert_int_sat_rtp convert_int_sat_rtz convert_int16 convert_int16_rte convert_int16_rtn convert_int16_rtp convert_int16_rtz convert_int16_sat convert_int16_sat_rte "
    "convert_int16_sat_rtn convert_int16_sat_rtp convert_int16_sat_rtz convert_int2 convert_int2_rte convert_int2_rtn convert_int2_rtp convert_int2_rtz convert_int2_sat convert_int2_sat_rte convert_int2_sat_rtn convert_int2_sat_rtp convert_int2_sat_rtz convert_int3 convert_int3_rte convert_int3_rtn convert_int3_rtp convert_int3_rtz convert_int3_sat convert_int3_sat_rte "
    "convert_int3_sat_rtn convert_int3_sat_rtp convert_int3_sat_rtz convert_int4 convert_int4_rte convert_int4_rtn convert_int4_rtp convert_int4_rtz convert_int4_sat convert_int4_sat_rte convert_int4_sat_rtn convert_int4_sat_rtp convert_int4_sat_rtz convert_int8 convert_int8_rte convert_int8_rtn convert_int8_rtp convert_int8_rtz convert_int8_sat convert_int8_sat_rte "
    "convert_int8_sat_rtn convert_int8_sat_rtp convert_int8_sat_rtz convert_long convert_long_rte convert_long_rtn convert_long_rtp convert_long_rtz convert_long_sat convert_long_sat_rte convert_long_sat_rtn convert_long_sat_rtp convert_long_sat_rtz convert_long16 convert_long16_rte convert_long16_rtn convert_long16_rtp convert_long16_rtz convert_long16_sat convert_long16_sat_rte "
    "convert_long16_sat_rtn convert_long16_sat_rtp convert_long16_sat_rtz convert_long2 convert_long2_rte convert_long2_rtn convert_long2_rtp convert_long2_rtz convert_long2_sat convert_long2_sat_rte convert_long2_sat_rtn convert_long2_sat_rtp convert_long2_sat_rtz convert_long3 convert_long3_rte convert_long3_rtn convert_long3_rtp convert_long3_rtz convert_long3_sat convert_long3_sat_rte "
    "convert_long3_sat_rtn convert_long3_sat_rtp convert_long3_sat_rtz convert_long4 convert_long4_rte convert_long4_rtn convert_long4_rtp convert_long4_rtz convert_long4_sat convert_long4_sat_rte convert_long4_sat_rtn convert_long4_sat_rtp convert_long4_sat_rtz convert_long8 convert_long8_rte convert_long8_rtn convert_long8_rtp convert_long8_rtz convert_long8_sat convert_long8_sat_rte "
    "convert_long8_sat_rtn convert_long8_sat_rtp convert_long8_sat_rtz convert_short convert_short_rte convert_short_rtn convert_short_rtp convert_short_rtz convert_short_sat convert_short_sat_rte convert_short_sat_rtn convert_short_sat_rtp convert_short_sat_rtz convert_short16 convert_short16_rte convert_short16_rtn convert_short16_rtp convert_short16_rtz convert_short16_sat convert_short16_sat_rte "
    "convert_short16_sat_rtn convert_short16_sat_rtp convert_short16_sat_rtz convert_short2 convert_short2_rte convert_short2_rtn convert_short2_rtp convert_short2_rtz convert_short2_sat convert_short2_sat_rte convert_short2_sat_rtn convert_short2_sat_rtp convert_short2_sat_rtz convert_short3 convert_short3_rte convert_short3_rtn convert_short3_rtp convert_short3_rtz convert_short3_sat convert_short3_sat_rte "
    "convert_short3_sat_rtn convert_short3_sat_rtp convert_short3_sat_rtz convert_short4 convert_short4_rte convert_short4_rtn convert_short4_rtp convert_short4_rtz convert_short4_sat convert_short4_sat_rte convert_short4_sat_rtn convert_short4_sat_rtp convert_short4_sat_rtz convert_short8 convert_short8_rte convert_short8_rtn convert_short8_rtp convert_short8_rtz convert_short8_sat convert_short8_sat_rte "
    "convert_short8_sat_rtn convert_short8_sat_rtp convert_short8_sat_rtz convert_uchar convert_uchar_rte convert_uchar_rtn convert_uchar_rtp convert_uchar_rtz convert_uchar_sat convert_uchar_sat_rte convert_uchar_sat_rtn convert_uchar_sat_rtp convert_uchar_sat_rtz convert_uchar16 convert_uchar16_rte convert_uchar16_rtn convert_uchar16_rtp convert_uchar16_rtz convert_uchar16_sat convert_uchar16_sat_rte "
    "convert_uchar16_sat_rtn convert_uchar16_sat_rtp convert_uchar16_sat_rtz convert_uchar2 convert_uchar2_rte convert_uchar2_rtn convert_uchar2_rtp convert_uchar2_rtz convert_uchar2_sat convert_uchar2_sat_rte convert_uchar2_sat_rtn convert_uchar2_sat_rtp convert_uchar2_sat_rtz convert_uchar3 convert_uchar3_rte convert_uchar3_rtn convert_uchar3_rtp convert_uchar3_rtz convert_uchar3_sat convert_uchar3_sat_rte "
    "convert_uchar3_sat_rtn convert_uchar3_sat_rtp convert_uchar3_sat_rtz convert_uchar4 convert_uchar4_rte convert_uchar4_rtn convert_uchar4_rtp convert_uchar4_rtz convert_uchar4_sat convert_uchar4_sat_rte convert_uchar4_sat_rtn convert_uchar4_sat_rtp convert_uchar4_sat_rtz convert_uchar8 convert_uchar8_rte convert_uchar8_rtn convert_uchar8_rtp convert_uchar8_rtz convert_uchar8_sat convert_uchar8_sat_rte "
    "convert_uchar8_sat_rtn convert_uchar8_sat_rtp convert_uchar8_sat_rtz convert_uint convert_uint_rte convert_uint_rtn convert_uint_rtp convert_uint_rtz convert_uint_sat convert_uint_sat_rte convert_uint_sat_rtn convert_uint_sat_rtp convert_uint_sat_rtz convert_uint16 convert_uint16_rte convert_uint16_rtn convert_uint16_rtp convert_uint16_rtz convert_uint16_sat convert_uint16_sat_rte "
    "convert_uint16_sat_rtn convert_uint16_sat_rtp convert_uint16_sat_rtz convert_uint2 convert_uint2_rte convert_uint2_rtn convert_uint2_rtp convert_uint2_rtz convert_uint2_sat convert_uint2_sat_rte convert_uint2_sat_rtn convert_uint2_sat_rtp convert_uint2_sat_rtz convert_uint3 convert_uint3_rte convert_uint3_rtn convert_uint3_rtp convert_uint3_rtz convert_uint3_sat convert_uint3_sat_rte "
    "convert_uint3_sat_rtn convert_uint3_sat_rtp convert_uint3_sat_rtz convert_uint4 convert_uint4_rte convert_uint4_rtn convert_uint4_rtp convert_uint4_rtz convert_uint4_sat convert_uint4_sat_rte convert_uint4_sat_rtn convert_uint4_sat_rtp convert_uint4_sat_rtz convert_uint8 convert_uint8_rte convert_uint8_rtn convert_uint8_rtp convert_uint8_rtz convert_uint8_sat convert_uint8_sat_rte "
    "convert_uint8_sat_rtn convert_uint8_sat_rtp convert_uint8_sat_rtz convert_ulong convert_ulong_rte convert_ulong_rtn convert_ulong_rtp convert_ulong_rtz convert_ulong_sat convert_ulong_sat_rte convert_ulong_sat_rtn convert_ulong_sat_rtp convert_ulong_sat_rtz convert_ulong16 convert_ulong16_rte convert_ulong16_rtn convert_ulong16_rtp convert_ulong16_rtz convert_ulong16_sat convert_ulong16_sat_rte "
    "convert_ulong16_sat_rtn convert_ulong16_sat_rtp convert_ulong16_sat_rtz convert_ulong2 convert_ulong2_rte convert_ulong2_rtn convert_ulong2_rtp convert_ulong2_rtz convert_ulong2_sat convert_ulong2_sat_rte convert_ulong2_sat_rtn convert_ulong2_sat_rtp convert_ulong2_sat_rtz convert_ulong3 convert_ulong3_rte convert_ulong3_rtn convert_ulong3_rtp convert_ulong3_rtz convert_ulong3_sat convert_ulong3_sat_rte "
    "convert_ulong3_sat_rtn convert_ulong3_sat_rtp convert_ulong3_sat_rtz convert_ulong4 convert_ulong4_rte convert_ulong4_rtn convert_ulong4_rtp convert_ulong4_rtz convert_ulong4_sat convert_ulong4_sat_rte convert_ulong4_sat_rtn convert_ulong4_sat_rtp convert_ulong4_sat_rtz convert_ulong8 convert_ulong8_rte convert_ulong8_rtn convert_ulong8_rtp convert_ulong8_rtz convert_ulong8_sat convert_ulong8_sat_rte "
    "convert_ulong8_sat_rtn convert_ulong8_sat_rtp convert_ulong8_sat_rtz convert_ushort convert_ushort_rte convert_ushort_rtn convert_ushort_rtp convert_ushort_rtz convert_ushort_sat convert_ushort_sat_rte convert_ushort_sat_rtn convert_ushort_sat_rtp convert_ushort_sat_rtz convert_ushort16 convert_ushort16_rte convert_ushort16_rtn convert_ushort16_rtp convert_ushort16_rtz convert_ushort16_sat convert_ushort16_sat_rte "
    "convert_ushort16_sat_rtn convert_ushort16_sat_rtp convert_ushort16_sat_rtz convert_ushort2 convert_ushort2_rte convert_ushort2_rtn convert_ushort2_rtp convert_ushort2_rtz convert_ushort2_sat convert_ushort2_sat_rte convert_ushort2_sat_rtn convert_ushort2_sat_rtp convert_ushort2_sat_rtz convert_ushort3 convert_ushort3_rte convert_ushort3_rtn convert_ushort3_rtp convert_ushort3_rtz convert_ushort3_sat convert_ushort3_sat_rte "
    "convert_ushort3_sat_rtn convert_ushort3_sat_rtp convert_ushort3_sat_rtz convert_ushort4 convert_ushort4_rte convert_ushort4_rtn convert_ushort4_rtp convert_ushort4_rtz convert_ushort4_sat convert_ushort4_sat_rte convert_ushort4_sat_rtn convert_ushort4_sat_rtp convert_ushort4_sat_rtz convert_ushort8 convert_ushort8_rte convert_ushort8_rtn convert_ushort8_rtp convert_ushort8_rtz convert_ushort8_sat convert_ushort8_sat_rte "
    "convert_ushort8_sat_rtn convert_ushort8_sat_rtp convert_ushort8_sat_rtz copysign cos cosh cospi create_user_event cross ctz degrees distance dot enqueue_kernel enqueue_marker erf erfc exp exp10 exp2 "
    "expm1 fabs fast_distance fast_length fast_normalize fdim floor fma fmax fmin fmod fract frexp get_default_queue get_enqueued_local_size get_fence get_global_id get_global_linear_id get_global_offset get_global_size "
    "get_group_id get_image_array_size get_image_channel_data_type get_image_channel_order get_image_depth get_image_dim get_image_height get_image_width get_kernel_preferred_work_group_size_multiple get_kernel_work_group_size get_local_id get_local_linear_id get_local_size get_num_groups get_pipe_max_packets get_pipe_num_packets get_work_dim hadd half_cos half_divide "
    "half_exp half_exp10 half_exp2 half_log10 half_log2 half_powr half_recip half_rsqrt half_sin half_sqrt half_tan hypot ilogb is_valid_event is_valid_reserve_id isequal isfinite isgreater isgreaterequal isinf "
    "isless islessequal islessgreater isnan isnormal isnoteuqal isordered isunordered ldexp length lgamma lgamma_r log log10 log1p log2 logb mad mad_hi mad_sat "
    "mad24 max maxmag min minmag mix modf mul_hi mul24 nan native_cos native_divide native_exp native_exp10 native_exp2 native_log native_log10 native_log2 native_powr native_recip "
    "native_rsqrt native_sin native_sqrt native_tan ndrange_1D ndrange_2D ndrange_3D nextafter normalize popcount pow pown powr prefetch radians read_imagef read_imagei read_imageui read_pipe release_event "
    "remainder remquo reserve_read_pipe reserve_write_pipe retain_event rhadd rint rootn rotate round rsqrt select set_user_event_statue shuffle shuffle2 sign signbit sin sincos sinh "
    "sinpi smoothstep sqrt step sub_sat tan tanh tanpi tgamma to_global to_local to_private trunc upsample vec_step vload_half vload_half_rte vload_half_rtn vload_half_rtp vload_half_rtz "
    "vload_half16 vload_half16_rte vload_half16_rtn vload_half16_rtp vload_half16_rtz vload_half2 vload_half2_rte vload_half2_rtn vload_half2_rtp vload_half2_rtz vload_half3 vload_half3_rte vload_half3_rtn vload_half3_rtp vload_half3_rtz vload_half4 vload_half4_rte vload_half4_rtn vload_half4_rtp vload_half4_rtz "
    "vload_half8 vload_half8_rte vload_half8_rtn vload_half8_rtp vload_half8_rtz vload16 vload2 vload3 vload4 vload8 vloada_half16 vloada_half16_rte vloada_half16_rtn vloada_half16_rtp vloada_half16_rtz vloada_half2 vloada_half2_rte vloada_half2_rtn vloada_half2_rtp vloada_half2_rtz "
    "vloada_half3 vloada_half3_rte vloada_half3_rtn vloada_half3_rtp vloada_half3_rtz vloada_half4 vloada_half4_rte vloada_half4_rtn vloada_half4_rtp vloada_half4_rtz vloada_half8 vloada_half8_rte vloada_half8_rtn vloada_half8_rtp vloada_half8_rtz vstore_half vstore_half_rte vstore_half_rtn vstore_half_rtp vstore_half_rtz "
    "vstore_half16 vstore_half16_rte vstore_half16_rtn vstore_half16_rtp vstore_half16_rtz vstore_half2 vstore_half2_rte vstore_half2_rtn vstore_half2_rtp vstore_half2_rtz vstore_half3 vstore_half3_rte vstore_half3_rtn vstore_half3_rtp vstore_half3_rtz vstore_half4 vstore_half4_rte vstore_half4_rtn vstore_half4_rtp vstore_half4_rtz "
    "vstore_half8 vstore_half8_rte vstore_half8_rtn vstore_half8_rtp vstore_half8_rtz vstore16 vstore2 vstore3 vstore4 vstore8 vstorea_half16 vstorea_half16_rte vstorea_half16_rtn vstorea_half16_rtp vstorea_half16_rtz vstorea_half2 vstorea_half2_rte vstorea_half2_rtn vstorea_half2_rtp vstorea_half2_rtz "
    "vstorea_half3 vstorea_half3_rte vstorea_half3_rtn vstorea_half3_rtp vstorea_half3_rtz vstorea_half4 vstorea_half4_rte vstorea_half4_rtn vstorea_half4_rtp vstorea_half4_rtz vstorea_half8 vstorea_half8_rte vstorea_half8_rtn vstorea_half8_rtp vstorea_half8_rtz wait_group_events work_group_all work_group_any work_group_barrier work_group_broadcast "
    "work_group_commit_read_pipe work_group_commit_write_pipe work_group_reduce_add work_group_reduce_max work_group_reduce_min work_group_reserve_read_pipe work_group_reserve_write_pipe work_group_scan_exclusive_add work_group_scan_exclusive_max work_group_scan_exclusive_min work_group_scan_inclusive_add work_group_scan_inclusive_max work_group_scan_inclusive_min write_imagef write_imagei write_imageui write_pipe ";

const char* ClBuiltInConstants =
    "__ENDIAN_LITTLE__ __FAST_RELAXED_MATH__ __FILE__ __IMAGE_SUPPORT__ __LINE__ __OPENCL_C_VERSION__ __OPENCL_VERSION__ ATOMIC_FLAG_INIT ATOMIC_VAR_INIT CHAR_BIT "
    "CHAR_MAX CHAR_MIN CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE CL_VERSION_1_0 CL_VERSION_1_1 CL_VERSION_1_2 CL_VERSION_2_0 CLK_A CLK_ABGR CLK_ADDRESS_CLAMP "
    "CLK_ADDRESS_CLAMP_TO_EDGE CLK_ADDRESS_MIRRORED_REPEAT CLK_ADDRESS_NONE CLK_ADDRESS_REPEAT CLK_ARGB CLK_BGRA CLK_DEPTH CLK_DEVICE_QUEUE_FULL CLK_ENQUEUE_FLAGS_NO_WAIT CLK_ENQUEUE_FLAGS_WAIT_KERNEL "
    "CLK_ENQUEUE_FLAGS_WAIT_WORK_GROUP CLK_EVENT_ALLOCATION_FAILURE CLK_FILTER_LINEAR CLK_FILTER_NEAREST CLK_FLOAT CLK_GLOBAL_MEM_FENCE CLK_HALF_FLOAT CLK_IMAGE_MEM_FENCE CLK_INTENSITY CLK_INVALID_ARG_SIZE "
    "CLK_INVALID_EVENT_WAIT_LIST CLK_INVALID_NDRANGE CLK_INVALID_QUEUE CLK_LOCAL_MEM_FENCE CLK_LUMINANCE CLK_NORMALIZED_COORDS_FALSE CLK_NORMALIZED_COORDS_TRUE CLK_NULL_RESERVE_ID CLK_OUT_OF_RESOURCES CLK_R "
    "CLK_RA CLK_RG CLK_RGB CLK_RGBA CLK_RGBx CLK_RGx CLK_Rx CLK_sBGRA CLK_SIGNED_INT16 CLK_SIGNED_INT32 "
    "CLK_SIGNED_INT8 CLK_SNORM_INT16 CLK_SNORM_INT8 CLK_sRGB CLK_sRGBA CLK_sRGBx CLK_UNORM_INT16 CLK_UNORM_INT8 CLK_UNORM_SHORT_101010 CLK_UNORM_SHORT_555 "
    "CLK_UNORM_SHORT_565 CLK_UNSIGNED_INT16 CLK_UNSIGNED_INT32 CLK_UNSIGNED_INT8 DBL_DIG DBL_EPSILSON DBL_MANT_DIG DBL_MAX DBL_MAX_10_EXP DBL_MAX_EXP "
    "DBL_MIN DBL_MIN_10_EXP DBL_MIN_EXP FLT_DIG FLT_EPSILSON FLT_MANT_DIG FLT_MAX FLT_MAX_10_EXP FLT_MAX_EXP FLT_MIN "
    "FLT_MIN_10_EXP FLT_MIN_EXP FLT_RADIX FP_CONTRACT FP_FAST_FMAF HIGE_VAL HUGE_VALF INFINITY INT_MAX INT_MIN "
    "LONG_MAX LONG_MIN M_1_PI M_1_PI_F M_2_PI M_2_PI_F M_2_SQRTPI M_2_SQRTPI_F M_E M_E_F "
    "M_LN10 M_LN10_F M_LN2 M_LN2_F M_LOG10E M_LOG10E_F M_LOG2E M_LOG2E_F M_PI M_PI_2 "
    "M_PI_2_F M_PI_4 M_PI_4_F M_PI_F M_SQRT1_2 M_SQRT1_2_F M_SQRT2 M_SQRT2_F MAXFLOAT memory_order_acq_rel "
    "memory_order_acquire memory_order_relaxed memory_order_release memory_order_seq_cst memory_scope_all_svm_devices memory_scope_device memory_scope_work_group memory_scope_work_item NAN NULL "
    "SCHAR_MAX SCHAR_MIN SHRT_MAX SHRT_MIN UCHAR_MAX UINT_MAX ULONG_MAX USHRT_MAX ";



// OpenGL
#define AC_OPENGL_FUNCTIONS_LIST_FOR_SCINTILLA1\
    "glAccum glAlphaFunc glAreTexturesResident glArrayElement glBegin glBindTexture glBitmap glBlendFunc "\
    "glCallList glCallLists glClear glClearAccum glClearColor glClearDepth glClearIndex glClearStencil "\
    "glClipPlane glColor3b glColor3bv glColor3d glColor3dv glColor3f glColor3fv glColor3i "\
    "glColor3iv glColor3s glColor3sv glColor3ub glColor3ubv glColor3ui glColor3uiv glColor3us "\
    "glColor3usv glColor4b glColor4bv glColor4d glColor4dv glColor4f glColor4fv glColor4i "\
    "glColor4iv glColor4s glColor4sv glColor4ub glColor4ubv glColor4ui glColor4uiv glColor4us "\
    "glColor4usv glColorMask glColorMaterial glColorPointer glCopyPixels glCopyTexImage1D glCopyTexImage2D glCopyTexSubImage1D "\
    "glCopyTexSubImage2D glCullFace glDeleteLists glDeleteTextures glDepthFunc glDepthMask glDepthRange glDisable "\
    "glDisableClientState glDrawArrays glDrawBuffer glDrawElements glDrawPixels glEdgeFlag glEdgeFlagPointer glEdgeFlagv "\
    "glEnable glEnableClientState glEnd glEndList glEvalCoord1d glEvalCoord1dv glEvalCoord1f glEvalCoord1fv "\
    "glEvalCoord2d glEvalCoord2dv glEvalCoord2f glEvalCoord2fv glEvalMesh1 glEvalMesh2 glEvalPoint1 glEvalPoint2 "\
    "glFeedbackBuffer glFinish glFlush glFogf glFogfv glFogi glFogiv glFrontFace "\
    "glFrustum glGenLists GLuint glGenTextures glGetBooleanv glGetClipPlane glGetDoublev glGetError glGetFloatv "\
    "glGetIntegerv glGetLightfv glGetLightiv glGetMapdv glGetMapfv glGetMapiv glGetMaterialfv glGetMaterialiv "\
    "glGetPixelMapfv glGetPixelMapuiv glGetPixelMapusv glGetPointerv glGetPolygonStipple glGetString glGetTexEnvfv glGetTexEnviv "\
    "glGetTexGendv glGetTexGenfv glGetTexGeniv glGetTexImage glGetTexLevelParameterfv glGetTexLevelParameteriv glGetTexParameterfv glGetTexParameteriv "\
    "glHint glIndexMask glIndexPointer glIndexd glIndexdv glIndexf glIndexfv glIndexi "\
    "glIndexiv glIndexs glIndexsv glIndexub glIndexubv glInitNames glInterleavedArrays glIsEnabled glIsList glIsTexture "\
    "GLboolean GLboolean glLightModelf glLightModelfv glLightModeli glLightModeliv glLightf glLightfv "\
    "glLighti glLightiv glLineStipple glLineWidth glListBase glLoadIdentity glLoadMatrixd glLoadMatrixf "\
    "glLoadName glLogicOp glMap1d glMap1f glMap2d glMap2f glMapGrid1d glMapGrid1f "\
    "glMapGrid2d glMapGrid2f glMaterialf glMaterialfv glMateriali glMaterialiv glMatrixMode glMultMatrixd "\
    "glMultMatrixf glNewList glNormal3b glNormal3bv glNormal3d glNormal3dv glNormal3f glNormal3fv "\
    "glNormal3i glNormal3iv glNormal3s glNormal3sv glNormalPointer glOrtho glPassThrough glPixelMapfv "\
    "glPixelMapuiv glPixelMapusv glPixelStoref glPixelStorei glPixelTransferf glPixelTransferi glPixelZoom glPointSize "\
    "glPolygonMode glPolygonOffset glPolygonStipple glPopAttrib glPopClientAttrib glPopMatrix glPopName glPrioritizeTextures "\
    "glPushAttrib glPushClientAttrib glPushMatrix glPushName glRasterPos2d glRasterPos2dv glRasterPos2f glRasterPos2fv "\
    "glRasterPos2i glRasterPos2iv glRasterPos2s glRasterPos2sv glRasterPos3d glRasterPos3dv glRasterPos3f glRasterPos3fv "\
    "glRasterPos3i glRasterPos3iv glRasterPos3s glRasterPos3sv glRasterPos4d glRasterPos4dv glRasterPos4f glRasterPos4fv "\
    "glRasterPos4i glRasterPos4iv glRasterPos4s glRasterPos4sv glReadBuffer glReadPixels glRectd glRectdv "\
    "glRectf glRectfv glRecti glRectiv glRects glRectsv glRenderMode glRotated glRotatef "\
    "glScaled glScalef glScissor glSelectBuffer glShadeModel glStencilFunc glStencilMask "\
    "glStencilOp glTexCoord1d glTexCoord1dv glTexCoord1f glTexCoord1fv glTexCoord1i glTexCoord1iv glTexCoord1s "\
    "glTexCoord1sv glTexCoord2d glTexCoord2dv glTexCoord2f glTexCoord2fv glTexCoord2i glTexCoord2iv glTexCoord2s "\
    "glTexCoord2sv glTexCoord3d glTexCoord3dv glTexCoord3f glTexCoord3fv glTexCoord3i glTexCoord3iv glTexCoord3s "\
    "glTexCoord3sv glTexCoord4d glTexCoord4dv glTexCoord4f glTexCoord4fv glTexCoord4i glTexCoord4iv glTexCoord4s "\
    "glTexCoord4sv glTexCoordPointer glTexEnvf glTexEnvfv glTexEnvi glTexEnviv glTexGend glTexGendv "\
    "glTexGenf glTexGenfv glTexGeni glTexGeniv glTexImage1D glTexImage2D glTexParameterf glTexParameterfv "\
    "glTexParameteri glTexParameteriv glTexSubImage1D glTexSubImage2D glTranslated glTranslatef glVertex2d glVertex2dv "\
    "glVertex2f glVertex2fv glVertex2i glVertex2iv glVertex2s glVertex2sv glVertex3d glVertex3dv "\
    "glVertex3f glVertex3fv glVertex3i glVertex3iv glVertex3s glVertex3sv glVertex4d glVertex4dv "\
    "glVertex4f glVertex4fv glVertex4i glVertex4iv glVertex4s glVertex4sv glVertexPointer glViewport "\
    \
    /* OpenGL types */\
    "GLint GLenum GLboolean GLbitfield GLbyte GLshort GLsizei GLubyte GLushort GLuint GLfloat GLclampf GLdouble GLclampd GLvoid "\
    \
    /* Basic OpenGL ES */\
    "glAlphaFuncx glClearColorx glClearDepthf glClearDepthx glClipPlanef glClipPlanex glColor4x glDepthRangef glDepthRangex "\
    "glFogx glFogxv glFrustumf glFrustumx glGetClipPlanef glGetClipPlanex glGetFixedv glGetLightxv glGetMaterialxv glGetTexEnvxv "\
    "glGetTexParameterxv glLightModelx glLightModelxv glLightx glLightxv glLineWidthx glLoadMatrixx glMaterialx glMaterialxv "\
    "glMultMatrixx glMultiTexCoord4x glNormal3x glOrthof glOrthox glPointParameterx glPointParameterxv glPointSizex glPolygonOffsetx "\
    "glRotatex glSampleCoveragex glScalex glTexEnvx glTexEnvxv glTexParameterx glTexParameterxv glTranslatex "\
    \
    /* OpenGL ES 2.0*/\
    "glGetShaderPrecisionFormat glReleaseShaderCompiler glShaderBinary "\
    \
    /* OpenGL advanced versions [1.2 - 3.1]*/\
    "glBlendColor glBlendEquation glDrawRangeElements glColorTable glColorTableParameterfv glColorTableParameteriv "\
    "glCopyColorTable glGetColorTable glGetColorTableParameterfv glGetColorTableParameteriv glColorSubTable glCopyColorSubTable "\
    "glConvolutionFilter1D glConvolutionFilter2D glConvolutionParameterf glConvolutionParameterfv glConvolutionParameteri glConvolutionParameteriv "\
    "glCopyConvolutionFilter1D glCopyConvolutionFilter2D glGetConvolutionFilter glGetConvolutionParameterfv glGetConvolutionParameteriv glGetSeparableFilter "\
    "glSeparableFilter2D glGetHistogram glGetHistogramParameterfv glGetHistogramParameteriv glGetMinmax glGetMinmaxParameterfv "\
    "glGetMinmaxParameteriv glHistogram glMinmax glResetHistogram glResetMinmax glTexImage3D "\
    "glTexSubImage3D glCopyTexSubImage3D glActiveTexture glClientActiveTexture glMultiTexCoord1d glMultiTexCoord1dv "\
    "glMultiTexCoord1f glMultiTexCoord1fv glMultiTexCoord1i glMultiTexCoord1iv glMultiTexCoord1s glMultiTexCoord1sv "\
    "glMultiTexCoord2d glMultiTexCoord2dv glMultiTexCoord2f glMultiTexCoord2fv glMultiTexCoord2i glMultiTexCoord2iv "\
    "glMultiTexCoord2s glMultiTexCoord2sv glMultiTexCoord3d glMultiTexCoord3dv glMultiTexCoord3f glMultiTexCoord3fv "\
    "glMultiTexCoord3i glMultiTexCoord3iv glMultiTexCoord3s glMultiTexCoord3sv glMultiTexCoord4d glMultiTexCoord4dv "\
    "glMultiTexCoord4f glMultiTexCoord4fv glMultiTexCoord4i glMultiTexCoord4iv glMultiTexCoord4s glMultiTexCoord4sv "\
    "glLoadTransposeMatrixf glLoadTransposeMatrixd glMultTransposeMatrixf glMultTransposeMatrixd glSampleCoverage glSamplePass glCompressedTexImage3D "\
    "glCompressedTexImage2D glCompressedTexImage1D glCompressedTexSubImage3D glCompressedTexSubImage2D glCompressedTexSubImage1D glGetCompressedTexImage "\
    "glBlendFuncSeparate glFogCoordf glFogCoordfv glFogCoordd glFogCoorddv glFogCoordPointer "\
    "glMultiDrawArrays glMultiDrawElements glPointParameterf glPointParameterfv glPointParameteri glPointParameteriv "\
    "glSecondaryColor3b glSecondaryColor3bv glSecondaryColor3d glSecondaryColor3dv glSecondaryColor3f glSecondaryColor3fv "\
    "glSecondaryColor3i glSecondaryColor3iv glSecondaryColor3s glSecondaryColor3sv glSecondaryColor3ub glSecondaryColor3ubv "\
    "glSecondaryColor3ui glSecondaryColor3uiv glSecondaryColor3us glSecondaryColor3usv glSecondaryColorPointer glWindowPos2d "\
    "glWindowPos2dv glWindowPos2f glWindowPos2fv glWindowPos2i glWindowPos2iv glWindowPos2s "\
    "glWindowPos2sv glWindowPos3d glWindowPos3dv glWindowPos3f glWindowPos3fv glWindowPos3i "\
    "glWindowPos3iv glWindowPos3s glWindowPos3sv glGenQueries glDeleteQueries glIsQuery "\
    "glBeginQuery glEndQuery glGetQueryiv glGetQueryObjectiv glGetQueryObjectuiv glBindBuffer "\
    "glDeleteBuffers glGenBuffers glIsBuffer glBufferData glBufferSubData glGetBufferSubData "\
    "glMapBuffer glUnmapBuffer glGetBufferParameteriv glGetBufferPointerv glBlendEquationSeparate glDrawBuffers "\
    "glStencilOpSeparate glStencilFuncSeparate glStencilMaskSeparate glAttachShader glBindAttribLocation glCompileShader "\
    "glCreateProgram glCreateShader glDeleteProgram glDeleteShader glDetachShader glDisableVertexAttribArray "\
    "glEnableVertexAttribArray glGetActiveAttrib glGetActiveUniform glGetAttachedShaders glGetAttribLocation glGetProgramiv "\
    "glGetProgramInfoLog glGetShaderiv glGetShaderInfoLog glGetShaderSource glGetUniformLocation glGetUniformfv "\
    "glGetUniformiv glGetVertexAttribdv glGetVertexAttribfv glGetVertexAttribiv glGetVertexAttribPointerv glIsProgram "\
    "glIsShader glLinkProgram glShaderSource glUseProgram glUniform1f glUniform2f "\
    "glUniform3f glUniform4f glUniform1i glUniform2i glUniform3i glUniform4i "\
    "glUniform1fv glUniform2fv glUniform3fv glUniform4fv glUniform1iv glUniform2iv "\
    "glUniform3iv glUniform4iv glUniformMatrix2fv glUniformMatrix3fv glUniformMatrix4fv glValidateProgram "\
    "glVertexAttrib1d glVertexAttrib1dv glVertexAttrib1f glVertexAttrib1fv glVertexAttrib1s glVertexAttrib1sv "\
    "glVertexAttrib2d glVertexAttrib2dv glVertexAttrib2f glVertexAttrib2fv glVertexAttrib2s glVertexAttrib2sv "\
    "glVertexAttrib3d glVertexAttrib3dv glVertexAttrib3f glVertexAttrib3fv glVertexAttrib3s glVertexAttrib3sv "\
    "glVertexAttrib4Nbv glVertexAttrib4Niv glVertexAttrib4Nsv glVertexAttrib4Nub glVertexAttrib4Nubv glVertexAttrib4Nuiv "\
    "glVertexAttrib4Nusv glVertexAttrib4bv glVertexAttrib4d glVertexAttrib4dv glVertexAttrib4f glVertexAttrib4fv "\
    "glVertexAttrib4iv glVertexAttrib4s glVertexAttrib4sv glVertexAttrib4ubv glVertexAttrib4uiv glVertexAttrib4usv glVertexAttribPointer "\
    "glUniformMatrix2x3fv glUniformMatrix3x2fv glUniformMatrix2x4fv glUniformMatrix4x2fv glUniformMatrix3x4fv glUniformMatrix4x3fv "\
    "glColorMaski glGetBooleani_v glGetIntegeri_v glEnablei glDisablei glIsEnabledi glBeginTransformFeedback glEndTransformFeedback "\
    "glBindBufferRange glBindBufferBase glTransformFeedbackVaryings glGetTransformFeedbackVarying glClampColor glBeginConditionalRender "\
    "glEndConditionalRender glVertexAttribI1i glVertexAttribI2i glVertexAttribI3i glVertexAttribI4i glVertexAttribI1ui glVertexAttribI2ui "\
    "glVertexAttribI3ui glVertexAttribI4ui glVertexAttribI1iv glVertexAttribI2iv glVertexAttribI3iv glVertexAttribI4iv glVertexAttribI1uiv "\
    "glVertexAttribI2uiv glVertexAttribI3uiv glVertexAttribI4uiv glVertexAttribI4bv glVertexAttribI4sv glVertexAttribI4ubv glVertexAttribI4usv "\
    "glVertexAttribIPointer glGetVertexAttribIiv glGetVertexAttribIuiv glGetUniformuiv glBindFragDataLocation glGetFragDataLocation "\
    "glUniform1ui glUniform2ui glUniform3ui glUniform4ui glUniform1uiv glUniform2uiv glUniform3uiv glUniform4uiv glTexParameterIiv "\
    "glTexParameterIuiv glGetTexParameterIiv glGetTexParameterIuiv glClearBufferiv glClearBufferuiv glClearBufferfv "\
    "glClearBufferfi glGetStringi glDrawArraysInstanced glDrawElementsInstanced glTexBuffer glPrimitiveRestartIndex "\
    /* GL 3.2 */ \
    "glGetInteger64i_v glGetBufferParameteri64v glProgramParameteri glFramebufferTexture glFramebufferTextureFace "\
    /* GL 3.3 */ \
    "glBindFragDataLocationIndexed glGetFragDataIndex glGenSamplers glDeleteSamplers glIsSampler glBindSampler glSamplerParameteri glSamplerParameteriv "\
    "glSamplerParameterf glSamplerParameterfv glSamplerParameterIiv glSamplerParameterIuiv glGetSamplerParameteriv glGetSamplerParameterIiv "\
    "glGetSamplerParameterfv glGetSamplerParameterIuiv glQueryCounter glGetQueryObjecti64v glGetQueryObjectui64v glVertexAttribDivisor glVertexAttribP1ui "\
    "glVertexAttribP1uiv glVertexAttribP2ui glVertexAttribP2uiv glVertexAttribP3ui glVertexAttribP3uiv glVertexAttribP4ui glVertexAttribP4uiv glVertexP2ui "\
    "glVertexP2uiv glVertexP3ui glVertexP3uiv glVertexP4ui glVertexP4uiv glTexCoordP1ui glTexCoordP1uiv glTexCoordP2ui glTexCoordP2uiv glTexCoordP3ui glTexCoordP3uiv "\
    "glTexCoordP4ui glTexCoordP4uiv glMultiTexCoordP1ui glMultiTexCoordP1uiv glMultiTexCoordP2ui glMultiTexCoordP2uiv glMultiTexCoordP3ui glMultiTexCoordP3uiv "\
    "glMultiTexCoordP4ui glMultiTexCoordP4uiv glNormalP3ui glNormalP3uiv glColorP3ui glColorP3uiv glColorP4ui glColorP4uiv glSecondaryColorP3ui glSecondaryColorP3uiv "\
    /* GL 4.0 */ \
    "glMinSampleShading glBlendEquationi glBlendEquationSeparatei glBlendFunci glBlendFuncSeparatei glDrawArraysIndirect glDrawElementsIndirect "\
    "glUniform1d glUniform2d glUniform3d glUniform4d glUniform1dv glUniform2dv glUniform3dv glUniform4dv glUniformMatrix2dv glUniformMatrix3dv "\
    "glUniformMatrix4dv glUniformMatrix2x3dv glUniformMatrix2x4dv glUniformMatrix3x2dv glUniformMatrix3x4dv glUniformMatrix4x2dv "\
    "glUniformMatrix4x3dv glGetUniformdv glGetSubroutineUniformLocation glGetSubroutineIndex glGetActiveSubroutineUniformiv "\
    "glGetActiveSubroutineUniformName glGetActiveSubroutineName glUniformSubroutinesuiv glGetUniformSubroutineuiv glGetProgramStageiv "\
    "glPatchParameteri glPatchParameterfv glBindTransformFeedback glDeleteTransformFeedbacks glGenTransformFeedbacks glIsTransformFeedback glPauseTransformFeedback "\
    "glResumeTransformFeedback glDrawTransformFeedback glDrawTransformFeedbackStream glBeginQueryIndexed glEndQueryIndexed glGetQueryIndexediv "\
    /* GL 4.1 */ \
    "glReleaseShaderCompiler glShaderBinary glGetShaderPrecisionFormat glDepthRangef glClearDepthf glGetProgramBinary glProgramBinary glProgramParameteri "\
    "glUseProgramStages glActiveShaderProgram glCreateShaderProgramv glBindProgramPipeline glDeleteProgramPipelines glGenProgramPipelines glIsProgramPipeline "\
    "glGetProgramPipelineiv glProgramUniform1i glProgramUniform1iv glProgramUniform1f glProgramUniform1fv glProgramUniform1d glProgramUniform1dv glProgramUniform1ui "\
    "glProgramUniform1uiv glProgramUniform2i glProgramUniform2iv glProgramUniform2f glProgramUniform2fv glProgramUniform2d glProgramUniform2dv glProgramUniform2ui "\
    "glProgramUniform2uiv glProgramUniform3i glProgramUniform3iv glProgramUniform3f glProgramUniform3fv glProgramUniform3d glProgramUniform3dv glProgramUniform3ui "\
    "glProgramUniform3uiv glProgramUniform4i glProgramUniform4iv glProgramUniform4f glProgramUniform4fv glProgramUniform4d glProgramUniform4dv glProgramUniform4ui "\
    "glProgramUniform4uiv glProgramUniformMatrix2fv glProgramUniformMatrix3fv glProgramUniformMatrix4fv glProgramUniformMatrix2dv glProgramUniformMatrix3dv "\
    "glProgramUniformMatrix4dv glProgramUniformMatrix2x3fv glProgramUniformMatrix3x2fv glProgramUniformMatrix2x4fv glProgramUniformMatrix4x2fv "\
    "glProgramUniformMatrix3x4fv glProgramUniformMatrix4x3fv glProgramUniformMatrix2x3dv glProgramUniformMatrix3x2dv glProgramUniformMatrix2x4dv "\
    "glProgramUniformMatrix4x2dv glProgramUniformMatrix3x4dv glProgramUniformMatrix4x3dv glValidateProgramPipeline glGetProgramPipelineInfoLog glVertexAttribL1d "\
    "glVertexAttribL2d glVertexAttribL3d glVertexAttribL4d glVertexAttribL1dv glVertexAttribL2dv glVertexAttribL3dv glVertexAttribL4dv glVertexAttribLPointer "\
    "glGetVertexAttribLdv glViewportArrayv glViewportIndexedf glViewportIndexedfv glScissorArrayv glScissorIndexed glScissorIndexedv glDepthRangeArrayv "\
    "glDepthRangeIndexed glGetFloati_v glGetDoublei_v "\
    /* GL 4.2 */ \
    "glDrawArraysInstancedBaseInstance glDrawElementsInstancedBaseInstance glDrawElementsInstancedBaseVertexBaseInstance glGetInternalformativ "\
    "glGetActiveAtomicCounterBufferiv glBindImageTexture glMemoryBarrier glTexStorage1D glTexStorage2D glTexStorage3D glDrawTransformFeedbackInstanced "\
    "glDrawTransformFeedbackStreamInstanced "\
    /* GL 4.3 */ \
    "glClearBufferData glClearBufferSubData glDispatchCompute glDispatchComputeIndirect glCopyImageSubData glFramebufferParameteri glGetFramebufferParameteriv "\
    "glGetInternalformati64v glInvalidateTexSubImage glInvalidateTexImage glInvalidateBufferSubData glInvalidateBufferData glInvalidateFramebuffer "\
    "glInvalidateSubFramebuffer glMultiDrawArraysIndirect glMultiDrawElementsIndirect glGetProgramInterfaceiv glGetProgramResourceIndex glGetProgramResourceName "\
    "glGetProgramResourceiv glGetProgramResourceLocation glGetProgramResourceLocationIndex glShaderStorageBlockBinding glTexBufferRange glTexStorage2DMultisample "\
    "glTexStorage3DMultisample glTextureView glBindVertexBuffer glVertexAttribFormat glVertexAttribIFormat glVertexAttribLFormat glVertexAttribBinding "\
    "glVertexBindingDivisor glDebugMessageControl glDebugMessageInsert glDebugMessageCallback glGetDebugMessageLog glPushDebugGroup glPopDebugGroup glObjectLabel "\
    "glGetObjectLabel glObjectPtrLabel glGetObjectPtrLabel "\
    /* GL 4.4 */ \
    "glBufferStorage glClearTexImage glClearTexSubImage glBindBuffersBase glBindBuffersRange glBindTextures glBindSamplers glBindImageTextures glBindVertexBuffers "\
    /* GL 4.5 */ \
    "glClipControl glCreateTransformFeedbacks glTransformFeedbackBufferBase glTransformFeedbackBufferRange glGetTransformFeedbackiv glGetTransformFeedbacki_v "\
    " glGetTransformFeedbacki64_v glCreateBuffers glNamedBufferStorage glNamedBufferData glNamedBufferSubData glCopyNamedBufferSubData glClearNamedBufferData "\
    "glClearNamedBufferSubData glMapNamedBuffer glMapNamedBufferRange glUnmapNamedBuffer glFlushMappedNamedBufferRange glGetNamedBufferParameteriv "\
    "glGetNamedBufferParameteri64v glGetNamedBufferPointerv glGetNamedBufferSubData glCreateFramebuffers glNamedFramebufferRenderbuffer glNamedFramebufferParameteri "\
    "glNamedFramebufferTexture glNamedFramebufferTextureLayer glNamedFramebufferDrawBuffer glNamedFramebufferDrawBuffers glNamedFramebufferReadBuffer "\
    "glInvalidateNamedFramebufferData glInvalidateNamedFramebufferSubData glClearNamedFramebufferiv glClearNamedFramebufferuiv glClearNamedFramebufferfv "\
    "glClearNamedFramebufferfi glBlitNamedFramebuffer glCheckNamedFramebufferStatus glGetNamedFramebufferParameteriv glGetNamedFramebufferAttachmentParameteriv "\
    "glCreateRenderbuffers glNamedRenderbufferStorage glNamedRenderbufferStorageMultisample glGetNamedRenderbufferParameteriv glCreateTextures glTextureBuffer "\
    "glTextureBufferRange glTextureStorage1D glTextureStorage2D glTextureStorage3D glTextureStorage2DMultisample glTextureStorage3DMultisample glTextureSubImage1D "\
    "glTextureSubImage2D glTextureSubImage3D glCompressedTextureSubImage1D glCompressedTextureSubImage2D glCompressedTextureSubImage3D glCopyTextureSubImage1D "\
    "glCopyTextureSubImage2D glCopyTextureSubImage3D glTextureParameterf glTextureParameterfv glTextureParameteri glTextureParameterIiv glTextureParameterIuiv "\
    "glTextureParameteriv glGenerateTextureMipmap glBindTextureUnit glGetTextureImage glGetCompressedTextureImage glGetTextureLevelParameterfv "\
    "glGetTextureLevelParameteriv glGetTextureParameterfv glGetTextureParameterIiv glGetTextureParameterIuiv glGetTextureParameteriv glCreateVertexArrays "\
    "glDisableVertexArrayAttrib glEnableVertexArrayAttrib glVertexArrayElementBuffer glVertexArrayVertexBuffer glVertexArrayVertexBuffers glVertexArrayAttribBinding "\
    "glVertexArrayAttribFormat glVertexArrayAttribIFormat glVertexArrayAttribLFormat glVertexArrayBindingDivisor glGetVertexArrayiv glGetVertexArrayIndexediv "\
    "glGetVertexArrayIndexed64iv glCreateSamplers glCreateProgramPipelines glCreateQueries glGetQueryBufferObjecti64v glGetQueryBufferObjectiv "\
    "glGetQueryBufferObjectui64v glGetQueryBufferObjectuiv glMemoryBarrierByRegion glGetTextureSubImage glGetCompressedTextureSubImage glGetGraphicsResetStatus "\
    "glGetnCompressedTexImage glGetnTexImage glGetnUniformdv glGetnUniformfv glGetnUniformiv glGetnUniformuiv glReadnPixels glGetnMapdv glGetnMapfv glGetnMapiv "\
    "glGetnPixelMapfv glGetnPixelMapuiv glGetnPixelMapusv glGetnPolygonStipple glGetnColorTable glGetnConvolutionFilter glGetnSeparableFilter glGetnHistogram "\
    "glGetnMinmax glTextureBarrier "\
    /* OpenGL Extensions (Updated until GL_GLEXT_VERSION 44):*/\
    "glActiveTextureARB glClientActiveTextureARB glMultiTexCoord1dARB glMultiTexCoord1dvARB glMultiTexCoord1fARB glMultiTexCoord1fvARB glMultiTexCoord1iARB "\
    "glMultiTexCoord1ivARB glMultiTexCoord1sARB glMultiTexCoord1svARB glMultiTexCoord2dARB glMultiTexCoord2dvARB glMultiTexCoord2fARB glMultiTexCoord2fvARB "\
    "glMultiTexCoord2iARB glMultiTexCoord2ivARB glMultiTexCoord2sARB glMultiTexCoord2svARB glMultiTexCoord3dARB glMultiTexCoord3dvARB glMultiTexCoord3fARB "\
    "glMultiTexCoord3fvARB glMultiTexCoord3iARB glMultiTexCoord3ivARB glMultiTexCoord3sARB glMultiTexCoord3svARB glMultiTexCoord4dARB glMultiTexCoord4dvARB "\
    "glMultiTexCoord4fARB glMultiTexCoord4fvARB glMultiTexCoord4iARB glMultiTexCoord4ivARB glMultiTexCoord4sARB glMultiTexCoord4svARB "\
    "glEnableTraceMESA glDisableTraceMESA glNewTraceMESA glEndTraceMESA glTraceAssertAttribMESA glTraceCommentMESA glTraceTextureMESA glTraceListMESA glTracePointerMESA glTracePointerRangeMESA "\
    "glProgramCallbackMESA glGetProgramRegisterfvMESA glBlendEquationSeparateATI "\
    "glLoadTransposeMatrixfARB glLoadTransposeMatrixdARB glMultTransposeMatrixfARB glMultTransposeMatrixdARB glSampleCoverageARB glCompressedTexImage3DARB glCompressedTexImage2DARB glCompressedTexImage1DARB "\
    "glCompressedTexSubImage3DARB glCompressedTexSubImage2DARB glCompressedTexSubImage1DARB glGetCompressedTexImageARB glPointParameterfARB glPointParameterfvARB glWeightbvARB "\
    "glWeightsvARB glWeightivARB glWeightfvARB glWeightdvARB glWeightubvARB glWeightusvARB glWeightuivARB "\
    "glWeightPointerARB glVertexBlendARB glCurrentPaletteMatrixARB glMatrixIndexubvARB glMatrixIndexusvARB glMatrixIndexuivARB glMatrixIndexPointerARB "\
    "glWindowPos2dARB glWindowPos2dvARB glWindowPos2fARB glWindowPos2fvARB glWindowPos2iARB glWindowPos2ivARB glWindowPos2sARB "\
    "glWindowPos2svARB glWindowPos3dARB glWindowPos3dvARB glWindowPos3fARB glWindowPos3fvARB glWindowPos3iARB glWindowPos3ivARB "\
    "glWindowPos3sARB glWindowPos3svARB glVertexAttrib1dARB glVertexAttrib1dvARB glVertexAttrib1fARB glVertexAttrib1fvARB glVertexAttrib1sARB "\
    "glVertexAttrib1svARB glVertexAttrib2dARB glVertexAttrib2dvARB glVertexAttrib2fARB glVertexAttrib2fvARB glVertexAttrib2sARB glVertexAttrib2svARB "\
    "glVertexAttrib3dARB glVertexAttrib3dvARB glVertexAttrib3fARB glVertexAttrib3fvARB glVertexAttrib3sARB glVertexAttrib3svARB glVertexAttrib4NbvARB "\
    "glVertexAttrib4NivARB glVertexAttrib4NsvARB glVertexAttrib4NubARB glVertexAttrib4NubvARB glVertexAttrib4NuivARB glVertexAttrib4NusvARB glVertexAttrib4bvARB "\
    "glVertexAttrib4dARB glVertexAttrib4dvARB glVertexAttrib4fARB glVertexAttrib4fvARB glVertexAttrib4ivARB glVertexAttrib4sARB glVertexAttrib4svARB "\
    "glVertexAttrib4ubvARB glVertexAttrib4uivARB glVertexAttrib4usvARB glVertexAttribPointerARB glEnableVertexAttribArrayARB glDisableVertexAttribArrayARB glProgramStringARB "\
    "glBindProgramARB glDeleteProgramsARB glGenProgramsARB glProgramEnvParameter4dARB glProgramEnvParameter4dvARB glProgramEnvParameter4fARB glProgramEnvParameter4fvARB "\
    "glProgramLocalParameter4dARB glProgramLocalParameter4dvARB glProgramLocalParameter4fARB glProgramLocalParameter4fvARB glGetProgramEnvParameterdvARB glGetProgramEnvParameterfvARB glGetProgramLocalParameterdvARB "\
    "glGetProgramLocalParameterfvARB glGetProgramivARB glGetProgramStringARB glGetVertexAttribdvARB glGetVertexAttribfvARB glGetVertexAttribivARB glGetVertexAttribPointervARB "\
    "glIsProgramARB glBindBufferARB glDeleteBuffersARB glGenBuffersARB glIsBufferARB glBufferDataARB glBufferSubDataARB "\
    "glGetBufferSubDataARB glMapBufferARB glUnmapBufferARB glGetBufferParameterivARB glGetBufferPointervARB glGenQueriesARB glDeleteQueriesARB "\
    "glIsQueryARB glBeginQueryARB glEndQueryARB glGetQueryivARB glGetQueryObjectivARB glGetQueryObjectuivARB glDeleteObjectARB "\
    "glGetHandleARB glDetachObjectARB glCreateShaderObjectARB glShaderSourceARB glCompileShaderARB glCreateProgramObjectARB glAttachObjectARB "\
    "glLinkProgramARB glUseProgramObjectARB glValidateProgramARB glUniform1fARB glUniform2fARB glUniform3fARB glUniform4fARB "\
    "glUniform1iARB glUniform2iARB glUniform3iARB glUniform4iARB glUniform1fvARB glUniform2fvARB glUniform3fvARB "\
    "glUniform4fvARB glUniform1ivARB glUniform2ivARB glUniform3ivARB glUniform4ivARB glUniformMatrix2fvARB glUniformMatrix3fvARB "\
    "glUniformMatrix4fvARB glGetObjectParameterfvARB glGetObjectParameterivARB glGetInfoLogARB glGetAttachedObjectsARB glGetUniformLocationARB glGetActiveUniformARB "\
    "glGetUniformfvARB glGetUniformivARB glGetShaderSourceARB glBindAttribLocationARB glGetActiveAttribARB glGetAttribLocationARB glDrawBuffersARB "\
    "glClampColorARB glDrawArraysInstancedARB glDrawElementsInstancedARB "\
    "glIsRenderbuffer glBindRenderbuffer glDeleteRenderbuffers glGenRenderbuffers glRenderbufferStorage glGetRenderbufferParameteriv glIsFramebuffer "\
    "glBindFramebuffer glDeleteFramebuffers glGenFramebuffers glCheckFramebufferStatus glFramebufferTexture1D glFramebufferTexture2D glFramebufferTexture3D glFramebufferRenderbuffer "\
    "glGetFramebufferAttachmentParameteriv glGenerateMipmap glBlitFramebuffer glRenderbufferStorageMultisample glFramebufferTextureLayer "\
    "glProgramParameteriARB glFramebufferTextureARB glFramebufferTextureLayerARB glFramebufferTextureFaceARB glVertexAttribDivisor glVertexAttribDivisorARB glMapBufferRange glFlushMappedBufferRange glTexBufferARB "\
    "glBindVertexArray glDeleteVertexArrays glGenVertexArrays glIsVertexArray "\
    "glBlendColorEXT glPolygonOffsetEXT glTexImage3DEXT glTexSubImage3DEXT glGetTexFilterFuncSGIS glTexFilterFuncSGIS "\
    "glTexSubImage1DEXT glTexSubImage2DEXT glCopyTexImage1DEXT glCopyTexImage2DEXT glCopyTexSubImage1DEXT glCopyTexSubImage2DEXT glCopyTexSubImage3DEXT "\
    "glGetHistogramEXT glGetHistogramParameterfvEXT glGetHistogramParameterivEXT glGetMinmaxEXT glGetMinmaxParameterfvEXT glGetMinmaxParameterivEXT glHistogramEXT "\
    "glMinmaxEXT glResetHistogramEXT glResetMinmaxEXT glConvolutionFilter1DEXT glConvolutionFilter2DEXT glConvolutionParameterfEXT glConvolutionParameterfvEXT "\
    "glConvolutionParameteriEXT glConvolutionParameterivEXT glCopyConvolutionFilter1DEXT glCopyConvolutionFilter2DEXT glGetConvolutionFilterEXT glGetConvolutionParameterfvEXT glGetConvolutionParameterivEXT "\
    "glGetSeparableFilterEXT glSeparableFilter2DEXT glColorTableSGI glColorTableParameterfvSGI glColorTableParameterivSGI glCopyColorTableSGI glGetColorTableSGI "\
    "glGetColorTableParameterfvSGI glGetColorTableParameterivSGI glPixelTexGenSGIX glPixelTexGenParameteriSGIS glPixelTexGenParameterivSGIS glPixelTexGenParameterfSGIS glPixelTexGenParameterfvSGIS "\
    "glGetPixelTexGenParameterivSGIS glGetPixelTexGenParameterfvSGIS glTexImage4DSGIS glTexSubImage4DSGIS glAreTexturesResidentEXT glBindTextureEXT glDeleteTexturesEXT "\
    "glGenTexturesEXT glIsTextureEXT glPrioritizeTexturesEXT glDetailTexFuncSGIS glGetDetailTexFuncSGIS glSharpenTexFuncSGIS glGetSharpenTexFuncSGIS "\
    "glSampleMaskSGIS glSamplePatternSGIS glArrayElementEXT glColorPointerEXT glDrawArraysEXT glEdgeFlagPointerEXT glGetPointervEXT "\
    "glIndexPointerEXT glNormalPointerEXT glTexCoordPointerEXT glVertexPointerEXT glBlendEquationEXT glSpriteParameterfSGIX glSpriteParameterfvSGIX "\
    "glSpriteParameteriSGIX glSpriteParameterivSGIX glPointParameterfEXT glPointParameterfvEXT glPointParameterfSGIS glPointParameterfvSGIS glGetInstrumentsSGIX "\
    "glInstrumentsBufferSGIX glPollInstrumentsSGIX glReadInstrumentsSGIX glStartInstrumentsSGIX glStopInstrumentsSGIX glFrameZoomSGIX glTagSampleBufferSGIX "\
    "glDeformationMap3dSGIX glDeformationMap3fSGIX glDeformSGIX glLoadIdentityDeformationMapSGIX glReferencePlaneSGIX glFlushRasterSGIX glFogFuncSGIS "\
    "glGetFogFuncSGIS glImageTransformParameteriHP glImageTransformParameterfHP glImageTransformParameterivHP glImageTransformParameterfvHP glGetImageTransformParameterivHP glGetImageTransformParameterfvHP "\
    "glColorSubTableEXT glCopyColorSubTableEXT glHintPGI glColorTableEXT glGetColorTableEXT glGetColorTableParameterivEXT glGetColorTableParameterfvEXT "\
    "glGetListParameterfvSGIX glGetListParameterivSGIX glListParameterfSGIX glListParameterfvSGIX glListParameteriSGIX glListParameterivSGIX glIndexMaterialEXT "\
    "glIndexFuncEXT glLockArraysEXT glUnlockArraysEXT glCullParameterdvEXT glCullParameterfvEXT glFragmentColorMaterialSGIX glFragmentLightfSGIX "\
    "glFragmentLightfvSGIX glFragmentLightiSGIX glFragmentLightivSGIX glFragmentLightModelfSGIX glFragmentLightModelfvSGIX glFragmentLightModeliSGIX glFragmentLightModelivSGIX "\
    "glFragmentMaterialfSGIX glFragmentMaterialfvSGIX glFragmentMaterialiSGIX glFragmentMaterialivSGIX glGetFragmentLightfvSGIX glGetFragmentLightivSGIX glGetFragmentMaterialfvSGIX "\
    "glGetFragmentMaterialivSGIX glLightEnviSGIX glDrawRangeElementsEXT glApplyTextureEXT glTextureLightEXT glTextureMaterialEXT glAsyncMarkerSGIX "\
    "glFinishAsyncSGIX glPollAsyncSGIX glGenAsyncMarkersSGIX glDeleteAsyncMarkersSGIX glIsAsyncMarkerSGIX glVertexPointervINTEL glNormalPointervINTEL "\
    "glColorPointervINTEL glTexCoordPointervINTEL glPixelTransformParameteriEXT glPixelTransformParameterfEXT glPixelTransformParameterivEXT glPixelTransformParameterfvEXT glSecondaryColor3bEXT "\
    "glSecondaryColor3bvEXT glSecondaryColor3dEXT glSecondaryColor3dvEXT glSecondaryColor3fEXT glSecondaryColor3fvEXT glSecondaryColor3iEXT glSecondaryColor3ivEXT "\
    "glSecondaryColor3sEXT glSecondaryColor3svEXT glSecondaryColor3ubEXT glSecondaryColor3ubvEXT glSecondaryColor3uiEXT glSecondaryColor3uivEXT glSecondaryColor3usEXT "\
    "glSecondaryColor3usvEXT glSecondaryColorPointerEXT glTextureNormalEXT glMultiDrawArraysEXT glMultiDrawElementsEXT glFogCoordfEXT glFogCoordfvEXT "\
    "glFogCoorddEXT glFogCoorddvEXT glFogCoordPointerEXT glTangent3bEXT glTangent3bvEXT glTangent3dEXT glTangent3dvEXT "\
    "glTangent3fEXT glTangent3fvEXT glTangent3iEXT glTangent3ivEXT glTangent3sEXT glTangent3svEXT glBinormal3bEXT "\
    "glBinormal3bvEXT glBinormal3dEXT glBinormal3dvEXT glBinormal3fEXT glBinormal3fvEXT glBinormal3iEXT glBinormal3ivEXT "\
    "glBinormal3sEXT glBinormal3svEXT glTangentPointerEXT glBinormalPointerEXT glFinishTextureSUNX glGlobalAlphaFactorbSUN glGlobalAlphaFactorsSUN "\
    "glGlobalAlphaFactoriSUN glGlobalAlphaFactorfSUN glGlobalAlphaFactordSUN glGlobalAlphaFactorubSUN glGlobalAlphaFactorusSUN glGlobalAlphaFactoruiSUN glReplacementCodeuiSUN "\
    "glReplacementCodeusSUN glReplacementCodeubSUN glReplacementCodeuivSUN glReplacementCodeusvSUN glReplacementCodeubvSUN glReplacementCodePointerSUN glColor4ubVertex2fSUN "\
    "glColor4ubVertex2fvSUN glColor4ubVertex3fSUN glColor4ubVertex3fvSUN glColor3fVertex3fSUN glColor3fVertex3fvSUN glNormal3fVertex3fSUN glNormal3fVertex3fvSUN "\
    "glColor4fNormal3fVertex3fSUN glColor4fNormal3fVertex3fvSUN glTexCoord2fVertex3fSUN glTexCoord2fVertex3fvSUN glTexCoord4fVertex4fSUN glTexCoord4fVertex4fvSUN glTexCoord2fColor4ubVertex3fSUN "\
    "glTexCoord2fColor4ubVertex3fvSUN glTexCoord2fColor3fVertex3fSUN glTexCoord2fColor3fVertex3fvSUN glTexCoord2fNormal3fVertex3fSUN glTexCoord2fNormal3fVertex3fvSUN glTexCoord2fColor4fNormal3fVertex3fSUN glTexCoord2fColor4fNormal3fVertex3fvSUN "\
    "glTexCoord4fColor4fNormal3fVertex4fSUN glTexCoord4fColor4fNormal3fVertex4fvSUN glReplacementCodeuiVertex3fSUN glReplacementCodeuiVertex3fvSUN glReplacementCodeuiColor4ubVertex3fSUN glReplacementCodeuiColor4ubVertex3fvSUN glReplacementCodeuiColor3fVertex3fSUN "\
    "glReplacementCodeuiColor3fVertex3fvSUN glReplacementCodeuiNormal3fVertex3fSUN glReplacementCodeuiNormal3fVertex3fvSUN glReplacementCodeuiColor4fNormal3fVertex3fSUN glReplacementCodeuiColor4fNormal3fVertex3fvSUN glReplacementCodeuiTexCoord2fVertex3fSUN glReplacementCodeuiTexCoord2fVertex3fvSUN "\
    "glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN glBlendFuncSeparateEXT glBlendFuncSeparateINGR glVertexWeightfEXT "\
    "glVertexWeightfvEXT glVertexWeightPointerEXT glFlushVertexArrayRangeNV glVertexArrayRangeNV glCombinerParameterfvNV glCombinerParameterfNV glCombinerParameterivNV "\
    "glCombinerParameteriNV glCombinerInputNV glCombinerOutputNV glFinalCombinerInputNV glGetCombinerInputParameterfvNV glGetCombinerInputParameterivNV glGetCombinerOutputParameterfvNV "\
    "glGetCombinerOutputParameterivNV glGetFinalCombinerInputParameterfvNV glGetFinalCombinerInputParameterivNV glWindowPos2dMESA glWindowPos2dvMESA glWindowPos2fMESA glWindowPos2fvMESA "\
    "glWindowPos2iMESA glWindowPos2ivMESA glWindowPos2sMESA glWindowPos2svMESA glWindowPos3dMESA glWindowPos3dvMESA glWindowPos3fMESA "\
    "glWindowPos3fvMESA glWindowPos3iMESA glWindowPos3ivMESA glWindowPos3sMESA glWindowPos3svMESA glWindowPos4dMESA glWindowPos4dvMESA "\
    "glWindowPos4fMESA glWindowPos4fvMESA glWindowPos4iMESA glWindowPos4ivMESA glWindowPos4sMESA glWindowPos4svMESA glMultiModeDrawArraysIBM "\
    "glMultiModeDrawElementsIBM glColorPointerListIBM glSecondaryColorPointerListIBM glEdgeFlagPointerListIBM glFogCoordPointerListIBM glIndexPointerListIBM glNormalPointerListIBM "\
    "glTexCoordPointerListIBM glVertexPointerListIBM glTbufferMask3DFX glSampleMaskEXT glSamplePatternEXT glTextureColorMaskSGIS glIglooInterfaceSGIX "\
    "glDeleteFencesNV glGenFencesNV glIsFenceNV glTestFenceNV glGetFenceivNV glFinishFenceNV glSetFenceNV "\
    "glMapControlPointsNV glMapParameterivNV glMapParameterfvNV glGetMapControlPointsNV glGetMapParameterivNV glGetMapParameterfvNV glGetMapAttribParameterivNV "\
    "glGetMapAttribParameterfvNV glEvalMapsNV glCombinerStageParameterfvNV glGetCombinerStageParameterfvNV glAreProgramsResidentNV glBindProgramNV glDeleteProgramsNV "\
    "glExecuteProgramNV glGenProgramsNV glGetProgramParameterdvNV glGetProgramParameterfvNV glGetProgramivNV glGetProgramStringNV glGetTrackMatrixivNV "\
    "glGetVertexAttribdvNV glGetVertexAttribfvNV glGetVertexAttribivNV glGetVertexAttribPointervNV glIsProgramNV glLoadProgramNV glProgramParameter4dNV "\
    "glProgramParameter4dvNV glProgramParameter4fNV glProgramParameter4fvNV glProgramParameters4dvNV glProgramParameters4fvNV glRequestResidentProgramsNV glTrackMatrixNV "\
    "glVertexAttribPointerNV glVertexAttrib1dNV glVertexAttrib1dvNV glVertexAttrib1fNV glVertexAttrib1fvNV glVertexAttrib1sNV glVertexAttrib1svNV "\
    "glVertexAttrib2dNV glVertexAttrib2dvNV glVertexAttrib2fNV glVertexAttrib2fvNV glVertexAttrib2sNV glVertexAttrib2svNV glVertexAttrib3dNV "\
    "glVertexAttrib3dvNV glVertexAttrib3fNV glVertexAttrib3fvNV glVertexAttrib3sNV glVertexAttrib3svNV glVertexAttrib4dNV glVertexAttrib4dvNV "\
    "glVertexAttrib4fNV glVertexAttrib4fvNV glVertexAttrib4sNV glVertexAttrib4svNV glVertexAttrib4ubNV glVertexAttrib4ubvNV glVertexAttribs1dvNV "\
    "glVertexAttribs1fvNV glVertexAttribs1svNV glVertexAttribs2dvNV glVertexAttribs2fvNV glVertexAttribs2svNV glVertexAttribs3dvNV glVertexAttribs3fvNV "\
    "glVertexAttribs3svNV glVertexAttribs4dvNV glVertexAttribs4fvNV glVertexAttribs4svNV glVertexAttribs4ubvNV glTexBumpParameterivATI glTexBumpParameterfvATI "\
    "glGetTexBumpParameterivATI glGetTexBumpParameterfvATI glGenFragmentShadersATI glBindFragmentShaderATI glDeleteFragmentShaderATI glBeginFragmentShaderATI glEndFragmentShaderATI "\
    "glPassTexCoordATI glSampleMapATI glColorFragmentOp1ATI glColorFragmentOp2ATI glColorFragmentOp3ATI glAlphaFragmentOp1ATI glAlphaFragmentOp2ATI "\
    "glAlphaFragmentOp3ATI glSetFragmentShaderConstantATI glPNTrianglesiATI glPNTrianglesfATI glNewObjectBufferATI glIsObjectBufferATI glUpdateObjectBufferATI "\
    "glGetObjectBufferfvATI glGetObjectBufferivATI glFreeObjectBufferATI glArrayObjectATI glGetArrayObjectfvATI glGetArrayObjectivATI glVariantArrayObjectATI "\
    "glGetVariantArrayObjectfvATI glGetVariantArrayObjectivATI glBeginVertexShaderEXT glEndVertexShaderEXT glBindVertexShaderEXT glGenVertexShadersEXT glDeleteVertexShaderEXT "\
    "glShaderOp1EXT glShaderOp2EXT glShaderOp3EXT glSwizzleEXT glWriteMaskEXT glInsertComponentEXT glExtractComponentEXT "\
    "glGenSymbolsEXT glSetInvariantEXT glSetLocalConstantEXT glVariantbvEXT glVariantsvEXT glVariantivEXT glVariantfvEXT "\
    "glVariantdvEXT glVariantubvEXT glVariantusvEXT glVariantuivEXT glVariantPointerEXT glEnableVariantClientStateEXT glDisableVariantClientStateEXT "\
    "glBindLightParameterEXT glBindMaterialParameterEXT glBindTexGenParameterEXT glBindTextureUnitParameterEXT glBindParameterEXT glIsVariantEnabledEXT glGetVariantBooleanvEXT "\
    "glGetVariantIntegervEXT glGetVariantFloatvEXT glGetVariantPointervEXT glGetInvariantBooleanvEXT glGetInvariantIntegervEXT glGetInvariantFloatvEXT glGetLocalConstantBooleanvEXT "\
    "glGetLocalConstantIntegervEXT glGetLocalConstantFloatvEXT glVertexStream1sATI glVertexStream1svATI glVertexStream1iATI glVertexStream1ivATI glVertexStream1fATI "\
    "glVertexStream1fvATI glVertexStream1dATI glVertexStream1dvATI glVertexStream2sATI glVertexStream2svATI glVertexStream2iATI glVertexStream2ivATI "\
    "glVertexStream2fATI glVertexStream2fvATI glVertexStream2dATI glVertexStream2dvATI glVertexStream3sATI glVertexStream3svATI glVertexStream3iATI "\
    "glVertexStream3ivATI glVertexStream3fATI glVertexStream3fvATI glVertexStream3dATI glVertexStream3dvATI glVertexStream4sATI glVertexStream4svATI "\
    "glVertexStream4iATI glVertexStream4ivATI glVertexStream4fATI glVertexStream4fvATI glVertexStream4dATI glVertexStream4dvATI glNormalStream3bATI "\
    "glNormalStream3bvATI glNormalStream3sATI glNormalStream3svATI glNormalStream3iATI glNormalStream3ivATI glNormalStream3fATI glNormalStream3fvATI "\
    "glNormalStream3dATI glNormalStream3dvATI glClientActiveVertexStreamATI glVertexBlendEnviATI glVertexBlendEnvfATI glElementPointerATI glDrawElementArrayATI "\
    "glDrawRangeElementArrayATI glDrawMeshArraysSUN glGenOcclusionQueriesNV glDeleteOcclusionQueriesNV glIsOcclusionQueryNV glBeginOcclusionQueryNV glEndOcclusionQueryNV "\
    "glGetOcclusionQueryivNV glGetOcclusionQueryuivNV glPointParameteriNV glPointParameterivNV glActiveStencilFaceEXT glElementPointerAPPLE glDrawElementArrayAPPLE "\
    "glDrawRangeElementArrayAPPLE glMultiDrawElementArrayAPPLE glMultiDrawRangeElementArrayAPPLE glGenFencesAPPLE glDeleteFencesAPPLE glSetFenceAPPLE glIsFenceAPPLE "\
    "glTestFenceAPPLE glFinishFenceAPPLE glTestObjectAPPLE glFinishObjectAPPLE glBindVertexArrayAPPLE glDeleteVertexArraysAPPLE glGenVertexArraysAPPLE "\
    "glIsVertexArrayAPPLE glVertexArrayRangeAPPLE glFlushVertexArrayRangeAPPLE glVertexArrayParameteriAPPLE glDrawBuffersATI glProgramNamedParameter4fNV glProgramNamedParameter4dNV "\
    "glProgramNamedParameter4fvNV glProgramNamedParameter4dvNV glGetProgramNamedParameterfvNV glGetProgramNamedParameterdvNV glVertex2hNV glVertex2hvNV glVertex3hNV "\
    "glVertex3hvNV glVertex4hNV glVertex4hvNV glNormal3hNV glNormal3hvNV glColor3hNV glColor3hvNV "\
    "glColor4hNV glColor4hvNV glTexCoord1hNV glTexCoord1hvNV glTexCoord2hNV glTexCoord2hvNV glTexCoord3hNV "\
    "glTexCoord3hvNV glTexCoord4hNV glTexCoord4hvNV glMultiTexCoord1hNV glMultiTexCoord1hvNV glMultiTexCoord2hNV glMultiTexCoord2hvNV "\
    "glMultiTexCoord3hNV glMultiTexCoord3hvNV glMultiTexCoord4hNV glMultiTexCoord4hvNV glFogCoordhNV glFogCoordhvNV glSecondaryColor3hNV "\
    "glSecondaryColor3hvNV glVertexWeighthNV glVertexWeighthvNV glVertexAttrib1hNV glVertexAttrib1hvNV glVertexAttrib2hNV glVertexAttrib2hvNV "\
    "glVertexAttrib3hNV glVertexAttrib3hvNV glVertexAttrib4hNV glVertexAttrib4hvNV glVertexAttribs1hvNV glVertexAttribs2hvNV glVertexAttribs3hvNV "\
    "glVertexAttribs4hvNV glPixelDataRangeNV glFlushPixelDataRangeNV glPrimitiveRestartNV glPrimitiveRestartIndexNV glMapObjectBufferATI glUnmapObjectBufferATI "\
    "glStencilOpSeparateATI glStencilFuncSeparateATI glVertexAttribArrayObjectATI glGetVertexAttribArrayObjectfvATI glGetVertexAttribArrayObjectivATI glDepthBoundsEXT glBlendEquationSeparateEXT "\
    "glIsRenderbufferEXT glBindRenderbufferEXT glDeleteRenderbuffersEXT glGenRenderbuffersEXT glRenderbufferStorageEXT glGetRenderbufferParameterivEXT glIsFramebufferEXT "\
    "glBindFramebufferEXT glDeleteFramebuffersEXT glGenFramebuffersEXT glCheckFramebufferStatusEXT glFramebufferTexture1DEXT glFramebufferTexture2DEXT glFramebufferTexture3DEXT "\
    "glFramebufferRenderbufferEXT glGetFramebufferAttachmentParameterivEXT glGenerateMipmapEXT glStencilClearTagEXT glBlitFramebufferEXT glRenderbufferStorageMultisampleEXT glGetQueryObjecti64vEXT "\
    "glGetQueryObjectui64vEXT glProgramEnvParameters4fvEXT glProgramLocalParameters4fvEXT glBufferParameteriAPPLE glFlushMappedBufferRangeAPPLE glFlushRenderAPPLE glFinishRenderAPPLE "\
    "glProgramLocalParameterI4iNV glProgramLocalParameterI4ivNV glProgramLocalParametersI4ivNV glProgramLocalParameterI4uiNV "\
    "glProgramLocalParameterI4uivNV glProgramLocalParametersI4uivNV glProgramEnvParameterI4iNV glProgramEnvParameterI4ivNV glProgramEnvParametersI4ivNV glProgramEnvParameterI4uiNV "\
    "glProgramEnvParameterI4uivNV glProgramEnvParametersI4uivNV glGetProgramLocalParameterIivNV glGetProgramLocalParameterIuivNV glGetProgramEnvParameterIivNV glGetProgramEnvParameterIuivNV "\
    "glProgramVertexLimitNV glFramebufferTextureEXT glFramebufferTextureLayerEXT glFramebufferTextureFaceEXT "\
    "glMakeBufferResidentNV glMakeBufferNonResidentNV glIsBufferResidentNV glNamedMakeBufferResidentNV glNamedMakeBufferNonResidentNV "\
    "glMakeNamedBufferNonResidentNV glMakeNamedBufferResidentNV glIsNamedBufferResidentNV glGetBufferParameterui64vNV "\
    "glGetNamedBufferParameterui64vNV glGetIntegerui64vNV glUniformui64NV glUniformui64vNV glGetUniformui64vNV glProgramUniformui64NV glProgramUniformui64vNV "\
    "glBufferAddressRangeNV glVertexFormatNV glNormalFormatNV glColorFormatNV glIndexFormatNV glTexCoordFormatNV glEdgeFlagFormatNV glSecondaryColorFormatNV glFogCoordFormatNV glVertexAttribFormatNV glVertexAttribIFormatNV glGetIntegerui64i_vNV "\
    "glProgramParameteriEXT glVertexAttribI1iEXT glVertexAttribI2iEXT glVertexAttribI3iEXT "

#define AC_OPENGL_FUNCTIONS_LIST_FOR_SCINTILLA2\
    "glVertexAttribI4iEXT glVertexAttribI1uiEXT glVertexAttribI2uiEXT glVertexAttribI3uiEXT glVertexAttribI4uiEXT glVertexAttribI1ivEXT glVertexAttribI2ivEXT glVertexAttribI3ivEXT "\
    "glVertexAttribI4ivEXT glVertexAttribI1uivEXT glVertexAttribI2uivEXT glVertexAttribI3uivEXT glVertexAttribI4uivEXT glVertexAttribI4bvEXT glVertexAttribI4svEXT glVertexAttribI4ubvEXT "\
    "glVertexAttribI4usvEXT glVertexAttribIPointerEXT glGetVertexAttribIivEXT glGetVertexAttribIuivEXT glGetUniformuivEXT glBindFragDataLocationEXT glGetFragDataLocationEXT glUniform1uiEXT "\
    "glUniform2uiEXT glUniform3uiEXT glUniform4uiEXT glUniform1uivEXT glUniform2uivEXT glUniform3uivEXT glUniform4uivEXT glDrawArraysInstancedEXT glDrawElementsInstancedEXT glTexBufferEXT "\
    "glDepthRangedNV glClearDepthdNV glDepthBoundsdNV glRenderbufferStorageMultisampleCoverageNV glProgramBufferParametersfvNV glProgramBufferParametersIivNV glProgramBufferParametersIuivNV "\
    "glColorMaskIndexedEXT glGetBooleanIndexedvEXT glGetIntegerIndexedvEXT glEnableIndexedEXT glDisableIndexedEXT NTRY glIsEnabledIndexedEXT glBeginTransformFeedbackNV glEndTransformFeedbackNV "\
    "glTransformFeedbackAttribsNV glBindBufferRangeNV glBindBufferOffsetNV glBindBufferBaseNV glTransformFeedbackVaryingsNV glActiveVaryingNV glGetVaryingLocationNV glGetActiveVaryingNV "\
    "glGetTransformFeedbackVaryingNV glUniformBufferEXT glGetUniformBufferSizeEXT TRY glGetUniformOffsetEXT glTexParameterIivEXT glTexParameterIuivEXT glGetTexParameterIivEXT "\
    "glGetTexParameterIuivEXT glClearColorIiEXT glClearColorIuiEXT glStringMarkerGREMEDY glFrameTerminatorGREMEDY "\
    "glBeginConditionalRenderNV glEndConditionalRenderNV glBeginTransformFeedbackEXT glEndTransformFeedbackEXT glBindBufferRangeEXT glBindBufferOffsetEXT glBindBufferBaseEXT "\
    "glTransformFeedbackVaryingsEXT glGetTransformFeedbackVaryingEXT glClientAttribDefaultEXT glPushClientAttribDefaultEXT glMatrixLoadfEXT glMatrixLoaddEXT "\
    "glMatrixMultfEXT glMatrixMultdEXT glMatrixLoadIdentityEXT glMatrixRotatefEXT glMatrixRotatedEXT glMatrixScalefEXT glMatrixScaledEXT glMatrixTranslatefEXT "\
    "glMatrixTranslatedEXT glMatrixFrustumEXT glMatrixOrthoEXT glMatrixPopEXT glMatrixPushEXT glMatrixLoadTransposefEXT glMatrixLoadTransposedEXT "\
    "glMatrixMultTransposefEXT glMatrixMultTransposedEXT glTextureParameterfEXT glTextureParameterfvEXT glTextureParameteriEXT glTextureParameterivEXT "\
    "glTextureImage1DEXT glTextureImage2DEXT glTextureSubImage1DEXT glTextureSubImage2DEXT glCopyTextureImage1DEXT glCopyTextureImage2DEXT "\
    "glCopyTextureSubImage1DEXT glCopyTextureSubImage2DEXT glGetTextureImageEXT glGetTextureParameterfvEXT glGetTextureParameterivEXT "\
    "glGetTextureLevelParameterfvEXT glGetTextureLevelParameterivEXT glTextureImage3DEXT glTextureSubImage3DEXT glCopyTextureSubImage3DEXT "\
    "glMultiTexParameterfEXT glMultiTexParameterfvEXT glMultiTexParameteriEXT glMultiTexParameterivEXT glMultiTexImage1DEXT glMultiTexImage2DEXT "\
    "glMultiTexSubImage1DEXT glMultiTexSubImage2DEXT glCopyMultiTexImage1DEXT glCopyMultiTexImage2DEXT glCopyMultiTexSubImage1DEXT glCopyMultiTexSubImage2DEXT "\
    "glGetMultiTexImageEXT glGetMultiTexParameterfvEXT glGetMultiTexParameterivEXT glGetMultiTexLevelParameterfvEXT glGetMultiTexLevelParameterivEXT "\
    "glMultiTexImage3DEXT glMultiTexSubImage3DEXT glCopyMultiTexSubImage3DEXT glBindMultiTextureEXT glEnableClientStateIndexedEXT glDisableClientStateIndexedEXT "\
    "glMultiTexCoordPointerEXT glMultiTexEnvfEXT glMultiTexEnvfvEXT glMultiTexEnviEXT glMultiTexEnvivEXT glMultiTexGendEXT glMultiTexGendvEXT "\
    "glMultiTexGenfEXT glMultiTexGenfvEXT glMultiTexGeniEXT glMultiTexGenivEXT glGetMultiTexEnvfvEXT glGetMultiTexEnvivEXT glGetMultiTexGendvEXT "\
    "glGetMultiTexGenfvEXT glGetMultiTexGenivEXT glGetFloatIndexedvEXT glGetDoubleIndexedvEXT glGetPointerIndexedvEXT "\
    "glCompressedTextureImage3DEXT glCompressedTextureImage2DEXT glCompressedTextureImage1DEXT glCompressedTextureSubImage3DEXT "\
    "glCompressedTextureSubImage2DEXT glCompressedTextureSubImage1DEXT glGetCompressedTextureImageEXT glCompressedMultiTexImage3DEXT "\
    "glCompressedMultiTexImage2DEXT glCompressedMultiTexImage1DEXT glCompressedMultiTexSubImage3DEXT glCompressedMultiTexSubImage2DEXT "\
    "glCompressedMultiTexSubImage1DEXT glGetCompressedMultiTexImageEXT glNamedProgramStringEXT "\
    "glNamedProgramLocalParameter4dEXT glNamedProgramLocalParameter4dvEXT glNamedProgramLocalParameter4fEXT glNamedProgramLocalParameter4fvEXT "\
    "glGetNamedProgramLocalParameterdvEXT glGetNamedProgramLocalParameterfvEXT glGetNamedProgramivEXT glGetNamedProgramStringEXT "\
    "glNamedProgramLocalParameters4fvEXT glNamedProgramLocalParameterI4iEXT glNamedProgramLocalParameterI4ivEXT glNamedProgramLocalParametersI4ivEXT "\
    "glNamedProgramLocalParameterI4uiEXT glNamedProgramLocalParameterI4uivEXT glNamedProgramLocalParametersI4uivEXT glGetNamedProgramLocalParameterIivEXT "\
    "glGetNamedProgramLocalParameterIuivEXT glTextureParameterIivEXT glTextureParameterIuivEXT glGetTextureParameterIivEXT glGetTextureParameterIuivEXT "\
    "glMultiTexParameterIivEXT glMultiTexParameterIuivEXT glGetMultiTexParameterIivEXT glGetMultiTexParameterIuivEXT "\
    "glProgramUniform1fEXT glProgramUniform2fEXT glProgramUniform3fEXT glProgramUniform4fEXT glProgramUniform1iEXT glProgramUniform2iEXT glProgramUniform3iEXT glProgramUniform4iEXT "\
    "glProgramUniform1fvEXT glProgramUniform2fvEXT glProgramUniform3fvEXT glProgramUniform4fvEXT glProgramUniform1ivEXT glProgramUniform2ivEXT glProgramUniform3ivEXT "\
    "glProgramUniform4ivEXT glProgramUniformMatrix2fvEXT glProgramUniformMatrix3fvEXT glProgramUniformMatrix4fvEXT glProgramUniformMatrix2x3fvEXT "\
    "glProgramUniformMatrix3x2fvEXT glProgramUniformMatrix2x4fvEXT glProgramUniformMatrix4x2fvEXT glProgramUniformMatrix3x4fvEXT glProgramUniformMatrix4x3fvEXT "\
    "glProgramUniform1uiEXT glProgramUniform2uiEXT glProgramUniform3uiEXT glProgramUniform4uiEXT glProgramUniform1uivEXT glProgramUniform2uivEXT glProgramUniform3uivEXT glProgramUniform4uivEXT "\
    "glNamedBufferDataEXT glNamedBufferSubDataEXT glMapNamedBufferEXT glUnmapNamedBufferEXT glMapNamedBufferRangeEXT glFlushMappedNamedBufferRangeEXT glNamedCopyBufferSubDataEXT "\
    "glGetNamedBufferParameterivEXT glGetNamedBufferPointervEXT glGetNamedBufferSubDataEXT "\
    "glTextureBufferEXT glMultiTexBufferEXT glNamedRenderbufferStorageEXT glGetNamedRenderbufferParameterivEXT glCheckNamedFramebufferStatusEXT "\
    "glNamedFramebufferTexture1DEXT glNamedFramebufferTexture2DEXT glNamedFramebufferTexture3DEXT glNamedFramebufferRenderbufferEXT "\
    "glGetNamedFramebufferAttachmentParameterivEXT glGenerateTextureMipmapEXT glGenerateMultiTexMipmapEXT glFramebufferDrawBufferEXT glFramebufferDrawBuffersEXT "\
    "glFramebufferReadBufferEXT glGetFramebufferParameterivEXT glNamedRenderbufferStorageMultisampleEXT glNamedRenderbufferStorageMultisampleCoverageEXT "\
    "glNamedFramebufferTextureEXT glNamedFramebufferTextureLayerEXT glNamedFramebufferTextureFaceEXT glTextureRenderbufferEXT glMultiTexRenderbufferEXT "\
    "glGetMultisamplefvNV glSampleMaskIndexedNV glTexRenderbufferNV glBindTransformFeedbackNV glDeleteTransformFeedbacksNV glGenTransformFeedbacksNV "\
    "glIsTransformFeedbackNV glPauseTransformFeedbackNV glResumeTransformFeedbackNV glDrawTransformFeedbackNV "\
    "glEnableVertexAttribAPPLE glDisableVertexAttribAPPLE glIsVertexAttribEnabledAPPLE glMapVertexAttrib1dAPPLE glMapVertexAttrib1fAPPLE glMapVertexAttrib2dAPPLE glMapVertexAttrib2fAPPLE "\
    "glObjectPurgeableAPPLE glObjectUnpurgeableAPPLE glGetObjectParameterivAPPLE glTextureRangeAPPLE glGetTexParameterPointervAPPLE glCopyBufferSubData "\
    /* wgl */\
    "wglChoosePixelFormat wglCopyContext wglCreateContext wglCreateLayerContext wglDeleteContext wglDescribeLayerPlane wglDescribePixelFormat wglGetCurrentContext wglGetCurrentDC wglGetDefaultProcAddress "\
    "wglGetLayerPaletteEntries wglGetPixelFormat wglGetProcAddress wglMakeCurrent wglRealizeLayerPalette wglSetLayerPaletteEntries wglSetPixelFormat wglShareLists wglSwapBuffers wglSwapLayerBuffers "\
    "wglSwapMultipleBuffers wglUseFontBitmapsA wglUseFontBitmapsW wglUseFontOutlinesA wglUseFontOutlinesW "\
    \
    /* wgl Extensions*/\
    "wglCreateBufferRegionARB wglCreateContextAttribsARB wglDeleteBufferRegionARB wglSaveBufferRegionARB wglRestoreBufferRegionARB wglGetExtensionsStringARB "\
    "wglGetPixelFormatAttribivARB wglGetPixelFormatAttribfvARB wglChoosePixelFormatARB wglMakeContextCurrentARB wglGetCurrentReadDCARB "\
    "wglCreatePbufferARB wglGetPbufferDCARB wglReleasePbufferDCARB wglDestroyPbufferARB wglQueryPbufferARB "\
    "wglBindTexImageARB wglReleaseTexImageARB wglSetPbufferAttribARB wglCreateDisplayColorTableEXT wglLoadDisplayColorTableEXT "\
    "wglBindDisplayColorTableEXT wglDestroyDisplayColorTableEXT wglGetExtensionsStringEXT wglMakeContextCurrentEXT wglGetCurrentReadDCEXT "\
    "wglCreatePbufferEXT wglGetPbufferDCEXT wglReleasePbufferDCEXT wglDestroyPbufferEXT wglQueryPbufferEXT "\
    "wglGetPixelFormatAttribivEXT wglGetPixelFormatAttribfvEXT wglChoosePixelFormatEXT wglSwapIntervalEXT wglGetSwapIntervalEXT "\
    "wglAllocateMemoryNV wglFreeMemoryNV wglGetSyncValuesOML wglGetMscRateOML wglSwapBuffersMscOML "\
    "wglSwapLayerBuffersMscOML wglWaitForMscOML wglWaitForSbcOML wglGetDigitalVideoParametersI3D wglSetDigitalVideoParametersI3D "\
    "wglGetGammaTableParametersI3D wglSetGammaTableParametersI3D wglGetGammaTableI3D wglSetGammaTableI3D wglEnableGenlockI3D "\
    "wglDisableGenlockI3D wglIsEnabledGenlockI3D wglGenlockSourceI3D wglGetGenlockSourceI3D wglGenlockSourceEdgeI3D "\
    "wglGetGenlockSourceEdgeI3D wglGenlockSampleRateI3D wglGetGenlockSampleRateI3D wglGenlockSourceDelayI3D wglGetGenlockSourceDelayI3D "\
    "wglQueryGenlockMaxSourceDelayI3D wglCreateImageBufferI3D wglDestroyImageBufferI3D wglAssociateImageBufferEventsI3D wglReleaseImageBufferEventsI3D "\
    "wglEnableFrameLockI3D wglDisableFrameLockI3D wglIsEnabledFrameLockI3D wglQueryFrameLockMasterI3D wglGetFrameUsageI3D "\
    "wglBeginFrameTrackingI3D wglEndFrameTrackingI3D wglQueryFrameTrackingI3D wglEnumerateVideoDevicesNV wglBindVideoDeviceNV wglQueryCurrentContextNV "\
    "wglGetVideoDeviceNV wglReleaseVideoDeviceNV wglBindVideoImageNV wglReleaseVideoImageNV wglSendPbufferToVideoNV wglGetVideoInfoNV wglJoinSwapGroupNV "\
    "wglBindSwapBarrierNV wglQuerySwapGroupNV wglQueryMaxSwapGroupsNV wglQueryFrameCountNV wglResetFrameCountNV wglEnumGpusNV wglEnumGpuDevicesNV "\
    "wglCreateAffinityDCNV wglEnumGpusFromAffinityDCNV wglDeleteDCNV wglGetGPUIDsAMD wglGetGPUInfoAMD wglGetContextGPUIDAMD wglCreateAssociatedContextAMD "\
    "wglCreateAssociatedContextAttribsAMD wglDeleteAssociatedContextAMD wglMakeAssociatedContextCurrentAMD wglGetCurrentAssociatedContextAMD wglBlitContextFramebufferAMD "\
    \
    /* glX */\
    "glXChooseVisual glXCreateContext glXDestroyContext glXMakeCurrent glXCopyContext glXSwapBuffers glXCreateGLXPixmap glXDestroyGLXPixmap glXQueryExtension "\
    "glXQueryVersion glXIsDirect glXGetConfig glXGetCurrentContext glXGetCurrentDrawable glXWaitGL glXWaitX glXUseXFont glXQueryExtensionsString glXQueryServerString "\
    "glXGetClientString glXGetCurrentDisplay glXChooseFBConfig glXGetFBConfigAttrib glXGetFBConfigs glXGetVisualFromFBConfig glXCreateWindow glXDestroyWindow glXCreatePixmap "\
    "glXDestroyPixmap glXCreatePbuffer glXDestroyPbuffer glXQueryDrawable glXCreateNewContext glXMakeContextCurrent glXGetCurrentReadDrawable glXQueryContext glXSelectEvent glXGetSelectedEvent glXGetProcAddress "\
    \
    /* glX extensions */\
    "glXGetVideoSyncSGI glXWaitVideoSyncSGI glXSwapIntervalSGI glXMakeCurrentReadSGI glXGetCurrentReadDrawableSGI glXCreateGLXVideoSourceSGIX glXDestroyGLXVideoSourceSGIX GLXVideoSourceSGIX glXFreeContextEXT glXGetContextIDEXT "\
    "glXGetCurrentDrawableEXT glXGetCurrentDisplayEXT glXImportContextEXT glXGetFBConfigAttribSGIX glXChooseFBConfigSGIX glXCreateGLXPixmapWithConfigSGIX glXCreateContextWithConfigSGIX glXGetVisualFromFBConfigSGIX glXGetFBConfigFromVisualSGIX glXCreateGLXPbufferSGIX "\
    "glXDestroyGLXPbufferSGIX glXQueryGLXPbufferSGIX glXSelectEventSGIX glXGetSelectedEventSGIX glXCushionSGI glXBindChannelToWindowSGIX glXChannelRectSGIX glXQueryChannelRectSGIX "\
    "glXQueryChannelDeltasSGIX glXChannelRectSyncSGIX glXAssociateDMPbufferSGIX glXJoinSwapGroupSGIX glXBindSwapBarrierSGIX glXQueryMaxSwapBarriersSGIX glXGetTransparentIndexSUN "\
    "glXQueryContextInfoEXT glXCopySubBufferMESA glXCreateGLXPixmapMESA glXReleaseBuffersMESA glXSet3DfxModeMESA glXGetProcAddressARB glXCreateContextAttribsARB glXAllocateMemoryNV glXFreeMemoryNV "\
    "glXGetAGPOffsetMESA glXAllocateMemoryMESA glXFreeMemoryMESA glXGetMemoryOffsetMESA glXBindTexImageARB glXReleaseTexImageARB glXDrawableAttribARB "\
    "glXGetSyncValuesOML glXGetMscRateOML glXSwapBuffersMscOML glXWaitForMscOML glXWaitForSbcOML glXBeginFrameTrackingMESA glXEndFrameTrackingMESA glXGetFrameUsageMESA glXQueryFrameTrackingMESA "\
    "GLXHyperpipeNetworkSGIX glXQueryHyperpipeNetworkSGIX glXHyperpipeConfigSGIX glXQueryHyperpipeConfigSGIX glXDestroyHyperpipeConfigSGIX glXBindHyperpipeSGIX "\
    "glXQueryHyperpipeBestAttribSGIX glXHyperpipeAttribSGIX glXQueryHyperpipeAttribSGIX GLXHyperpipeNetworkSGIX GLXHyperpipeConfigSGIX glXBindTexImageEXT glXReleaseTexImageEXT "\
    \
    /* EGL */\
    "eglGetError eglGetDisplay eglInitialize eglTerminate eglQueryString eglGetProcAddress eglGetConfigs eglChooseConfig eglGetConfigAttrib eglCreateWindowSurface eglCreatePixmapSurface "\
    "eglCreatePbufferSurface eglDestroySurface eglQuerySurface eglSurfaceAttrib eglBindTexImage eglReleaseTexImage eglSwapInterval eglCreateContext eglDestroyContext eglMakeCurrent "\
    "eglGetCurrentContext eglGetCurrentSurface eglGetCurrentDisplay eglQueryContext eglWaitGL eglWaitNative eglSwapBuffers eglCopyBuffers eglMakeWindowNV eglDestroyWindowNV "\
    \
    /* OpenGL ES Extensions*/\
    "glBlendEquationOES glIsRenderbufferOES glBindRenderbufferOES glDeleteRenderbuffersOES glGenRenderbuffersOES glRenderbufferStorageOES "\
    "glGetRenderbufferParameterivOES glIsFramebufferOES glBindFramebufferOES glDeleteFramebuffersOES glGenFramebuffersOES glCheckFramebufferStatusOES "\
    "glFramebufferRenderbufferOES glFramebufferTexture2DOES glGetFramebufferAttachmentParameterivOES glGenerateMipmapOES glGetBufferPointervOES "\
    "glMapBufferOES glUnmapBufferOES glDrawTexsOES glDrawTexiOES glDrawTexxOES glDrawTexsvOES glDrawTexivOES glDrawTexxvOES glDrawTexfOES glDrawTexfvOES "\
    "glEGLImageTargetTexture2DOES glEGLImageTargetRenderbufferStorageOES glAlphaFuncxOES glClearColorxOES glClearDepthxOES glClipPlanexOES glColor4xOES "\
    "glDepthRangexOES glFogxOES glFogxvOES glFrustumxOES glGetClipPlanexOES glGetFixedvOES glGetLightxvOES glGetMaterialxvOES glGetTexEnvxvOES glGetTexParameterxvOES "\
    "glLightModelxOES glLightModelxvOES glLightxOES glLightxvOES glLineWidthxOES glLoadMatrixxOES glMaterialxOES glMaterialxvOES glMultMatrixxOES glMultiTexCoord4xOES "\
    "glNormal3xOES glOrthoxOES glPointParameterxOES glPointParameterxvOES glPointSizePointerOES glPointSizexOES glPolygonOffsetxOES glRotatexOES glSampleCoveragexOES glScalexOES glTexEnvxOES "\
    "glTexEnvxvOES glTexParameterxOES glTexParameterxvOES glTranslatexOES glCurrentPaletteMatrixOES glLoadPaletteFromModelViewMatrixOES "\
    "glMatrixIndexPointerOES glWeightPointerOES glQueryMatrixxOES glDepthRangefOES glFrustumfOES glOrthofOES glClipPlanefOES glGetClipPlanefOES glClearDepthfOES "\
    "glTexGenfOES glTexGenfvOES glTexGeniOES glTexGenivOES glTexGenxOES glTexGenxvOES glGetTexGenfvOES glGetTexGenivOES glGetTexGenxvOES glDeleteFencesNV "\
    "glGenFencesNV glIsFenceNV glTestFenceNV glGetFenceivNV glFinishFenceNV glSetFenceNV glGetDriverControlsQCOM glGetDriverControlStringQCOM "\
    "glEnableDriverControlQCOM glDisableDriverControlQCOM "\
    \
    /* CGL Functions */\
    "CGLError CGLChoosePixelFormat CGLDestroyPixelFormat CGLDescribePixelFormat CGLReleasePixelFormat CGLPixelFormatObj CGLRetainPixelFormat CGLGetPixelFormatRetainCount "\
    "CGLReleasePixelFormat CGLRetainPixelFormat CGLGetPixelFormatRetainCount CGLQueryRendererInfo CGLDestroyRendererInfo CGLDescribeRenderer CGLCreateContext CGLDestroyContext CGLCopyContext "\
    "CGLRetainContext CGLReleaseContext CGLGetContextRetainCount CGLGetPixelFormat CGLContextObj CGLRetainContext CGLReleaseContext CGLGetContextRetainCount "\
    "CGLGetPixelFormat CGLCreatePBuffer CGLDestroyPBuffer CGLDescribePBuffer CGLTexImagePBuffer CGLRetainPBuffer CGLReleasePBuffer CGLGetPBufferRetainCount "\
    "CGLRetainPBuffer CGLReleasePBuffer CGLGetPBufferRetainCount CGLSetOffScreen CGLGetOffScreen CGLSetFullScreen CGLSetPBuffer CGLGetPBuffer "\
    "CGLClearDrawable CGLFlushDrawable CGLEnable CGLDisable CGLIsEnabled CGLSetParameter CGLGetParameter CGLSetVirtualScreen CGLGetVirtualScreen CGLSetOption CGLGetOption "\
    "CGLLockContext CGLUnlockContext CGLGetVersion CGLErrorString CGLGlobalOption CGLPBufferObj CGLGetCurrentContext CGLSetCurrentContext "

#define AC_OPENCL_FUNCTIONS_LIST_FOR_SCINTILLA\
    "clGetPlatformIDs clGetPlatformInfo clGetDeviceIDs clGetDeviceInfo clCreateContext clCreateContextFromType clRetainContext clReleaseContext clGetContextInfo "\
    "clCreateCommandQueue clRetainCommandQueue clReleaseCommandQueue clGetCommandQueueInfo clSetCommandQueueProperty clCreateBuffer clCreateSubBuffer clCreateImage2D clCreateImage3D "\
    "clRetainMemObject clReleaseMemObject clGetSupportedImageFormats clGetMemObjectInfo clGetImageInfo clSetMemObjectDestructorCallback clCreateSampler clRetainSampler clReleaseSampler clGetSamplerInfo "\
    "clCreateProgramWithSource clCreateProgramWithBinary clRetainProgram clReleaseProgram clBuildProgram clUnloadCompiler clGetProgramInfo clGetProgramBuildInfo "\
    "clCreateKernel clCreateKernelsInProgram clRetainKernel clReleaseKernel clSetKernelArg clGetKernelInfo clGetKernelWorkGroupInfo clWaitForEvents clGetEventInfo "\
    "clCreateUserEvent clRetainEvent clReleaseEvent clSetUserEventStatus clSetEventCallback clGetEventProfilingInfo clFlush clFinish "\
    "clEnqueueReadBuffer clEnqueueReadBufferRect clEnqueueWriteBuffer clEnqueueWriteBufferRect clEnqueueCopyBuffer clEnqueueCopyBufferRect clEnqueueReadImage clEnqueueWriteImage "\
    "clEnqueueCopyImage clEnqueueCopyImageToBuffer clEnqueueCopyBufferToImage clEnqueueMapBuffer clEnqueueMapImage clEnqueueUnmapMemObject clEnqueueNDRangeKernel "\
    "clEnqueueTask clEnqueueNativeKernel clEnqueueMarker clEnqueueWaitForEvents clEnqueueBarrier clGetExtensionFunctionAddress "\
    "clSVMAlloc clSVMFree clSetKernelArgSVMPointer clSetKernelExecInfo clEnqueueSVMFree clEnqueueSVMMemcpy clEnqueueSVMMemFill clEnqueueSVMMap clEnqueueSVMUnmap "\
    "clCreateFromGLBuffer clCreateFromGLTexture2D clCreateCommandQueueWithProperties clCreatePipe clGetPipeInfo clCreateSamplerWithProperties" \
    "clCreateFromGLTexture3D clCreateFromGLRenderbuffer clGetGLObjectInfo clGetGLTextureInfo clEnqueueAcquireGLObjects clEnqueueReleaseGLObjects clGetGLContextInfoKHR clCreateEventFromGLsyncKHR "\
    "clBeginComputationFrameAMD clEndComputationFrameAMD clNameContextAMD clNameCommandQueueAMD clNameMemObjectAMD clNameSamplerAMD clNameProgramAMD "\
    "clNameKernelAMD clNameEventAMD "\
    \
    /* OpenCL data structures: */\
    "cl_platform_id cl_device_id cl_context cl_command_queue cl_mem cl_program cl_kernel cl_event cl_sampler cl_uint cl_bool cl_ulong cl_bitfield cl_device_type "\
    "cl_platform_info cl_device_info cl_device_address_info cl_device_fp_config cl_device_mem_cache_type cl_device_local_mem_type cl_device_exec_capabilities cl_command_queue_properties "\
    "cl_context_properties cl_context_info cl_command_queue_info cl_channel_order cl_channel_type cl_mem_flags cl_mem_object_type cl_mem_info cl_image_info cl_addressing_mode cl_filter_mode "\
    "cl_sampler_info cl_map_flags cl_program_info cl_program_build_info cl_build_status cl_kernel_info cl_kernel_work_group_info cl_event_info cl_command_type cl_profiling_info "\
    "_cl_image_format cl_channel_order image_channel_order cl_channel_type image_channel_data_type cl_image_format "\

    // Basic OpenGL:
    // Unicode - Break this string into 2 strings, since in unicode compile, the compiler cannot use it:
    const char* OpenGLWordlist1 = AC_OPENGL_FUNCTIONS_LIST_FOR_SCINTILLA1;
    const char* OpenGLWordlist2 = AC_OPENGL_FUNCTIONS_LIST_FOR_SCINTILLA2;

    // Basic OpenCL:
    const char* OpenCLWordlist1 = AC_OPENCL_FUNCTIONS_LIST_FOR_SCINTILLA;

    // CS
    const char* CsWordlist1 =
        "abstract as base bool break byte case catch char checked class "
        "const continue decimal default delegate do double else enum event "
        "explicit extern false finally fixed float for foreach goto if "
        "implicit in int interface internal is lock long namespace new "
        "null object operator out override params private protected public "
        "readonly ref return sbyte sealed short sizeof stackalloc static "
        "string struct switch this throw true try typeof uint ulong "
        "unchecked unsafe ushort using virtual void while";

    // CSS
    const char* CssWordlist1 =
        "left right top bottom position font-family font-style font-variant "
        "font-weight font-size font color background-color background-image "
        "background-repeat background-attachment background-position background "
        "word-spacing letter-spacing text-decoration vertical-align text-transform "
        "text-align text-indent line-height margin-top margin-right margin-bottom "
        "margin-left margin padding-top padding-right padding-bottom padding-left "
        "padding border-top-width border-right-width border-bottom-width "
        "border-left-width border-width border-top border-right border-bottom "
        "border-left border border-color border-style width height float clear "
        "display white-space list-style-type list-style-image list-style-position "
        "BACKGROUND-COLOR BACKGROUND-IMAGE list-style";
    const char* CssWordlist2 =
        "first-letter first-line active link visited";
    // Fortran
    const char* FortranWordlist1 =
        "allocatable allocate assignment backspace block blockdata call "
        "case character close common complex contains continue cycle data "
        "deallocate default dimension direct do double doubleprecision "
        "elemental else elseif elsewhere end endblock endblockdata enddo "
        "endfile endforall endfunction endif endinterface endmodule "
        "endprogram endselect endsubroutine endtype endwhere entry "
        "equivalence err exist exit external forall format formatted "
        "function go goto if implicit in inout include inquire integer "
        "intent interface intrinsic iolength iostat kind len logical "
        "module namelist none null nullify only open operator optional "
        "parameter pointer position precision print private procedure "
        "program public pure out read readwrite real rec recursive result "
        "return rewind save select selectcase sequence sequential stat "
        "status stop subroutine target then to type unformatted unit use "
        "where while write";
    const char* FortranWordlist2 =
        "abs achar acos acosd adjustl adjustr aimag aimax0 aimin0 aint "
        "ajmax0 ajmin0 akmax0 akmin0 all allocated alog alog10 amax0 amax1 "
        "amin0 amin1 amod anint any asin asind associated atan atan2 "
        "atan2d atand bitest bitl bitlr bitrl bjtest bit_size bktest break "
        "btest cabs ccos cdabs cdcos cdexp cdlog cdsin cdsqrt ceiling cexp "
        "char clog cmplx conjg cos cosd cosh count cpu_time cshift csin "
        "csqrt dabs dacos dacosd dasin dasind datan datan2 datan2d datand "
        "date date_and_time dble dcmplx dconjg dcos dcosd dcosh dcotan "
        "ddim dexp dfloat dflotk dfloti dflotj digits dim dimag dint dlog "
        "dlog10 dmax1 dmin1 dmod dnint dot_product dprod dreal dsign dsin "
        "dsind dsinh dsqrt dtan dtand dtanh eoshift epsilon errsns exp "
        "exponent float floati floatj floatk floor fraction free huge iabs "
        "iachar iand ibclr ibits ibset ichar idate idim idint idnint ieor "
        "ifix iiabs iiand iibclr iibits iibset iidim iidint iidnnt iieor "
        "iifix iint iior iiqint iiqnnt iishft iishftc iisign ilen imax0 "
        "imax1 imin0 imin1 imod index inint inot int int1 int2 int4 int8 "
        "iqint iqnint ior ishft ishftc isign isnan izext jiand jibclr "
        "jibits jibset jidim jidint jidnnt jieor jifix jint jior jiqint "
        "jiqnnt jishft jishftc jisign jmax0 jmax1 jmin0 jmin1 jmod jnint "
        "jnot jzext kiabs kiand kibclr kibits kibset kidim kidint kidnnt "
        "kieor kifix kind kint kior kishft kishftc kisign kmax0 kmax1 "
        "kmin0 kmin1 kmod knint knot kzext lbound leadz len len_trim "
        "lenlge lge lgt lle llt log log10 logical lshift malloc matmul "
        "max max0 max1 maxexponent maxloc maxval merge min min0 min1 "
        "minexponent minloc minval mod modulo mvbits nearest nint not "
        "nworkers number_of_processors pack popcnt poppar precision "
        "present product radix random random_number random_seed range real "
        "repeat reshape rrspacing rshift scale scan secnds "
        "selected_int_kind selected_real_kind set_exponent shape sign sin "
        "sind sinh size sizeof sngl snglq spacing spread sqrt sum "
        "system_clock tan tand tanh tiny transfer transpose trim ubound "
        "unpack verify";

    // HTML
    const char* HtmlWordlist1 =
        "a abbr acronym address applet area b base basefont bdo big "
        "blockquote body br button caption center cite code col colgroup "
        "dd del dfn dir div dl dt em fieldset font form frame frameset h1 "
        "h2 h3 h4 h5 h6 head hr html i iframe img input ins isindex kbd "
        "label legend li link map menu meta noframes noscript object ol "
        "optgroup option p param pre q s samp script select small span "
        "strike strong style sub sup table tbody td textarea tfoot th "
        "thead title tr tt u ul var xml xmlns";
    const char* HtmlWordlist2 =
        "abbr accept-charset accept accesskey action align alink alt "
        "archive axis background bgcolor border cellpadding cellspacing "
        "char charoff charset checked cite class classid clear codebase "
        "codetype color cols colspan compact content coords data datafld "
        "dataformatas datapagesize datasrc datetime declare defer dir "
        "disabled enctype event face for frame frameborder headers height "
        "href hreflang hspace http-equiv id ismap label lang language "
        "leftmargin link longdesc marginwidth marginheight maxlength media "
        "method multiple name nohref noresize noshade nowrap object onblur "
        "onchange onclick ondblclick onfocus onkeydown onkeypress onkeyup "
        "onload onmousedown onmousemove onmouseover onmouseout onmouseup "
        "onreset onselect onsubmit onunload profile prompt readonly rel "
        "rev rows rowspan rules scheme scope selected shape size span src "
        "standby start style summary tabindex target text title topmargin "
        "type usemap valign value valuetype version vlink vspace width "
        "text password checkbox radio submit reset file hidden image";
    // Java
    const char* JavaWordlist1 =
        "abstract assert boolean break byte case catch char class const "
        "continue default do double else extends final finally float for "
        "future generic goto if implements import inner instanceof int "
        "interface long native new null outer package private protected "
        "public rest return short static super switch synchronized this "
        "throw throws transient try var void volatile while";

    // Javascript
    const char* JavascriptWordlist1 =
        "abstract boolean break byte case catch char class const continue "
        "debugger default delete do double else enum export extends final "
        "finally float for function goto if implements import in "
        "instanceof int interface long native new package private "
        "protected public return short static super switch synchronized "
        "this throw throws transient try typeof var void volatile while "
        "with";
    // Pascal
    const char* PascalWordlist1 =
        "program const type var begin end array set packed record string "
        "if then else while for to downto do with repeat until case of "
        "goto exit label procedure function nil file and or not xor div "
        "mod unit uses implementation interface external asm inline object "
        "constructor destructor virtual far assembler near inherited "
        "stdcall cdecl library export exports end. class ansistring raise "
        "try except on index name finally resourcestring false true "
        "initialization finalization override overload";

    // Perl
    const char* PerlWordlist1 =
        "NULL __FILE__ __LINE__ __PACKAGE__ __DATA__ __END__ AUTOLOAD "
        "BEGIN CORE DESTROY END EQ GE GT INIT LE LT NE CHECK abs accept "
        "alarm and atan2 bind binmode bless caller chdir chmod chomp chop "
        "chown chr chroot close closedir cmp connect continue cos crypt "
        "dbmclose dbmopen defined delete die do dump each else elsif "
        "endgrent endhostent endnetent endprotoent endpwent endservent eof "
        "eq eval exec exists exit exp fcntl fileno flock for foreach fork "
        "format formline ge getc getgrent getgrgid getgrnam gethostbyaddr "
        "gethostbyname gethostent getlogin getnetbyaddr getnetbyname "
        "getnetent getpeername getpgrp getppid getpriority getprotobyname "
        "getprotobynumber getprotoent getpwent getpwnam getpwuid "
        "getservbyname getservbyport getservent getsockname getsockopt "
        "glob gmtime goto grep gt hex if index int ioctl join keys kill "
        "last lc lcfirst le length link listen local localtime lock log "
        "lstat lt m map mkdir msgctl msgget msgrcv msgsnd my ne next no "
        "not oct open opendir or ord our pack package pipe pop pos print "
        "printf prototype push q qq qr quotemeta qu qw qx rand read "
        "readdir readline readlink readpipe recv redo ref rename require "
        "reset return reverse rewinddir rindex rmdir s scalar seek "
        "seekdir select semctl semget semop send setgrent sethostent "
        "setnetent setpgrp setpriority setprotoent setpwent setservent "
        "setsockopt shift shmctl shmget shmread shmwrite shutdown sin "
        "sleep socket socketpair sort splice split sprintf sqrt srand "
        "stat study sub substr symlink syscall sysopen sysread sysseek "
        "system syswrite tell telldir tie tied time times tr truncate uc "
        "ucfirst umask undef unless unlink unpack unshift untie until use "
        "utime values vec wait waitpid wantarray warn while write x xor "
        "y";

    // PHP
    const char* PhpWordlist1 =
        "a abbr acronym address applet area b base basefont bdo big "
        "blockquote body br button caption center cite code col colgroup "
        "dd del dfn dir div dl dt em fieldset font form frame frameset h1 "
        "h2 h3 h4 h5 h6 head hr html i iframe img input ins isindex kbd "
        "label legend li link map menu meta noframes noscript object ol "
        "optgroup option p param pre q s samp script select small span "
        "strike strong style sub sup table tbody td textarea tfoot th "
        "thead title tr tt u ul var xml xmlns";
    const char* PhpWordlist2 =
        "abbr accept-charset accept accesskey action align alink alt "
        "archive axis background bgcolor border cellpadding cellspacing "
        "char charoff charset checked cite class classid clear codebase "
        "codetype color cols colspan compact content coords data datafld "
        "dataformatas datapagesize datasrc datetime declare defer dir "
        "disabled enctype event face for frame frameborder headers height "
        "href hreflang hspace http-equiv id ismap label lang language "
        "leftmargin link longdesc marginwidth marginheight maxlength media "
        "method multiple name nohref noresize noshade nowrap object onblur "
        "onchange onclick ondblclick onfocus onkeydown onkeypress onkeyup "
        "onload onmousedown onmousemove onmouseover onmouseout onmouseup "
        "onreset onselect onsubmit onunload profile prompt readonly rel "
        "rev rows rowspan rules scheme scope selected shape size span src "
        "standby start style summary tabindex target text title topmargin "
        "type usemap valign value valuetype version vlink vspace width "
        "text password checkbox radio submit reset file hidden image";
    // Python
    const char* PythonWordlist1 =
        "and assert break class continue def del elif else except exec "
        "finally for from global if import in is lambda None not or pass "
        "print raise return try while yield";
    const char* PythonWordlist2 =
        "ACCELERATORS ALT AUTO3STATE AUTOCHECKBOX AUTORADIOBUTTON BEGIN "
        "BITMAP BLOCK BUTTON CAPTION CHARACTERISTICS CHECKBOX CLASS "
        "COMBOBOX CONTROL CTEXT CURSOR DEFPUSHBUTTON DIALOG DIALOGEX "
        "DISCARDABLE EDITTEXT END EXSTYLE FONT GROUPBOX ICON LANGUAGE "
        "LISTBOX LTEXT MENU MENUEX MENUITEM MESSAGETABLE POPUP PUSHBUTTON "
        "RADIOBUTTON RCDATA RTEXT SCROLLBAR SEPARATOR SHIFT STATE3 "
        "STRINGTABLE STYLE TEXTINCLUDE VALUE VERSION VERSIONINFO VIRTKEY";
    // TCL
    const char* TclWordlist1 =
        "after append array auto_execok auto_import auto_load "
        "auto_load_index auto_qualify beep binary break case catch cd "
        "clock close concat continue dde default echo else elseif encoding "
        "eof error eval exec exit expr fblocked fconfigure fcopy file "
        "fileevent flush for foreach format gets glob global history if "
        "incr info interp join lappend lindex linsert list llength load "
        "lrange lreplace lsearch lsort namespace open package pid "
        "pkg_mkIndex proc puts pwd read regexp regsub rename resource "
        "return scan seek set socket source split string subst switch "
        "tclLog tclMacPkgSearch tclPkgSetup tclPkgUnknown tell time trace "
        "unknown unset update uplevel upvar variable vwait while bell bind "
        "bindtags button canvas checkbutton console destroy entry event "
        "focus font frame grab grid image label listbox menu menubutton "
        "message pack place radiobutton raise scale scrollbar text tk "
        "tkwait toplevel winfo wm tkButtonDown tkButtonEnter "
        "tkButtonInvoke tkButtonLeave tkButtonUp tkCancelRepeat "
        "tkCheckRadioInvoke tkDarken tkEntryAutoScan tkEntryBackspace "
        "tkEntryButton1 tkEntryClosestGap tkEntryInsert tkEntryKeySelect "
        "tkEntryMouseSelect tkEntryNextWord tkEntryPaste "
        "tkEntryPreviousWord tkEntrySeeInsert tkEntrySetCursor "
        "tkEntryTranspose tkEventMotifBindings tkFDGetFileTypes "
        "tkFirstMenu tkFocusGroup_Destroy tkFocusGroup_In tkFocusGroup_Out "
        "tkFocusOK tkListboxAutoScan tkListboxBeginExtend "
        "tkListboxBeginSelect tkListboxBeginToggle tkListboxCancel "
        "tkListboxDataExtend tkListboxExtendUpDown tkListboxMotion "
        "tkListboxSelectAll tkListboxUpDown tkMbButtonUp tkMbEnter "
        "tkMbLeave tkMbMotion tkMbPost tkMenuButtonDown tkMenuDownArrow "
        "tkMenuDup tkMenuEscape tkMenuFind tkMenuFindName tkMenuFirstEntry "
        "tkMenuInvoke tkMenuLeave tkMenuLeftArrow tkMenuMotion "
        "tkMenuNextEntry tkMenuNextMenu tkMenuRightArrow tkMenuUnpost "
        "tkMenuUpArrow tkMessageBox tkPostOverPoint tkRecolorTree "
        "tkRestoreOldGrab tkSaveGrabInfo tkScaleActivate "
        "tkScaleButton2Down tkScaleButtonDown tkScaleControlPress "
        "tkScaleDrag tkScaleEndDrag tkScaleIncrement tkScreenChanged "
        "tkScrollButton2Down tkScrollButtonDown tkScrollButtonUp "
        "tkScrollByPages tkScrollByUnits tkScrollDrag tkScrollEndDrag "
        "tkScrollSelect tkScrollStartDrag tkScrollToPos tkScrollTopBottom "
        "tkTabToWindow tkTearOffMenu tkTextAutoScan tkTextButton1 "
        "tkTextClosestGap tkTextInsert tkTextKeyExtend tkTextKeySelect "
        "tkTextNextPara tkTextNextPos tkTextNextWord tkTextPaste "
        "tkTextPrevPara tkTextPrevPos tkTextResetAnchor tkTextScrollPages "
        "tkTextSelectTo tkTextSetCursor tkTextTranspose tkTextUpDownLine "
        "tkTraverseToMenu tkTraverseWithinMenu tk_bisque "
        "tk_chooseColor tk_dialog tk_focusFollowsMouse tk_focusNext "
        "tk_focusPrev tk_getOpenFile tk_getSaveFile tk_messageBox "
        "tk_optionMenu tk_popup tk_setPalette tk_textCopy tk_textCut "
        "tk_textPaste";
    // Visual Basic
    const char* VbWordlist1 =
        "addhandler addressof andalso alias and ansi as assembly auto "
        "boolean byref byte byval call case catch cbool cbyte cchar cdate "
        "cdec cdbl char cint class clng cobj const cshort csng cstr ctype "
        "date decimal declare default delegate dim do double each else "
        "elseif end enum erase error event exit false finally for friend "
        "function get gettype goto  handles if implements imports in "
        "inherits integer interface is let lib like long loop me mod "
        "module mustinherit mustoverride mybase myclass namespace new next "
        "not nothing notinheritable notoverridable object on option "
        "optional or orelse overloads overridable overrides paramarray "
        "preserve private property protected public raiseevent readonly "
        "redim rem removehandler resume return select set shadows shared "
        "short single static step stop string structure sub synclock then "
        "throw to true try typeof unicode until variant when while with "
        "withevents writeonly xor";

    // VB-Script
    const char* VbsWordlist1 =
        "and begin case call continue do each else elseif end erase error "
        "event exit false for function get gosub goto if implement in load "
        "loop lset me mid new next not nothing on or property raiseevent "
        "rem resume return rset select set stop sub then to true unload "
        "until wend while with withevents attribute alias as boolean byref "
        "byte byval const compare currency date declare dim double enum "
        "explicit friend global integer let lib long module object option "
        "optional preserve private property public redim single static "
        "string type variant";

    // Objective-C
    const char* ObjectiveCWordlist1 =
        "self super id Class SEL IMP BOOL Nil nil YES NO #import @interface "
        "@property @protocol @private @protected @public @try @catch @throw "
        "@finally @class @selector @encode @synchronized @synthesize "
        "@implementation @package @end @dynamic oneway in out inout bycopy "
        "byref readonly readwrite assign retain copy nonatomic NSObject "
        "NSUInteger NSCountedSet NSData NSDictionary NSUserDefaults NSPointerArray "
        "NSCalendar NSKeyValueCoding NSString NSOutputStream NSDateFormatter "
        "NSNumberFormatter NSMutableArray NSAtomicStore NSAttributedString "
        "NSCharacterSet NSDate NSDateComponents NSExpression NSFileWrapper "
        "NSIndexPath NSKeyedArchiver NSKeyedUnarchiver NSManagedObject "
        "NSManagedObjectContext NSMutableParagraphStyle NSPointerFunctions "
        "NSPredicateEditor NSScanner NSSet NSSound NSStream NSArray "
        "NSPropertyListSerialization NSDictionaryController NSDictionaryControllerKeyValuePair "
        "NSCalendarDate NSPersistentStoreCoordinator NSFetchRequest NSNumber "
        "NSPersistentDocument NSTimeZone NSObjectController NSAttributedString "
        "NSDecimalNumber NSKeyValueObserving NSSortDescriptor NSValue NSValueTransformer "
        "NSMapTable NSPersistentStore NSBundle NSFormatter NSHashTable NSEntityMapping "
        "NSEntityMigrationPolicy NSMappingModel NSMigrationManager NSPropertyMapping "
        "NSComparisonPredicate NSDecimalNumberHandler NSKeyValueBindingCreation "
        "NSTreeController NSArrayController NSInputStream NSController NSMutableData "
        "NSIndexSet NSMutableString NSXMLDocument NSXMLDTD NSXMLDTDNode NSXMLElement "
        "NSXMLNode NSEnumerator NSMutableDictionary NSOutlineViewDataSource NSXMLParser "
        "NSManagedObjectModel NSUndoManager NSPredicateEditorRowTemplate NSAtomicStoreCacheNode "
        "NSManagedObjectID NSMutableSet NSPredicate NSCoder NSFetchRequestExpression "
        "NSTreeNode NSCompoundPredicate NSMutableAttributedString NSArchiver NSClassDescription "
        "NSCoder NSCoding NSDecimalNumberBehaviors NSEditor NSEditorRegistration "
        "NSMutableAttributedString NSMutableCharacterSet NSMutableIndexSet NSNull "
        "NSObjCTypeSerializationCallBack NSPlaceholders NSSerializer NSUnarchiver NSUserDefaultsControlle "
        AC_CPP_RESERVED_WORDS_LIST_FOR_SCINTILLA;

    // Objective-C Functions
    const char* ObjectiveCWordlist2 =
        "initWithAPI: sharegroup: EAGLGetVersion setCurrentContext: "
        "renderbufferStorage: fromDrawable: presentRenderbuffer: alloc dealloc "
        "CAEAGLLayer EAGLContext EAGLSharegroup EAGLRenderingAPI EAGLDrawable "
        "kEAGLRenderingAPIOpenGLES1 "
        AC_OPENGL_FUNCTIONS_LIST_FOR_SCINTILLA1;

    // ---------------------------------------------------------------------------
    // ---------------------------------------------------------------------------

    const char* IlTypeKeywords =
        "__attribute__ __generic __kernel __kernel_exec __read_only __read_write __write_only aligned atomic_double atomic_flag atomic_float "
        "atomic_int atomic_intptr_t atomic_long atomic_ptrdiff_t atomic_size_t atomic_uint atomic_uintptr_t atomic_ulong bool char "
        "char16 char2 char3 char4 char8 cl_mem_fence_flags clk_event_t clk_profiling_info const device "
        "double double16 double2 double3 double4 double8 endian even event_t extern "
        "float float16 float2 float3 float4 float8 generic half hi host "
        "image1d_array_t image1d_buffer_t image1d_t image2d_array_depth_t image2d_array_t image2d_depth_t image2d_t image3d_t int int16 "
        "int2 int3 int4 int8 intptr_t kernel_enqueue_flags_t lo long long16 long2 "
        "long3 long4 long8 memory_order memory_scope ndrange_t nosvm odd opencl_unroll_hint packed "
        "pipe ptrdiff_t queue_t read_only read_write reqd_work_group_size reserve_id_t restrict sampler_t short "
        "short16 short2 short3 short4 short8 size_t static typedef uchar uchar16 "
        "uchar2 uchar3 uchar4 uchar8 uint uint16 uint2 uint3 uint4 uint8 "
        "uintptr_t ulong ulong16 ulong2 ulong3 ulong4 ulonx g8 uniform unsigned ushort "
        "ushort16 ushort2 ushort3 ushort4 ushort8 vec_type_hint void volatile work_group_size_hint write_only ";

    const char* IlTypeReservedKeywords =
        "inegate dcl_literal ";

    const char* IlBuiltInFunctions =
        "abs acos add and append_buf_alloc append_buf_consume asin atanballotbfi "
        "bfmbitalign break break_logicalnzbreak_logicalzbreakc bufinfo bufinfo_extbytealign "
        "call call_logicalnz call_logicalz callnz case clamp classclgcmovcmov_logical cmp "
        "colorclamp comment continue continue_logicalnz, continue_logicalzcontinuec cos cos_vec crs "
        "cu_id cubeid cubema cubesc cubetc cut cut_stream d_div d_div_fixup d_div_fmas "
        "d_div_scale d_eq  d_frac d_frexp d_ge d_ldexp d_lt d_max d_min d_mov d_movc d_mul "
        "d_muladd d_ne d_rcp d_rsq d_sqrt d2f dabs dadd dcl_arena_uav dcl_cb dcl_function_body "
        "dcl_function_table dcl_gds dcl_global_flagsdcl_gws_thread_coun dcl_indexed_temp_array "
        "dcl_input dcl_input_custom dcl_inputprimitive dcl_interface_ptr dcl_lds dcl_lds_sharing_mode "
        "dcl_lds_size_per_thread dcl_linear_center dcl_linear_centroid dcl_linear_sample dcl_literal "
        "dcl_max_output dcl_max_output_vertex_count dcl_max_tess dcl_max_tessfactor dcl_max_thread_per_group "
        "dcl_num_icp dcl_num_instances dcl_num_ocp dcl_num_thread_per_group dcl_odepth dcl_output dcl_output_topology "
        "dcl_persistent dcl_persistent_array dcl_persp_center dcl_persp_centroid dcl_persp_pull_model dcl_persp_sample "
        "dcl_raw_srv dcl_raw_uav dcl_resource dcl_semaphoredcl_shared_temp dcl_stream dcl_struct_gds dcl_struct_lds "
        "dcl_struct_srv dcl_struct_uav dcl_total_num_thread_groupdcl_ts_domain dcl_ts_output_primitive dcl_ts_partition "
        "dcl_typed_uavdcl_typeless_uavdcl_uav dcl_user_datadcl_vprimdcl_wavesizedclarraydclassdcldefdclpidclpin "
        "dclpp dclpt dclv dclvout dcos ddivdef default defb deqdet dexp dfma dfrac dfrexp dfrexp_exp dfrexp_mant "
        "dge discard_logicalnz discard_logicalz dist div div_fixup div_fmas div_precise div_scale dldexp "
        "dlog dlt dmad dmax dmin dmov dmovc dmul dne dp2 dp2add "
        "dp3 dp4 drcp drcp_native dround_nearest dround_neginf dround_plusinf dround_z drsq dsin dsqrt "
        "dsqrt_precise dst  dsub dsx  dsy  dtoi dtoi64 dtou dtou64 dtrig_preop dxsincos "
        "else emit emit_cut_sream emit_stream  emit_then_cut emit_then_cut_stream emitcut end endfunc endif endloop "
        "endmain endphase endswitch eq eval_centroid eval_sample_ind exeval_snapped exn exp exp_vec expp "
        "f_2_f16 f_2_u4 f16_2_f f16_reflect f162f f16abs f16add f16class f16clg f16cos "
        "f16dist f16div f16div_precise f16dp3 f16dp4 f16eq f16exn f16exp f16faceforward f16flr f16fma "
        "f16frc f16frexp_exp f16frexp_mant f16ge f16ldexp f16len f16ln f16log f16lt f16mad f16max f16min "
        "f16mod f16mod_precise f16mov f16mul f16ne f16nrm f16pow f16rcp f16round_nearest f16round_neginf "
        "f16round_plusinf f16round_z f16rsq f16sgn f16sin f16sqrt f16sub f16tan f16toi16 f16tou16 f16trc "
        "f2d f2d f2f16 f2f16_near f2f16_neg_inf f2f16_plus_inf f2u4 faceforward fcall fence fetch4 "
        "fetch4_b fetch4_b_ext fetch4_c_lz fetch4_c_lz_ext fetch4_ext fetch4_l fetch4_l_ext fetch4_po_b "
        "fetch4_po_b_ext fetch4_po_c fetch4_po_c_lz fetch4_po_c_lz_ext fetch4_po_l fetch4_po_l_ext fetch4c "
        "fetch4c fetch4c_ext fetch4po fetch4po_ext fetch4poc fetch4poc_ext ffb flat_atomic flat_load "
        "flat_store fldexp flr  fma  frc  frexp frexp_exp frexp_mant ftoi  ftoi_flr ftoi_rpi ftou func "
        "fwidth gds_add gds_add64 gds_and gds_and64gds_cmp_store gds_cmp_store64 gds_condxchg32_rtn_b64 "
        "gds_dec gds_dec64 gds_dec64 gds_fcmp_store gds_fcmp_store gds_fcmp_store64 gds_fcmp_store64 "
        "gds_fmax gds_fmax gds_fmax64 gds_fmax64 gds_fmin gds_fmin gds_fmin64 gds_fmin64 gds_inc "
        "gds_inc gds_inc64 gds_load gds_load_byte gds_load_short gds_load_ubyte gds_load_ushort "
        "gds_load64 gds_max  gds_max64 gds_min gds_min64 gds_mskor  gds_mskor64 gds_or  gds_or64 "
        "gds_read_add gds_read_add64 gds_read_and  gds_read_and64 gds_read_cmp_xchg gds_read_cmp_xch "
        "g64 gds_read_dec gds_read_dec64 gds_read_fcmp_xchg gds_read_fcmp_xchg64 gds_read_fmax "
        "gds_read_fmax64 gds_read_fmin gds_read_fmin64 gds_read_inc  gds_read_inc64 gds_read_max "
        "gds_read_max64 gds_read_min gds_read_min64 gds_read_mskor gds_read_mskor64 gds_read_or "
        "gds_read_or64 gds_read_rsub  gds_read_rsub64 gds_read_sub  gds_read_sub64 gds_read_umax  gds_read_umax64 "
        "gds_read_umin gds_read_umin64 gds_read_xchg gds_read_xchg64 gds_read_xor gds_read_xor64 gds_rsub gds_rsub64 "
        "gds_store gds_store_byte gds_store_short gds_store64 gds_sub gds_sub64 gds_umax gds_umax64 gds_umin "
        "gds_umin64 gds_wrap_rtn_b32 gds_xor gds_xor64 ge getlod getlod_ext hs_cp_phase hs_fork_phase hs_join_phase "
        "i16eq i16ge i16lt i16mad i16max i16min i16ne i16shl i16shr i16tof16 i64_add  i64add i64div i64eq "
        "i64ge i64lt i64mad i64max i64min i64mod i64mul i64ne i64negate i64shl i64shr i64sub i64sub i64tod "
        "iadd iand ibit_extract  iborrow icarry icarry icbits icountbits idiv ieq if_logicalnz if_logicalz "
        "ifc ifirstbit ifnz ige il_dcl_const_buffer il_dcl_gds il_dcl_global_flags il_dcl_indexed_temp_array "
        "il_dcl_input il_dcl_input_primitive il_dcl_lds il_op_fcall il_dcl_literal il_dcl_max_output_vertex_count "
        "il_dcl_max_tessfactor il_dcl_num_icp il_dcl_num_instances il_dcl_num_ocp il_dcl_odepth il_dcl_output "
        "il_dcl_output_topology il_dcl_resource il_dcl_stream il_dcl_struct_gds il_dcl_struct_lds il_dcl_ts_domain "
        "il_dcl_ts_output_primitive il_dcl_ts_partition il_dcl_vprim il_op_append_buf_alloc il_op_append_buf_consume "
        "il_op_bufinfo il_op_colorclamp il_op_cut il_op_cut_stream il_op_dcl_arena_uav il_op_dcl_function_body il_op_dcl_function_table "
        "il_op_dcl_interface_ptr il_op_dcl_lds_sharing_mode il_op_dcl_lds_size_per_thread il_op_dcl_num_thread_per_group il_op_dcl_raw_srv "
        "il_op_dcl_raw_uav il_op_dcl_shared_temp il_op_dcl_struct_srv "
        "il_op_dcl_struct_uav il_op_dcl_total_num_thread_group il_op_dcl_uav il_op_dcldef il_op_dclpi il_op_dclpin "
        "il_op_dclpp il_op_dclpt il_op_dclv il_op_dclvout il_op_def il_op_defb il_op_det il_op_discard_logicalnz "
        "il_op_discard_logicalz il_op_dist il_op_emit il_op_emit_stream il_op_emit_then_cut il_op_emit_then_cut_stream il_op_endphase "
        "il_op_eval_centroid il_op_eval_sample_index il_op_eval_snapped il_op_fence il_op_fetch4 il_op_fetch4_c "
        "il_op_fetch4_po il_op_fetch4_po_c il_op_gds_add il_op_gds_and il_op_gds_cmp_store il_op_gds_dec il_op_gds_inc "
        "il_op_gds_max il_op_gds_min il_op_gds_mskor il_op_gds_or il_op_gds_read_add il_op_gds_read_and il_op_gds_read_cmp_xchg "
        "il_op_gds_read_dec il_op_gds_read_inc il_op_gds_read_max il_op_gds_read_min il_op_gds_read_mskoril_op_gds_read_or "
        "il_op_gds_read_rsub il_op_gds_read_sub il_op_gds_read_umax il_op_gds_read_umin il_op_gds_read_xchg il_op_gds_read_xor "
        "il_op_gds_rsub il_op_gds_store il_op_gds_sub il_op_gds_umax il_op_gds_umin il_op_gds_xor il_op_getlod il_op_sampleinfo "
        "il_op_hs_cp_phase il_op_srv_raw_load il_op_hs_fork_phase il_op_srv_struct_loadil_op_hs_join_phase il_op_u_bit_insert "
        "il_op_init_sr il_op_uav_add il_op_init_sr_helper il_op_uav_and il_op_lds_add il_op_uav_arena_load il_op_lds_and il_op_uav_arena_store "
        "il_op_lds_cmp il_op_lds_dec il_op_lds_inc il_op_lds_load il_op_lds_load_byte il_op_lds_load_short "
        "il_op_lds_load_ubyte il_op_lds_load_ushort il_op_lds_load_vec il_op_lds_max il_op_lds_min il_op_lds_or il_op_lds_read_add "
        "il_op_lds_read_and il_op_lds_read_cmp_xchg il_op_lds_read_dec il_op_lds_read_inc il_op_lds_read_max  il_op_lds_read_min"
        "il_op_lds_read_or il_op_lds_read_rsub il_op_lds_read_sub il_op_lds_read_umax il_op_lds_read_umin il_op_lds_read_vec "
        "il_op_lds_read_xchg il_op_lds_read_xor il_op_lds_rsub il_op_lds_store il_op_lds_store_byte il_op_lds_store_short "
        "il_op_lds_store_vec il_op_lds_sub il_op_lds_umax il_op_lds_umin il_op_lds_write_vec il_op_lds_xor il_op_load "
        "il_op_memexport il_op_memimport il_op_noise il_op_resinfo il_op_sample il_op_sample_b il_op_sample_c il_op_sample_c_b "
        "il_op_sample_c_g il_op_sample_c_l il_op_sample_c_lz il_op_sample_g il_op_sample_l il_op_texld il_op_texldb il_op_texldd "
        "il_op_texldms il_op_texweight il_op_transpose il_op_uav_cmp il_op_uav_load il_op_uav_max il_op_uav_min il_op_uav_or "
        "il_op_uav_raw_load il_op_uav_raw_store il_op_uav_read_add il_op_uav_read_and il_op_uav_read_cmp_xchg il_op_uav_read_max "
        "il_op_uav_read_min il_op_uav_read_or il_op_uav_read_rsub il_op_uav_read_sub il_op_uav_read_umax il_op_uav_read_umi "
        "il_op_uav_read_xchg il_op_uav_read_xoril_op_uav_rsub il_op_uav_store il_op_uav_struct_load il_op_uav_struct_store "
        "il_op_uav_sub il_op_uav_udec il_op_uav_uinc il_op_uav_umax il_op_uav_umin il_op_uav_xor il_regtype_absolute_thread_id "
        "il_regtype_absolute_thread_id_flat il_regtype_addr il_regtype_clip il_regtype_const_bool il_regtype_const_buff il_regtype_const_float "
        "il_regtype_const_int il_regtype_depth_ge il_regtype_depth_le il_regtype_domainlocationil_regtype_edgeflag il_regtype_face il_regtype_fog "
        "il_regtype_generic_mem il_regtype_immed_const_buff il_regtype_input il_regtype_input_coverage_mask il_regtype_inputcp il_regtype_interp "
        "il_regtype_itemp il_regtype_literal il_regtype_ocp_id il_regtype_omask il_regtype_output il_regtype_outputcp il_regtype_patchconst "
        "il_regtype_pcolor il_regtype_pinput il_regtype_pos il_regtype_pricolor il_regtype_seccolor il_regtype_shader_instance_id "
        "il_regtype_shared_temp il_regtype_sprite il_regtype_spritecoord il_regtype_texcoord il_regtype_this il_regtype_thread_group_id "
        "il_regtype_thread_group_id_flat il_regtype_thread_id_in_group il_regtype_thread_id_in_group_flat il_regtype_timer il_regtype_vertex"
        "il_regtype_voutput il_regtype_wincoord "
        "ilt imad imad24 imax imax3 imed3 imin imin3 imod imul imul_high imul24 imul24_high ineinegate init_shared_registers init_sr_helper "
        "initv inot interp_mov_p0 interp_mov_p1 interp_mov_p2 inv_mov invariant_mov invariant_move "
        "ior ishl ishr itod itof itof ixor kill lane_id ldexp lds_add lds_add64 lds_and lds_and64 lds_cmp lds_cmp64 lds_condxchg32_rtn_b64 lds_dec "
        "lds_dec64 lds_fcmp lds_fcmp64 lds_fmax lds_fmax64 lds_fmin lds_fmin64 lds_inc lds_inc lds_inc64 lds_load lds_load_byte lds_load_short "
        "lds_load_ubyte lds_load_ushort lds_load_vec lds_load64lds_max lds_max64 lds_min lds_min64 lds_mskor lds_mskor64 lds_or lds_or64 "
        "lds_read_add lds_read_add64 lds_read_and lds_read_and64 lds_read_cmp_xchg lds_read_cmp_xchg64 lds_read_dec lds_read_dec64 "
        "lds_read_fcmp_xchg lds_read_fcmp_xchg64 lds_read_fmax lds_read_fmax64 lds_read_fmin lds_read_fmin64 lds_read_inc lds_read_inc64 "
        "lds_read_max lds_read_max64 lds_read_min lds_read_min64 lds_read_mskor lds_read_mskor64 lds_read_or lds_read_or64 lds_read_rsub "
        "lds_read_rsub64 lds_read_sub lds_read_sub64 lds_read_umax lds_read_umax64 lds_read_umin lds_read_umin64 lds_read_vec lds_read_xchg "
        "lds_read_xchg64 lds_read_xor lds_read_xor64 lds_rsub lds_rsub64 lds_store lds_store_byte lds_store_short lds_store_vec lds_store64 "
        "lds_sub lds_sub64 lds_umax lds_umax64 lds_umin lds_umin64 lds_wrap_rtn_b32 lds_write_vec lds_xor lds_xor64 len lit ln load "
        "load_dword_at_addr load_ext load_fptr load_fptr_ext lod log log_vec logp loop loop_rep lrp lt macrodef macroend mad max max3 "
        "mbcnt mcall mdef med3 memexport memimport mend min min3 mmul mod mov mova mova_round mqsad mqsad_u32_u8 msad mul ne noise nop "
        "nrm nrm_precise ordered_count pireduce pk_f16tof32 pk_f32u8tou32 pk_i16toi32 pk_snorm16tou32 pk_u16tou32 pk_unorm16tou32 "
        "poisons poisonv pow power prefix project qsad queue_id rcp rcp_vec read_other readfirstlane readlane receivelane reflect "
        "resinfo resinfo_ext ret ret_dyn ret_logicalnz ret_logicalz rnd round_nearest round_neg_inf round_neginf round_plus_inf "
        "round_plusinf round_zround_zero rsq rsq_precise rsq_vec sad sad_hi sad_u16 sad_u32 sad4 sad4 sample sample_b sample_b_ext "
        "sample_c sample_c_b sample_c_b_ext sample_c_ext sample_c_g sample_c_g_ext sample_c_l sample_c_l_ext sample_c_lz sample_c_lz_ext "
        "sample_ext sample_g sample_g_ext sample_l sample_l_ext sample_return_code sampleinfo sampleinfo_ext samplepos samplepos_ext "
        "scatter se_id semaphore_init semaphore_signal semaphore_wait set sgn sh_id simd_id sin sin_vec sincos sleep sqrt "
        "sqrt_precise sqrt_vec srv_raw_load srv_raw_load_ext srv_struct_load srv_struct_load_ext state_id stream_id sub "
        "switch tan texld texldb texldd texldms texweight tg_id transpose trc u16add u16ge u16lt u16mad u16max "
        "u16min u16mul u16shr u16sub u16tof16 u4lerp u64div u64ge u64lt u64mad u64max u64min u64mod u64mul u64shr u64tod uav_add "
        "uav_add_ext uav_and uav_and_ext uav_arena_load uav_arena_store uav_byte_load uav_byte_load_ext uav_byte_store uav_byte_store_ext "
        "uav_cmp uav_cmp_ext uav_fmax uav_fmax_ext uav_fmin uav_fmin_ext uav_load uav_load_ext uav_load_mip uav_load_mip_ext uav_max "
        "uav_max_ext uav_min uav_min_ext uav_or uav_or_ext uav_raw_load uav_raw_load_ext uav_raw_store uav_raw_store_ext uav_read_add "
        "uav_read_add_ext uav_read_and uav_read_and_ext uav_read_cmp_xchg uav_read_cmp_xchg_ext uav_read_fcmp_xchg uav_read_fcmp_xchg_ext "
        "uav_read_fmax uav_read_fmax_ext uav_read_fmin uav_read_fmin_ext uav_read_max uav_read_max_ext uav_read_min uav_read_min_ext "
        "uav_read_or uav_read_or_ext uav_read_rsub uav_read_rsub_ext uav_read_sub uav_read_sub_ext uav_read_udec uav_read_udec_ext "
        "uav_read_uinc uav_read_uinc_ext uav_read_umax uav_read_umax_ext uav_read_umin uav_read_umin_ext uav_read_xchg uav_read_xchg_ext "
        "uav_read_xor uav_read_xor_extuav_rsub uav_rsub_ext uav_short_load  "
        "uav_short_load_ext uav_short_store uav_short_store_ext uav_store uav_store_ext uav_store_mask uav_store_mask_ext "
        "uav_store_mip uav_store_mip_ext uav_struct_load uav_struct_load_ext uav_struct_store uav_struct_store_ext uav_sub uav_sub_ext "
        "uav_ubyte_load uav_ubyte_load_ext uav_udec uav_udec_ext uav_uinc uav_uinc_ext uav_umax "
        "uav_umax_ext uav_umin  uav_umin_ext uav_ushort_load uav_ushort_load_ext uav_xor  uav_xor_ext "
        "ubit_extract  ubit_insert ubit_reverse ubit_reverser udiv uge ult umad umad24 umax "
        "umax3 umed3 umin umin3 umod umul umul_high umul24 umul24_high unpack0 "
        "unpack1 unpack2 unpack3 ushr utod utof vmad_i64_i32 vmad_u64_u32 wave_id whileloop "
        "dcl_output_generic dcl_typeless_uav_id dcl_lds_id dcl_wavesize lds_store_id  dcl_output_edge_tessfactor "
        "uav_raw_load_id lds_load_id uav_raw_store_id fence_threads_lds div_zeroop dcl_output_inside_tessfactor "
        "dcl_input_primitive_point dcl_output_topology_point_list dcl_output_position "
        "dcl_output_pointsize dcl_output_clipdistance dcl_output_culldistance "
        "dcl_output_rendertarget_array_index dcl_output_viewport_array_index "
        "dcl_output_primitiveid dcl_num_ocp3 dcl_max_tessfactor64 dcl_num_icp3 dcl_ts_domain_quad "
        "dcl_ts_partition_integer dcl_ts_output_primitive_triangle_cw dcl_input_generic cmov_logical ";

    const char* IlBuiltInConstants =
        "r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 ";

    // ---------------------------------------------------------------------------
    // ---------------------------------------------------------------------------

    const char* IsaTypeKeywords =
        "__ATTRIBUTE__ __GENERIC __KERNEL __KERNEL_EXEC __READ_ONLY __READ_WRITE __WRITE_ONLY ALIGNED ATOMIC_DOUBLE ATOMIC_FLAG ATOMIC_FLOAT "
        "ATOMIC_INT ATOMIC_INTPTR_T ATOMIC_LONG ATOMIC_PTRDIFF_T ATOMIC_SIZE_T ATOMIC_UINT ATOMIC_UINTPTR_T ATOMIC_ULONG BOOL CHAR "
        "CHAR16 CHAR2 CHAR3 CHAR4 CHAR8 CL_MEM_FENCE_FLAGS CLK_EVENT_T CLK_PROFILING_INFO CONST DEVICE "
        "DOUBLE DOUBLE16 DOUBLE2 DOUBLE3 DOUBLE4 DOUBLE8 ENDIAN EVEN EVENT_T EXTERN "
        "FLOAT FLOAT16 FLOAT2 FLOAT3 FLOAT4 FLOAT8 GENERIC HALF HI HOST "
        "IMAGE1D_ARRAY_T IMAGE1D_BUFFER_T IMAGE1D_T IMAGE2D_ARRAY_DEPTH_T IMAGE2D_ARRAY_T IMAGE2D_DEPTH_T IMAGE2D_T IMAGE3D_T INT INT16 "
        "INT2 INT3 INT4 INT8 INTPTR_T KERNEL_ENQUEUE_FLAGS_T LO LONG LONG16 LONG2 "
        "LONG3 LONG4 LONG8 MEMORY_ORDER MEMORY_SCOPE NDRANGE_T NOSVM ODD OPENCL_UNROLL_HINT PACKED "
        "PIPE PTRDIFF_T QUEUE_T READ_ONLY READ_WRITE REQD_WORK_GROUP_SIZE RESERVE_ID_T RESTRICT SAMPLER_T SHORT "
        "SHORT16 SHORT2 SHORT3 SHORT4 SHORT8 SIZE_T STATIC TYPEDEF UCHAR UCHAR16 "
        "UCHAR2 UCHAR3 UCHAR4 UCHAR8 UINT UINT16 UINT2 UINT3 UINT4 UINT8 "
        "UINTPTR_T ULONG ULONG16 ULONG2 ULONG3 ULONG4 ULONX G8 UNIFORM UNSIGNED USHORT "
        "USHORT16 USHORT2 USHORT3 USHORT4 USHORT8 VEC_TYPE_HINT VOID VOLATILE WORK_GROUP_SIZE_HINT WRITE_ONLY ";

    const char* IsaTypeReservedKeywords =
        "ALU_INST BANK_SWIZZLE";

    const char* IsaBuiltInFunctions =
        "ADD ALU_BREAK ALU_CONTINUE ALU_ELSE_AFTER ALU_POP2_AFTER ALU_SRC_INTERVA ASHR_INT CALL CALL_COUNT "
        "CALL_FS CETL CF_DWORD0 CF_DWORD1 CF_INST_JUMP CF_INST_LOOP_END CF_INST_LOOP_START CF_INST_MEM "
        "CF_INST_MEM_REDOCTION CF_INST_MEM_STREAM0 CF_INST_MEM_STREAM1 CF_INST_NOP CF_INST_PUSH "
        "CMOVE_INT CMOVEGE_INT CMOVGT ELSE EMIT_CUT_VERTEX EXPORT_DONE EXPORT_MODE EXPORT_PARAM EXPORT_PIXEL "
        "EXPORT_POS FLOOR FLT_TO_INT FLT32_TO_FLT64 FLT64_TO_FLT32 FRACT FRACT_64 FREXP_64 IMPORT_READ_INT "
        "INDEX_AR_X INDEX_AR_Y INDEX_AR_W INDEX_AR_Z INDEX_MODE INT_TO_FLT JUMP "
        "KCACHE_ADDR KCACHE_BANK KCACHE_MODE KILL LAST LDEXP_64 LITERAL LOG_IEEE LOOP LOOP_BREAK "
        "LOOP_CONTINUE LOOP_END LOOP_START LOOP_START_NO_AL LSHL_INT LSHR_INT "
        "MAX MAX_DX10 MAX_INT MAX_UINT MAX4 MEM_EXPORT MEM_REDOCTION MEM_RING MOVA_FLOOR MOVA_INT "
        "MUL_64 MUL_IEEE MUL_LIT MUL_LIT_M4 MULADD_64 MULADD_IEEE_D2 MULADD_IEEE_M4 MULHI_INT "
        "NOP_INT OMOD PGM_START POP_COUNT PRED_SET PRED_SET_CLR PRED_SET_INV PRED_SET_POP PRED_SET_RESTORE "
        "PRED_SETE PRED_SETE_64 PRED_SETE_INT PRED_SETE_PUSH PRED_SETGE PRED_SETGT PRED_SETGT_INT "
        "PRED_SETGT_PUSH PRED_SETLE_INT PRED_SETLT_INT PRED_SETNE PRED_SETNE_PUSH PUSH "
        "RECIP_CLAMPED RECIP_FF RETURN_FS RW_GPR MULLO_INT";

    const char* IsaBuiltInConstants =
        "ALU_OMOD_D2+A322 ALU_OMOD_M2 ALU_OMOD_M4 ALU_OMOD_OFF ALU_SCL_122 ALU_SCL_210 ALU_SCL_212"
        "ALU_SCL_221 ALU_VEC_012 ALU_VEC_021 ALU_VEC_102 ALU_VEC_120 ALU_VEC_201 ALU_VEC_210 "
        "OP2_INST_ADD OP2_INST_ADD_64 OP2_INST_ADD_INT OP2_INST_AND_INT OP2_INST_ASHR_INT OP2_INST_CELL"
        "OP2_INST_COS OP2_INST_CUBE OP2_INST_DOT4 OP2_INST_DOT4_IEEE OP2_INST_EXP_IEEE OP2_INST_FLOOR"
        "OP2_INST_FLT_TO_INT OP2_INST_FLT_TO_UINT OP2_INST_FLT32_TO_FLT64 OP2_INST_FLT64_TO_FLT32 "
        "OP2_INST_FRACT OP2_INST_FRACT_64 OP2_INST_FREXP_64 OP2_INST_INT_TO_FLT OP2_INST_KILLE "
        "OP2_INST_KILLE OP2_INST_KILLE_INT OP2_INST_KILLGE OP2_INST_KILLGE_INT OP2_INST_KILLGE_UINT "
        "OP2_INST_KILLGT_INT OP2_INST_KILLGT_UINT OP2_INST_KILLNE OP2_INST_KILLNE_INT OP2_INST_LDEXP_64 "
        "OP2_INST_LOG_CLAMPED OP2_INST_LOG_IEEE OP2_INST_LSHL_INT OP2_INST_LSHR_INT OP2_INST_MAX "
        "OP2_INST_MAX_DX10 OP2_INST_MAX_INT OP2_INST_MAX_UINT OP2_INST_MAX4 OP2_INST_MIN "
        "OP2_INST_MIN_DX10 OP2_INST_MIN_INT OP2_INST_MIN_UINT OP2_INST_MIN_UINT OP2_INST_MOV "
        "OP2_INST_MOVA OP2_INST_MOVA_FLOOR OP2_INST_MOVA_GPR_INT OP2_INST_MOVA_INT OP2_INST_MUL "
        "OP2_INST_MUL_64 OP2_INST_MUL_IEEE OP2_INST_MULHI_INT OP2_INST_MULHI_UINT OP2_INST_MULLO_INT"
        "OP2_INST_MULLO_UINT OP2_INST_NOP OP2_INST_NOT_INT OP2_INST_OR_INT OP2_INST_PRED_SET_CLR "
        "OP2_INST_PRED_SET_INV OP2_INST_PRED_SET_POP OP2_INST_PRED_SET_RESTORE OP2_INST_PRED_SETE "
        "OP2_INST_PRED_SETE_64 OP2_INST_PRED_SETE_INT OP2_INST_PRED_SETE_PUSH OP2_INST_PRED_SETE_PUSH_INT "
        "OP2_INST_PRED_SETGE OP2_INST_PRED_SETGE_64 OP2_INST_PRED_SETGE_INT OP2_INST_PRED_SETGE_PUSH "
        "OP2_INST_PRED_SETGE_PUSH_INT OP2_INST_PRED_SETGE_UINT OP2_INST_PRED_SETGT OP2_INST_PRED_SETGT_64 "
        "OP2_INST_PRED_SETGT_INT OP2_INST_PRED_SETGT_PUSH OP2_INST_PRED_SETGT_PUSH_INT OP2_INST_PRED_SETGT_UINT "
        "OP2_INST_PRED_SETLE_PUSH_INT OP2_INST_PRED_SETLT_PUSH_INT OP2_INST_PRED_SETNE OP2_INST_PRED_SETNE_INT "
        "OP2_INST_PRED_SETNE_PUSH OP2_INST_PRED_SETNE_PUSH_INT OP2_INST_RECIP_CLAMPED OP2_INST_RECIP_FF "
        "OP2_INST_RECIP_IEEE OP2_INST_RECIP_INT OP2_INST_RECIP_UINT OP2_INST_RECIPSQRT_CLAMPED "
        "OP2_INST_RECIPSQRT_FF OP2_INST_RECIPSQRT_IEEE OP2_INST_RNDNE OP2_INST_SETE "
        "OP2_INST_SETE_DX10 OP2_INST_SETE_INT OP2_INST_SETGE OP2_INST_SETGE_DX10 OP2_INST_SETGE_INT "
        "OP2_INST_SETGE_UINT OP2_INST_SETGT OP2_INST_SETGT_DX10 OP2_INST_SETGT_INT OP2_INST_SETGT_UINT "
        "OP2_INST_SETNE OP2_INST_SETNE_DX10 OP2_INST_SETNE_INT OP2_INST_SIN OP2_INST_SQRT_IEEE "
        "OP2_INST_SUB_INT OP2_INST_TRUNC OP2_INST_XOR_INT ";

    // ---------------------------------------------------------------------------
    // Name:        acQsciLexerCL::acQsciLexerCL
    // Description: Constructor
    // Author:      Uri Shomroni
    // Date:        30/9/2014
    // ---------------------------------------------------------------------------
    acQsciLexerCL::acQsciLexerCL()
        : m_keywordStrings(5)
    {

    }

    // ---------------------------------------------------------------------------
    // Name:        acQsciLexerCL::~acQsciLexerCL
    // Description: Destructor
    // Author:      Uri Shomroni
    // Date:        30/9/2014
    // ---------------------------------------------------------------------------
    acQsciLexerCL::~acQsciLexerCL()
    {

    }

    // ---------------------------------------------------------------------------
    // Name:        acQsciLexerCL::language
    // Description: Return the language name
    // Author:      Uri Shomroni
    // Date:        30/9/2014
    // ---------------------------------------------------------------------------
    const char* acQsciLexerCL::language() const
    {
        return "OpenCL";
    }

    // ---------------------------------------------------------------------------
    // Name:        * acQsciLexerCL::keywords
    // Description: Return the keywords for CL lexer
    // Arguments:   int set
    // Return Val:  const char
    // Author:      Sigal Algranaty
    // Date:        14/9/2011
    // ---------------------------------------------------------------------------
    const char* acQsciLexerCL::keywords(int set) const
    {
        const char* pRetVal = NULL;

        acQsciLexerCL* pNonConstMe = (acQsciLexerCL*)this;

        // Expand the vector as needed:
        if ((int)m_keywordStrings.size() <= set)
        {
            size_t targetSize = ((size_t)set) + 1;
            pNonConstMe->m_keywordStrings.resize(targetSize);
        }

        // Check the cache:
        if (m_keywordStrings[set].isEmpty())
        {
            gtASCIIString& nonConstSet = pNonConstMe->m_keywordStrings[set];

            // Get the key words for the CPP lexer:
            const char* pCPPKeywords = QsciLexerCPP::keywords(set);

            if (pCPPKeywords != NULL)
            {
                nonConstSet = pCPPKeywords;
                nonConstSet.append(' ');
            }

            switch (set)
            {
                case 1: // Primary keywords and identifiers
                    nonConstSet.append(ClTypeKeywords);
                    break;

                case 2: // Secondary keywords and identifiers
                    nonConstSet.append(ClTypeReservedKeywords);
                    nonConstSet.append(ClBuiltInFunctions);
                    break;

                case 3: // Documentation comment keywords
                    break;

                case 4: // global classes and typedefs
                case 5: // global classes and typedefs
                    nonConstSet.append(ClBuiltInConstants);
                    break;

                default:
                    // Add nothing
                    break;
            }
        }

        if (!m_keywordStrings[set].isEmpty())
        {
            pRetVal = m_keywordStrings[set].asCharArray();
        }

        return pRetVal;
    }
    // ---------------------------------------------------------------------------
    // Name:        acQsciLexerCL::defaultColor
    // Description: Return the color for a given style
    // Author:      Uri Shomroni
    // Date:        30/9/2014
    // ---------------------------------------------------------------------------
    QColor acQsciLexerCL::defaultColor(int style) const
    {
        if (QsciLexerCPP::KeywordSet2 == style)
        {
            return QColor(0x7f, 0x00, 0x00);
        }
        else if (QsciLexerCPP::GlobalClass == style)
        {
            return QColor(0x7f, 0x00, 0x7f);
        }

        return QsciLexerCPP::defaultColor(style);
    }

    // ---------------------------------------------------------------------------
    // ---------------------------------------------------------------------------

    acQsciLexerIL::acQsciLexerIL()
        : m_keywordStrings(5)
    {
    }

    // ---------------------------------------------------------------------------

    acQsciLexerIL::~acQsciLexerIL()
    {
    }

    // ---------------------------------------------------------------------------

    const char* acQsciLexerIL::language() const
    {
        return "IL";
    }

    // ---------------------------------------------------------------------------

    const char* acQsciLexerIL::keywords(int set) const
    {
        const char* pRetVal = NULL;

        acQsciLexerIL* pNonConstMe = (acQsciLexerIL*)this;

        // Expand the vector as needed:
        if ((int)m_keywordStrings.size() <= set)
        {
            size_t targetSize = ((size_t)set) + 1;
            pNonConstMe->m_keywordStrings.resize(targetSize);
        }

        // Check the cache:
        if (m_keywordStrings[set].isEmpty())
        {
            gtASCIIString& nonConstSet = pNonConstMe->m_keywordStrings[set];

            switch (set)
            {
                case 1: // Primary keywords and identifiers
                    nonConstSet.append(IlTypeKeywords);
                    break;

                case 2: // Secondary keywords and identifiers
                    nonConstSet.append(IlTypeReservedKeywords);
                    nonConstSet.append(IlBuiltInFunctions);
                    break;

                case 3: // Documentation comment keywords
                    break;

                case 4: // global classes and typedefs
                case 5: // global classes and typedefs
                    nonConstSet.append(IlBuiltInConstants);
                    break;

                default:
                    // Add nothing
                    break;
            }
        }

        if (!m_keywordStrings[set].isEmpty())
        {
            pRetVal = m_keywordStrings[set].asCharArray();
        }

        return pRetVal;
    }

    // ---------------------------------------------------------------------------

    QColor acQsciLexerIL::defaultColor(int style) const
    {
        if (QsciLexerCPP::KeywordSet2 == style)
        {
            // dark red
            return QColor(0x7f, 0x00, 0x00);
        }
        else if (QsciLexerCPP::GlobalClass == style)
        {
            // purple
            return QColor(0x7f, 0x00, 0x7f);
        }

        return QsciLexerCPP::defaultColor(style);
    }

    // ---------------------------------------------------------------------------
    // ---------------------------------------------------------------------------

    acQsciLexerISA::acQsciLexerISA()
        : m_keywordStrings(5)
    {
    }

    // ---------------------------------------------------------------------------

    acQsciLexerISA::~acQsciLexerISA()
    {
    }

    // ---------------------------------------------------------------------------

    const char* acQsciLexerISA::language() const
    {
        return "ISA";
    }

    // ---------------------------------------------------------------------------

    const char* acQsciLexerISA::keywords(int set) const
    {
        const char* pRetVal = NULL;

        acQsciLexerISA* pNonConstMe = (acQsciLexerISA*)this;

        // Expand the vector as needed:
        if ((int)m_keywordStrings.size() <= set)
        {
            size_t targetSize = ((size_t)set) + 1;
            pNonConstMe->m_keywordStrings.resize(targetSize);
        }

        // Check the cache:
        if (m_keywordStrings[set].isEmpty())
        {
            gtASCIIString& nonConstSet = pNonConstMe->m_keywordStrings[set];

            // Get the key words for the CPP lexer:
            const char* pCPPKeywords = QsciLexerCPP::keywords(set);

            if (pCPPKeywords != NULL)
            {
                nonConstSet = pCPPKeywords;
                nonConstSet.append(' ');
            }

            switch (set)
            {
                case 1: // Primary keywords and identifiers
                    nonConstSet.append(IsaTypeKeywords);
                    break;

                case 2: // Secondary keywords and identifiers
                    nonConstSet.append(IsaTypeReservedKeywords);
                    nonConstSet.append(IsaBuiltInFunctions);
                    break;

                case 3: // Documentation comment keywords
                    break;

                case 4: // global classes and typedefs
                case 5: // global classes and typedefs
                    nonConstSet.append(IsaBuiltInConstants);
                    break;

                default:
                    // Add nothing
                    break;
            }
        }

        if (!m_keywordStrings[set].isEmpty())
        {
            pRetVal = m_keywordStrings[set].asCharArray();
        }

        return pRetVal;
    }

    // ---------------------------------------------------------------------------

    QColor acQsciLexerISA::defaultColor(int style) const
    {
        if (QsciLexerCPP::KeywordSet2 == style)
        {
            // dark red
            return QColor(0x7f, 0x00, 0x00);
        }
        else if (QsciLexerCPP::GlobalClass == style)
        {
            // purpel
            return QColor(0x7f, 0x00, 0x7f);
        }

        return QsciLexerCPP::defaultColor(style);
    }


    // ---------------------------------------------------------------------------
    // ---------------------------------------------------------------------------

    acQsciLexerHLSL::acQsciLexerHLSL()
        : m_keywordStrings(5)
    {
    }

    // ---------------------------------------------------------------------------

    acQsciLexerHLSL::~acQsciLexerHLSL()
    {
    }

    // ---------------------------------------------------------------------------

    const char* acQsciLexerHLSL::language() const
    {
        return "Shader";
    }

    // ---------------------------------------------------------------------------

    const char* acQsciLexerHLSL::keywords(int set) const
    {
        const char* pRetVal = NULL;

        acQsciLexerHLSL* pNonConstMe = (acQsciLexerHLSL*)this;

        // Expand the vector as needed:
        if ((int)m_keywordStrings.size() <= set)
        {
            size_t targetSize = ((size_t)set) + 1;
            pNonConstMe->m_keywordStrings.resize(targetSize);
        }

        // Check the cache:
        if (m_keywordStrings[set].isEmpty())
        {
            gtASCIIString& nonConstSet = pNonConstMe->m_keywordStrings[set];

            // Get the key words for the CPP lexer:
            const char* pCPPKeywords = QsciLexerCPP::keywords(set);

            if (pCPPKeywords != NULL)
            {
                nonConstSet = pCPPKeywords;
                nonConstSet.append(' ');
            }

            switch (set)
            {
                case 1: // Primary keywords and identifiers
                    nonConstSet.append(IsaTypeKeywords);
                    break;

                case 2: // Secondary keywords and identifiers
                    nonConstSet.append(IsaTypeReservedKeywords);
                    nonConstSet.append(IsaBuiltInFunctions);
                    break;

                case 3: // Documentation comment keywords
                    break;

                case 4: // global classes and typedefs
                case 5: // global classes and typedefs
                    nonConstSet.append(IsaBuiltInConstants);
                    break;

                default:
                    // Add nothing
                    break;
            }
        }

        if (!m_keywordStrings[set].isEmpty())
        {
            pRetVal = m_keywordStrings[set].asCharArray();
        }

        return pRetVal;
    }

    // ---------------------------------------------------------------------------

    QColor acQsciLexerHLSL::defaultColor(int style) const
    {
        if (QsciLexerCPP::KeywordSet2 == style)
        {
            // dark red
            return QColor(0x7f, 0x00, 0x00);
        }
        else if (QsciLexerCPP::GlobalClass == style)
        {
            // purpel
            return QColor(0x7f, 0x00, 0x7f);
        }

        return QsciLexerCPP::defaultColor(style);
    }
