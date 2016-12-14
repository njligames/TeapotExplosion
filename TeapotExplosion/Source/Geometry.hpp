//
//  Geometry.hpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#ifndef Geometry_hpp
#define Geometry_hpp

#import <OpenGLES/ES2/glext.h>
#import <OpenGLES/ES2/gl.h>

//#include <bitset>
#include <vector>
#include <string>

#include "btTransform.h"
#include "btVector2.h"

namespace jamesfolk
{
    static const GLfloat TRANSFORM_IDENTITY_MATRIX[] =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
    
    ATTRIBUTE_ALIGNED16(struct)
    TexturedColoredVertex
    {
        static void computeTangentBasis(TexturedColoredVertex *v, unsigned int numVerts)
        {
            for (unsigned int i=0; i<numVerts; i+=3 )
            {
                // Shortcuts for vertices
                btVector3  v0 = (v + (i + 0))->vertex;
                btVector3  v1 = (v + (i + 1))->vertex;
                btVector3  v2 = (v + (i + 2))->vertex;
                
                // Shortcuts for UVs
                btVector2  uv0 = (v + (i + 0))->texture;
                btVector2  uv1 = (v + (i + 1))->texture;
                btVector2  uv2 = (v + (i + 2))->texture;
                
                // Edges of the triangle : postion delta
                btVector3 deltaPos1 = v1-v0;
                btVector3 deltaPos2 = v2-v0;
                
                // UV delta
                btVector2 deltaUV1 = uv1-uv0;
                btVector2 deltaUV2 = uv2-uv0;
                
                float d = (deltaUV1.x() * deltaUV2.y() - deltaUV1.y() * deltaUV2.x());
                
                float r = (d!=0.0f)?(1.0f / d):0.0f;
                btVector3 tangent = (deltaPos1 * deltaUV2.y()   - deltaPos2 * deltaUV1.y())*r;
                btVector3 bitangent = (deltaPos2 * deltaUV1.x()   - deltaPos1 * deltaUV2.x())*r;
                
                // Set the same tangent for all three vertices of the triangle.
                // They will be merged later, in vboindexer.cpp
                (v + (i + 0))->tangent = tangent;
                (v + (i + 1))->tangent = tangent;
                (v + (i + 2))->tangent = tangent;
                
                // Same thing for binormals
                (v + (i + 0))->bitangent = bitangent;
                (v + (i + 1))->bitangent = bitangent;
                (v + (i + 2))->bitangent = bitangent;
            }
            
            // See "Going Further"
//            for (unsigned int i=0; i<numVerts; i+=1 )
//            {
//                btVector3 & n = (v + (i))->normal;
//                btVector3 & t = (v + (i))->tangent;
//                btVector3 & b = (v + (i))->bitangent;
//                
//                
//                // Gram-Schmidt orthogonalize
//                t = (t - n * n.dot(t));
//                
//                if(t.length() > 0.0f)
//                    t.normalize();
//                
//                // Calculate handedness
//                if (n.cross(t).dot(b) < 0.0f)
//                {
//                    t = t * -1.0f;
//                }
//            }
        }
        
        TexturedColoredVertex()
        : vertex(0, 0, 0)
        , color(1, 1, 1, 1)
        , texture(0, 0)
        , normal(1,1,1)
        , tangent(0, 0, 0)
        , bitangent(0, 0, 0)
        {
        }
        TexturedColoredVertex(const btVector3 vertex,
                              const btVector4 color,
                              const btVector2 texture,
                              const btVector3 normal,
                              const btVector3 tangent,
                              const btVector3 bitangent
                              )
        : vertex(vertex)
        , color(color)
        , texture(texture)
        , normal(normal)
        , tangent(tangent)
        , bitangent(bitangent)
        {
        }
        btVector3 vertex;
        btVector4 color;
        btVector2 texture;
        btVector3 normal;
        btVector3 tangent;
        btVector3 bitangent;
        
        TexturedColoredVertex& operator=(const TexturedColoredVertex& rhs)
        {
            if (this != &rhs) {
                vertex = rhs.vertex;
                color = rhs.color;
                texture = rhs.texture;
                normal = rhs.normal;
                tangent = rhs.tangent;
                bitangent = rhs.bitangent;
            }
            return *this;
        }
        
        operator std::string() const
        {
            char buffer[4098];
            sprintf(buffer, "{{%f, %f, %f}, {%f, %f}, {%f, %f, %f}}",
                    vertex.x(), vertex.y(), vertex.z(),
                    texture.x(), texture.y(),
                    normal.x(), normal.y(), normal.z()
                    );
            
            return std::string(buffer);
        }
        
        static TexturedColoredVertex average(TexturedColoredVertex a, TexturedColoredVertex b)
        {
            TexturedColoredVertex ret;
            
            ret.vertex = (a.vertex + b.vertex) / 2.0f;
            ret.color = (a.color + b.color) / 2.0f;
            ret.texture = (a.texture + b.texture) / 2.0f;
            ret.normal = (a.normal + b.normal) / 2.0f;
            ret.tangent = (a.tangent + b.tangent) / 2.0f;
            ret.bitangent = (a.vertex + b.bitangent) / 2.0f;
            
//            ret.vertex.setW(1.0);
            ret.color.setW(1.0);
            
            if(ret.normal.length2() > 0.0)
                ret.normal.normalize();
            if(ret.tangent.length2() > 0.0)
                ret.tangent.normalize();
            if(ret.bitangent.length2() > 0.0)
                ret.bitangent.normalize();
            
            return ret;
        }
        
    };
    
