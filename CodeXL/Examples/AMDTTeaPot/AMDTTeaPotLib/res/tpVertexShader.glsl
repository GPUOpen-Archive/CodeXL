//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpVertexShader.glsl 
/// 
//==================================================================================

//------------------------------ tpVertexShader.glsl ------------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

#version 120

// Will get the surface normal:
varying vec3 surfaceNormal;
varying vec3 surfaceNormalForGeom;

// Will get the vertex position:
varying vec3 vertexPosition;
varying vec3 vertexPositionForGeom;

uniform float spike;


// ---------------------------------------------------------------------------
// Name:        Teapot application vertex shader main function
// ---------------------------------------------------------------------------
void main(void)
{
    // Calculate the vertex position in eye coordinates:
    vertexPosition = vec3(gl_ModelViewMatrix * gl_Vertex);

    // Calculate the surface normal in eye coordinates:
    surfaceNormal = normalize(gl_NormalMatrix * gl_Normal);

    // Sends out the varying variables twice, this instance is only
    //  used by a geometry shader, if active:
    surfaceNormalForGeom = surfaceNormal;
    vertexPositionForGeom = vertexPosition;

    // Output texture unit 0 coordinates:
    gl_TexCoord[0] = gl_MultiTexCoord0;

    // Output the transformed vertex position:
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
