#version 100

#ifdef GL_ES
precision highp float;
#endif

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

uniform vec3 RimLightColor;
uniform float RimLightStart;
uniform float RimLightEnd;
uniform float RimLightCoefficient;

uniform vec4 LightSourcePosition_worldspace;
uniform vec3 LightSourceAmbientColor;
uniform vec3 LightSourceDiffuseColor;
uniform vec3 LightSourceSpecularColor;

uniform vec3 LightSourceSpotDirection;
uniform float LightSourceSpotExponent;
uniform float LightSourceSpotCutoff;
uniform float LightSourceSpotCosCutoff;
uniform float LightSourceConstantAttenuation;
uniform float LightSourceLinearAttenuation;
uniform float LightSourceQuadraticAttenuation;
uniform vec3 LightAmbientColor;

uniform float MaterialShininess;

uniform float FogMaxDistance;
uniform float FogMinDistance;
uniform vec3 FogColor;
uniform float FogDensity;

vec3 computeRim(vec3 color, float start, float end, float coef)
{
//    EyePosition_worldspace
//    EyeDirection_cameraspace, Normal_modelspace
    vec3 normal = normalize(Normal_modelspace);
    vec3 eye = normalize(-EyeDirection_cameraspace);
    float rim = smoothstep(start, end, 1.0 - dot(normal, eye));
    return clamp(rim, 0.0, 1.0) * coef * color;
}

vec4 computeLinearFogColor(in vec4 baseColor)
{
    mat4 ViewTransform = modelView;
    
    float viewDistance = sqrt((Position_worldspace.x - EyePosition_worldspace.x) *
                             (Position_worldspace.x - EyePosition_worldspace.x) +
                             (Position_worldspace.y - EyePosition_worldspace.y) *
                             (Position_worldspace.y - EyePosition_worldspace.y) +
                             (Position_worldspace.z - EyePosition_worldspace.z) *
                             (Position_worldspace.z - EyePosition_worldspace.z));
    
    // 20 - fog starts; 80 - fog ends
    float fogFactor = (FogMaxDistance - viewDistance)/(FogMaxDistance - FogMinDistance);
    fogFactor = clamp( fogFactor, 0.0, 1.0 );
    
    //if you inverse color in glsl mix function you have to
    //put 1.0 - fogFactor
    return mix(vec4(FogColor, 1.0), baseColor, fogFactor);
}

vec4 computeExponentialFogColor(in vec4 baseColor)
{
    mat4 ViewTransform = modelView;
    
    float viewDistance = sqrt((Position_worldspace.x - EyePosition_worldspace.x) *
                             (Position_worldspace.x - EyePosition_worldspace.x) +
                             (Position_worldspace.y - EyePosition_worldspace.y) *
                             (Position_worldspace.y - EyePosition_worldspace.y) +
                             (Position_worldspace.z - EyePosition_worldspace.z) *
                             (Position_worldspace.z - EyePosition_worldspace.z));
    
    float fogFactor = 1.0 /exp(viewDistance * FogDensity);
    fogFactor = clamp( fogFactor, 0.0, 1.0 );
    
    return mix(vec4(FogColor, 1.0), baseColor, fogFactor);
}

vec4 computeFog(in vec4 baseColor)
{
    mat4 ViewTransform = modelView;
    
    float viewDistance = sqrt((Position_worldspace.x - EyePosition_worldspace.x) *
                              (Position_worldspace.x - EyePosition_worldspace.x) +
                              (Position_worldspace.y - EyePosition_worldspace.y) *
                              (Position_worldspace.y - EyePosition_worldspace.y) +
                              (Position_worldspace.z - EyePosition_worldspace.z) *
                              (Position_worldspace.z - EyePosition_worldspace.z));
    
    float fogFactor = 1.0 /exp( (viewDistance * FogDensity) * (viewDistance * FogDensity));
    fogFactor = clamp( fogFactor, 0.0, 1.0 );
    
    return mix(vec4(FogColor, 1.0), baseColor, fogFactor);
}





