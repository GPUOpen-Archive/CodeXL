//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenGLBuiltInUniformManager.cpp
///
//==================================================================================

//------------------------------ apOpenGLBuiltInUniformManager.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apOpenGLBuiltInUniformManager.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>

// Symbolic name for indexed non-struct variables:
#define AP_INDEX_MEMBER_NAME L""

// Static member initializations:
apOpenGLBuiltInUniformManager* apOpenGLBuiltInUniformManager::m_spMySingleInstance = nullptr;


apOpenGLBuiltInUniformManager& apOpenGLBuiltInUniformManager::instance()
{
    if (nullptr == m_spMySingleInstance)
    {
        m_spMySingleInstance = new apOpenGLBuiltInUniformManager;
    }

    return *m_spMySingleInstance;
}

bool apOpenGLBuiltInUniformManager::ParseUniformMemberAccess(const gtString& fullName, apBuiltInUniformId& id, int& index, apBuiltInUniformMemberId& mid) const
{
    bool retVal = false;

    id = ap_numberOfSupportedBuiltInUniforms;
    index = -1;
    mid = ap_numberOfSupportedBuiltInUniformMembers;

    // The allowed formats are:
    // gl_Xxxx
    // gl_Xxxx[123]
    // gl_Xxxx.Yyyy
    // gl_Xxxx[123].Yyyy

    const int firstOpenBracket = fullName.find('[');
    const int firstCloseBracket = fullName.find(']');
    const int firstDot = fullName.find('.');
    const int openBracketCount = fullName.count('[');
    const int closeBracketCount = fullName.count(']');
    const int dotCount = fullName.count('.');
    const int nameLen = fullName.length();

    if ((1 >= openBracketCount) && (openBracketCount == closeBracketCount) && (1 >= dotCount) &&
        (firstOpenBracket <= firstCloseBracket) && (firstDot < (nameLen - 1)) &&
        ((0 > firstDot) || (0 > firstCloseBracket) || (firstDot == (firstCloseBracket + 1))))
    {
        // Format is legal, parse:
        const bool hasBrackets = (0 != openBracketCount);
        const bool hasDot = (0 != dotCount);
        const int mainNameEnd = (hasBrackets ? firstOpenBracket : (hasDot ? firstDot : nameLen)) - 1;
        gtString mainName;
        fullName.getSubString(0, mainNameEnd, mainName);

        retVal = GetBuiltInUniformId(mainName, id);

        if (retVal)
        {
            const apBuiltInUniformDataWithMembers& data = m_uniformData[id];

            // Get the index:
            if (hasBrackets)
            {
                retVal = false;
                gtString indexStr;
                fullName.getSubString(firstOpenBracket + 1, firstCloseBracket - 1, indexStr);

                if (!indexStr.isEmpty() && indexStr.toIntNumber(index))
                {
                    retVal = (0 <= index);
                }
            }

            // Get the member:
            gtString memberName;

            if (retVal && hasBrackets)
            {
                memberName = AP_INDEX_MEMBER_NAME;
            }

            if (retVal && hasDot)
            {
                fullName.getSubString(firstDot + 1, nameLen - 1, memberName);
            }

            if (hasDot || hasBrackets)
            {
                // Note that we write indexed variables as having a member named "".
                // This allows the following search to succeed for those cases (iff the
                // fullName did not have a dot). Alternatively, we could do this with a special
                // name, e.g. "[]".
                gtMap<gtString, apBuiltInUniformMemberId>::const_iterator findIter = data.m_memberMapping.find(memberName);
                gtMap<gtString, apBuiltInUniformMemberId>::const_iterator endIter = data.m_memberMapping.end();

                // If we expected a member, it has to be legal:
                retVal = (endIter != findIter);

                if (retVal)
                {
                    mid = findIter->second;
                }
            }

#if AMDT_BUILD_TARGET == AMDT_DEBUG_BUILD

            // Validate the name format. This is redundant and should only be done in debug builds:
            if (retVal)
            {
                gtString expectedName = UniformName(id);

                if (0 <= index)
                {
                    expectedName.appendFormattedString(L"[%d]", index);
                }

                if (ap_numberOfSupportedBuiltInUniformMembers != mid)
                {
                    const gtString& mName = m_uniformMemberData[mid].m_name;

                    if (AP_INDEX_MEMBER_NAME != mName)
                    {
                        expectedName.append('.').append(mName);
                    }
                }

                GT_ASSERT(expectedName == fullName);
            }

#endif
        }
    }

    return retVal;
}
bool apOpenGLBuiltInUniformManager::GetBuiltInUniformOrMemberFormula(const gtString& fullName, gtString& formula, bool htmlFormat) const
{
    bool retVal = true;

    formula = AP_STR_UnknownBuiltInUniformFormula;

    apBuiltInUniformId id = ap_numberOfSupportedBuiltInUniforms;
    int index = -1;
    apBuiltInUniformMemberId mid = ap_numberOfSupportedBuiltInUniformMembers;
    bool rcNm = ParseUniformMemberAccess(fullName, id, index, mid);

    // Do not use if, since we want to recover if possible:
    GT_ASSERT(rcNm);

    if (ap_numberOfSupportedBuiltInUniforms != id)
    {
        // Start with the generic formula:
        formula = UniformCalculationFormula(id, htmlFormat);

        if (ap_numberOfSupportedBuiltInUniformMembers != mid)
        {
            // Make sure everything is as expected:
            const gtString& memberFormula = UniformMemberCalculationFormula(mid, htmlFormat);
            static const gtString percentD = L"%d";
            const bool formulaIndexable = (0 <= memberFormula.find(percentD));

            if (!formulaIndexable)
            {
                // Only non-indexed access is allowed for a non-indexed variable:
                if (0 > index)
                {
                    formula = UniformMemberCalculationFormula(mid, htmlFormat);
                }
            }
            else // formulaIndexable
            {
                // Only indexed access is allowed for an indexed variable:
                if (0 <= index)
                {
                    formula.makeEmpty().appendFormattedString(memberFormula.asCharArray(), index);
                }
            }
        }
    }

    return retVal;
}

