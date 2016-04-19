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
    // Defines a content type named "GLSL", makes it inherit C/C++'s properties, and sets it as associated to the
    // .glsl file extension:
    [ComVisible(true)]
    [Export(typeof(ITaggerProvider))]
    [ContentType("GLSL")]
    [TagType(typeof(ClassificationTag))]
    class GLSLContentExtension : ITaggerProvider
    {
        // The content type:
        [Export]
        [Name("GLSL")]
        [BaseDefinition("C/C++")]
        internal static ContentTypeDefinition glslContentTypeDef = null;

        // The file association:
        [Export]
        [FileExtension(".glsl")]
        [ContentType("GLSL")]
        internal static FileExtensionToContentTypeDefinition glslFileExtensionTypeDef = null;

        [Import]
        internal IClassificationTypeRegistryService ClassificationTypeRegistry = null;

        [Import]
        internal IBufferTagAggregatorFactoryService aggregatorFactory = null;

        public ITagger<T> CreateTagger<T>(ITextBuffer buffer) where T : ITag
        {
            return new GLSLClassifier(buffer, ClassificationTypeRegistry) as ITagger<T>;
        }
    }

    // A classifier which will tag GLSL reserved words as such:
    internal sealed class GLSLClassifier : ITagger<ClassificationTag>
    {
        ITextBuffer _buffer;
        IClassificationType _keywordType;

        // A list of GLSL-only keywords, from the OpenGL specification:
        static string[] glslKeywords = {"attribute", "uniform", "varying", "layout", "centroid", "flat", "smooth", "noperspective", "patch", "sample", "subroutine", "in", "out", "inout",
                                        "invariant", "discard", "mat2", "mat3", "mat4", "dmat2", "dmat3", "dmat4", "mat2x2", "mat2x3", "mat2x4", "dmat2x2", "dmat2x3", "dmat2x4", "mat3x2",
                                        "mat3x3", "mat3x4", "dmat3x2", "dmat3x3", "dmat3x4", "mat4x2", "mat4x3", "mat4x4", "dmat4x2", "dmat4x3", "dmat4x4", "vec2", "vec3", "vec4", "ivec2",
                                        "ivec3", "ivec4", "bvec2", "bvec3", "bvec4", "dvec2", "dvec3", "dvec4", "uint", "uvec2", "uvec3", "uvec4", "lowp", "mediump", "highp", "precision",
                                        "sampler1D", "sampler2D", "sampler3D", "samplerCube", "sampler1DShadow", "sampler2DShadow", "samplerCubeShadow", "sampler1DArray", "sampler2DArray",
                                        "sampler1DArrayShadow", "sampler2DArrayShadow", "isampler1D", "isampler2D", "isampler3D", "isamplerCube", "isampler1DArray", "isampler2DArray",
                                        "usampler1D", "usampler2D", "usampler3D", "usamplerCube", "usampler1DArray", "usampler2DArray", "sampler2DRect", "sampler2DRectShadow",
                                        "isampler2DRect", "usampler2DRect", "samplerBuffer", "isamplerBuffer", "usamplerBuffer", "sampler2DMS", "isampler2DMS", "usampler2DMS",
                                        "sampler2DMSArray", "isampler2DMSArray", "usampler2DMSArray", "samplerCubeArray", "samplerCubeArrayShadow", "isamplerCubeArray", "usamplerCubeArray"};

        static char[] delimiters = { ' ', '\t', '\n', '\r', '(', ')', '{', '}', '[', ']', '.', ',', '<', '>', ';', '=', '+', '-', '*', '/', '%', '^', '~', '!', '|', '&' };

        static HashSet<string> hashedKeywords = new HashSet<string>(glslKeywords.ToList());

        internal GLSLClassifier(ITextBuffer buffer,
                                IClassificationTypeRegistryService typeService)
        {
            _buffer = buffer;
            _keywordType = typeService.GetClassificationType(PredefinedClassificationTypeNames.Keyword);
        }

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
                string[] tokens = containingLine.GetText().Split(delimiters);

                foreach (string strToken in tokens)
                {
                    if (0 < strToken.Length)
                    {
                        if (hashedKeywords.Contains(strToken))
                        {
                            // If it's inside the span of a (single or multiline) comment, do not highlight the keyword:
                            bool isInsideMultilineComment = snapshotText.LastIndexOf("/*", currentLocation) > snapshotText.LastIndexOf("*/", currentLocation);
                            bool isInsideSingleLineComment = snapshotText.LastIndexOf("//", currentLocation) > containingLine.Start.Position;
                            if (!(isInsideMultilineComment || isInsideSingleLineComment))
                            {
                                // If this is a keyword and it is part of the input span, return it:
                                var tokenSpan = new SnapshotSpan(curSpan.Snapshot, new Span(currentLocation, strToken.Length));
                                if (tokenSpan.IntersectsWith(curSpan))
                                    yield return new TagSpan<ClassificationTag>(tokenSpan, new ClassificationTag(_keywordType));
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
