//
//  Geometry.cpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#include "Geometry.hpp"

#include <assert.h>
#include <iostream>

#include "Shader.hpp"
#include "Camera.hpp"
#include "Node.hpp"

namespace jamesfolk
{
    
    Geometry::Geometry():
    m_MatrixBuffer(new GLfloat[16]),
    m_MatrixBufferFullSize(new float[16]),
    m_ModelViewTransformData(NULL),
    m_NormalMatrixTransformData(NULL),
    m_VertexArray(0),
    m_ModelViewBuffer(0),
    m_NormalMatrixTransformBuffer(0),
    m_VerticesBuffer(0),
    m_IndexBuffer(0),
    m_NumberInstances(1),
    m_NumberSubDivisions(1),
    m_ExtraSubdivisionBuffer(1),
    m_Shader(NULL),
    m_OpacityModifyRGB(false),
    m_ElementBufferChanged(true),
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
        
        if(m_ModelViewTransformData)
            delete [] m_ModelViewTransformData;
        m_ModelViewTransformData = NULL;
        
        delete [] m_MatrixBufferFullSize;
        m_MatrixBufferFullSize = NULL;
        
        delete [] m_MatrixBuffer;
        m_MatrixBuffer = NULL;
    }
    
    void Geometry::load(Shader *shader, const std::string &filecontent, unsigned int numInstances, unsigned int numSubDivisions)
    {
        assert(shader);
        
        setShader(shader);
        
        m_NumberInstances = numInstances;
        m_NumberSubDivisions = numSubDivisions;
        for (int i = 0; i < m_NumberSubDivisions; i++)
            m_ExtraSubdivisionBuffer *= 4;
        
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
                glBufferData(GL_ARRAY_BUFFER, getModelViewTransformArrayBufferSize() * subdivisionBufferSize(), getModelViewTransformArrayBufferPtr(), GL_STREAM_DRAW);
                int inTransformAttrib = getShader()->getAttributeLocation("inTransform");
                glEnableVertexAttribArray(inTransformAttrib + 0);
                glEnableVertexAttribArray(inTransformAttrib + 1);
                glEnableVertexAttribArray(inTransformAttrib + 2);
                glEnableVertexAttribArray(inTransformAttrib + 3);
                glVertexAttribPointer(inTransformAttrib + 0, 4, GL_FLOAT, 0, sizeof(GLfloat) * 16, (GLvoid*)0);
                glVertexAttribPointer(inTransformAttrib + 1, 4, GL_FLOAT, 0, sizeof(GLfloat) * 16, (GLvoid*)16);
                glVertexAttribPointer(inTransformAttrib + 2, 4, GL_FLOAT, 0, sizeof(GLfloat) * 16, (GLvoid*)32);
                glVertexAttribPointer(inTransformAttrib + 3, 4, GL_FLOAT, 0, sizeof(GLfloat) * 16, (GLvoid*)48);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            
            {
                assert(m_NormalMatrixTransformBuffer == 0);
                glGenBuffers(1, &m_NormalMatrixTransformBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, m_NormalMatrixTransformBuffer);
                glBufferData(GL_ARRAY_BUFFER, getNormalMatrixTransformArrayBufferSize() * subdivisionBufferSize(), getNormalMatrixTransformArrayBufferPtr(), GL_STREAM_DRAW);
                int inNormalMatrixAttrib = getShader()->getAttributeLocation("inNormalMatrix");
                glEnableVertexAttribArray(inNormalMatrixAttrib + 0);
                glEnableVertexAttribArray(inNormalMatrixAttrib + 1);
                glEnableVertexAttribArray(inNormalMatrixAttrib + 2);
                glEnableVertexAttribArray(inNormalMatrixAttrib + 3);
                glVertexAttribPointer(inNormalMatrixAttrib + 0, 4, GL_FLOAT, 0, sizeof(GLfloat) * 16, (GLvoid*)0);
                glVertexAttribPointer(inNormalMatrixAttrib + 1, 4, GL_FLOAT, 0, sizeof(GLfloat) * 16, (GLvoid*)16);
                glVertexAttribPointer(inNormalMatrixAttrib + 2, 4, GL_FLOAT, 0, sizeof(GLfloat) * 16, (GLvoid*)32);
                glVertexAttribPointer(inNormalMatrixAttrib + 3, 4, GL_FLOAT, 0, sizeof(GLfloat) * 16, (GLvoid*)48);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            
            {
                assert(m_VerticesBuffer == 0);
                glGenBuffers(1, &m_VerticesBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, m_VerticesBuffer);
                glBufferData(GL_ARRAY_BUFFER, getVertexArrayBufferSize() * subdivisionBufferSize(), getVertexArrayBufferPtr(), GL_STREAM_DRAW);
                int inPositionAttrib = getShader()->getAttributeLocation("inPosition");
                int inColorAttrib = getShader()->getAttributeLocation("inColor");
                int inNormalAttrib = getShader()->getAttributeLocation("inNormal");
                int inTexCoordAttrib = getShader()->getAttributeLocation("inTexCoord");
                
                int inTangentAttrib = getShader()->getAttributeLocation("inTangent");
                int inBiTangentAttrib = getShader()->getAttributeLocation("inBiTangent");
                
                glEnableVertexAttribArray(inPositionAttrib);
                glVertexAttribPointer(inPositionAttrib,
                                      3,
                                      GL_FLOAT,
                                      GL_FALSE,
                                      sizeof(TexturedColoredVertex),
                                      (const GLvoid*) offsetof(TexturedColoredVertex, vertex));
                
                
                glEnableVertexAttribArray(inTexCoordAttrib);
                glVertexAttribPointer(inTexCoordAttrib,
                                      2,
                                      GL_FLOAT,
                                      GL_FALSE,
                                      sizeof(TexturedColoredVertex),
                                      (const GLvoid*) offsetof(TexturedColoredVertex, texture));
                
                glEnableVertexAttribArray(inNormalAttrib);
                glVertexAttribPointer(inNormalAttrib,
                                      3,
                                      GL_FLOAT,
                                      GL_FALSE,
                                      sizeof(TexturedColoredVertex),
                                      (const GLvoid*) offsetof(TexturedColoredVertex, normal));
                
                glEnableVertexAttribArray(inColorAttrib);
                glVertexAttribPointer(inColorAttrib,
                                      4,
                                      GL_FLOAT,
                                      GL_FALSE,
                                      sizeof(TexturedColoredVertex),
                                      (const GLvoid*) offsetof(TexturedColoredVertex, color));
                
                glEnableVertexAttribArray(inTangentAttrib);
                glVertexAttribPointer(inTangentAttrib,
                                      3,
                                      GL_FLOAT,
                                      GL_FALSE,
                                      sizeof(TexturedColoredVertex),
                                      (const GLvoid*) offsetof(TexturedColoredVertex, tangent));
                
                glEnableVertexAttribArray(inBiTangentAttrib);
                glVertexAttribPointer(inBiTangentAttrib,
                                      3,
                                      GL_FLOAT,
                                      GL_FALSE,
                                      sizeof(TexturedColoredVertex),
                                      (const GLvoid*) offsetof(TexturedColoredVertex, bitangent));
                
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            
            {
                glGenBuffers(1, &m_IndexBuffer);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, getElementArrayBufferSize() * subdivisionBufferSize(), getElementArrayBufferPtr(), GL_STREAM_DRAW);
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
            
            shader->setUniformValue("RimLightColor", btVector3(1.0f, 1.0f, 1.0f));
            shader->setUniformValue("RimLightStart", 0.0f);
            shader->setUniformValue("RimLightEnd", 1.0f);
            shader->setUniformValue("RimLightCoefficient", 0.6f);
            
            shader->setUniformValue("LightSourceAmbientColor", btVector3(1.0f, 1.0f, 1.0f));
            shader->setUniformValue("LightSourceDiffuseColor", btVector3(1.0f, 1.0f, 1.0f));
            shader->setUniformValue("LightSourceSpecularColor", btVector3(1.0f, 1.0f, 1.0f));
            
            shader->setUniformValue("LightSourcePosition_worldspace", btVector4(0.0f, 0.0f, -1.0f, 1.0));
            
            shader->setUniformValue("LightSourceSpotDirection", btVector3(0.0f, 0.0f, 1.0f));
            shader->setUniformValue("LightSourceSpotExponent", 100.0f);
            
            shader->setUniformValue("LightSourceSpotCutoff", 180.0f);
            shader->setUniformValue("LightSourceSpotCosCutoff", 30.0f);
            
            shader->setUniformValue("LightSourceConstantAttenuation", 1.0f);
            shader->setUniformValue("LightSourceLinearAttenuation", 0.045f);
            shader->setUniformValue("LightSourceQuadraticAttenuation", 0.0075f);
            
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
                printf("m_ModelViewBuffer %ld\n", getModelViewTransformArrayBufferSize());
                glBufferSubData(GL_ARRAY_BUFFER, 0, getModelViewTransformArrayBufferSize(), getModelViewTransformArrayBufferPtr());
                enableModelViewBufferChanged(false);
            }
            
            if(isNormalMatrixBufferChanged())
            {
                glBindBuffer(GL_ARRAY_BUFFER, m_NormalMatrixTransformBuffer);
                printf("m_NormalMatrixTransformBuffer %ld\n", getNormalMatrixTransformArrayBufferSize());
                glBufferSubData(GL_ARRAY_BUFFER, 0, getNormalMatrixTransformArrayBufferSize(), getNormalMatrixTransformArrayBufferPtr());
                enableNormalMatrixBufferChanged(false);
            }
            
            if(isVertexArrayBufferChanged())
            {
                glBindBuffer(GL_ARRAY_BUFFER, m_VerticesBuffer);
                printf("m_VerticesBuffer %ld\n", getVertexArrayBufferSize());
                glBufferSubData(GL_ARRAY_BUFFER, 0, getVertexArrayBufferSize(), getVertexArrayBufferPtr());
                enableVertexArrayBufferChanged(false);
            }
            
            if(isIndiceArrayBufferChanged())
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
                printf("m_IndexBuffer %ld\n", getElementArrayBufferSize());
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, getElementArrayBufferSize(), getElementArrayBufferPtr());
                enableIndiceArrayBufferChanged(false);
            }
            
            glDrawElements(GL_TRIANGLES, maxNumberOfInstances() * numberOfIndices(), getElementIndexType(), (const GLvoid*)0);