bool apOpenGLBuiltInUniformManager::GetBuiltInUniformId(const gtString& uniformName, apBuiltInUniformId& id) const
{
    const wchar_t* pUniformNameStr = uniformName.asCharArray();

    // Shorthands for comparison:
#define LTTR(i) (pUniformNameStr[i])
#define LTTRC(i, c) (c == LTTR(i))
#define LTTRCProjection(i)  (LTTRC(i, 'P') && LTTRC(i + 1, 'r') && LTTRC(i + 2, 'o') && LTTRC(i + 3, 'j') && LTTRC(i + 4, 'e') && LTTRC(i + 5, 'c') && LTTRC(i + 6, 't') && LTTRC(i + 7, 'i') && LTTRC(i + 8, 'o') && LTTRC(i + 9, 'n'))
#define LTTRCMatrix(i)      (LTTRC(i, 'M') && LTTRC(i + 1, 'a') && LTTRC(i + 2, 't') && LTTRC(i + 3, 'r') && LTTRC(i + 4, 'i') && LTTRC(i + 5, 'x'))
#define LTTRCInverse(i)     (LTTRC(i, 'I') && LTTRC(i + 1, 'n') && LTTRC(i + 2, 'v') && LTTRC(i + 3, 'e') && LTTRC(i + 4, 'r') && LTTRC(i + 5, 's') && LTTRC(i + 6, 'e'))
#define RET_ID(vid) {id = vid; retVal = true;}
#define TEST_VAR_AND_RET(id) if (UniformName(id) == uniformName) RET_ID(id);


    bool retVal = false;

    int uniformNameLength = uniformName.length();

    // Shortest-named uniform we support is gl_Fog:
    if (5 < uniformNameLength)
    {
        //////////////////////////////////////////////////////////////////////////
        // Uri, 19/7/2015:
        // This function is implemented as a parse tree to improve performance.
        // Note that since for a (gt/std::)string s of length N, accessing s[N] is
        // legal and yields the null terminator, this way of getting the values is
        // safe, since C's lazy evaluation guarantees that the previous character was
        // not a '\0'.
        //////////////////////////////////////////////////////////////////////////
        if (LTTRC(0, 'g') && LTTRC(1, 'l') && LTTRC(2, '_'))
        {
            switch (LTTR(3))
            {
                case 'B':
                    if (LTTRC(4, 'a') && LTTRC(5, 'c') && LTTRC(6, 'k'))
                    {
                        if (LTTRC(7, 'L') && LTTRC(8, 'i') && LTTRC(9, 'g') && LTTRC(10, 'h') && LTTRC(11, 't'))
                        {
                            if (LTTRC(12, 'M'))
                            {
                                TEST_VAR_AND_RET(ap_gl_BackLightModelProduct);
                            }
                            else if (LTTRC(12, 'P'))
                            {
                                TEST_VAR_AND_RET(ap_gl_BackLightProduct);
                            }
                        }
                        else if (LTTRC(7, 'M'))
                        {
                            TEST_VAR_AND_RET(ap_gl_BackMaterial);
                        }
                    }

                    break;

                case 'C':
                    TEST_VAR_AND_RET(ap_gl_ClipPlane);
                    break;

                case 'D':
                    TEST_VAR_AND_RET(ap_gl_DepthRange);
                    break;

                case 'E':
                    if (LTTRC(4, 'y') && LTTRC(5, 'e') && LTTRC(6, 'P') && LTTRC(7, 'l') && LTTRC(8, 'a') && LTTRC(9, 'n') && LTTRC(10, 'e'))
                    {
                        if (12 == uniformNameLength)
                        {
                            switch (LTTR(11))
                            {
                                case 'Q':
                                    RET_ID(ap_gl_EyePlaneQ);
                                    break;

                                case 'R':
                                    RET_ID(ap_gl_EyePlaneR);
                                    break;

                                case 'S':
                                    RET_ID(ap_gl_EyePlaneS);
                                    break;

                                case 'T':
                                    RET_ID(ap_gl_EyePlaneT);
                                    break;

                                default:
                                    break;
                            }
                        }
                    }

                    break;

                case 'F':
                    if (LTTRC(4, 'o'))
                    {
                        TEST_VAR_AND_RET(ap_gl_Fog)
                    }
                    else if (LTTRC(4, 'r') && LTTRC(5, 'o') && LTTRC(6, 'n') && LTTRC(7, 't'))
                    {
                        if (LTTRC(8, 'L') && LTTRC(9, 'i') && LTTRC(10, 'g') && LTTRC(11, 'h') && LTTRC(12, 't'))
                        {
                            if (LTTRC(13, 'M'))
                            {
                                TEST_VAR_AND_RET(ap_gl_FrontLightModelProduct);
                            }
                            else if (LTTRC(13, 'P'))
                            {
                                TEST_VAR_AND_RET(ap_gl_FrontLightProduct);
                            }
                        }
                        else if (LTTRC(8, 'M'))
                        {
                            TEST_VAR_AND_RET(ap_gl_FrontMaterial);
                        }
                    }

                    break;

                case 'L':
                    if (LTTRC(4, 'i') && LTTRC(5, 'g') && LTTRC(6, 'h') && LTTRC(7, 't'))
                    {
                        if (LTTRC(8, 'M'))
                        {
                            TEST_VAR_AND_RET(ap_gl_LightModel);
                        }
                        else if (LTTRC(8, 'S'))
                        {
                            TEST_VAR_AND_RET(ap_gl_LightSource);
                        }
                    }

                    break;

                case 'M':
                    if (LTTRC(4, 'o') && LTTRC(5, 'd') && LTTRC(6, 'e') && LTTRC(7, 'l') && LTTRC(8, 'V') && LTTRC(9, 'i') && LTTRC(10, 'e') && LTTRC(11, 'w'))
                    {
                        if (LTTRCMatrix(12))
                        {
                            if (LTTRC(18, '\0'))
                            {
                                RET_ID(ap_gl_ModelViewMatrix);
                            }
                            else if (LTTRCInverse(18))
                            {
                                if (LTTRC(25, '\0'))
                                {
                                    RET_ID(ap_gl_ModelViewMatrixInverse);
                                }
                                else if (LTTRC(25, 'T'))
                                {
                                    TEST_VAR_AND_RET(ap_gl_ModelViewMatrixInverseTranspose);
                                }
                            }
                            else if (LTTRC(18, 'T'))
                            {
                                TEST_VAR_AND_RET(ap_gl_ModelViewMatrixTranspose);
                            }
                        }
                        else if (LTTRCProjection(12) && LTTRCMatrix(22))
                        {
                            if (LTTRC(28, '\0'))
                            {
                                RET_ID(ap_gl_ModelViewProjectionMatrix);
                            }
                            else if (LTTRCInverse(28))
                            {
                                if (LTTRC(35, '\0'))
                                {
                                    RET_ID(ap_gl_ModelViewProjectionMatrixInverse);
                                }
                                else if (LTTRC(35, 'T'))
                                {
                                    TEST_VAR_AND_RET(ap_gl_ModelViewProjectionMatrixInverseTranspose);
                                }
                            }
                            else if (LTTRC(28, 'T'))
                            {
                                TEST_VAR_AND_RET(ap_gl_ModelViewProjectionMatrixTranspose);
                            }
                        }
                    }

                    break;

                case 'N':
                    if (LTTRC(4, 'o') && LTTRC(5, 'r') && LTTRC(6, 'm') && LTTRC(7, 'a') && LTTRC(8, 'l'))
                    {
                        if (LTTRC(9, 'M'))
                        {
                            TEST_VAR_AND_RET(ap_gl_NormalMatrix);
                        }
                        else if (LTTRC(9, 'S'))
                        {
                            TEST_VAR_AND_RET(ap_gl_NormalScale);
                        }
                    }
                    else if (LTTRC(4, 'u'))
                    {
                        TEST_VAR_AND_RET(ap_gl_NumSamples);
                    }

                    break;

                case 'O':
                    if (LTTRC(4, 'b') && LTTRC(5, 'j') && LTTRC(6, 'e') && LTTRC(7, 'c') && LTTRC(8, 't') && LTTRC(9, 'P') && LTTRC(10, 'l') && LTTRC(11, 'a') && LTTRC(12, 'n') && LTTRC(13, 'e'))
                    {
                        if (15 == uniformNameLength)
                        {
                            switch (LTTR(14))
                            {
                                case 'Q':
                                    RET_ID(ap_gl_ObjectPlaneQ);
                                    break;

                                case 'R':
                                    RET_ID(ap_gl_ObjectPlaneR);
                                    break;

                                case 'S':
                                    RET_ID(ap_gl_ObjectPlaneS);
                                    break;

                                case 'T':
                                    RET_ID(ap_gl_ObjectPlaneT);
                                    break;

                                default:
                                    break;
                            }
                        }
                    }

                    break;

                case 'P':
                    if (LTTRC(4, 'o'))
                    {
                        TEST_VAR_AND_RET(ap_gl_Point);
                    }
                    else if (LTTRCProjection(3) && LTTRCMatrix(13))
                    {
                        if (LTTRC(19, '\0'))
                        {
                            RET_ID(ap_gl_ProjectionMatrix);
                        }
                        else if (LTTRCInverse(19))
                        {
                            if (LTTRC(26, '\0'))
                            {
                                RET_ID(ap_gl_ProjectionMatrixInverse);
                            }
                            else if (LTTRC(26, 'T'))
                            {
                                TEST_VAR_AND_RET(ap_gl_ProjectionMatrixInverseTranspose);
                            }
                        }
                        else if (LTTRC(19, 'T'))
                        {
                            TEST_VAR_AND_RET(ap_gl_ProjectionMatrixTranspose);
                        }
                    }

                    break;

                case 'T':
                    if (LTTRC(4, 'e') && LTTRC(5, 'x') && LTTRC(6, 't') && LTTRC(7, 'u') && LTTRC(8, 'r') && LTTRC(9, 'e'))
                    {
                        if (LTTRC(10, 'E'))
                        {
                            TEST_VAR_AND_RET(ap_gl_TextureEnvColor);
                        }
                        else if (LTTRCMatrix(10))
                        {
                            if (LTTRC(16, '\0'))
                            {
                                RET_ID(ap_gl_TextureMatrix);
                            }
                            else if (LTTRCInverse(16))
                            {
                                if (LTTRC(23, '\0'))
                                {
                                    RET_ID(ap_gl_TextureMatrixInverse);
                                }
                                else if (LTTRC(23, 'T'))
                                {
                                    TEST_VAR_AND_RET(ap_gl_TextureMatrixInverseTranspose);
                                }
                            }
                            else if (LTTRC(16, 'T'))
                            {
                                TEST_VAR_AND_RET(ap_gl_TextureMatrixTranspose);
                            }
                        }
                    }

                    break;

                default:
                    break;
            }
        }
    }

#undef LTTR
#undef LTTRC
#undef LTTRCProjection
#undef LTTRCMatrix
#undef LTTRCInverse
#undef RET_ID
#undef TEST_VAR_AND_RET

    if (!retVal)
    {
        id = ap_numberOfSupportedBuiltInUniforms;
    }

    return retVal;
}
apOpenGLBuiltInUniformManager::apOpenGLBuiltInUniformManager()
{
    InitializeUniformsData();
}
apOpenGLBuiltInUniformManager::~apOpenGLBuiltInUniformManager()
{

}

