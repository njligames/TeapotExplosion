//
//  MeshGeometry.hpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#ifndef MeshGeometry_hpp
#define MeshGeometry_hpp

#include "Geometry.hpp"

namespace jamesfolk
{
    class MeshGeometry : public Geometry
    {
    public:
        
        /* members */
        MeshGeometry();
        MeshGeometry(const MeshGeometry &rhs);
        const MeshGeometry &operator=(const MeshGeometry &rhs);
        ~MeshGeometry();
        
        virtual void load(Shader *shader, const std::string &filecontent, unsigned int numInstances, unsigned int numSubDivisions);
        
        void subdivide();
        
        btVector3 getVertexPosition(const GLsizei instanceIdx, const GLsizei verticeIdx)const;
        btVector4 getVertexColor(const GLsizei instanceIdx, const GLsizei verticeIdx)const;
        btVector2 getVertexTexture(const GLsizei instanceIdx, const GLsizei verticeIdx)const;
        btVector3 getVertexNormal(const GLsizei instanceIdx, const GLsizei verticeIdx)const;
        btVector3 getVertexTangent(const GLsizei instanceIdx, const GLsizei verticeIdx)const;
        btVector3 getVertexBitangent(const GLsizei instanceIdx, const GLsizei verticeIdx)const;
        
        virtual GLsizei numberOfVertices()const;
        virtual GLsizei numberOfIndices()const;
    protected:
        void subdivideTriangle(TexturedColoredVertex p0,
                               TexturedColoredVertex p1,
                               TexturedColoredVertex p2,
                               TexturedColoredVertex *triangleBuffer,
                               GLsizei newIndiceStart,
                               GLuint *indiceBuffer);
        
        virtual void loadData();
        virtual void unLoadData();
        
        virtual const void *getVertexArrayBufferPtr()const;
        virtual GLsizeiptr getVertexArrayBufferSize()const;
        
        virtual const void *getElementArrayBufferPtr()const;
        virtual GLsizeiptr getElementArrayBufferSize()const;
        
        virtual GLenum getElementIndexType()const;
        
        virtual void setOpacity(Node *node);
        virtual void setHidden(Node *node);
        virtual void setColorBase(Node *node);
        
        
        
    private:
        TexturedColoredVertex *m_VertexData;
        GLuint *m_IndiceData;
        
        TexturedColoredVertex *m_VertexDataBuffer;
        GLuint *m_IndiceDataBuffer;
        
        std::string m_Filedata;
        GLsizei m_NumberOfVertices;
        GLsizei m_NumberOfIndices;
        GLsizei m_TotalSubdivisions;
        
        TexturedColoredVertex *m_triangleBuffer;
        GLuint *m_indiceBuffer;
        
    };
}

#endif /* MeshGeometry_hpp */