struct LightSourceParameters
{
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 halfVector;
    vec3 spotDirection;
    float spotExponent;
    float spotCutoff;
    float spotCosCutoff;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
};

struct MaterialParameters
{
    vec4 emission;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
};





















//https://gist.github.com/patriciogonzalezvivo/3a81453a24a542aabc63

//#define EMISSION_TEXTURE 1
//#define AMBIENT_TEXTURE 1
//#define DIFFUSE_TEXTURE 1
//#define SPECULAR_TEXTURE 1
//#define ALPHA_TEXTURE 1
//#define NORMAL_TEXTURE 1


#ifdef EMISSION_TEXTURE
uniform sampler2D tEmissionColor;
#endif

#ifdef AMBIENT_TEXTURE
uniform sampler2D tAmbientColor;
#endif

#ifdef DIFFUSE_TEXTURE
uniform sampler2D tDiffuseColor;
#endif

#ifdef SPECULAR_TEXTURE
uniform sampler2D tSpecularColor;
#endif

#ifdef ALPHA_TEXTURE
uniform sampler2D tAlpha;
#endif

#ifdef NORMAL_TEXTURE
uniform sampler2D tNormal;
#endif


mat3 transpose(mat3 m)
{
    return mat3(  m[0][0], m[1][0], m[2][0],  // new col 0
                m[0][1], m[1][1], m[2][1],  // new col 1
                m[0][2], m[1][2], m[2][2]   // new col 1
                );
}

void processNormalTexture(in vec3 lightPosition,
                          inout vec3 lightDirection_tangentspace,
                          inout vec3 eyeDirection_tangentspace,
                          inout vec3 textureNormal_tangentspace)
{
    mat4 ViewTransform = modelView;
    
#ifdef NORMAL_TEXTURE
    textureNormal_tangentspace = texture( tNormal, vec2(VertexUV_modelspace.x,-VertexUV_modelspace.y) );
#else
    textureNormal_tangentspace = vec3(0.0, 0.0, -1.0);
#endif
    
    textureNormal_tangentspace = normalize(textureNormal_tangentspace.rgb * 2.0 - 1.0);
    
    vec3 lightPosition_cameraspace = ( ViewTransform * vec4(lightPosition, 0.0)).xyz;
    vec3 lightDirection_cameraspace = normalize( lightPosition_cameraspace + EyeDirection_cameraspace );
    
    mat3 TBN = transpose(mat3(
                              Tangent_modelspace,
                              Bitangent_modelspace,
                              Normal_modelspace
                              )); // You can use dot products instead of building this matrix and transposing it. See References for details.
    
    lightDirection_tangentspace = TBN * lightDirection_cameraspace;
    eyeDirection_tangentspace =  TBN * EyeDirection_cameraspace;
    
}

void DirectionalLight(in float materialShininess,
                      in LightSourceParameters lightSource,
                      in vec3 normal,
                      in vec3 lightDirection_tangentspace,
                      in vec3 eyeDirection_tangentspace,
                      in vec3 textureNormal_tangentspace,
                      inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
    float nDotVP;         // normal . light direction
    float nDotHV;         // normal . light half vector
    float pf;             // power factor
    
#ifdef NORMAL_TEXTURE
    nDotVP = clamp(max(0.0, dot(lightDirection_tangentspace, normalize(vec3(lightSource.position)))), 0.0, 1.0);
    nDotHV = clamp(max(0.0, dot(eyeDirection_tangentspace, vec3(lightSource.halfVector))), 0.0, 1.0);
#else
    nDotVP = clamp(max(0.0, dot(normal, normalize(vec3(lightSource.position)))), 0.0, 1.0);
    nDotHV = clamp(max(0.0, dot(normal, vec3(lightSource.halfVector))), 0.0, 1.0);
#endif
    
    if (nDotVP == 0.0)
        pf = 0.0;
    else
        pf = pow(nDotHV, materialShininess);
    
    ambient  += lightSource.ambient;
    diffuse  += lightSource.diffuse * nDotVP;
    specular += lightSource.specular * pf;
}