void apOpenGLBuiltInUniformManager::InitializeUniformsData()
{
#define H_SPRSCR(s) L"<sup>" s L"</sup>"
#define H_SUBSCR(s) L"<sub>" s L"</sub>"
#define H_NL L"<br />"
#define N_SPRSCR(s) L" ^ " s
#define N_SUBSCR(s) L"[" s L"]"
#define N_NL L"\n"
    SetUniformData(ap_gl_NumSamples, L"gl_NumSamples",
                   L"Number of samples in the target framebuffer, or 1 if multisampling is disabled.",
                   L"Number of samples in the target framebuffer, or 1 if multisampling is disabled.");

    SetUniformData(ap_gl_ModelViewMatrix, L"gl_ModelViewMatrix",
                   L"GL_MODELVIEW_MATRIX",
                   L"GL_MODELVIEW_MATRIX");

    SetUniformData(ap_gl_ProjectionMatrix, L"gl_ProjectionMatrix",
                   L"GL_PROJECTION_MATRIX",
                   L"GL_PROJECTION_MATRIX");

    SetUniformData(ap_gl_ModelViewProjectionMatrix, L"gl_ModelViewProjectionMatrix",
                   L"GL_MODELVIEW_MATRIX * GL_PROJECTION_MATRIX",
                   L"GL_MODELVIEW_MATRIX * GL_PROJECTION_MATRIX");

    SetUniformData(ap_gl_TextureMatrix, L"gl_TextureMatrix",
                   L"GL_TEXTURE_MATRIX with GL_ACTIVE_TEXTURE == GL_TEXTUREi",
                   L"GL_TEXTURE_MATRIX with GL_ACTIVE_TEXTURE == GL_TEXTUREi");
    SetUniformMemberData(ap_gl_TextureMatrix_i, ap_gl_TextureMatrix,
                         L"GL_TEXTURE_MATRIX with GL_ACTIVE_TEXTURE == GL_TEXTURE%d",
                         L"GL_TEXTURE_MATRIX with GL_ACTIVE_TEXTURE == GL_TEXTURE%d");

    SetUniformData(ap_gl_NormalMatrix, L"gl_NormalMatrix",
                   L"(mat3(GL_MODELVIEW_MATRIX)" N_SPRSCR(L"-1") L")" N_SPRSCR(L"t"),
                   L"(mat3(GL_MODELVIEW_MATRIX)" H_SPRSCR(L"-1") L")" H_SPRSCR(L"t"));

    SetUniformData(ap_gl_ModelViewMatrixInverse, L"gl_ModelViewMatrixInverse",
                   L"GL_MODELVIEW_MATRIX" N_SPRSCR(L"-1"),
                   L"GL_MODELVIEW_MATRIX" H_SPRSCR(L"-1"));

    SetUniformData(ap_gl_ProjectionMatrixInverse, L"gl_ProjectionMatrixInverse",
                   L"GL_PROJECTION_MATRIX" N_SPRSCR(L"-1"),
                   L"GL_PROJECTION_MATRIX" H_SPRSCR(L"-1"));

    SetUniformData(ap_gl_ModelViewProjectionMatrixInverse, L"gl_ModelViewProjectionMatrixInverse",
                   L"(GL_MODELVIEW_MATRIX * GL_PROJECTION_MATRIX)" N_SPRSCR(L"-1"),
                   L"(GL_MODELVIEW_MATRIX * GL_PROJECTION_MATRIX)" H_SPRSCR(L"-1"));

    SetUniformData(ap_gl_TextureMatrixInverse, L"gl_TextureMatrixInverse",
                   L"GL_TEXTURE_MATRIX" N_SPRSCR(L"-1") L" with GL_ACTIVE_TEXTURE == GL_TEXTUREi",
                   L"GL_TEXTURE_MATRIX" H_SPRSCR(L"-1") L" with GL_ACTIVE_TEXTURE == GL_TEXTUREi");
    SetUniformMemberData(ap_gl_TextureMatrixInverse_i, ap_gl_TextureMatrixInverse,
                         L"GL_TEXTURE_MATRIX" N_SPRSCR(L"-1") L" with GL_ACTIVE_TEXTURE == GL_TEXTURE%d",
                         L"GL_TEXTURE_MATRIX" H_SPRSCR(L"-1") L" with GL_ACTIVE_TEXTURE == GL_TEXTURE%d");

    SetUniformData(ap_gl_ModelViewMatrixTranspose, L"gl_ModelViewMatrixTranspose",
                   L"GL_MODELVIEW_MATRIX" N_SPRSCR(L"t"),
                   L"GL_MODELVIEW_MATRIX" H_SPRSCR(L"t"));

    SetUniformData(ap_gl_ProjectionMatrixTranspose, L"gl_ProjectionMatrixTranspose",
                   L"GL_PROJECTION_MATRIX" N_SPRSCR(L"t"),
                   L"GL_PROJECTION_MATRIX" H_SPRSCR(L"t"));

    SetUniformData(ap_gl_ModelViewProjectionMatrixTranspose, L"gl_ModelViewProjectionMatrixTranspose",
                   L"(GL_MODELVIEW_MATRIX * GL_PROJECTION_MATRIX)" N_SPRSCR(L"t"),
                   L"(GL_MODELVIEW_MATRIX * GL_PROJECTION_MATRIX)" H_SPRSCR(L"t"));

    SetUniformData(ap_gl_TextureMatrixTranspose, L"gl_TextureMatrixTranspose",
                   L"GL_TEXTURE_MATRIX" N_SPRSCR(L"t") L" with GL_ACTIVE_TEXTURE == GL_TEXTUREi",
                   L"GL_TEXTURE_MATRIX" H_SPRSCR(L"t") L" with GL_ACTIVE_TEXTURE == GL_TEXTUREi");
    SetUniformMemberData(ap_gl_TextureMatrixTranspose_i, ap_gl_TextureMatrixTranspose,
                         L"GL_TEXTURE_MATRIX" N_SPRSCR(L"t") L" with GL_ACTIVE_TEXTURE == GL_TEXTURE%d",
                         L"GL_TEXTURE_MATRIX" H_SPRSCR(L"t") L" with GL_ACTIVE_TEXTURE == GL_TEXTURE%d");

    SetUniformData(ap_gl_ModelViewMatrixInverseTranspose, L"gl_ModelViewMatrixInverseTranspose",
                   L"(GL_MODELVIEW_MATRIX" N_SPRSCR(L"-1") L")" N_SPRSCR(L"t"),
                   L"(GL_MODELVIEW_MATRIX" H_SPRSCR(L"-1") L")" H_SPRSCR(L"t"));

    SetUniformData(ap_gl_ProjectionMatrixInverseTranspose, L"gl_ProjectionMatrixInverseTranspose",
                   L"(GL_PROJECTION_MATRIX" N_SPRSCR(L"-1") L")" N_SPRSCR(L"t"),
                   L"(GL_PROJECTION_MATRIX" H_SPRSCR(L"-1") L")" H_SPRSCR(L"t"));

    SetUniformData(ap_gl_ModelViewProjectionMatrixInverseTranspose, L"gl_ModelViewProjectionMatrixInverseTranspose",
                   L"((GL_MODELVIEW_MATRIX * GL_PROJECTION_MATRIX)" N_SPRSCR(L"-1") L")" N_SPRSCR(L"t"),
                   L"((GL_MODELVIEW_MATRIX * GL_PROJECTION_MATRIX)" H_SPRSCR(L"-1") L")" H_SPRSCR(L"t"));

    SetUniformData(ap_gl_TextureMatrixInverseTranspose, L"gl_TextureMatrixInverseTranspose",
                   L"(GL_TEXTURE_MATRIX" N_SPRSCR(L"-1") L")" N_SPRSCR(L"t") L" with GL_ACTIVE_TEXTURE == GL_TEXTUREi",
                   L"(GL_TEXTURE_MATRIX" H_SPRSCR(L"-1") L")" H_SPRSCR(L"t") L" with GL_ACTIVE_TEXTURE == GL_TEXTUREi");
    SetUniformMemberData(ap_gl_TextureMatrixInverseTranspose_i, ap_gl_TextureMatrixInverseTranspose,
                         L"(GL_TEXTURE_MATRIX" N_SPRSCR(L"-1") L")" N_SPRSCR(L"t") L" with GL_ACTIVE_TEXTURE == GL_TEXTURE%d",
                         L"(GL_TEXTURE_MATRIX" H_SPRSCR(L"-1") L")" H_SPRSCR(L"t") L" with GL_ACTIVE_TEXTURE == GL_TEXTURE%d");

    SetUniformData(ap_gl_NormalScale, L"gl_NormalScale",
                   L"1 / |GL_MODELVIEW_MATRIX|",
                   L"1 / |GL_MODELVIEW_MATRIX|");

    SetUniformData(ap_gl_DepthRange, L"gl_DepthRange",
                   L"near = GL_DEPTH_RANGE" N_SUBSCR(L"0") N_NL L"far = GL_DEPTH_RANGE" N_SUBSCR(L"1") N_NL L"diff = far - near",
                   L"near = GL_DEPTH_RANGE" H_SUBSCR(L"0") H_NL L"far = GL_DEPTH_RANGE" H_SUBSCR(L"1") H_NL L"diff = far - near");
    SetUniformMemberData(ap_gl_DepthRange_near, ap_gl_DepthRange, L"near",
                         L"GL_DEPTH_RANGE" N_SUBSCR(L"0"),
                         L"GL_DEPTH_RANGE" H_SUBSCR(L"0"));
    SetUniformMemberData(ap_gl_DepthRange_far, ap_gl_DepthRange, L"far",
                         L"GL_DEPTH_RANGE" N_SUBSCR(L"1"),
                         L"GL_DEPTH_RANGE" H_SUBSCR(L"1"));
    SetUniformMemberData(ap_gl_DepthRange_diff, ap_gl_DepthRange, L"diff",
                         L"GL_DEPTH_RANGE" N_SUBSCR(L"1") L" - GL_DEPTH_RANGE" N_SUBSCR(L"0"),
                         L"GL_DEPTH_RANGE" H_SUBSCR(L"1") L" - GL_DEPTH_RANGE" H_SUBSCR(L"0"));

    SetUniformData(ap_gl_ClipPlane, L"gl_ClipPlane",
                   L"GL_CLIP_PLANEi - equation",
                   L"GL_CLIP_PLANEi - equation");
    SetUniformMemberData(ap_gl_ClipPlane_i, ap_gl_ClipPlane,
                         L"GL_CLIP_PLANE%d - equation",
                         L"GL_CLIP_PLANE%d - equation");

    SetUniformData(ap_gl_Point, L"gl_Point",
                   L"size = GL_POINT_SIZE" N_NL L"sizeMin = GL_POINT_SIZE_MIN" N_NL L"sizeMax = GL_POINT_SIZE_MAX" N_NL L"fadeThresholdSize = GL_POINT_FADE_THRESHOLD_SIZE" N_NL L"distanceConstantAttenuation = GL_POINT_DISTANCE_ATTENUATION" N_SUBSCR(L"0") N_NL L"distanceLinearAttenuation = GL_POINT_DISTANCE_ATTENUATION" N_SUBSCR(L"1") N_NL L"distanceQuadraticAttenuation = GL_POINT_DISTANCE_ATTENUATION" N_SUBSCR(L"2"),
                   L"size = GL_POINT_SIZE" H_NL L"sizeMin = GL_POINT_SIZE_MIN" H_NL L"sizeMax = GL_POINT_SIZE_MAX" H_NL L"fadeThresholdSize = GL_POINT_FADE_THRESHOLD_SIZE" H_NL L"distanceConstantAttenuation = GL_POINT_DISTANCE_ATTENUATION" H_SUBSCR(L"0") H_NL L"distanceLinearAttenuation = GL_POINT_DISTANCE_ATTENUATION" H_SUBSCR(L"1") H_NL L"distanceQuadraticAttenuation = GL_POINT_DISTANCE_ATTENUATION" H_SUBSCR(L"2"));
    SetUniformMemberData(ap_gl_Point_size, ap_gl_Point, L"size",
                         L"GL_POINT_SIZE",
                         L"GL_POINT_SIZE");
    SetUniformMemberData(ap_gl_Point_sizeMin, ap_gl_Point, L"sizeMin",
                         L"GL_POINT_SIZE_MIN",
                         L"GL_POINT_SIZE_MIN");
    SetUniformMemberData(ap_gl_Point_sizeMax, ap_gl_Point, L"sizeMax",
                         L"GL_POINT_SIZE_MAX",
                         L"GL_POINT_SIZE_MAX");
    SetUniformMemberData(ap_gl_Point_fadeThresholdSize, ap_gl_Point, L"fadeThresholdSize",
                         L"GL_POINT_FADE_THRESHOLD_SIZE",
                         L"GL_POINT_FADE_THRESHOLD_SIZE");
    SetUniformMemberData(ap_gl_Point_distanceConstantAttenuation, ap_gl_Point, L"distanceConstantAttenuation",
                         L"GL_POINT_DISTANCE_ATTENUATION" N_SUBSCR(L"0"),
                         L"GL_POINT_DISTANCE_ATTENUATION" H_SUBSCR(L"0"));
    SetUniformMemberData(ap_gl_Point_distanceLinearAttenuation, ap_gl_Point, L"distanceLinearAttenuation",
                         L"GL_POINT_DISTANCE_ATTENUATION" N_SUBSCR(L"1"),
                         L"GL_POINT_DISTANCE_ATTENUATION" H_SUBSCR(L"1"));
    SetUniformMemberData(ap_gl_Point_distanceQuadraticAttenuation, ap_gl_Point, L"distanceQuadraticAttenuation",
                         L"GL_POINT_DISTANCE_ATTENUATION" N_SUBSCR(L"2"),
                         L"GL_POINT_DISTANCE_ATTENUATION" H_SUBSCR(L"2"));

    SetUniformData(ap_gl_FrontMaterial, L"gl_FrontMaterial",
                   L"emission = GL_EMISSION - front" N_NL L"ambient = GL_AMBIENT - front" N_NL L"diffuse = GL_DIFFUSE - front" N_NL L"specular = GL_SPECULAR - front" N_NL L"shininess = GL_SHININESS - front",
                   L"emission = GL_EMISSION - front" H_NL L"ambient = GL_AMBIENT - front" H_NL L"diffuse = GL_DIFFUSE - front" H_NL L"specular = GL_SPECULAR - front" H_NL L"shininess = GL_SHININESS - front");
    SetUniformMemberData(ap_gl_FrontMaterial_emission, ap_gl_FrontMaterial, L"emission",
                         L"GL_EMISSION - front",
                         L"GL_EMISSION - front");
    SetUniformMemberData(ap_gl_FrontMaterial_ambient, ap_gl_FrontMaterial, L"ambient",
                         L"GL_AMBIENT - front",
                         L"GL_AMBIENT - front");
    SetUniformMemberData(ap_gl_FrontMaterial_diffuse, ap_gl_FrontMaterial, L"diffuse",
                         L"GL_DIFFUSE - front",
                         L"GL_DIFFUSE - front");
    SetUniformMemberData(ap_gl_FrontMaterial_specular, ap_gl_FrontMaterial, L"specular",
                         L"GL_SPECULAR - front",
                         L"GL_SPECULAR - front");
    SetUniformMemberData(ap_gl_FrontMaterial_shininess, ap_gl_FrontMaterial, L"shininess",
                         L"GL_SHININESS - front",
                         L"GL_SHININESS - front");

    SetUniformData(ap_gl_BackMaterial, L"gl_BackMaterial",
                   L"emission = GL_EMISSION - back" N_NL L"ambient = GL_AMBIENT - back" N_NL L"diffuse = GL_DIFFUSE - back" N_NL L"specular = GL_SPECULAR - back" N_NL L"shininess = GL_SHININESS - back",
                   L"emission = GL_EMISSION - back" H_NL L"ambient = GL_AMBIENT - back" H_NL L"diffuse = GL_DIFFUSE - back" H_NL L"specular = GL_SPECULAR - back" H_NL L"shininess = GL_SHININESS - back"); \
    SetUniformMemberData(ap_gl_BackMaterial_emission, ap_gl_BackMaterial, L"emission",
                         L"GL_EMISSION - back",
                         L"GL_EMISSION - back");
    SetUniformMemberData(ap_gl_BackMaterial_ambient, ap_gl_BackMaterial, L"ambient",
                         L"GL_AMBIENT - back",
                         L"GL_AMBIENT - back");
    SetUniformMemberData(ap_gl_BackMaterial_diffuse, ap_gl_BackMaterial, L"diffuse",
                         L"GL_DIFFUSE - back",
                         L"GL_DIFFUSE - back");
    SetUniformMemberData(ap_gl_BackMaterial_specular, ap_gl_BackMaterial, L"specular",
                         L"GL_SPECULAR - back",
                         L"GL_SPECULAR - back");
    SetUniformMemberData(ap_gl_BackMaterial_shininess, ap_gl_BackMaterial, L"shininess",
                         L"GL_SHININESS - back",
                         L"GL_SHININESS - back");

    SetUniformData(ap_gl_LightSource, L"gl_LightSource",
                   L"ambient = GL_LIGHTi - ambient" N_NL L"diffuse = GL_LIGHTi - diffuse" N_NL L"specular = GL_LIGHTi - specular" N_NL L"position = GL_LIGHTi - position" N_NL L"halfVector = normalize(GL_LIGHTi - position + GL_MODELVIEW_MATRIX * {0, 0, 1, 0})" N_NL L"spotDirection = GL_LIGHTi - spot direction" N_NL L"spotExponent = GL_LIGHTi - spot exponent" L"spotCutoff = GL_LIGHTi - spot cutoff" N_NL L"spotCosCutoff = cos(GL_LIGHTi - spot cutoff)" N_NL L"constantAttenuation = GL_LIGHTi - constant attenuation" N_NL L"linearAttenuation = GL_LIGHTi - linear attenuation" N_NL L"quadraticAttenuation = GL_LIGHTi - quadratic attenuation",
                   L"ambient = GL_LIGHTi - ambient" H_NL L"diffuse = GL_LIGHTi - diffuse" H_NL L"specular = GL_LIGHTi - specular" H_NL L"position = GL_LIGHTi - position" H_NL L"halfVector = normalize(GL_LIGHTi - position + GL_MODELVIEW_MATRIX * {0, 0, 1, 0})" H_NL L"spotDirection = GL_LIGHTi - spot direction" H_NL L"spotExponent = GL_LIGHTi - spot exponent" L"spotCutoff = GL_LIGHTi - spot cutoff" H_NL L"spotCosCutoff = cos(GL_LIGHTi - spot cutoff)" H_NL L"constantAttenuation = GL_LIGHTi - constant attenuation" H_NL L"linearAttenuation = GL_LIGHTi - linear attenuation" H_NL L"quadraticAttenuation = GL_LIGHTi - quadratic attenuation");
    SetUniformMemberData(ap_gl_LightSource_i_ambient, ap_gl_LightSource, L"ambient",
                         L"GL_LIGHT%d - ambient",
                         L"GL_LIGHT%d - ambient");
    SetUniformMemberData(ap_gl_LightSource_i_diffuse, ap_gl_LightSource, L"diffuse",
                         L"GL_LIGHT%d - diffuse",
                         L"GL_LIGHT%d - diffuse");
    SetUniformMemberData(ap_gl_LightSource_i_specular, ap_gl_LightSource, L"specular",
                         L"GL_LIGHT%d - specular",
                         L"GL_LIGHT%d - specular");
    SetUniformMemberData(ap_gl_LightSource_i_position, ap_gl_LightSource, L"position",
                         L"GL_LIGHT%d - position",
                         L"GL_LIGHT%d - position");
    SetUniformMemberData(ap_gl_LightSource_i_halfVector, ap_gl_LightSource, L"halfVector",
                         L"normalize(GL_LIGHT%d - position + GL_MODELVIEW_MATRIX * {0, 0, 1, 0})",
                         L"normalize(GL_LIGHT%d - position + GL_MODELVIEW_MATRIX * {0, 0, 1, 0})");
    SetUniformMemberData(ap_gl_LightSource_i_spotDirection, ap_gl_LightSource, L"spotDirection",
                         L"GL_LIGHT%d - spot direction",
                         L"GL_LIGHT%d - spot direction");
    SetUniformMemberData(ap_gl_LightSource_i_spotExponent, ap_gl_LightSource, L"spotExponent",
                         L"GL_LIGHT%d - spot exponent",
                         L"GL_LIGHT%d - spot exponent");
    SetUniformMemberData(ap_gl_LightSource_i_spotCutoff, ap_gl_LightSource, L"spotCutoff",
                         L"GL_LIGHT%d - spot cutoff",
                         L"GL_LIGHT%d - spot cutoff");
    SetUniformMemberData(ap_gl_LightSource_i_spotCosCutoff, ap_gl_LightSource, L"spotCosCutoff",
                         L"cos(GL_LIGHT%d - spot cutoff)",
                         L"cos(GL_LIGHT%d - spot cutoff)");
    SetUniformMemberData(ap_gl_LightSource_i_constantAttenuation, ap_gl_LightSource, L"constantAttenuation",
                         L"GL_LIGHT%d - constant attenuation",
                         L"GL_LIGHT%d - constant attenuation");
    SetUniformMemberData(ap_gl_LightSource_i_linearAttenuation, ap_gl_LightSource, L"linearAttenuation",
                         L"GL_LIGHT%d - linear attenuation",
                         L"GL_LIGHT%d - linear attenuation");
    SetUniformMemberData(ap_gl_LightSource_i_quadraticAttenuation, ap_gl_LightSource, L"quadraticAttenuation",
                         L"GL_LIGHT%d - quadratic attenuation",
                         L"GL_LIGHT%d - quadratic attenuation");

    SetUniformData(ap_gl_LightModel, L"gl_LightModel",
                   L"ambient = GL_LIGHT_MODEL_AMBIENT",
                   L"ambient = GL_LIGHT_MODEL_AMBIENT");
    SetUniformMemberData(ap_gl_LightModel_ambient, ap_gl_LightModel, L"ambient",
                         L"GL_LIGHT_MODEL_AMBIENT",
                         L"GL_LIGHT_MODEL_AMBIENT");

    SetUniformData(ap_gl_FrontLightModelProduct, L"gl_FrontLightModelProduct",
                   L"sceneColor = (GL_EMISSION - front) + (GL_AMBIENT - front) * GL_LIGHT_MODEL_AMBIENT",
                   L"sceneColor = (GL_EMISSION - front) + (GL_AMBIENT - front) * GL_LIGHT_MODEL_AMBIENT");
    SetUniformMemberData(ap_gl_FrontLightModelProduct_sceneColor, ap_gl_FrontLightModelProduct, L"sceneColor",
                         L"(GL_EMISSION - front) + (GL_AMBIENT - front) * GL_LIGHT_MODEL_AMBIENT",
                         L"(GL_EMISSION - front) + (GL_AMBIENT - front) * GL_LIGHT_MODEL_AMBIENT");

    SetUniformData(ap_gl_BackLightModelProduct, L"gl_BackLightModelProduct",
                   L"sceneColor = (GL_EMISSION - back) + (GL_AMBIENT - back) * GL_LIGHT_MODEL_AMBIENT",
                   L"sceneColor = (GL_EMISSION - back) + (GL_AMBIENT - back) * GL_LIGHT_MODEL_AMBIENT");
    SetUniformMemberData(ap_gl_BackLightModelProduct_sceneColor, ap_gl_BackLightModelProduct, L"sceneColor",
                         L"(GL_EMISSION - back) + (GL_AMBIENT - back) * GL_LIGHT_MODEL_AMBIENT",
                         L"(GL_EMISSION - back) + (GL_AMBIENT - back) * GL_LIGHT_MODEL_AMBIENT");

    SetUniformData(ap_gl_FrontLightProduct, L"gl_FrontLightProduct",
                   L"ambient = (GL_AMBIENT - front) * (GL_LIGHTi - ambient)" N_NL L"diffuse = (GL_DIFFUSE - front) * (GL_LIGHTi - diffuse)" N_NL L"specular = (GL_SPECULAR - front) * (GL_LIGHTi - specular)",
                   L"ambient = (GL_AMBIENT - front) * (GL_LIGHTi - ambient)" H_NL L"diffuse = (GL_DIFFUSE - front) * (GL_LIGHTi - diffuse)" H_NL L"specular = (GL_SPECULAR - front) * (GL_LIGHTi - specular)");
    SetUniformMemberData(ap_gl_FrontLightProduct_i_ambient, ap_gl_FrontLightProduct, L"ambient",
                         L"(GL_AMBIENT - front) * (GL_LIGHT%d - ambient)",
                         L"(GL_AMBIENT - front) * (GL_LIGHT%d - ambient)");
    SetUniformMemberData(ap_gl_FrontLightProduct_i_diffuse, ap_gl_FrontLightProduct, L"diffuse",
                         L"(GL_DIFFUSE - front) * (GL_LIGHT%d - diffuse)",
                         L"(GL_DIFFUSE - front) * (GL_LIGHT%d - diffuse)");
    SetUniformMemberData(ap_gl_FrontLightProduct_i_specular, ap_gl_FrontLightProduct, L"specular",
                         L"(GL_SPECULAR - front) * (GL_LIGHT%d - specular)",
                         L"(GL_SPECULAR - front) * (GL_LIGHT%d - specular)");

    SetUniformData(ap_gl_BackLightProduct, L"gl_BackLightProduct",
                   L"ambient = (GL_AMBIENT - back) * (GL_LIGHTi - ambient)" N_NL L"diffuse = (GL_DIFFUSE - back) * (GL_LIGHTi - diffuse)" N_NL L"specular = (GL_SPECULAR - back) * (GL_LIGHTi - specular)",
                   L"ambient = (GL_AMBIENT - back) * (GL_LIGHTi - ambient)" H_NL L"diffuse = (GL_DIFFUSE - back) * (GL_LIGHTi - diffuse)" H_NL L"specular = (GL_SPECULAR - back) * (GL_LIGHTi - specular)");
    SetUniformMemberData(ap_gl_BackLightProduct_i_ambient, ap_gl_BackLightProduct, L"ambient",
                         L"(GL_AMBIENT - back) * (GL_LIGHT%d - ambient)",
                         L"(GL_AMBIENT - back) * (GL_LIGHT%d - ambient)");
    SetUniformMemberData(ap_gl_BackLightProduct_i_diffuse, ap_gl_BackLightProduct, L"diffuse",
                         L"(GL_DIFFUSE - back) * (GL_LIGHT%d - diffuse)",
                         L"(GL_DIFFUSE - back) * (GL_LIGHT%d - diffuse)");
    SetUniformMemberData(ap_gl_BackLightProduct_i_specular, ap_gl_BackLightProduct, L"specular",
                         L"(GL_SPECULAR - back) * (GL_LIGHT%d - specular)",
                         L"(GL_SPECULAR - back) * (GL_LIGHT%d - specular)");

    SetUniformData(ap_gl_TextureEnvColor, L"gl_TextureEnvColor",
                   L"GL_TEXTURE_ENV_COLOR with GL_ACTIVE_TEXTURE == GL_TEXTUREi",
                   L"GL_TEXTURE_ENV_COLOR with GL_ACTIVE_TEXTURE == GL_TEXTUREi");
    SetUniformMemberData(ap_gl_TextureEnvColor_i, ap_gl_TextureEnvColor,
                         L"GL_TEXTURE_ENV_COLOR with GL_ACTIVE_TEXTURE == GL_TEXTURE%d",
                         L"GL_TEXTURE_ENV_COLOR with GL_ACTIVE_TEXTURE == GL_TEXTURE%d");

    SetUniformData(ap_gl_EyePlaneS, L"gl_EyePlaneS",
                   L"GL_EYE_PLANE - s with GL_ACTIVE_TEXTURE == GL_TEXTUREi",
                   L"GL_EYE_PLANE - s with GL_ACTIVE_TEXTURE == GL_TEXTUREi");
    SetUniformMemberData(ap_gl_EyePlaneS_i, ap_gl_EyePlaneS,
                         L"GL_EYE_PLANE - s with GL_ACTIVE_TEXTURE == GL_TEXTURE%d",
                         L"GL_EYE_PLANE - s with GL_ACTIVE_TEXTURE == GL_TEXTURE%d");

    SetUniformData(ap_gl_EyePlaneT, L"gl_EyePlaneT",
                   L"GL_EYE_PLANE - t with GL_ACTIVE_TEXTURE == GL_TEXTUREi",
                   L"GL_EYE_PLANE - t with GL_ACTIVE_TEXTURE == GL_TEXTUREi");
    SetUniformMemberData(ap_gl_EyePlaneT_i, ap_gl_EyePlaneT,
                         L"GL_EYE_PLANE - t with GL_ACTIVE_TEXTURE == GL_TEXTURE%d",
                         L"GL_EYE_PLANE - t with GL_ACTIVE_TEXTURE == GL_TEXTURE%d");

    SetUniformData(ap_gl_EyePlaneR, L"gl_EyePlaneR",
                   L"GL_EYE_PLANE - r with GL_ACTIVE_TEXTURE == GL_TEXTUREi",
                   L"GL_EYE_PLANE - r with GL_ACTIVE_TEXTURE == GL_TEXTUREi");
    SetUniformMemberData(ap_gl_EyePlaneR_i, ap_gl_EyePlaneR,
                         L"GL_EYE_PLANE - r with GL_ACTIVE_TEXTURE == GL_TEXTURE%d",
                         L"GL_EYE_PLANE - r with GL_ACTIVE_TEXTURE == GL_TEXTURE%d");

    SetUniformData(ap_gl_EyePlaneQ, L"gl_EyePlaneQ",
                   L"GL_EYE_PLANE - q with GL_ACTIVE_TEXTURE == GL_TEXTUREi",
                   L"GL_EYE_PLANE - q with GL_ACTIVE_TEXTURE == GL_TEXTUREi");
    SetUniformMemberData(ap_gl_EyePlaneQ_i, ap_gl_EyePlaneQ,
                         L"GL_EYE_PLANE - q with GL_ACTIVE_TEXTURE == GL_TEXTURE%d",
                         L"GL_EYE_PLANE - q with GL_ACTIVE_TEXTURE == GL_TEXTURE%d");

    SetUniformData(ap_gl_ObjectPlaneS, L"gl_ObjectPlaneS",
                   L"GL_OBJECT_PLANE - s with GL_ACTIVE_TEXTURE == GL_TEXTUREi",
                   L"GL_OBJECT_PLANE - s with GL_ACTIVE_TEXTURE == GL_TEXTUREi");
    SetUniformMemberData(ap_gl_ObjectPlaneS_i, ap_gl_ObjectPlaneS,
                         L"GL_OBJECT_PLANE - s with GL_ACTIVE_TEXTURE == GL_TEXTURE%d",
                         L"GL_OBJECT_PLANE - s with GL_ACTIVE_TEXTURE == GL_TEXTURE%d");

    SetUniformData(ap_gl_ObjectPlaneT, L"gl_ObjectPlaneT",
                   L"GL_OBJECT_PLANE - t with GL_ACTIVE_TEXTURE == GL_TEXTUREi",
                   L"GL_OBJECT_PLANE - t with GL_ACTIVE_TEXTURE == GL_TEXTUREi");
    SetUniformMemberData(ap_gl_ObjectPlaneT_i, ap_gl_ObjectPlaneT,
                         L"GL_OBJECT_PLANE - t with GL_ACTIVE_TEXTURE == GL_TEXTURE%d",
                         L"GL_OBJECT_PLANE - t with GL_ACTIVE_TEXTURE == GL_TEXTURE%d");

    SetUniformData(ap_gl_ObjectPlaneR, L"gl_ObjectPlaneR",
                   L"GL_OBJECT_PLANE - r with GL_ACTIVE_TEXTURE == GL_TEXTUREi",
                   L"GL_OBJECT_PLANE - r with GL_ACTIVE_TEXTURE == GL_TEXTUREi");
    SetUniformMemberData(ap_gl_ObjectPlaneR_i, ap_gl_ObjectPlaneR,
                         L"GL_OBJECT_PLANE - r with GL_ACTIVE_TEXTURE == GL_TEXTURE%d",
                         L"GL_OBJECT_PLANE - r with GL_ACTIVE_TEXTURE == GL_TEXTURE%d");

    SetUniformData(ap_gl_ObjectPlaneQ, L"gl_ObjectPlaneQ",
                   L"GL_OBJECT_PLANE - q with GL_ACTIVE_TEXTURE == GL_TEXTUREi",
                   L"GL_OBJECT_PLANE - q with GL_ACTIVE_TEXTURE == GL_TEXTUREi");
    SetUniformMemberData(ap_gl_ObjectPlaneQ_i, ap_gl_ObjectPlaneQ,
                         L"GL_OBJECT_PLANE - q with GL_ACTIVE_TEXTURE == GL_TEXTURE%d",
                         L"GL_OBJECT_PLANE - q with GL_ACTIVE_TEXTURE == GL_TEXTURE%d");

    SetUniformData(ap_gl_Fog, L"gl_Fog",
                   L"color = GL_FOG_COLOR" N_NL L"density = GL_FOG_DENSITY" N_NL L"start = GL_FOG_START" N_NL L"end = GL_FOG_END" N_NL L"scale = 1 / (GL_FOG_END - GL_FOG_START)",
                   L"color = GL_FOG_COLOR" H_NL L"density = GL_FOG_DENSITY" H_NL L"start = GL_FOG_START" H_NL L"end = GL_FOG_END" H_NL L"scale = 1 / (GL_FOG_END - GL_FOG_START)");
    SetUniformMemberData(ap_gl_Fog_color, ap_gl_Fog, L"color",
                         L"GL_FOG_COLOR",
                         L"GL_FOG_COLOR");
    SetUniformMemberData(ap_gl_Fog_density, ap_gl_Fog, L"density",
                         L"GL_FOG_DENSITY",
                         L"GL_FOG_DENSITY");
    SetUniformMemberData(ap_gl_Fog_start, ap_gl_Fog, L"start",
                         L"GL_FOG_START",
                         L"GL_FOG_START");
    SetUniformMemberData(ap_gl_Fog_end, ap_gl_Fog, L"end",
                         L"GL_FOG_END",
                         L"GL_FOG_END");
    SetUniformMemberData(ap_gl_Fog_scale, ap_gl_Fog, L"scale",
                         L"1 / (GL_FOG_END - GL_FOG_START)",
                         L"1 / (GL_FOG_END - GL_FOG_START)");
    ;
#undef H_SPRSCR
#undef H_SUBSCR
#undef H_NL
#undef N_SPRSCR
#undef N_SUBSCR
#undef N_NL
}

