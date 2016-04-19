//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpFragmentShader.glsl 
/// 
//==================================================================================

//------------------------------ tpFragmentShader.glsl ------------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------
#version 120

// Controls texture influence [0,1]:
uniform float textureInfluence;

// Allows access to our texture:
uniform sampler2D textureSampler;

// Contains the surface normal:
varying vec3 surfaceNormal;

// Contains the vertex interpulated position:
varying vec3 vertexPosition;


// ---------------------------------------------------------------------------
// Name:        Teapot application fragment shader main function
// ---------------------------------------------------------------------------
void main(void)
{
    // Calculate the Phong model eye, L and R vectors:
    vec3 eyeVec = normalize(-vertexPosition);
    vec3 L = normalize(gl_LightSource[0].position.xyz - vertexPosition);
    vec3 R = normalize(-reflect(L, surfaceNormal));

    // Calculate ambient term:
    vec4 ambientTerm = gl_FrontLightProduct[0].ambient;

    // Calculate diffuse term:
    vec4 diffuseTerm = vec4(0.0, 0.0, 0.0, 1.0);
    float dotSurfaceNormalAndL = dot(surfaceNormal, L);

    if (dotSurfaceNormalAndL > 0.0)
    {
        diffuseTerm = gl_FrontLightProduct[0].diffuse * max(dotSurfaceNormalAndL, 0.0);
    }

    // Calculate specular term:
    vec4 specularTerm = vec4(0.0, 0.0, 0.0, 1.0);
    float dotRAndEyeVec = dot(R, eyeVec);

    if (dotRAndEyeVec > 0.0)
    {
        specularTerm = gl_FrontLightProduct[0].specular * pow(max(dotRAndEyeVec, 0.0), 0.3 * gl_FrontMaterial.shininess);
    }

    // Get the texel color:
    vec4 texelColor = texture2D(textureSampler, gl_TexCoord[0].st);

    // Apply alpha component on the texture color:
    float alpha = texelColor.a;
    vec4 newTexelColor = texelColor * alpha;
    newTexelColor.a = 1.0;

    // Mix texture color and ambient color:
    vec4 texAndAmbientColor = ((1.0 - textureInfluence) * ambientTerm) + (textureInfluence * newTexelColor);

    // Instead of adding to the diffuse (orange) color, replace it:
    diffuseTerm *= (1.0 - (textureInfluence * alpha));

    // Calculate final color:
    vec4 finalColor = gl_FrontLightModelProduct.sceneColor + texAndAmbientColor + diffuseTerm + specularTerm;

    // Write final color:
    gl_FragColor = finalColor;

    // For debugging - hard coded "gold" color:
    // gl_FragColor = vec4(0.79, 0.59, 0.2, 0);

    // New final color calculation:
    // gl_FragColor = finalColor + vec4(0, 0.5, 0.1, 0);
}
