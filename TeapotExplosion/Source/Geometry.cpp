//
//  Geometry.cpp
//  VerizonTest
//
//  Created by James Folk on 6/27/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#include "Geometry.hpp"

#include <assert.h>
#include <iostream>

#include "Shader.hpp"
#include "Camera.hpp"
#include "Node.hpp"
//https://en.wikipedia.org/wiki/Wavefront_.obj_file#Material_template_library

namespace jamesfolk
{
    
    const unsigned int HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP = 0x38000000;
    const unsigned int HALF_FLOAT_MAX_BIASED_EXP_AS_SINGLE_FP_EXP = 0x47800000;
    const unsigned int FLOAT_MAX_BIASED_EXP = (0xFF << 23);
    const unsigned int HALF_FLOAT_MAX_BIASED_EXP = (0x1F << 10);
    
    static hfloat convertFloatToHFloat(float *f)
    {
        unsigned int x = *(unsigned int *)f;
        unsigned int sign = (unsigned short)(x >> 31);
        unsigned int mantissa;
        unsigned int exp;
        hfloat hf;
        
        //get mantissa
        mantissa = x & ((1<<23) - 1);
        //get exponent bits
        exp = x & FLOAT_MAX_BIASED_EXP;
        if(exp >= HALF_FLOAT_MAX_BIASED_EXP_AS_SINGLE_FP_EXP)
        {
            //check if the original single precision float number is a NaN
            if (mantissa && (exp == FLOAT_MAX_BIASED_EXP))
            {
                //we have  a single precision NaN
                mantissa = (1<<23) - 1;
            }
            else
            {
                // 16-bit half-float representation stores number as Inf;
                mantissa = 0;
            }
            hf = (((hfloat)sign) << 15) | (hfloat)(HALF_FLOAT_MAX_BIASED_EXP) | (hfloat)(mantissa >> 13);
        }
        //check if exponent is <= -15
        else if (exp <= HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP)
        {
            //store a denorm half-float value or zero
            exp = (HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP - exp) >> 23;
            mantissa >>= (14 + exp);
            
            hf = (((hfloat)sign) << 15) | (hfloat)(mantissa);
        }
        else
        {
            hf = (((hfloat)sign) << 15) |
            (hfloat)((exp - HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP) >> 13) |
            (hfloat)(mantissa >> 13);
        }
        return hf;
    }
    
    static float convertHFloatToFloat(hfloat hf)
    {
        unsigned int sign = (unsigned int)(hf >> 15);
        unsigned int mantissa = (unsigned int)(hf & ((1 << 10) - 1));
        unsigned int exp = (unsigned int)(hf & HALF_FLOAT_MAX_BIASED_EXP);
        unsigned int f;
        
        if(exp == HALF_FLOAT_MAX_BIASED_EXP)
        {
            /*
             we have a half-float NaN or Inf
             half-float NaNas will be converted to a single precision NaN
             half-float Infs will be converted to a single precision Inf
             */
            exp = FLOAT_MAX_BIASED_EXP;
            if(mantissa)
                mantissa = (1 << 23) - 1;
        }
        else if (exp == 0x0)
        {
            //convert half-float zero/denorm to single precision value
            if(mantissa)
            {
                mantissa <<= 1;
                exp = HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP;
                //check for leading 1 in denorm mantissa
                while ((mantissa & (1 << 10)) == 0)
                {
                    /*
                     for every leading 0, decrement single precision exponent by 1
                     and shift half-float mantissa value to the left
                     */
                    mantissa <<= 1;
                    exp -= (1 << 23);
                }
                //clamp the mantissa to 10-bits
                mantissa &= ((1 << 10) - 1);
                //shift left to generate single-precision mantissa of 23-bits
                mantissa <<= 13;
            }
        }
        else
        {
            //shift left to generate single-precision mantissa of 23-bits
            mantissa <<= 13;
            //generate single precision biased exponent value
            exp = (exp << 13) + HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP;
        }
        f = (sign << 31) | exp | mantissa;
        return *((float*)&f);
    }
    
    static const GLfptype TRANSFORM_IDENTITY_MATRIX[] =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
    
    static const GLfptype COLOR_IDENTITY_MATRIX[] =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 0,
    };
    
    
    Geometry::Geometry():
    m_MatrixBuffer(new GLfptype[16]),
    m_MatrixBufferFullSize(new float[16]),
    m_ModelViewTransformData(NULL),
//    m_ColorTransformData(NULL),
    m_NormalMatrixTransformData(NULL),
    m_VertexArray(0),
    m_ModelViewBuffer(0),