    class Shader;
    class Camera;
    class Node;
    
    class Geometry
    {
        friend class Node;
        friend class Scene;
        
    public:
        enum MeshType
        {
            MeshType_Obj
        };
        
        Geometry();
        Geometry(const Geometry &rhs);
        const Geometry &operator=(const Geometry &rhs);
        virtual ~Geometry();
        
        virtual void load(Shader *shader, const std::string &filecontent="", unsigned int numInstances = 1, unsigned int numSubDivisions = 1);
        void unLoad();
        bool isLoaded()const;
        
        Shader *const getShader();
        
        void setShader(Shader *const shader);
        
        void render(Camera *camera);
        
        virtual void subdivide() = 0;
        
        virtual bool isMaxSubdivisions() = 0;
        
        inline void setVerticeTransform(const GLsizei instanceIdx, const GLsizei verticeIdx, const btTransform &t)
        {
            if(instanceIdx < maxNumberOfInstances() &&
               verticeIdx < numberOfVertices())
            {
                GLsizei idx = (instanceIdx * numberOfVertices());
                idx += (verticeIdx * 16);
                
                t.getOpenGLMatrix(m_MatrixBufferFullSize);
                memcpy(m_ModelViewTransformData + idx,
                       m_MatrixBufferFullSize,
                       sizeof(TRANSFORM_IDENTITY_MATRIX));
                enableModelViewBufferChanged();
            }
        }
        
        inline bool getVerticeTransform(const GLsizei instanceIdx, const GLsizei verticeIdx, btTransform &t)const
        {
            if(instanceIdx < maxNumberOfInstances() &&
               verticeIdx < numberOfVertices())
            {
                GLsizei idx = (instanceIdx * numberOfVertices());
                idx += (verticeIdx * 16);
                
                memcpy(m_MatrixBufferFullSize,
                       m_ModelViewTransformData + idx,
                       sizeof(TRANSFORM_IDENTITY_MATRIX));
                t.setFromOpenGLMatrix(m_MatrixBufferFullSize);
                return true;
            }
            
            return false;
        }
        
        inline void transformVertice(const GLsizei instanceIdx, const GLsizei verticeIdx, const btTransform &t)
        {
            btTransform ret(btTransform::getIdentity());
            
            if(getVerticeTransform(instanceIdx, verticeIdx, ret))
            {
                setVerticeTransform(instanceIdx, verticeIdx, ret * t);
            }
        }
        
        virtual btVector3 getVertexPosition(const GLsizei instanceIdx, const GLsizei verticeIdx)const = 0;
        virtual btVector4 getVertexColor(const GLsizei instanceIdx, const GLsizei verticeIdx)const = 0;
        virtual btVector2 getVertexTexture(const GLsizei instanceIdx, const GLsizei verticeIdx)const = 0;
        virtual btVector3 getVertexNormal(const GLsizei instanceIdx, const GLsizei verticeIdx)const = 0;
        virtual btVector3 getVertexTangent(const GLsizei instanceIdx, const GLsizei verticeIdx)const = 0;
        virtual btVector3 getVertexBitangent(const GLsizei instanceIdx, const GLsizei verticeIdx)const = 0;
        
        virtual GLsizei numberOfVertices()const = 0;
        virtual GLsizei numberOfIndices()const = 0;
        
        void setRimLightColor(const btVector3 &color);
        const btVector3 &getRimLightColor()const;
        
        void setRimLightStart(const float v);
        float getRimLightStart()const;
        
        void setRimLightEnd(const float v);
        float getRimLightEnd()const;
        
        void setRimLightCoefficient(const float v);
        float getRimLightCoefficient()const;
        
        void setLightSourceAmbientColor(const btVector3 &color);
        const btVector3 &getLightSourceAmbientColor()const;
        
        void setLightSourceDiffuseColor(const btVector3 &color);
        const btVector3 &getLightSourceDiffuseColor()const;
        
        void setLightSourceSpecularColor(const btVector3 &color);
        const btVector3 &getLightSourceSpecularColor()const;
        
        void setLightSourcePosition(const btVector4 &pos);
        const btVector4 &getLightSourcePosition()const;
        
        void setLightSourceSpotDirection(const btVector3 &dir);
        const btVector3 &getLightSourceSpotDirection()const;
        
        void setLightSourceSpotExponent(const float v);
        float getLightSourceSpotExponent()const;
        
        void setLightSourceSpotCutoff(const float v);
        float getLightSourceSpotCutoff()const;
        
        void setLightSourceSpotCosCutoff(const float v);
        float getLightSourceSpotCosCutoff()const;
        
