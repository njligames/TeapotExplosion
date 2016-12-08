//
//  MeshGeometry.cpp
//  VerizonTest
//
//  Created by James Folk on 6/28/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#include "MeshGeometry.hpp"

#include "Node.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include <map>

//static bool isFloat( std::string myString )
//{
//    std::istringstream iss(myString);
//    float f;
//    iss >> std::noskipws >> f; // noskipws considers leading whitespace invalid
//    // Check the entire string was consumed and if either failbit or badbit is set
//    return iss.eof() && !iss.fail();
//}
namespace jamesfolk
{
    MeshGeometry::MeshGeometry():
    Geometry(),
    m_VertexData(NULL),
    m_IndiceData(NULL),
    m_Filedata(""),
    m_NumberOfVertices(0),
    m_NumberOfIndices(0)
    {
    }
    
    
    MeshGeometry::~MeshGeometry()
    {
    }
    
    void MeshGeometry::load(Shader *shader, const std::string &filedata, unsigned int numInstances, MeshType type)
    {
        m_Filedata = filedata;
        
        Geometry::load(shader, filedata, numInstances, type);
    }
    
    void MeshGeometry::loadData()
    {
        std::vector<btVector3> vertices;
        std::vector<btVector3> normals;
        std::vector<btVector2> texture;
        std::vector<std::string> faces;
        
        std::stringstream ss_line(m_Filedata);
        std::string line;
        
        enum parsemode{none,v,vn,vt,f};
        parsemode mode = none;
        btVector3 vec3;
        btVector2 vec2;
        
        float maxX=0, maxY=0, maxZ=0;
        
        while(std::getline(ss_line, line, '\n'))
        {
            if(line[0] == '#')
                continue;

            std::replace(line.begin(), line.end(), '\t', ' ');
            std::replace(line.begin(), line.end(), '\r', ' ');
            
            std::stringstream ss_token(line);
            std::string token;
            int tokencount = 0;
            vec3 = btVector3(0,0,0);
            vec2 = btVector2(0,0);
            
            while(std::getline(ss_token, token, ' '))
            {
                if(tokencount == 0)
                {
                    if(token == "v")
                        mode = v;
                    else if(token == "vt")
                        mode = vt;
                    else if(token == "vn")
                        mode = vn;
                    else if (token == "f")
                        mode = f;
//                    else
//                        mode = none;
                }
                else
                {
//                    if(!isFloat(token))
//                        continue;
                    switch (mode)
                    {
                        case v:
                        case vn:
                        {
                            switch (tokencount)
                            {
                                case 1:vec3.setX(atof(token.c_str()));break;
                                case 2:vec3.setY(atof(token.c_str()));break;
                                case 3:vec3.setZ(atof(token.c_str()));break;
                                default:
                                    assert(false);
                            }
                        }
                            break;
                        case vt:
                        {
                            switch (tokencount)
                            {
                                case 1:vec2.setX(atof(token.c_str()));break;
                                case 2:vec2.setY(atof(token.c_str()));break;
                                default:
                                    assert(false);
                            }
                        }
                            break;
                        case f:
                        {
                            faces.push_back(token);
                        }
                            break;
                            
                        default:
                            break;
                    }
                }
                tokencount++;
            }
            
            switch (mode)
            {
                case v:
                    maxX = std::max<float>(vec3.x(), maxX);
                    maxY = std::max<float>(vec3.y(), maxY);
                    maxZ = std::max<float>(vec3.z(), maxZ);
                    
                    vertices.push_back(vec3);
                    break;
                case vn:
                    normals.push_back(vec3);
                    break;
                case vt:
                    texture.push_back(vec2);
                    break;
                case f:
                {
                }
                    break;
                    
                default:
                    break;
            }
            mode = none;
        }
        
        m_NumberOfIndices = (GLsizei)faces.size();
        m_NumberOfVertices = (GLsizei)faces.size();
        TexturedColoredVertex *vertexData = new TexturedColoredVertex[numberOfVertices()];
        GLuint *indiceData = new GLuint[numberOfIndices()];
        unsigned long idx = 0;
        
        for (std::vector<std::string>::iterator i = faces.begin(); i != faces.end(); i++,idx++)
        {
            std::string faceString = *i;
            std::stringstream ss_faceData(*i);
            std::string faceData;
            int ii = 0;
            TexturedColoredVertex t;
            
            while(std::getline(ss_faceData, faceData, '/'))
            {
                unsigned long idx = atoi(faceData.c_str()) - 1;
                
                switch (ii)
                {
                    case 0:
                        //vertex idx
                        assert(idx < vertices.size());
                        t.vertex = vertices.at(idx);
                        break;
                    case 1:
                        assert(idx < texture.size());
                        //texture idx
//                            vertexData[vertexIndex].texture = texture.at(idx);
                        break;
                    case 2:
                        assert(idx < normals.size());
                        //normal idx
                        t.normal = normals.at(idx);
                        break;
                        
                    default:
                        assert(false);
                        break;
                }
                ii++;
            }
//            t.hidden = 0.0f;
//            t.opacity = 1.0f;
            t.color = btVector4(1.0f, 1.0f, 1.0f, 1.0f);
            
            vertexData[idx] = t;
            indiceData[idx] = (GLuint)idx;
        }
        
        Geometry::loadData();
        
        assert(m_VertexData == NULL);
        m_VertexData = new TexturedColoredVertex[numberOfVertices() * numberOfInstances()];
        enableVertexArrayBufferChanged(true);
        
        assert(m_IndiceData == NULL);
        m_IndiceData = new GLuint[numberOfIndices() * numberOfInstances()];
        
        unsigned long vertexInstanceIndex = 0;
        unsigned long indiceInstanceIndex = 0;
        for (unsigned long meshIndex = 0;
             meshIndex < numberOfInstances();
             meshIndex++)
        {
            for (unsigned long verticeIndex = 0;
                 verticeIndex < numberOfVertices();
                 verticeIndex++)
            {
                m_VertexData[vertexInstanceIndex] = vertexData[verticeIndex];
                vertexInstanceIndex++;
            }
            
            for (unsigned long indiceIndex = 0;
                 indiceIndex < numberOfIndices();
                 indiceIndex++)
            {
                m_IndiceData[indiceInstanceIndex] = (GLuint)((meshIndex * numberOfVertices()) + indiceData[indiceIndex]);
                indiceInstanceIndex++;
            }
        }
        
        TexturedColoredVertex::computeTangentBasis(m_VertexData, numberOfVertices());
        
        delete [] indiceData;
        indiceData = NULL;
        
        delete [] vertexData;
        vertexData = NULL;
    }
    