//    m_ColorTransformBuffer(0),
    m_NormalMatrixTransformBuffer(0),
    m_VerticesBuffer(0),
    m_IndexBuffer(0),
    m_NumberInstances(1),
    m_Shader(NULL),
    m_OpacityModifyRGB(false),
    m_VertexBufferChanged(true),
    m_NormalMatrixBufferChanged(true),
    m_ModelViewBufferChanged(true),
    m_ShaderChanged(true)
    {
        assert(m_MatrixBuffer);
        assert(m_MatrixBufferFullSize);
        
        m_References.resize(m_NumberInstances);
    }
    
    
    Geometry::~Geometry()
    {
        if(m_NormalMatrixTransformData)
            delete [] m_NormalMatrixTransformData;
        m_NormalMatrixTransformData = NULL;
        
//        if(m_ColorTransformData)
//            delete [] m_ColorTransformData;
//        m_ColorTransformData = NULL;
        
        if(m_ModelViewTransformData)
            delete [] m_ModelViewTransformData;
        m_ModelViewTransformData = NULL;
        
        delete [] m_MatrixBufferFullSize;
        m_MatrixBufferFullSize = NULL;
        
        delete [] m_MatrixBuffer;
        m_MatrixBuffer = NULL;
    }
    
    void Geometry::load(Shader *shader, const std::string &filecontent, unsigned int numInstances, MeshType type)
    {
        assert(shader);
        
        setShader(shader);
        
        m_NumberInstances = numInstances;
        m_References.resize(m_NumberInstances);
        
        loadData();
        
        assert(m_VertexArray == 0);
        glGenVertexArraysOES(1, &m_VertexArray);
        glBindVertexArrayOES(m_VertexArray);
        {
            {
                assert(m_ModelViewBuffer == 0);
                glGenBuffers(1, &m_ModelViewBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, m_ModelViewBuffer);
                glBufferData(GL_ARRAY_BUFFER, getModelViewTransformArrayBufferSize(), getModelViewTransformArrayBufferPtr(), GL_STREAM_DRAW);
                int inTransformAttrib = getShader()->getAttributeLocation("inTransform");
                glEnableVertexAttribArray(inTransformAttrib + 0);
                glEnableVertexAttribArray(inTransformAttrib + 1);
                glEnableVertexAttribArray(inTransformAttrib + 2);
                glEnableVertexAttribArray(inTransformAttrib + 3);
                glVertexAttribPointer(inTransformAttrib + 0, 4, GL_INDEX_TYPE, 0, sizeof(GLfptype) * 16, (GLvoid*)0);
                glVertexAttribPointer(inTransformAttrib + 1, 4, GL_INDEX_TYPE, 0, sizeof(GLfptype) * 16, (GLvoid*)16);
                glVertexAttribPointer(inTransformAttrib + 2, 4, GL_INDEX_TYPE, 0, sizeof(GLfptype) * 16, (GLvoid*)32);
                glVertexAttribPointer(inTransformAttrib + 3, 4, GL_INDEX_TYPE, 0, sizeof(GLfptype) * 16, (GLvoid*)48);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            
//            {
//                assert(m_ColorTransformBuffer == 0);
//                glGenBuffers(1, &m_ColorTransformBuffer);
//                glBindBuffer(GL_ARRAY_BUFFER, m_ColorTransformBuffer);
//                glBufferData(GL_ARRAY_BUFFER, getColorTransformArrayBufferSize(), getColorTransformArrayBufferPtr(), GL_STREAM_DRAW);
//                int inColorTransform = getShader()->getAttributeLocation("inColorTransform");
//                glEnableVertexAttribArray(inColorTransform + 0);
//                glEnableVertexAttribArray(inColorTransform + 1);
//                glEnableVertexAttribArray(inColorTransform + 2);
//                glEnableVertexAttribArray(inColorTransform + 3);
//                glVertexAttribPointer(inColorTransform + 0, 4, GL_INDEX_TYPE, 0, sizeof(GLfptype) * 16, (GLvoid*)0);
//                glVertexAttribPointer(inColorTransform + 1, 4, GL_INDEX_TYPE, 0, sizeof(GLfptype) * 16, (GLvoid*)16);
//                glVertexAttribPointer(inColorTransform + 2, 4, GL_INDEX_TYPE, 0, sizeof(GLfptype) * 16, (GLvoid*)32);
//                glVertexAttribPointer(inColorTransform + 3, 4, GL_INDEX_TYPE, 0, sizeof(GLfptype) * 16, (GLvoid*)48);
//                glBindBuffer(GL_ARRAY_BUFFER, 0);
//            }
            
            {
                assert(m_NormalMatrixTransformBuffer == 0);
                glGenBuffers(1, &m_NormalMatrixTransformBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, m_NormalMatrixTransformBuffer);
                glBufferData(GL_ARRAY_BUFFER, getNormalMatrixTransformArrayBufferSize(), getNormalMatrixTransformArrayBufferPtr(), GL_STREAM_DRAW);
                int inNormalMatrixAttrib = getShader()->getAttributeLocation("inNormalMatrix");
                glEnableVertexAttribArray(inNormalMatrixAttrib + 0);
                glEnableVertexAttribArray(inNormalMatrixAttrib + 1);
                glEnableVertexAttribArray(inNormalMatrixAttrib + 2);
                glEnableVertexAttribArray(inNormalMatrixAttrib + 3);
                glVertexAttribPointer(inNormalMatrixAttrib + 0, 4, GL_INDEX_TYPE, 0, sizeof(GLfptype) * 16, (GLvoid*)0);
                glVertexAttribPointer(inNormalMatrixAttrib + 1, 4, GL_INDEX_TYPE, 0, sizeof(GLfptype) * 16, (GLvoid*)16);
                glVertexAttribPointer(inNormalMatrixAttrib + 2, 4, GL_INDEX_TYPE, 0, sizeof(GLfptype) * 16, (GLvoid*)32);
                glVertexAttribPointer(inNormalMatrixAttrib + 3, 4, GL_INDEX_TYPE, 0, sizeof(GLfptype) * 16, (GLvoid*)48);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            {
                assert(m_VerticesBuffer == 0);
                glGenBuffers(1, &m_VerticesBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, m_VerticesBuffer);
                glBufferData(GL_ARRAY_BUFFER, getVertexArrayBufferSize(), getVertexArrayBufferPtr(), GL_STREAM_DRAW);
                int inPositionAttrib = getShader()->getAttributeLocation("inPosition");
                int inColorAttrib = getShader()->getAttributeLocation("inColor");
                int inNormalAttrib = getShader()->getAttributeLocation("inNormal");
//                int inOpacityAttrib = getShader()->getAttributeLocation("inOpacity");
//                int inHiddenAttrib = getShader()->getAttributeLocation("inHidden");
                int inTexCoordAttrib = getShader()->getAttributeLocation("inTexCoord");
                
                int inTangentAttrib = getShader()->getAttributeLocation("inTangent");
                int inBiTangentAttrib = getShader()->getAttributeLocation("inBiTangent");
                
                glEnableVertexAttribArray(inPositionAttrib);
                glVertexAttribPointer(inPositionAttrib,
                                      3,
                                      GL_INDEX_TYPE,
                                      GL_FALSE,
                                      sizeof(TexturedColoredVertex),
                                      (const GLvoid*) offsetof(TexturedColoredVertex, vertex));
                
                
                glEnableVertexAttribArray(inTexCoordAttrib);
                glVertexAttribPointer(inTexCoordAttrib,
                                      2,
                                      GL_INDEX_TYPE,
                                      GL_FALSE,
                                      sizeof(TexturedColoredVertex),
                                      (const GLvoid*) offsetof(TexturedColoredVertex, texture));
                
                glEnableVertexAttribArray(inNormalAttrib);
                glVertexAttribPointer(inNormalAttrib,
                                      3,
                                      GL_INDEX_TYPE,
                                      GL_FALSE,
                                      sizeof(TexturedColoredVertex),
                                      (const GLvoid*) offsetof(TexturedColoredVertex, normal));
                
                glEnableVertexAttribArray(inColorAttrib);
                glVertexAttribPointer(inColorAttrib,
                                      4,
                                      GL_INDEX_TYPE,
                                      GL_FALSE,
                                      sizeof(TexturedColoredVertex),
                                      (const GLvoid*) offsetof(TexturedColoredVertex, color));
                
                glEnableVertexAttribArray(inTangentAttrib);
                glVertexAttribPointer(inTangentAttrib,
                                      3,
                                      GL_INDEX_TYPE,
                                      GL_FALSE,
                                      sizeof(TexturedColoredVertex),
                                      (const GLvoid*) offsetof(TexturedColoredVertex, tangent));
                
                glEnableVertexAttribArray(inBiTangentAttrib);
                glVertexAttribPointer(inBiTangentAttrib,
                                      3,
                                      GL_INDEX_TYPE,
                                      GL_FALSE,
                                      sizeof(TexturedColoredVertex),
                                      (const GLvoid*) offsetof(TexturedColoredVertex, bitangent));
                
//                glEnableVertexAttribArray(inOpacityAttrib);
//                glVertexAttribPointer(inOpacityAttrib,
//                                      1,
//                                      GL_INDEX_TYPE,
//                                      GL_FALSE,
//                                      sizeof(TexturedColoredVertex),
//                                      (const GLvoid*)offsetof(TexturedColoredVertex, opacity));
                
//                glEnableVertexAttribArray(inHiddenAttrib);
//                glVertexAttribPointer(inHiddenAttrib,
//                                      1,
//                                      GL_INDEX_TYPE,
//                                      GL_FALSE,
//                                      sizeof(TexturedColoredVertex),
//                                      (const GLvoid*)offsetof(TexturedColoredVertex, hidden));
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            
            {
                glGenBuffers(1, &m_IndexBuffer);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, getElementArrayBufferSize(), getElementArrayBufferPtr(), GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }
        }
        glBindVertexArrayOES(0);
    }
    
    void Geometry::unLoad()
    {
        if (m_IndexBuffer)
            glDeleteBuffers(1, &m_IndexBuffer);
        m_IndexBuffer = 0;
        
        if (m_VerticesBuffer)
            glDeleteBuffers(1, &m_VerticesBuffer);
        m_VerticesBuffer = 0;
        
        if (m_NormalMatrixTransformBuffer)
            glDeleteBuffers(1, &m_NormalMatrixTransformBuffer);
        m_NormalMatrixTransformBuffer = 0;
        
//        if (m_ColorTransformBuffer)
//            glDeleteBuffers(1, &m_ColorTransformBuffer);
//        m_ColorTransformBuffer = 0;
        
        if (m_ModelViewBuffer)
            glDeleteBuffers(1, &m_ModelViewBuffer);
        m_ModelViewBuffer = 0;
        
        if (m_VertexArray)
            glDeleteVertexArraysOES(1, &m_VertexArray);
        m_VertexArray = 0;
    }
    
    bool Geometry::isLoaded()const
    {
        return false;
    }
    
    Shader *const Geometry::getShader()
    {
        return m_Shader;
    }
    
    void Geometry::setShader(Shader *const shader)
    {
        m_Shader = shader;
        m_ShaderChanged = true;
    }
    
    void Geometry::render(Camera *camera)
    {
        Shader *shader = getShader();
        if(shader && camera)
        {
            assert(shader->use());
            
            camera->render(shader, m_ShaderChanged);
            
            struct LightSourceParameters
            {
                btVector4 ambient;
                btVector4 diffuse;
                btVector4 specular;
                btVector4 position;
                btVector4 halfVector;
                btVector3 spotDirection;
                float spotExponent;
                float spotCutoff;
                float spotCosCutoff;
                float constantAttenuation;
                float linearAttenuation;
                float quadraticAttenuation;
            };
            
            struct MaterialParameters
            {
                btVector4 emission;
                btVector4 ambient;
                btVector4 diffuse;
                btVector4 specular;
                float shininess;
            };
            
            
//            shader->setUniformValue("RimLightColor", btVector3(0.25008f, 0.250f, 0.8712f));
            shader->setUniformValue("RimLightColor", btVector3(1.0f, 1.0f, 1.0f));
            shader->setUniformValue("RimLightStart", 0.0f);
            shader->setUniformValue("RimLightEnd", 1.0f);
            shader->setUniformValue("RimLightCoefficient", 0.6f);
            /*
             uniform float RimLightStart;
             uniform float RimLightEnd;
             uniform float RimLightCoefficient;
             */
            
            shader->setUniformValue("LightSourceAmbientColor", btVector3(1.0f, 1.0f, 1.0f));
            shader->setUniformValue("LightSourceDiffuseColor", btVector3(1.0f, 1.0f, 1.0f));
            shader->setUniformValue("LightSourceSpecularColor", btVector3(1.0f, 1.0f, 1.0f));
            
            //set LightSourcePosition_worldspace.w == 0 for DirectionalLight
            //set LightSourcePosition_worldspace.w != 0 for PointlLight
            shader->setUniformValue("LightSourcePosition_worldspace", btVector4(0.0f, 0.0f, -1.0f, 1.0));
            
            shader->setUniformValue("LightSourceSpotDirection", btVector3(0.0f, 0.0f, 1.0f));
            shader->setUniformValue("LightSourceSpotExponent", 100.0f);
            //set LightSourceSpotCutoff != 180 for SpotLight
            shader->setUniformValue("LightSourceSpotCutoff", 180.0f);
            shader->setUniformValue("LightSourceSpotCosCutoff", 30.0f);
            //http://www.ogre3d.org/tikiwiki/-Point+Light+Attenuation
            
//             Range Constant Linear Quadratic
//             3250, 1.0, 0.0014, 0.000007
//            shader->setUniformValue("LightSourceConstantAttenuation", 1.0f);
//            shader->setUniformValue("LightSourceLinearAttenuation", 0.0014f);
//            shader->setUniformValue("LightSourceQuadraticAttenuation", 0.000007f);
            
//             600, 1.0, 0.007, 0.0002
//            shader->setUniformValue("LightSourceConstantAttenuation", 1.0f);
//            shader->setUniformValue("LightSourceLinearAttenuation", 0.007f);
//            shader->setUniformValue("LightSourceQuadraticAttenuation", 0.0002f);
            
            
//             325, 1.0, 0.014, 0.0007
//            shader->setUniformValue("LightSourceConstantAttenuation", 1.0f);
//            shader->setUniformValue("LightSourceLinearAttenuation", 0.014f);
//            shader->setUniformValue("LightSourceQuadraticAttenuation", 0.0007f);
            
//             200, 1.0, 0.022, 0.0019
//            shader->setUniformValue("LightSourceConstantAttenuation", 1.0f);
//            shader->setUniformValue("LightSourceLinearAttenuation", 0.022f);
//            shader->setUniformValue("LightSourceQuadraticAttenuation", 0.0019f);

            
//             160, 1.0, 0.027, 0.0028
//            shader->setUniformValue("LightSourceConstantAttenuation", 1.0f);
//            shader->setUniformValue("LightSourceLinearAttenuation", 0.027f);
//            shader->setUniformValue("LightSourceQuadraticAttenuation", 0.0028f);
            
//             100, 1.0, 0.045, 0.0075
            shader->setUniformValue("LightSourceConstantAttenuation", 1.0f);
            shader->setUniformValue("LightSourceLinearAttenuation", 0.045f);
            shader->setUniformValue("LightSourceQuadraticAttenuation", 0.0075f);
            
//             65, 1.0, 0.07, 0.017
//             50, 1.0, 0.09, 0.032
//             32, 1.0, 0.14, 0.07
//             20, 1.0, 0.22, 0.20
//             13, 1.0, 0.35, 0.44
//             7, 1.0, 0.7, 1.8
//            shader->setUniformValue("LightSourceConstantAttenuation", 1.0f);
//            shader->setUniformValue("LightSourceLinearAttenuation", 0.7f);
//            shader->setUniformValue("LightSourceQuadraticAttenuation", 1.8f);
            
            
            
            shader->setUniformValue("LightAmbientColor", btVector3(1.0f, 1.0f, 1.0f));
            
            shader->setUniformValue("MaterialShininess", 1000.0f);
            
            shader->setUniformValue("FogMaxDistance", 10.0f);
            shader->setUniformValue("FogMinDistance", 0.1f);
            shader->setUniformValue("FogColor", btVector3(0.7f, 0.7f, 0.7f));
            shader->setUniformValue("FogDensity", 0.000000001f);
            
            m_ShaderChanged = false;
            
            glBindVertexArrayOES(m_VertexArray);
            
            if(isModelViewBufferChanged())
            {
                glBindBuffer(GL_ARRAY_BUFFER, m_ModelViewBuffer);
                glBufferSubData(GL_ARRAY_BUFFER, 0, getModelViewTransformArrayBufferSize(), getModelViewTransformArrayBufferPtr());
                enableModelViewBufferChanged(false);
            }
            
//            glBindBuffer(GL_ARRAY_BUFFER, m_ColorTransformBuffer);
//            glBufferSubData(GL_ARRAY_BUFFER, 0, getColorTransformArrayBufferSize(), getColorTransformArrayBufferPtr());
//            glBindBuffer(GL_ARRAY_BUFFER, 0);
            
            if(isNormalMatrixBufferChanged())
            {
                glBindBuffer(GL_ARRAY_BUFFER, m_NormalMatrixTransformBuffer);
                glBufferSubData(GL_ARRAY_BUFFER, 0, getNormalMatrixTransformArrayBufferSize(), getNormalMatrixTransformArrayBufferPtr());
                enableNormalMatrixBufferChanged(false);
            }
            
            if(isVertexArrayBufferChanged())
            {
                glBindBuffer(GL_ARRAY_BUFFER, m_VerticesBuffer);
                glBufferSubData(GL_ARRAY_BUFFER, 0, getVertexArrayBufferSize(), getVertexArrayBufferPtr());
                enableVertexArrayBufferChanged(false);
            }
            
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
            
            glDrawElements(GL_TRIANGLES, numberOfInstances() * numberOfIndices(), getElementIndexType(), (const GLvoid*)0);
            
//            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//            glBindVertexArrayOES(0);
        }
    }
    
    GLsizei Geometry::numberOfInstances()const
    {
        return m_NumberInstances;
    }
    
    unsigned long Geometry::getGeometryIndex(Node *const node)const
    {
        return node->getGeometryIndex();
    }
    
    void computeTangentBasis(
                             // inputs
                             const std::vector<btVector3> & vertices,
                             const std::vector<btVector2> & uvs,
                             const std::vector<btVector3> & normals,
                             // outputs
                             std::vector<btVector3> & tangents,
                             std::vector<btVector3> & bitangents
                             )
    {
        
        tangents.clear();
        bitangents.clear();
        tangents.resize(vertices.size());
        bitangents.resize(vertices.size());
        
        for (unsigned int i=0; i<vertices.size(); i+=3 )
        {
            
            // Shortcuts for vertices
            btVector3  v0 = vertices[i+0];
            btVector3  v1 = vertices[i+1];
            btVector3  v2 = vertices[i+2];
            
            // Shortcuts for UVs
            btVector2  uv0 = uvs[i+0];
            btVector2  uv1 = uvs[i+1];
            btVector2  uv2 = uvs[i+2];
            
            // Edges of the triangle : postion delta
            btVector3 deltaPos1 = v1-v0;
            btVector3 deltaPos2 = v2-v0;
            
            // UV delta
            btVector2 deltaUV1 = uv1-uv0;
            btVector2 deltaUV2 = uv2-uv0;
            
            float r = 1.0f / (deltaUV1.x() * deltaUV2.y() - deltaUV1.y() * deltaUV2.x());
            btVector3 tangent = (deltaPos1 * deltaUV2.y()   - deltaPos2 * deltaUV1.y())*r;
            btVector3 bitangent = (deltaPos2 * deltaUV1.x()   - deltaPos1 * deltaUV2.x())*r;
            
            // Set the same tangent for all three vertices of the triangle.
            // They will be merged later, in vboindexer.cpp
            tangents[i] = tangent;
            tangents[i+1] = tangent;
            tangents[i+2] = tangent;
            
            // Same thing for binormals
            bitangents[i] = bitangent;
            bitangents[i+1] = bitangent;
            bitangents[i+2] = bitangent;
            
        }
        
        // See "Going Further"
        for (unsigned int i=0; i<vertices.size(); i+=1 )
        {
            btVector3  n = normals[i];
            btVector3  t = tangents[i];
            btVector3  b = bitangents[i];
            
            
            // Gram-Schmidt orthogonalize
            t = (t - n * n.dot(t)).normalized();
            
            
            // Calculate handedness
            if (n.cross(t).dot(b) < 0.0f)
            {
                t = t * -1.0f;
            }
            
        }
        
        
    }
    
    const void *Geometry::getModelViewTransformArrayBufferPtr()const
    {
        assert(m_ModelViewTransformData);
        
        return m_ModelViewTransformData;
    }
    
    GLsizeiptr Geometry::getModelViewTransformArrayBufferSize()const
    {
        GLsizeiptr size = sizeof(GLfptype) * numberOfInstances() * numberOfVertices() * 16;
        return size;
    }
    
    bool Geometry::isModelViewBufferChanged()const
    {
        return m_ModelViewBufferChanged;
    }
    
    void Geometry::enableModelViewBufferChanged(bool changed)
    {
        m_ModelViewBufferChanged = changed;
    }
    
//    const void *Geometry::getColorTransformArrayBufferPtr()const
//    {
//        assert(m_ColorTransformData);
//        
//        return m_ColorTransformData;
//    }
//    
//    GLsizeiptr Geometry::getColorTransformArrayBufferSize()const
//    {
//        GLsizeiptr size = sizeof(GLfptype) * numberOfInstances() * numberOfVertices() * 16;
//        return size;
//    }
    
    const void *Geometry::getNormalMatrixTransformArrayBufferPtr()const
    {
        assert(m_NormalMatrixTransformData);
        
        return m_NormalMatrixTransformData;
    }
    
    GLsizeiptr Geometry::getNormalMatrixTransformArrayBufferSize()const
    {
        GLsizeiptr size = sizeof(GLfptype) * numberOfInstances() * numberOfVertices() * 16;
        return size;
    }
    
    bool Geometry::isNormalMatrixBufferChanged()const
    {
        return m_NormalMatrixBufferChanged;
    }
    
    void Geometry::enableNormalMatrixBufferChanged(bool changed)
    {
        m_NormalMatrixBufferChanged = changed;
    }
    
    void Geometry::loadData()
    {
        unLoadData();
        
        m_ModelViewTransformData = new GLfptype[numberOfInstances() * numberOfVertices() * 16];
//        m_ColorTransformData = new GLfptype[numberOfInstances() * numberOfVertices() * 16];
        m_NormalMatrixTransformData = new GLfptype[numberOfInstances() * numberOfVertices() * 16];
        enableNormalMatrixBufferChanged(true);
        
        assert(m_ModelViewTransformData);
//        assert(m_ColorTransformData);
        assert(m_NormalMatrixTransformData);
        
        unsigned long i;
        
        for (i = 0;
             i < (numberOfInstances() * numberOfVertices() * 16);
             i += 16)
            memcpy(m_ModelViewTransformData + i, TRANSFORM_IDENTITY_MATRIX, sizeof(TRANSFORM_IDENTITY_MATRIX));
        
//        for (i = 0;
//             i < (numberOfInstances() * numberOfVertices() * 16);
//             i += 16)
//            memcpy(m_ColorTransformData + i, COLOR_IDENTITY_MATRIX, sizeof(COLOR_IDENTITY_MATRIX));
        
        for (i = 0;
             i < (numberOfInstances() * numberOfVertices() * 16);
             i += 16)
            memcpy(m_NormalMatrixTransformData + i, TRANSFORM_IDENTITY_MATRIX, sizeof(TRANSFORM_IDENTITY_MATRIX));
        
        enableModelViewBufferChanged(true);
        enableNormalMatrixBufferChanged(true);
    }
    
    void Geometry::unLoadData()
    {
        if(m_NormalMatrixTransformData)
            delete [] m_NormalMatrixTransformData;
        m_NormalMatrixTransformData = NULL;
        
//        if(m_ColorTransformData)
//            delete [] m_ColorTransformData;
//        m_ColorTransformData = NULL;
        
        if(m_ModelViewTransformData)
            delete [] m_ModelViewTransformData;
        m_ModelViewTransformData = NULL;
    }
    
    bool Geometry::isVertexArrayBufferChanged()const
    {
        return m_VertexBufferChanged;
    }
    
    void Geometry::enableVertexArrayBufferChanged(bool changed)
    {
        m_VertexBufferChanged = changed;
    }
    
    void Geometry::addReference(Node *node)
    {
        for (unsigned long i = 0; i < m_References.size(); ++i)
        {
            if (!m_References[i])
            {
                m_References[i] = true;
                node->setGeometryIndex(i);
                return;
            }
        }
    }
    
    void Geometry::removeReference(Node *node)
    {
        unsigned long index = getGeometryIndex(node);
        
        if(index < m_References.size())
        {
            m_References[index] = false;
            
            setHidden(node);
        }
    }
    
    void Geometry::setTransform(const unsigned long index, const btTransform &transform)
    {
        if (index < numberOfInstances())
        {
            const unsigned long STRIDE = 16 * numberOfVertices();
            
#ifdef USE_HALF_FLOAT
            transform.getOpenGLMatrix(m_MatrixBufferFullSize);
            for (unsigned long i = 0; i < 16; i++)
                m_MatrixBuffer[i] = convertFloatToHFloat(&m_MatrixBufferFullSize[i]);
#else
            transform.getOpenGLMatrix(m_MatrixBuffer);
#endif
            
            for (int currentVertex = 0; currentVertex < numberOfVertices(); currentVertex++)
            {
                unsigned long p = ((index * STRIDE) + (16 * currentVertex));
                int cmp = memcmp(m_ModelViewTransformData + p,
                                 m_MatrixBuffer,
                                 sizeof(GLfptype) * 16);
                
                if(0 != cmp)
                {
                    memcpy(m_ModelViewTransformData + p, m_MatrixBuffer, sizeof(GLfptype) * 16);
                }
            }
            enableModelViewBufferChanged(true);
        }
    }
    
    btTransform Geometry::getTransform(const unsigned long index)
    {
        btTransform transform(btTransform::getIdentity());
        if (index < numberOfInstances())
        {
            const unsigned long STRIDE = 16 * numberOfVertices();
            
            for (int currentVertex = 0; currentVertex < numberOfVertices(); currentVertex++)
            {
                unsigned long p = ((index * STRIDE) + (16 * currentVertex));
                memcpy(m_MatrixBuffer,
                       m_ModelViewTransformData + p,
                       sizeof(GLfptype) * 16);
            }
            
#ifdef USE_HALF_FLOAT
            for (unsigned long i = 0; i < 16; i++)
                m_MatrixBufferFullSize[i] = convertHFloatToFloat(m_MatrixBuffer[i]);
            transform.setFromOpenGLMatrix(m_MatrixBufferFullSize);
#else
            transform.setFromOpenGLMatrix(m_MatrixBuffer);
#endif
        }
        return transform;
    }
    
//    void Geometry::setColorTransform(const unsigned long index, const btTransform &transform)
//    {
//        if (index < numberOfInstances())
//        {
//            const unsigned long STRIDE = 16 * numberOfVertices();
//            
//            transform.getOpenGLMatrix(m_MatrixBuffer);
//            
//            for (int currentVertex = 0; currentVertex < numberOfVertices(); currentVertex++)
//            {
//                unsigned long p = ((index * STRIDE) + (16 * currentVertex));
//                
//                int cmp = memcmp(m_ColorTransformData + p,
//                                 m_MatrixBuffer,
//                                 sizeof(GLfptype) * 16);
//                
//                if(0 != cmp)
//                {
//                    memcpy(m_ColorTransformData + p,
//                           m_MatrixBuffer,
//                           sizeof(GLfptype) * 16);
//                }
//            }
//        }
//    }
//    
//    btTransform Geometry::getColorTransform(const unsigned long index)
//    {
//        btTransform transform(btTransform::getIdentity());
//        if (index < numberOfInstances())
//        {
//            const unsigned long STRIDE = 16 * numberOfVertices();
//            
//            for (int currentVertex = 0; currentVertex < numberOfVertices(); currentVertex++)
//            {
//                unsigned long p = ((index * STRIDE) + (16 * currentVertex));
//                memcpy(m_MatrixBuffer,
//                       m_ColorTransformData + p,
//                       sizeof(GLfptype) * 16);
//            }
//            
//            transform.setFromOpenGLMatrix(m_MatrixBuffer);
//        }
//        return transform;
//    }
    
    void Geometry::setNormalMatrixTransform(const unsigned long index, const btTransform &transform)
    {
        if (index < numberOfInstances())
        {
            const unsigned long STRIDE = 16 * numberOfVertices();
            
#ifdef USE_HALF_FLOAT
            transform.getOpenGLMatrix(m_MatrixBufferFullSize);
            for (unsigned long i = 0; i < 16; i++)
                m_MatrixBuffer[i] = convertFloatToHFloat(&m_MatrixBufferFullSize[i]);
#else
            transform.getOpenGLMatrix(m_MatrixBuffer);
#endif
            
            for (int currentVertex = 0; currentVertex < numberOfVertices(); currentVertex++)
            {
                unsigned long p = ((index * STRIDE) + (16 * currentVertex));
                
                int cmp = memcmp(m_NormalMatrixTransformData + p,
                                 m_MatrixBuffer,
                                 sizeof(GLfptype) * 16);
                
                if(0 != cmp)
                {
                    memcpy(m_NormalMatrixTransformData + p,
                           m_MatrixBuffer,
                           sizeof(GLfptype) * 16);
                }
            }
            enableNormalMatrixBufferChanged(true);
        }
    }
    
    btTransform Geometry::getNormalMatrixTransform(const unsigned long index)
    {
        btTransform transform(btTransform::getIdentity());
        if (index < numberOfInstances())
        {
            const unsigned long STRIDE = 16 * numberOfVertices();
            
            for (int currentVertex = 0; currentVertex < numberOfVertices(); currentVertex++)
            {
                unsigned long p = ((index * STRIDE) + (16 * currentVertex));
                memcpy(m_MatrixBuffer,
                       m_NormalMatrixTransformData + p,
                       sizeof(GLfptype) * 16);
            }
            
#ifdef USE_HALF_FLOAT
            for (unsigned long i = 0; i < 16; i++)
                m_MatrixBufferFullSize[i] = convertHFloatToFloat(m_MatrixBuffer[i]);
            transform.setFromOpenGLMatrix(m_MatrixBufferFullSize);
#else
            transform.setFromOpenGLMatrix(m_MatrixBuffer);
#endif
            
        }
        return transform;
    }
}
