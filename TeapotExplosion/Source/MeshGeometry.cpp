//
//  MeshGeometry.cpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
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
    m_NumberOfIndices(0),
    m_TotalSubdivisions(0)
    {
    }
    
    
    MeshGeometry::~MeshGeometry()
    {
    }
    
    void MeshGeometry::load(Shader *shader, const std::string &filedata, unsigned int numInstances, unsigned int numSubDivisions)
    {
        m_Filedata = filedata;
        
        Geometry::load(shader, filedata, numInstances, numSubDivisions);
    }
    
    void MeshGeometry::subdivide()
    {
        if(m_TotalSubdivisions < maxNumberOfSubDivisions())
        {
            GLsizei previousNumberOfVertices = m_NumberOfVertices;
            GLsizei previousNumberOfIndices = m_NumberOfIndices;
            
            m_NumberOfVertices *= 4;
            m_NumberOfIndices *= 4;
            
            btTransform *transforms = new btTransform[maxNumberOfInstances()];
            btTransform *normalMatrices = new btTransform[maxNumberOfInstances()];
            for(GLsizei i = 0; i < maxNumberOfInstances(); i++)
            {
                transforms[i] = getTransform(i);
                normalMatrices[i] = getNormalMatrixTransform(i);
            }
            
            TexturedColoredVertex *vertexData = new TexturedColoredVertex[numberOfVertices() * maxNumberOfInstances()];
            GLuint *indiceData = new GLuint[numberOfIndices() * maxNumberOfInstances()];
            
            TexturedColoredVertex *triangleBuffer = new TexturedColoredVertex[12];
            GLuint *indiceBuffer = new GLuint[12];
            for (GLsizei currentIndice = 0, newIndice = 0;
                 currentIndice < previousNumberOfIndices;
                 currentIndice += 3, newIndice += 12)
            {
                subdivideTriangle(m_VertexData[currentIndice + 0],
                                  m_VertexData[currentIndice + 1],
                                  m_VertexData[currentIndice + 2],
                                  triangleBuffer,
                                  newIndice,
                                  indiceBuffer);
                
                for (GLsizei ii = 0; ii < 12; ii++)
                {
                    vertexData[newIndice + ii]  = triangleBuffer[ii];
                    indiceData[newIndice + ii]  = indiceBuffer[ii];
                }
            }
            delete [] indiceBuffer;
            delete [] triangleBuffer;
            
            enableVertexArrayBufferChanged(true);
            enableIndiceArrayBufferChanged(true);
            
            ++m_TotalSubdivisions;
            
            unsigned long vertexInstanceIndex = 0;
            unsigned long indiceInstanceIndex = 0;
            for (unsigned long meshIndex = 0;
                 meshIndex < maxNumberOfInstances();
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
            
            delete [] indiceData;
            delete [] vertexData;
            
//            unsigned long i;
//            for (i = 0;
//                 i < (maxNumberOfInstances() * numberOfVertices() * 16);
//                 i += 16)
//            {
//                memcpy(m_ModelViewTransformData + i, TRANSFORM_IDENTITY_MATRIX, sizeof(TRANSFORM_IDENTITY_MATRIX));
//            }
//            
//            for (i = 0;
//                 i < (maxNumberOfInstances() * numberOfVertices() * 16);
//                 i += 16)
//            {
//                memcpy(m_NormalMatrixTransformData + i, TRANSFORM_IDENTITY_MATRIX, sizeof(TRANSFORM_IDENTITY_MATRIX));
//            }

            for(GLsizei i = 0; i < maxNumberOfInstances(); i++)
            {
                setTransform(i, transforms[i]);
                setNormalMatrixTransform(i, normalMatrices[i]);
            }
            delete [] normalMatrices;
            delete [] transforms;
            
            enableModelViewBufferChanged(true);
            enableNormalMatrixBufferChanged(true);
        }
    }
    
    void MeshGeometry::subdivideTriangle(TexturedColoredVertex p0,
                                         TexturedColoredVertex p1,
                                         TexturedColoredVertex p2,
                                         TexturedColoredVertex *triangleBuffer,
                                         GLsizei newIndiceStart,
                                         GLuint *indiceBuffer)
    {
        TexturedColoredVertex p0p1 = TexturedColoredVertex::average(p0, p1);
        TexturedColoredVertex p1p2 = TexturedColoredVertex::average(p1, p2);
        TexturedColoredVertex p2p0 = TexturedColoredVertex::average(p2, p0);
        
        triangleBuffer[ 0] = p0;    
        triangleBuffer[ 1] = p0p1;  
        triangleBuffer[ 2] = p2p0;  
        
        triangleBuffer[ 3] = p0p1;  
        triangleBuffer[ 4] = p1;    
        triangleBuffer[ 5] = p1p2;  
        
        triangleBuffer[ 6] = p2p0;  
        triangleBuffer[ 7] = p1p2;  
        triangleBuffer[ 8] = p2;    
        
        triangleBuffer[ 9] = p0p1;  
        triangleBuffer[10] = p1p2;  
        triangleBuffer[11] = p2p0;
        
        for(GLsizei i = 0; i < 12; i++)
            indiceBuffer[i] = newIndiceStart + i;
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
            
            t.color = btVector4(1.0f, 1.0f, 1.0f, 1.0f);
            
            vertexData[idx] = t;
            indiceData[idx] = (GLuint)idx;
        }
        
        Geometry::loadData();
        
        assert(m_VertexData == NULL);
        m_VertexData = new TexturedColoredVertex[numberOfVertices() * maxNumberOfInstances() * subdivisionBufferSize()];
        memset(m_VertexData, std::numeric_limits<float>::max(), getVertexArrayBufferSize() * subdivisionBufferSize());
        enableVertexArrayBufferChanged(true);
        
        assert(m_IndiceData == NULL);
        m_IndiceData = new GLuint[numberOfIndices() * maxNumberOfInstances() * subdivisionBufferSize()];
        memset(m_IndiceData, std::numeric_limits<float>::max(), getElementArrayBufferSize() * subdivisionBufferSize());
        enableIndiceArrayBufferChanged(true);
        
        unsigned long vertexInstanceIndex = 0;
        unsigned long indiceInstanceIndex = 0;
        for (unsigned long meshIndex = 0;
             meshIndex < maxNumberOfInstances();
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
        GLsizeiptr size = sizeof(TexturedColoredVertex) * numberOfVertices() * maxNumberOfInstances();
        return size;
    }
    
    const void *MeshGeometry::getElementArrayBufferPtr()const
    {
        return m_IndiceData;
    }
    
    GLsizeiptr MeshGeometry::getElementArrayBufferSize()const
    {
        GLsizeiptr size = sizeof(GLuint) * numberOfIndices() * maxNumberOfInstances();
        return size;
    }
    
    GLenum MeshGeometry::getElementIndexType()const
    {
        return GL_UNSIGNED_INT;
    }
    
    void MeshGeometry::setOpacity(Node *node)
    {
        long index = getGeometryIndex(node);
        
        if(index >=0 && m_VertexData)
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
        long index = getGeometryIndex(node);
        
        if(index >= 0 && m_VertexData)
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
        long index = getGeometryIndex(node);
        
        if(index >= 0 && m_VertexData)
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