    void MeshGeometry::unLoadData()
    {
        Geometry::unLoadData();
        
        if(m_IndiceData)
            delete [] m_IndiceData;
        m_IndiceData = NULL;
        
        if(m_VertexData)
            delete [] m_VertexData;
        m_VertexData = NULL;
    }
    
    const void *MeshGeometry::getVertexArrayBufferPtr()const
    {
        return (const void *)m_VertexData;
    }
    
    GLsizeiptr MeshGeometry::getVertexArrayBufferSize()const
    {
        GLsizeiptr size = sizeof(TexturedColoredVertex) * numberOfVertices() * numberOfInstances();
        return size;
    }
    
    const void *MeshGeometry::getElementArrayBufferPtr()const
    {
        return m_IndiceData;
    }
    
    GLsizeiptr MeshGeometry::getElementArrayBufferSize()const
    {
        GLsizeiptr size = sizeof(GLuint) * numberOfIndices() * numberOfInstances();
        return size;
    }
    
    GLenum MeshGeometry::getElementIndexType()const
    {
        return GL_UNSIGNED_INT;
    }
    
    void MeshGeometry::setOpacity(Node *node)
    {
        unsigned long index = getGeometryIndex(node);
        
        if(m_VertexData)
        {
            float opacity = node->getOpacity();
            bool hidden = node->isHiddenGeometry();
            
            float o = (opacity > 1.0f)?1.0f:((opacity<0.0f)?0.0f:opacity);
            o = (hidden)?0.0f:o;
            
            unsigned long offset = index * numberOfVertices();
            for (unsigned long vertexIndex = 0; vertexIndex < numberOfVertices(); vertexIndex++)
            {
                btVector4 color(m_VertexData[vertexIndex + offset].color);
                color.setW(o);
                
                m_VertexData[vertexIndex + offset].color = color;
            }
            enableVertexArrayBufferChanged(true);
        }
    }
    
    void MeshGeometry::setHidden(Node *node)
    {
        unsigned long index = getGeometryIndex(node);
        
        if(m_VertexData)
        {
            bool hidden = node->isHiddenGeometry();
            
            float h = (hidden)?1.0f:0.0f;
            
            unsigned long offset = index * numberOfVertices();
            for (unsigned long vertexIndex = 0; vertexIndex < numberOfVertices(); vertexIndex++)
            {
                btVector4 color(m_VertexData[vertexIndex + offset].color);
                color.setW(h);
                
                m_VertexData[vertexIndex + offset].color = color;
            }
            enableVertexArrayBufferChanged(true);
        }
    }
    
    void MeshGeometry::setColorBase(Node *node)
    {
        unsigned long index = getGeometryIndex(node);
        
        if(m_VertexData)
        {
            bool hidden = node->isHiddenGeometry();
            
            btVector4 c(node->getColorBase());
            
            if(hidden)
                c.setW(0.0f);
            
            unsigned long offset = index * numberOfVertices();
            for (unsigned long vertexIndex = 0;
                 vertexIndex < numberOfVertices();
                 vertexIndex++)
            {
                m_VertexData[offset + vertexIndex].color = c;
            }
            enableVertexArrayBufferChanged(true);
        }
    }
    
    GLsizei MeshGeometry::numberOfVertices()const
    {
        return m_NumberOfVertices;
    }
    
    GLsizei MeshGeometry::numberOfIndices()const
    {
        return m_NumberOfIndices;
    }
}