void PointLight(in float materialShininess,
                in LightSourceParameters lightSource,
                in vec3 eye,
                in vec3 ecPosition3,
                in vec3 normal,
                in vec3 lightDirection_tangentspace,
                in vec3 eyeDirection_tangentspace,
                in vec3 textureNormal_tangentspace,
                inout vec4 ambient,
                inout vec4 diffuse,
                inout vec4 specular)
{
    float nDotVP;         // normal . light direction
    float nDotHV;         // normal . light half vector
    float pf;             // power factor
    float attenuation;    // computed attenuation factor
    float d;              // distance from surface to light source
    vec3  VP;             // direction from surface to light position
    vec3  halfVector;     // direction of maximum highlights
    
    // Compute vector from surface to light position
    VP = vec3(lightSource.position) - ecPosition3;
    
    // Compute distance between surface and light position
    d = length(VP);
    
    // Normalize the vector from surface to light position
    VP = normalize(VP);
    
    // Compute attenuation
    attenuation = 1.0 / (lightSource.constantAttenuation +
                         lightSource.linearAttenuation * d +
                         lightSource.quadraticAttenuation * d * d);
    
    halfVector = normalize(VP + eye);

#ifdef NORMAL_TEXTURE
    // Normal of the computed fragment, in camera space
    vec3 n = textureNormal_tangentspace;
    // Direction of the light (from the fragment to the light)
    vec3 l = normalize(lightDirection_tangentspace);
    // Cosine of the angle between the normal and the light direction,
    // clamped above 0
    //  - light is at the vertical of the triangle -> 1
    //  - light is perpendicular to the triangle -> 0
    //  - light is behind the triangle -> 0
    nDotVP = clamp( dot( n,l ), 0.0 , 1.0 );
    
    // Eye vector (towards the camera)
    vec3 E = normalize(eyeDirection_tangentspace);
    // Direction in which the triangle reflects the light
    vec3 R = reflect(-l,n);
    // Cosine of the angle between the Eye vector and the Reflect vector,
    // clamped to 0
    //  - Looking into the reflection -> 1
    //  - Looking elsewhere -> < 1
    nDotHV = clamp( dot( E, R ), 0.0, 1.0 );
#else
    nDotVP = clamp(max(0.0, dot(normal, VP)), 0.0, 1.0);
    nDotHV = clamp(max(0.0, dot(normal, halfVector)), 0.0, 1.0);
#endif
    
    
    
    
    if (nDotVP == 0.0)
        pf = 0.0;
    else
        pf = pow(nDotHV, materialShininess);
    
//    if (nDotVP > 0.95)
//        nDotVP = 0.95;
//    else if (nDotVP > 0.5)
//        nDotVP = 0.5;
//    else if (nDotVP > 0.25)
//        nDotVP = 0.25;
    
    ambient  += lightSource.ambient * attenuation;
    diffuse  += lightSource.diffuse * nDotVP * attenuation;
    specular += lightSource.specular * pf * attenuation;
}

void SpotLight(in float materialShininess,
                in LightSourceParameters lightSource,
                in vec3 eye, in vec3 ecPosition3, in vec3 normal, in vec3 lightDirection_tangentspace, in vec3 eyeDirection_tangentspace, in vec3 textureNormal_tangentspace, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
    float nDotVP;           // normal . light direction
    float nDotHV;           // normal . light half vector
    float pf;               // power factor
    float spotDot;          // cosine of angle between spotlight
    float spotAttenuation;  // spotlight attenuation factor
    float attenuation;      // computed attenuation factor
    float d;                // distance from surface to light source
    vec3 VP;                // direction from surface to light position
    vec3 halfVector;        // direction of maximum highlights
    
    // Compute vector from surface to light position
    VP = vec3(lightSource.position) - ecPosition3;
    
    // Compute distance between surface and light position
    d = length(VP);
    
    // Normalize the vector from surface to light position
    VP = normalize(VP);
    
    // Compute attenuation
    attenuation = 1.0 / (lightSource.constantAttenuation +
                         lightSource.linearAttenuation * d +
                         lightSource.quadraticAttenuation * d * d);
    
    // See if point on surface is inside cone of illumination
    spotDot = dot(-VP, normalize(lightSource.spotDirection));
    
    if (spotDot < lightSource.spotCosCutoff)
        spotAttenuation = 0.0; // light adds no contribution
    else
        spotAttenuation = pow(spotDot, lightSource.spotExponent);
    
    // Combine the spotlight and distance attenuation.
    attenuation *= spotAttenuation;
    
    halfVector = normalize(VP + eye);
    
#ifdef NORMAL_TEXTURE
    nDotVP = clamp(max(0.0, dot(lightDirection_tangentspace, VP)), 0.0, 1.0);
    nDotHV = clamp(max(0.0, dot(eyeDirection_tangentspace, halfVector)), 0.0, 1.0);
#else
    nDotVP = clamp(max(0.0, dot(normal, VP)), 0.0, 1.0);
    nDotHV = clamp(max(0.0, dot(normal, halfVector)), 0.0, 1.0);
#endif
    
    if (nDotVP == 0.0)
        pf = 0.0;
    else
        pf = pow(nDotHV, materialShininess);
    
    ambient  += lightSource.ambient * attenuation;
    diffuse  += lightSource.diffuse * nDotVP * attenuation;
    specular += lightSource.specular * pf * attenuation;
}

