//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file OpenCLCContentExtension.cs 
/// 
//==================================================================================

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel.Composition;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.Language.StandardClassification;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Text.Classification;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Text.Tagging;
using Microsoft.VisualStudio.Utilities;

namespace CodeXLVSPackageVSIX
{
    // Defines a content type named "OpenCL C", makes it inherit C/C++'s properties, and sets it as associated to the
    // .cl file extension:
    [ComVisible(true)]
    [Guid("CF90A284-AB86-44CE-82AD-7F8971249635")]
    [Export(typeof(ITaggerProvider))]
    [ContentType("OpenCL C")]
    [TagType(typeof(ClassificationTag))]
    class OpenCLCContentExtension : ITaggerProvider
    {
        // The content type:
        [Export]
        [Name("OpenCL C")]
        [BaseDefinition("C/C++")]
        internal static ContentTypeDefinition clContentTypeDef = null;

        // The file association:
        [Export]
        [FileExtension(".cl")]
        [ContentType("OpenCL C")]
        internal static FileExtensionToContentTypeDefinition clFileExtensionTypeDef = null;

        [Import]
        internal IClassificationTypeRegistryService ClassificationTypeRegistry = null;

        [Import]
        internal IBufferTagAggregatorFactoryService aggregatorFactory = null;

        public ITagger<T> CreateTagger<T>(ITextBuffer buffer) where T : ITag
        {
            return new OpenCLCClassifier(buffer, ClassificationTypeRegistry) as ITagger<T>;
        }
    }

    // A classifier which will tag OpenCL C reserved words as such:
    internal sealed class OpenCLCClassifier : ITagger<ClassificationTag>
    {
        ITextBuffer _buffer;
        IClassificationType _keywordType;
        IClassificationType _macroType;

        // A list of OpenCL-only keywords, from the OpenCL specification:
        static string[] openclCKeywords = {"__attribute__", "__constant", "__generic", "__global", "__kernel_exec", "__kernel", "__local", "__private",
                                           "__read_only", "__read_write", "__write_only", "aligned", "atomic_double", "atomic_flag", "atomic_float",
                                           "atomic_int", "atomic_intptr_t", "atomic_long", "atomic_ptrdiff_t", "atomic_size_t", "atomic_uint", "atomic_uintptr_t", "atomic_ulong", "bool", "char",
                                           "char16", "char2", "char3", "char4", "char8", "cl_mem_fence_flags", "clk_event_t", "clk_profiling_info", "const", "device",
                                           "double", "double16", "double2", "double3", "double4", "double8", "endian", "even", "event_t", "extern",
                                           "float", "float16", "float2", "float3", "float4", "float8", "generic", "half", "hi", "host",
                                           "image1d_array_t", "image1d_buffer_t", "image1d_t", "image2d_array_depth_t", "image2d_array_t", "image2d_depth_t", "image2d_t", "image3d_t", "int", "int16",
                                           "int2", "int3", "int4", "int8", "intptr_t", "kernel_enqueue_flags_t", "lo", "long", "long16", "long2",
                                           "long3", "long4", "long8", "memory_order", "memory_scope", "ndrange_t", "nosvm", "odd", "opencl_unroll_hint", "packed",
                                           "pipe", "ptrdiff_t", "queue_t", "read_only", "read_write", "reqd_work_group_size", "reserve_id_t", "restrict", "sampler_t", "short",
                                           "short16", "short2", "short3", "short4", "short8", "size_t", "static", "typedef", "uchar", "uchar16",
                                           "uchar2", "uchar3", "uchar4", "uchar8", "uint", "uint16", "uint2", "uint3", "uint4", "uint8",
                                           "uintptr_t", "ulong", "ulong16", "ulong2", "ulong3", "ulong4", "ulong8", "uniform", "unsigned", "ushort",
                                           "ushort16", "ushort2", "ushort3", "ushort4", "ushort8", "vec_type_hint", "void", "volatile", "work_group_size_hint", "write_only"};