        void setLightSourceConstantAttenuation(const float v);
        float getLightSourceConstantAttenuation()const;
        
        void setLightSourceLinearAttenuation(const float v);
        float getLightSourceLinearAttenuation()const;
        
        void setLightSourceQuadraticAttenuation(const float v);
        float getLightSourceQuadraticAttenuation()const;
        
        void setLightAmbientColor(const btVector3 &color);
        const btVector3 &getLightAmbientColor()const;
        
        void setMaterialShininess(const float v);
        float getMaterialShininess()const;
        
        void setFogMaxDistance(const float v);
        float getFogMaxDistance()const;
        
        void setFogMinDistance(const float v);
        float getFogMinDistance()const;
        
        void setFogColor(const btVector3 &color);
        const btVector3 &getFogColor()const;
        
        void setFogDensity(const float v);
        float getFogDensity()const;
        
        void setAmbientTexture(const GLuint t);
        void setDiffuseTexture(const GLuint t);
        void setNormalTexture(const GLuint t);
        void setSpecularTexture(const GLuint t);
        
    protected:
        
        const void *getModelViewTransformArrayBufferPtr()const;
        GLsizeiptr getModelViewTransformArrayBufferSize()const;
        bool isModelViewBufferChanged()const;
        void enableModelViewBufferChanged(bool changed = true);
        
        const void *getNormalMatrixTransformArrayBufferPtr()const;
        GLsizeiptr getNormalMatrixTransformArrayBufferSize()const;
        bool isNormalMatrixBufferChanged()const;
        void enableNormalMatrixBufferChanged(bool changed = true);
        
        virtual void loadData();
        virtual void unLoadData();
        
        virtual const void *getVertexArrayBufferPtr()const = 0;
        virtual GLsizeiptr getVertexArrayBufferSize()const = 0;
        bool isVertexArrayBufferChanged()const;
        void enableVertexArrayBufferChanged(bool changed = true);
        
        virtual const void *getElementArrayBufferPtr()const = 0;
        virtual GLsizeiptr getElementArrayBufferSize()const = 0;
        bool isIndiceArrayBufferChanged()const;
        void enableIndiceArrayBufferChanged(bool changed = true);
        
        virtual GLenum getElementIndexType()const = 0;
        
        void addReference(Node *node);
        void removeReference(Node *node);
        
        void setTransform(const unsigned long index, const btTransform &transform);
        btTransform getTransform(const unsigned long index);
        
        void setNormalMatrixTransform(const unsigned long index, const btTransform &transform);
        btTransform getNormalMatrixTransform(const unsigned long index);
        
        virtual void setOpacity(Node *node) = 0;
        virtual void setHidden(Node *node) = 0;
        virtual void setColorBase(Node *node) = 0;
        
        
        virtual GLsizei maxNumberOfInstances()const;
        virtual GLsizei maxNumberOfSubDivisions()const;
        virtual GLsizei subdivisionBufferSize()const;
        
        unsigned long getGeometryIndex(Node *const node)const;
        
        GLfloat *m_MatrixBuffer;
        float *m_MatrixBufferFullSize;
        
        GLfloat *m_ModelViewTransformData;
        GLfloat *m_NormalMatrixTransformData;
        
     private:
        GLuint m_VertexArray;
        GLuint m_ModelViewBuffer;
        GLuint m_NormalMatrixTransformBuffer;
        GLuint m_VerticesBuffer;
        GLuint m_IndexBuffer;
        
        std::vector<bool> m_References;
        GLsizei m_NumberInstances;
        GLsizei m_NumberSubDivisions;
        GLsizei m_ExtraSubdivisionBuffer;
        
        Shader *m_Shader;
        
        bool m_OpacityModifyRGB;
        bool m_ElementBufferChanged;
        bool m_VertexBufferChanged;
        bool m_NormalMatrixBufferChanged;
        bool m_ModelViewBufferChanged;
        bool m_ShaderChanged;
        
        btVector3 m_RimLightColor;
        float m_RimLightStart;
        float m_RimLightEnd;
        float m_RimLightCoefficient;
        btVector3 m_LightSourceAmbientColor;
        btVector3 m_LightSourceDiffuseColor;
        btVector3 m_LightSourceSpecularColor;
        btVector4 m_LightSourcePosition_worldspace;
        btVector3 m_LightSourceSpotDirection;
        float m_LightSourceSpotExponent;
        float m_LightSourceSpotCutoff;
        float m_LightSourceSpotCosCutoff;
        float m_LightSourceConstantAttenuation;
        float m_LightSourceLinearAttenuation;
        float m_LightSourceQuadraticAttenuation;
        btVector3 m_LightAmbientColor;
        float m_MaterialShininess;
        float m_FogMaxDistance;
        float m_FogMinDistance;
        btVector3 m_FogColor;
        float m_FogDensity;
        
        GLuint m_AmbientTexture;
        GLuint m_DiffuseTexture;
        GLuint m_NormalTexture;
        GLuint m_SpecularTexture;
    };
}

#endif /* Geometry_hpp */