vec4 calc_lighting_color(in MaterialParameters material,
                         in LightSourceParameters lightSource[1],
                         in vec4 lightAmbient,
                         in vec3 _ecPosition,
                         in vec3 _normal,
                         in vec3 _lightDirection_tangentspace,
                         in vec3 _eyeDirection_tangentspace,
                         in vec3 _textureNormal_tangentspace)
{
    mat4 ViewTransform = modelView;
    
    // Clear the light intensity accumulators
    vec4 amb  = vec4(0.0);
    vec4 diff = vec4(0.0);
    vec4 spec = vec4(0.0);
    
    // Loop through enabled lights, compute contribution from each
    for (int i = 0; i < 1; i++)
    {
        if (lightSource[i].spotCutoff == 180.0)
        {
            if (lightSource[i].position.w == 0.0)
            {
                DirectionalLight(material.shininess,
                                 lightSource[i],
                                 normalize(_normal),
                                 _lightDirection_tangentspace,
                                 _eyeDirection_tangentspace,
                                 _textureNormal_tangentspace,
                                 amb,
                                 diff,
                                 spec);
            }
            else
            {
                PointLight(material.shininess,
                           lightSource[i],
                           EyePosition_worldspace,
                           _ecPosition,
                           normalize(_normal),
                           _lightDirection_tangentspace,
                           _eyeDirection_tangentspace,
                           _textureNormal_tangentspace,
                           amb,
                           diff,
                           spec);
            }
        }
        else
        {
            SpotLight(material.shininess,
                      lightSource[i],
                      EyePosition_worldspace,
                      _ecPosition,
                      normalize(_normal),
                      _lightDirection_tangentspace,
                      _eyeDirection_tangentspace,
                      _textureNormal_tangentspace,
                      amb,
                      diff,
                      spec);
        }
    }
    
    
    // NOTE: gl_FrontLightModelProduct.sceneColor = gl_FrontMaterial.emission + gl_FrontMaterial.ambient * gl_LightModel.ambient
    vec4 sceneColor = material.emission * material.ambient * lightAmbient;
    
//    vec4 color =  gl_FrontLightModelProduct.sceneColor +
    vec4 color =  sceneColor +
    amb * material.ambient +
    diff * material.diffuse +
    spec * material.specular;
    
#ifdef ALPHA_TEXTURE
    color.a = texture2D(tAlpha, VertexUV_modelspace).a;
#endif
    
    return color;
}

