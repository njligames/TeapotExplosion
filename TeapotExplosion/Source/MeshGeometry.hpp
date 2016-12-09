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
        
        virtual void load(Shader *shader, const std::string &filecontent="", unsigned int numInstances = 1, MeshType type = MeshType_Obj);
    protected:
        
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
        
        virtual GLsizei numberOfVertices()const;
        virtual GLsizei numberOfIndices()const;
        
    private:
        TexturedColoredVertex *m_VertexData;
        GLuint *m_IndiceData;
        
        std::string m_Filedata;
        GLsizei m_NumberOfVertices;
        GLsizei m_NumberOfIndices;
        
    };
}

#endif /* MeshGeometry_hpp */