        static string[] openclBuiltInValues = {"__ENDIAN_LITTLE__", "__FAST_RELAXED_MATH__", "__FILE__", "__IMAGE_SUPPORT__", "__LINE__", "__OPENCL_C_VERSION__", "__OPENCL_VERSION__", "ATOMIC_FLAG_INIT", "ATOMIC_VAR_INIT", "CHAR_BIT",
                                               "CHAR_MAX", "CHAR_MIN", "CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE", "CL_VERSION_1_0", "CL_VERSION_1_1", "CL_VERSION_1_2", "CL_VERSION_2_0", "CLK_A", "CLK_ABGR", "CLK_ADDRESS_CLAMP",
                                               "CLK_ADDRESS_CLAMP_TO_EDGE", "CLK_ADDRESS_MIRRORED_REPEAT", "CLK_ADDRESS_NONE", "CLK_ADDRESS_REPEAT", "CLK_ARGB", "CLK_BGRA", "CLK_DEPTH", "CLK_DEVICE_QUEUE_FULL", "CLK_ENQUEUE_FLAGS_NO_WAIT", "CLK_ENQUEUE_FLAGS_WAIT_KERNEL",
                                               "CLK_ENQUEUE_FLAGS_WAIT_WORK_GROUP", "CLK_EVENT_ALLOCATION_FAILURE", "CLK_FILTER_LINEAR", "CLK_FILTER_NEAREST", "CLK_FLOAT", "CLK_GLOBAL_MEM_FENCE", "CLK_HALF_FLOAT", "CLK_IMAGE_MEM_FENCE", "CLK_INTENSITY", "CLK_INVALID_ARG_SIZE",
                                               "CLK_INVALID_EVENT_WAIT_LIST", "CLK_INVALID_NDRANGE", "CLK_INVALID_QUEUE", "CLK_LOCAL_MEM_FENCE", "CLK_LUMINANCE", "CLK_NORMALIZED_COORDS_FALSE", "CLK_NORMALIZED_COORDS_TRUE", "CLK_NULL_RESERVE_ID", "CLK_OUT_OF_RESOURCES", "CLK_R",
                                               "CLK_RA", "CLK_RG", "CLK_RGB", "CLK_RGBA", "CLK_RGBx", "CLK_RGx", "CLK_Rx", "CLK_sBGRA", "CLK_SIGNED_INT16", "CLK_SIGNED_INT32",
                                               "CLK_SIGNED_INT8", "CLK_SNORM_INT16", "CLK_SNORM_INT8", "CLK_sRGB", "CLK_sRGBA", "CLK_sRGBx", "CLK_UNORM_INT16", "CLK_UNORM_INT8", "CLK_UNORM_SHORT_101010", "CLK_UNORM_SHORT_555",
                                               "CLK_UNORM_SHORT_565", "CLK_UNSIGNED_INT16", "CLK_UNSIGNED_INT32", "CLK_UNSIGNED_INT8", "DBL_DIG", "DBL_EPSILSON", "DBL_MANT_DIG", "DBL_MAX", "DBL_MAX_10_EXP", "DBL_MAX_EXP",
                                               "DBL_MIN", "DBL_MIN_10_EXP", "DBL_MIN_EXP", "FLT_DIG", "FLT_EPSILSON", "FLT_MANT_DIG", "FLT_MAX", "FLT_MAX_10_EXP", "FLT_MAX_EXP", "FLT_MIN",
                                               "FLT_MIN_10_EXP", "FLT_MIN_EXP", "FLT_RADIX", "FP_CONTRACT", "FP_FAST_FMAF", "HIGE_VAL", "HUGE_VALF", "INFINITY", "INT_MAX", "INT_MIN",
                                               "LONG_MAX", "LONG_MIN", "M_1_PI", "M_1_PI_F", "M_2_PI", "M_2_PI_F", "M_2_SQRTPI", "M_2_SQRTPI_F", "M_E", "M_E_F",
                                               "M_LN10", "M_LN10_F", "M_LN2", "M_LN2_F", "M_LOG10E", "M_LOG10E_F", "M_LOG2E", "M_LOG2E_F", "M_PI", "M_PI_2",
                                               "M_PI_2_F", "M_PI_4", "M_PI_4_F", "M_PI_F", "M_SQRT1_2", "M_SQRT1_2_F", "M_SQRT2", "M_SQRT2_F", "MAXFLOAT", "memory_order_acq_rel",
                                               "memory_order_acquire", "memory_order_relaxed", "memory_order_release", "memory_order_seq_cst", "memory_scope_all_svm_devices", "memory_scope_device", "memory_scope_work_group", "memory_scope_work_item", "NAN", "NULL",
                                               "SCHAR_MAX", "SCHAR_MIN", "SHRT_MAX", "SHRT_MIN", "UCHAR_MAX", "UINT_MAX", "ULONG_MAX", "USHRT_MAX"};
        // Uri, 30/9/14 - This list is fairly huge, need to test for slowdown
        // static string[] openclBuiltinFuncs = { };

