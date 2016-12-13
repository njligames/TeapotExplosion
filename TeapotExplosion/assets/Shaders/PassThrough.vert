#version 100

#ifdef GL_ES
precision highp float;
#endif

attribute vec3 inPosition;
attribute vec2 inTexCoord;
attribute vec3 inNormal;
attribute vec4 inColor;
attribute mat4 inTransform;
attribute mat4 inNormalMatrix;
//attribute mat4 inColorTransform;

attribute vec3 inTangent;
attribute vec3 inBiTangent;

varying vec2 VertexUV_modelspace;
varying vec3 Position_worldspace;
varying vec3 Normal_modelspace;
varying vec3 Tangent_modelspace;
varying vec3 Bitangent_modelspace;
varying vec3 EyeDirection_cameraspace;
varying vec4 Vertex_color;
varying vec3 VertexPosition_cameraspace;
varying vec3 EyePosition_worldspace;

uniform mat4 modelView;
uniform mat4 projection;

void main ()
{
    gl_PointSize = 50.0;
    
    mat4 normalMatrix = inNormalMatrix;
    mat4 meshTransform = inTransform;
    mat4 ViewTransform = modelView;
    
    vec3 tangent = inTangent;
    vec3 bitangent = inBiTangent;
    
    vec3 vertexPosition_modelspace = inPosition;
    vec3 vertexNormal_modelspace = inNormal;
    vec3 vertexTangent_modelspace = inTangent;
    vec3 vertexBitangent_modelspace = inBiTangent;
    
    VertexUV_modelspace = inTexCoord;
    Vertex_color = inColor;
    
    // Output position of the vertex, in clip space : MVP * position
    gl_Position = (((ViewTransform * projection) * meshTransform) * vec4(vertexPosition_modelspace, 1.0));
    
//    // Position of the vertex, in worldspace : meshTransform * position
//    Position_worldspace = (meshTransform * vec4(vertexPosition_modelspace, 1.0)).xyz;
//    
//    // Vector that goes from the vertex to the camera, in camera space.
//    // In camera space, the camera is at the origin (0,0,0).
//    VertexPosition_cameraspace = ( ViewTransform * meshTransform * vec4(vertexPosition_modelspace, 1.0)).xyz;
//    
////    mat3 rotMat = mat3(ViewTransform);
////    vec3 d = vec3(ViewTransform[3]);
////    vec3 eye = -d * rotMat;
//    
//    EyePosition_worldspace = -ViewTransform[3].xyz / ViewTransform[3].w;
//    EyeDirection_cameraspace = normalize(VertexPosition_cameraspace - EyePosition_worldspace);
//    
//    // Normal of the the vertex, in camera space
////    Tangent_modelspace   = normalize( normalMatrix * vertexTangent_modelspace );
////    Bitangent_modelspace = normalize( normalMatrix * vertexBitangent_modelspace );
////    Normal_modelspace    = normalize( normalMatrix * vertexNormal_modelspace ); // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
//    
//    Tangent_modelspace    = normalize( normalMatrix * vec4(vertexTangent_modelspace,    1.0)).xyz;
//    Bitangent_modelspace    = normalize( normalMatrix * vec4(vertexBitangent_modelspace,    1.0)).xyz;
//    Normal_modelspace    = normalize( normalMatrix * vec4(vertexNormal_modelspace,    1.0)).xyz;
    
    
    
}