void main()
{
    MaterialParameters material;
    LightSourceParameters lightSource[1];
    
#ifdef EMISSION_TEXTURE
    material.emission    = texture2D(tEmissionColor, VertexUV_modelspace);
#else
    material.emission    = vec4(1.0, 1.0, 1.0, 1.0);
#endif
    
#ifdef AMBIENT_TEXTURE
    material.ambient     = texture2D(tAmbientColor, VertexUV_modelspace);
#else
    material.ambient     = vec4(0.0, 0.0, 0.0, 1.0);
#endif
    
#ifdef DIFFUSE_TEXTURE
    material.diffuse     = texture2D(tDiffuseColor, VertexUV_modelspace);
#else
    material.diffuse     = vec4(1.0, 1.0, 1.0, 1.0);
#endif
    
#ifdef SPECULAR_TEXTURE
    material.specular    = texture2D(tSpecularColor, VertexUV_modelspace);
#else
    material.specular    = vec4(1.0, 1.0, 1.0, 1.0);
#endif
    
    material.shininess  = MaterialShininess;
    
    vec4 lightAmbientColor  = vec4(LightAmbientColor, 1.0);
    
    //Lights...
    //http://www.glprogramming.com/red/chapter05.html
    lightSource[0].position = LightSourcePosition_worldspace;
    
    lightSource[0].ambient  = vec4(LightSourceAmbientColor, 1.0);
    lightSource[0].diffuse  = vec4(LightSourceDiffuseColor, 1.0);
    lightSource[0].specular = vec4(LightSourceSpecularColor, 1.0);
    
    mat4 ViewTransform = modelView;
    
    // Vector that goes from the vertex to the light, in camera space. meshTransform is ommited because it's identity.
    vec3 lightPosition_cameraspace = ( ViewTransform * vec4(lightSource[0].position.xyz, 1.0)).xyz;
    vec3 lightDirection_cameraspace = normalize( lightPosition_cameraspace + EyeDirection_cameraspace );
    
    vec3 lightDirection_tangentspace;
    vec3 eyeDirection_tangentspace;
    vec3 textureNormal_tangentspace;
    
    processNormalTexture(lightSource[0].position.xyz,
                         lightDirection_tangentspace,
                         eyeDirection_tangentspace,
                         textureNormal_tangentspace);
    
    // Direction of the light (from the fragment to the light)
    
//    vec3 P = VertexPosition_cameraspace;
//    vec3 L = normalize(vec3(lightSource[0].position) - P);
//    vec3 V = normalize(EyePosition_worldspace - P);
//    vec3 H = L + V;
//    lightSource[0].halfVector = vec4(H, 0.0);
    lightSource[0].halfVector = vec4(reflect(lightDirection_cameraspace, Normal_modelspace), 1.0);
    
    lightSource[0].spotDirection = LightSourceSpotDirection;
    lightSource[0].spotExponent = LightSourceSpotExponent;
    lightSource[0].spotCutoff = LightSourceSpotCutoff;
    lightSource[0].spotCosCutoff = LightSourceSpotCosCutoff;
    lightSource[0].constantAttenuation = LightSourceConstantAttenuation;
    lightSource[0].linearAttenuation = LightSourceLinearAttenuation;
    lightSource[0].quadraticAttenuation = LightSourceQuadraticAttenuation;
    
    vec4 color = calc_lighting_color(material,
                                     lightSource,
                                     lightAmbientColor,
                                     Position_worldspace,
                                     Normal_modelspace,
                                     lightDirection_tangentspace,
                                     eyeDirection_tangentspace,
                                     textureNormal_tangentspace);
    
    vec4 baseColor = color * Vertex_color;
    
    
    
//    vec3 rimColor = computeRim(RimLightColor, 0.0, 1.0, 0.4);
    vec3 rimColor = computeRim(RimLightColor, RimLightStart, RimLightEnd, RimLightCoefficient);
    baseColor += vec4(rimColor, 0.0);
    
//    float fresnel = pow(1.0 - abs( dot( EyeDirection_cameraspace, Normal_modelspace) ), 4.0);
//    
//    baseColor += (vec4(RimLightColor, 1.0) * fresnel);

    
    if(FogDensity > 0.0)
    {
        baseColor = computeExponentialFogColor(baseColor);
    }
    else
    {
        baseColor = computeLinearFogColor(baseColor);
    }
    
    gl_FragColor = baseColor;
    
//    gl_FragColor = vec4(Normal_modelspace, 1.0);
    
//    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0) + vec4(rimColor, 1.0);
}