//            glDrawElements(GL_LINES, maxNumberOfInstances() * numberOfIndices(), getElementIndexType(), (const GLvoid*)0);
//            glDrawElements(GL_POINTS, maxNumberOfInstances() * numberOfIndices(), getElementIndexType(), (const GLvoid*)0);
            
            glBindVertexArrayOES(0);
        }
    }
    
    GLsizei Geometry::maxNumberOfInstances()const
    {
        return m_NumberInstances;
    }
    
    GLsizei Geometry::maxNumberOfSubDivisions()const
    {
        return m_NumberSubDivisions;
    }
    
    GLsizei Geometry::subdivisionBufferSize()const
    {
        return m_ExtraSubdivisionBuffer;
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
        
        return (const void *)m_ModelViewTransformData;
    }
    
    GLsizeiptr Geometry::getModelViewTransformArrayBufferSize()const
    {
        GLsizeiptr size = (sizeof(GLfloat)* 16) * maxNumberOfInstances() * numberOfVertices();
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
//        GLsizeiptr size = sizeof(GLfloat) * maxNumberOfInstances() * numberOfVertices() * 16;
//        return size;
//    }
    
    const void *Geometry::getNormalMatrixTransformArrayBufferPtr()const
    {
        assert(m_NormalMatrixTransformData);
        
        return (const void *)m_NormalMatrixTransformData;
    }
    
    GLsizeiptr Geometry::getNormalMatrixTransformArrayBufferSize()const
    {
        GLsizeiptr size = (sizeof(GLfloat) * 16) * maxNumberOfInstances() * numberOfVertices();
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
        
        m_ModelViewTransformData    = new GLfloat[16 * maxNumberOfInstances() * numberOfVertices() * subdivisionBufferSize()];
        assert(m_ModelViewTransformData);
        memset(m_ModelViewTransformData, std::numeric_limits<float>::max(), getModelViewTransformArrayBufferSize() * subdivisionBufferSize());
        
        m_NormalMatrixTransformData = new GLfloat[16 * maxNumberOfInstances() * numberOfVertices() * subdivisionBufferSize()];
        assert(m_NormalMatrixTransformData);
        memset(m_NormalMatrixTransformData, std::numeric_limits<float>::max(), getNormalMatrixTransformArrayBufferSize() * subdivisionBufferSize());
        
        unsigned long i;
        
        for (i = 0;
             i < (16 * maxNumberOfInstances() * numberOfVertices() * subdivisionBufferSize());
             i += 16)
            memcpy(m_ModelViewTransformData + i, TRANSFORM_IDENTITY_MATRIX, sizeof(TRANSFORM_IDENTITY_MATRIX));
        
        for (i = 0;
             i < (16 * maxNumberOfInstances() * numberOfVertices() * subdivisionBufferSize());
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
    
    bool Geometry::isIndiceArrayBufferChanged()const
    {
        return m_ElementBufferChanged;
    }
    
    void Geometry::enableIndiceArrayBufferChanged(bool changed)
    {
        m_ElementBufferChanged = changed;
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
        if (index < maxNumberOfInstances())
        {
            const unsigned long STRIDE = 16 * numberOfVertices();
            
            transform.getOpenGLMatrix(m_MatrixBuffer);
            
            for (int currentVertex = 0; currentVertex < numberOfVertices(); currentVertex++)
            {
                unsigned long p = ((index * STRIDE) + (16 * currentVertex));
                int cmp = memcmp(m_ModelViewTransformData + p,
                                 m_MatrixBuffer,
                                 sizeof(GLfloat) * 16);
                
                if(0 != cmp)
                {
                    memcpy(m_ModelViewTransformData + p, m_MatrixBuffer, sizeof(GLfloat) * 16);
                }
            }
            enableModelViewBufferChanged(true);
        }
    }
    
    btTransform Geometry::getTransform(const unsigned long index)
    {
        btTransform transform(btTransform::getIdentity());
        if (index < maxNumberOfInstances())
        {
            const unsigned long STRIDE = 16 * numberOfVertices();
            
            for (int currentVertex = 0; currentVertex < numberOfVertices(); currentVertex++)
            {
                unsigned long p = ((index * STRIDE) + (16 * currentVertex));
                memcpy(m_MatrixBuffer,
                       m_ModelViewTransformData + p,
                       sizeof(GLfloat) * 16);
            }
            
            transform.setFromOpenGLMatrix(m_MatrixBuffer);
        }
        return transform;
    }
    
//    void Geometry::setColorTransform(const unsigned long index, const btTransform &transform)
//    {
//        if (index < maxNumberOfInstances())
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
//                                 sizeof(GLfloat) * 16);
//                
//                if(0 != cmp)
//                {
//                    memcpy(m_ColorTransformData + p,
//                           m_MatrixBuffer,
//                           sizeof(GLfloat) * 16);
//                }
//            }
//        }
//    }
//    
//    btTransform Geometry::getColorTransform(const unsigned long index)
//    {
//        btTransform transform(btTransform::getIdentity());
//        if (index < maxNumberOfInstances())
//        {
//            const unsigned long STRIDE = 16 * numberOfVertices();
//            
//            for (int currentVertex = 0; currentVertex < numberOfVertices(); currentVertex++)
//            {
//                unsigned long p = ((index * STRIDE) + (16 * currentVertex));
//                memcpy(m_MatrixBuffer,
//                       m_ColorTransformData + p,
//                       sizeof(GLfloat) * 16);
//            }
//            
//            transform.setFromOpenGLMatrix(m_MatrixBuffer);
//        }
//        return transform;
//    }
    
    void Geometry::setNormalMatrixTransform(const unsigned long index, const btTransform &transform)
    {
        if (index < maxNumberOfInstances())
        {
            const unsigned long STRIDE = 16 * numberOfVertices();
            
            transform.getOpenGLMatrix(m_MatrixBuffer);
            
            for (int currentVertex = 0; currentVertex < numberOfVertices(); currentVertex++)
            {
                unsigned long p = ((index * STRIDE) + (16 * currentVertex));
                
                int cmp = memcmp(m_NormalMatrixTransformData + p,
                                 m_MatrixBuffer,
                                 sizeof(GLfloat) * 16);
                
                if(0 != cmp)
                {
                    memcpy(m_NormalMatrixTransformData + p,
                           m_MatrixBuffer,
                           sizeof(GLfloat) * 16);
                }
            }
            enableNormalMatrixBufferChanged(true);
        }
    }
    
    btTransform Geometry::getNormalMatrixTransform(const unsigned long index)
    {
        btTransform transform(btTransform::getIdentity());
        if (index < maxNumberOfInstances())
        {
            const unsigned long STRIDE = 16 * numberOfVertices();
            
            for (int currentVertex = 0; currentVertex < numberOfVertices(); currentVertex++)
            {
                unsigned long p = ((index * STRIDE) + (16 * currentVertex));
                memcpy(m_MatrixBuffer,
                       m_NormalMatrixTransformData + p,
                       sizeof(GLfloat) * 16);
            }
            
            transform.setFromOpenGLMatrix(m_MatrixBuffer);
            
        }
        return transform;
    }
}