void apOpenGLBuiltInUniformManager::SetUniformData(apBuiltInUniformId id, const gtString& name, const gtString& formula, const gtString& formulaHTML)
{
    GT_IF_WITH_ASSERT((0 <= id) && (ap_numberOfSupportedBuiltInUniforms > id))
    {
        apBuiltInUniformData& uniformData = m_uniformData[id];
        uniformData.m_name = name;
        uniformData.m_calculationFormula = formula;
        uniformData.m_calculationFormulaHTML = formulaHTML;
    }
}

void apOpenGLBuiltInUniformManager::SetUniformMemberData(apBuiltInUniformMemberId mid, apBuiltInUniformId id, const gtString& formula, const gtString& formulaHTML)
{
    SetUniformMemberData(mid, id, AP_INDEX_MEMBER_NAME, formula, formulaHTML);
}
void apOpenGLBuiltInUniformManager::SetUniformMemberData(apBuiltInUniformMemberId mid, apBuiltInUniformId id, const gtString& memberName, const gtString& formula, const gtString& formulaHTML)
{
    GT_IF_WITH_ASSERT((0 <= mid) && (ap_numberOfSupportedBuiltInUniformMembers > mid))
    {
        apBuiltInUniformData& memberData = m_uniformMemberData[mid];
        memberData.m_name = memberName;
        memberData.m_calculationFormula = formula;
        memberData.m_calculationFormulaHTML = formulaHTML;

        // Note the conditions are not the same, we allow id == ap_numberOfSupportedBuiltInUniforms (but don't register the member as a member):
        GT_ASSERT((0 <= id) && (ap_numberOfSupportedBuiltInUniforms >= id));

        if ((0 <= id) && (ap_numberOfSupportedBuiltInUniforms > id))
        {
            apBuiltInUniformDataWithMembers& uniformData = m_uniformData[id];
            uniformData.m_memberMapping[memberName] = mid;
        }
    }
}

const gtString& apOpenGLBuiltInUniformManager::UniformName(apBuiltInUniformId id) const
{
    static const gtString errString = AP_STR_NotAvailable;

    return ((0 <= id) && (ap_numberOfSupportedBuiltInUniforms > id)) ? m_uniformData[id].m_name : errString;
}
const gtString& apOpenGLBuiltInUniformManager::UniformCalculationFormula(apBuiltInUniformId id, bool htmlFormat) const
{
    static const gtString errString = AP_STR_NotAvailable;

    return ((0 <= id) && (ap_numberOfSupportedBuiltInUniforms > id)) ? (htmlFormat ? m_uniformData[id].m_calculationFormulaHTML : m_uniformData[id].m_calculationFormula) : errString;
}
const gtString& apOpenGLBuiltInUniformManager::UniformMemberCalculationFormula(apBuiltInUniformMemberId id, bool htmlFormat) const
{
    static const gtString errString = AP_STR_NotAvailable;

    return ((0 <= id) && (ap_numberOfSupportedBuiltInUniformMembers > id)) ? (htmlFormat ? m_uniformMemberData[id].m_calculationFormulaHTML : m_uniformMemberData[id].m_calculationFormula) : errString;
}