        internal OpenCLCClassifier(ITextBuffer buffer,
                                   IClassificationTypeRegistryService typeService)
        {
            _buffer = buffer;
            _keywordType = typeService.GetClassificationType(PredefinedClassificationTypeNames.Keyword);
            _macroType = typeService.GetClassificationType("cppMacro");
            // _userTypesType = typeService.GetClassificationType("cppType");
        }
        // Allowed names: biDiNeutral, text, quickinfo-bold, MarkerPlaceHolder, word wrap glyph, sighelp-documentation, currentParam, natural language, formal language, comment,
        // identifier, keyword, whitespace, operator, literal, string, character, number, other, excluded code, preprocessor keyword, symbol definition, symbol reference, (TRANSIENT),
        // url, line number, VB XML Doc Tag, VB XML Doc Attribute, VB XML Doc Comment, VB XML Delimiter, VB XML Comment, VB XML Name, VB XML Attribute Name, VB XML CData Section,
        // VB XML Processing Instruction, VB XML Attribute Value, VB XML Attribute Quotes, VB XML Text, VB XML Embedded Expression, VB User Types, VB Excluded Code, XML Doc Comment,
        // XML Doc Tag, C/C++ User Keywords, CppInactiveCodeClassification, CppSolidColorClassification, cppMacro, cppEnumerator, cppGlobalVariable, cppLocalVariable, cppParameter,
        // cppType, cppRefType, cppValueType, cppFunction, cppMemberFunction, cppMemberField, cppStaticMemberFunction, cppStaticMemberField, cppProperty, cppEvent, cppClassTemplate,
        // cppGenericType, cppFunctionTemplate, cppNamespace, cppLabel, CSS Comment, CSS Keyword, CSS Selector, CSS Property Name, CSS Property Value, CSS String Value, RazorCode,
        // Razor Code, PowerShell 3 Parameter, PowerShell 3 Variable, PowerShell 3 Attribute, PowerShell 3 Type Name, PowerShell 3 Command Name, PowerShell 3 Generic, HTML Attribute Name,
        // HTML Attribute Value, HTML Comment, HTML Element Name, HTML Entity, HTML Operator, HTML Server-Side Script, HTML Tag Delimiter, HTML Priority Workaround, VBScript Keyword,
        // VBScript Comment, VBScript Operator, VBScript Number, VBScript String, VBScript Identifier, VBScript Function Block Start, RegularExpression, TypeScriptClassifier

        public event EventHandler<SnapshotSpanEventArgs> TagsChanged
        {
            add { }
            remove { }
        }

        public IEnumerable<ITagSpan<ClassificationTag>> GetTags(NormalizedSnapshotSpanCollection spans)
        {
            foreach (SnapshotSpan curSpan in spans)
            {
                string snapshotText = curSpan.Snapshot.GetText();
                ITextSnapshotLine containingLine = curSpan.Start.GetContainingLine();
                int currentLocation = containingLine.Start.Position;
                char[] delimiters = {' ', '\t', '\n', '\r', '(', ')', '{', '}', '[', ']', '.', ',', '<', '>', ';', '=', '+', '-', '*', '/', '%', '^', '~', '!', '|', '&'};
                string[] tokens = containingLine.GetText().Split(delimiters);

                foreach (string strToken in tokens)
                {
                    if (0 < strToken.Length)
                    {
                        bool isKeyWord = openclCKeywords.Contains(strToken);
                        bool isBuiltInValue = openclBuiltInValues.Contains(strToken);
                        // bool isBuiltInFunction = openclBuiltInFunctions.Contains(strToken);
                        if (isKeyWord || isBuiltInValue)
                        {
                            // If it's inside the span of a (single or multiline) comment, do not highlight the keyword:
                            bool isInsideMultilineComment = snapshotText.LastIndexOf("/*", currentLocation) > snapshotText.LastIndexOf("*/", currentLocation);
                            bool isInsideSingleLineComment = snapshotText.LastIndexOf("//", currentLocation) > containingLine.Start.Position;
                            if (!(isInsideMultilineComment || isInsideSingleLineComment))
                            {
                                // If this is a keyword and it is part of the input span, return it:
                                var tokenSpan = new SnapshotSpan(curSpan.Snapshot, new Span(currentLocation, strToken.Length));
                                if (tokenSpan.IntersectsWith(curSpan))
                                    yield return new TagSpan<ClassificationTag>(tokenSpan, new ClassificationTag(isKeyWord ? _keywordType : isBuiltInValue ? _macroType : null));
                            }
                        }
                    }

                    // Also add an index for the delimiter we skipped:
                    currentLocation += strToken.Length + 1;
                }
            }
        }
    }
}
