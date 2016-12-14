//
//  World.cpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#include "World.hpp"
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Shader.hpp"
#include "Geometry.hpp"
#include "MeshGeometry.hpp"
#include "Camera.hpp"
#include "Node.hpp"
#include "Scene.hpp"


static unsigned int MAXIMUM_TEAPOTS = 10;
static unsigned int MAXIMUM_SUBDIVISIONS = 2;

static float randomFloat(float min, float max)
{
    float r = (float)rand() / (float)RAND_MAX;
    return min + r * (max - min);
}

static const char *SHADERNAMES[] =
{
    "StandardShader",
    "PassThrough",
    0
};

namespace jamesfolk
{
    World *World::s_Instance = NULL;
    std::string World::s_BundlePath = "";
    
    World *const World::getInstance()
    {
        if(NULL == s_Instance)
            s_Instance = new World();
        return s_Instance;
    }
    
    void World::createInstance()
    {
        if(NULL == s_Instance)
            s_Instance = new World();
    }
    
    void World::destroyInstance()
    {
        if(NULL != s_Instance)
            delete s_Instance;
        s_Instance = NULL;
    }
    
    std::string World::loadASCIIFile(const std::string &filepath)
    {
        std::string filedata("");
        
        std::string fullPath = s_BundlePath + filepath;
        
        FILE *file = fopen(fullPath.c_str(), "r");
        
        if(file)
        {
            fseek(file, 0, SEEK_END);
            long fileSize = ftell(file);
            fseek(file, 0, SEEK_SET);
            
            char* buffer = (char*)malloc(fileSize);
            buffer[fileSize] = '\0';
            
            fread(buffer, 1, fileSize, file);
            filedata = std::string(buffer);
            
            fclose(file);
            free(buffer);
        }
        
        return filedata;
    }
    
    void World::loadPngFile(const std::string &filepath, GLubyte **row_pointers)
    {
        int width;
        int height;
        
        std::string fullPath = s_BundlePath + filepath;
        
        int components;
        *row_pointers = (GLubyte*)stbi_load(fullPath.c_str(), &width, &height, &components, 0);
        
        if(components == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, *row_pointers);
        if(components == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, *row_pointers);
    }
    
    void World::setBundlePath(const std::string &path)
    {
        s_BundlePath = path;
    }

    void World::create()
    {
        m_AmbientTexture = loadTexture("Images/Marble_COLOR.png", 0, &m_AmbientTextureImage);
        m_DiffuseTexture = loadTexture("Images/Marble_COLOR.png", 1, &m_DiffuseTextureImage);
        m_NormalTexture = loadTexture("Images/Marble_NRM.png", 2, &m_NormalTextureImage);
        m_SpecularTexture = loadTexture("Images/Marble_SPEC.png", 3, &m_SpecularTextureImage);
        
        
        std::string objFileData = loadASCIIFile("Models/utah-teapot-lowpoly.obj");
//        std::string objFileData = loadASCIIFile("Models/triangle.obj");
//        std::string objFileData = loadASCIIFile("Models/triangle_subdivide.obj");
        
        glClearColor(0.0, 0.0, 0.0, 1.0);
        
        glEnable(GL_DEPTH_TEST);
        glFrontFace(GL_CW);
//        glCullFace(GL_BACK);
//        glEnable(GL_CULL_FACE);
        
//        glEnable(GL_TEXTURE_2D);
//        glEnable(GL_BLEND);
//        glBlendFunc(GL_ONE, GL_SRC_COLOR);
        
        srand((unsigned int)time(0));
        
        m_CameraNode->addCamera(m_Camera);
        m_CameraNode->setOrigin(btVector3(0.0f, 0.0f, 0.0f));
        
        m_Scene->addActiveNode(m_CameraNode);
        m_Scene->addActiveCamera(m_Camera);
        
        unsigned long i = 0;
        for (std::vector<Shader*>::iterator iter = m_Shaders.begin();
             iter != m_Shaders.end();
             iter++)
        {
            Shader *shader = *iter;
            std::string shaderName = SHADERNAMES[i++];
            std::string vertexShader = loadASCIIFile(std::string("Shaders/") + shaderName + std::string(".vert"));
            std::string fragmentShader = loadASCIIFile(std::string("Shaders/") + shaderName + std::string(".frag"));
            
            assert(shader->load(vertexShader, fragmentShader));
            m_ShaderMap.insert(ShaderMapPair(shaderName, shader));
            
        }
        
        m_Scene->getRootNode()->setOrigin(btVector3(0.0f, 0.0f, 5.0f));
        
        m_Geometry->load(m_Shaders[0], objFileData, MAXIMUM_TEAPOTS, MAXIMUM_SUBDIVISIONS);
        
        float y = 0.0f;
        float z = -3.0f;
        for (std::vector<Node*>::iterator i = m_TeapotNodes.begin();
             i != m_TeapotNodes.end();
             i++)
        {
            Node *node = *i;
            
            m_Scene->addActiveNode(node);
            
            m_Scene->getRootNode()->addChildNode(node);
            
            node->addGeometry(m_Geometry);
            
            node->setOrigin(btVector3(0.0f, y, z));
            y += 1.1f;
            z += 3.0;
            
//            node->setColorBase(btVector4(randomFloat(0.9f, 1.0f), randomFloat(0.9f, 1.0f), randomFloat(0.9f, 1.0f), 1.0f));
        }
        
        m_Geometry->setAmbientTexture(m_AmbientTexture);
        m_Geometry->setDiffuseTexture(m_DiffuseTexture);
        m_Geometry->setNormalTexture(m_NormalTexture);
        m_Geometry->setSpecularTexture(m_SpecularTexture);
        
        resetTeapots();
        m_Touches.clear();
    }
    
    void World::destroy()
    {
        for (std::vector<Node*>::iterator i = m_TeapotNodes.begin();
             i != m_TeapotNodes.end();
             i++)
        {
            Node *node = *i;
            
            node->removeGeometry();
            
            m_Scene->removeActiveNode(node);
            m_Scene->getRootNode()->removeChildNode(node);
        }
    }

    void World::resize(float x, float y, float width, float height)
    {
        GLint params[4];
        glGetIntegerv(GL_VIEWPORT, params);
        
        if(params[2] != width && params[3] != height)
        {
            glViewport(x, y, width, height);
            
            m_Camera->setAspectRatio(fabs(width / height));
        }
    }
    
    void World::update(float step)
    {
        for (std::vector<Touch>::iterator i = m_Touches.begin();
             i != m_Touches.end();
             i++)
        {
            Touch t = *i;
        }
        m_Touches.clear();
        
        btQuaternion rotX(btVector3(1.0, 0.0, 0.0), m_Rotation);
        btQuaternion rotY(btVector3(0.0, 1.0, 0.0), m_Rotation);
        
        int ii = 0;
        for (std::vector<Node*>::iterator i = m_TeapotNodes.begin();
             i != m_TeapotNodes.end();
             i++)
        {
            Node *node = *i;
            
            
            node->setNormalMatrix(node->getWorldTransform().getBasis().inverse().transpose());
            
            if(ii == 0 && !isExploding())
            {
                node->setRotation(rotX * rotY);
                m_Rotation += step;
            }
            
            ii++;
        }
        
//        m_Scene->update(step);
        
        GLsizei verticeIdx = 0;
        GLsizei instanceIdx = 0;
        float mass = 1.0f;
        float maxSpeed = 1.0f;
        for (GLsizei i = 0; i < m_NumberOfTriangles; i++)
        {
            btVector3 acceleration(m_ImpulseForce[i] / mass);
            
            m_CurrentVelocity[i] = ((m_CurrentVelocity[i] + acceleration) * step);
            if(m_CurrentVelocity[i].length() > maxSpeed)
                m_CurrentVelocity[i] = m_CurrentVelocity[i].normalized() * maxSpeed;
            
            m_TriangleShrapnelTransforms[i].setOrigin(m_TriangleShrapnelTransforms[i].getOrigin() + m_CurrentVelocity[i] * step);
            
            m_Geometry->transformVertice(instanceIdx, (verticeIdx + 0), m_TriangleShrapnelTransforms[i]);
            m_Geometry->transformVertice(instanceIdx, (verticeIdx + 1), m_TriangleShrapnelTransforms[i]);
            m_Geometry->transformVertice(instanceIdx, (verticeIdx + 2), m_TriangleShrapnelTransforms[i]);
            
            verticeIdx += 3;
            
            m_ImpulseForce[i] = btVector3(0,0,0);
        }
    }
    
    void World::render()
    {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        
        m_Scene->render();
    }
    
    void World::addTouch(World::TouchState state, const btVector2 &touch, unsigned long taps)
    {
        World::Touch t;
        t.state = state;
        t.touch = touch;
        t.taps = taps;
        m_Touches.push_back(t);
    }
    
    void World::setShader(const std::string &shader)
    {
        ShaderMap::iterator i = m_ShaderMap.find(shader);
        if(i != m_ShaderMap.end())
        {
            Shader *shader = (*i).second;
            
            m_Geometry->setShader(shader);
        }
    }
    
    void World::setNumberOfTeapots(const int num)
    {
        if(num < MAXIMUM_TEAPOTS)
            m_NumberOfTeapots = num;
    }
    
    int World::numberOfTeapots()const
    {
        return m_NumberOfTeapots;
    }
    
    void World::explodeTeapots()
    {
        if(!m_IsExploding)
        {
            float min = 0.1;
            float max = 10.0;
            for (GLsizei i = 0; i < m_NumberOfTriangles; i++)
            {
                btVector3 force(m_Normals[i] * btVector3(randomFloat(min, max),
                                                         randomFloat(min, max),
                                                         randomFloat(min, max)));
                
                m_ImpulseForce[i] = force;
            }
            m_IsExploding = true;
        }
    }
    
    void World::subdivideTeapots()
    {
        m_Geometry->subdivide();
        
        resetTeapots();
    }
    
    void World::resetTeapots()
    {
        m_IsExploding = false;
        
        if(m_Normals)
            delete [] m_Normals;
        m_Normals = NULL;
        
        if(m_CurrentVelocity)
            delete [] m_CurrentVelocity;
        if(m_ImpulseForce)
            delete [] m_ImpulseForce;
        if(m_TriangleShrapnelTransforms)
            delete [] m_TriangleShrapnelTransforms;
        
        m_NumberOfTriangles = m_Geometry->numberOfVertices() / 3;
        
        m_TriangleShrapnelTransforms = new btTransform[m_NumberOfTriangles];
        m_ImpulseForce = new btVector3[m_NumberOfTriangles];
        m_CurrentVelocity = new btVector3[m_NumberOfTriangles];
        m_Normals = new btVector3[m_NumberOfTriangles];
        
        GLsizei instanceIdx = 0;
        GLsizei verticeIdx = 0;
        for (GLsizei i = 0; i < m_NumberOfTriangles; i++)
        {
            m_TriangleShrapnelTransforms[i] = btTransform::getIdentity();
            
            m_ImpulseForce[i] = btVector3(0.0, 0.0, 0.0);
            m_CurrentVelocity[i] = btVector3(0.0, 0.0, 0.0);
            
            btVector3 n0(m_Geometry->getVertexNormal(instanceIdx, (verticeIdx + 0)));
            btVector3 n1(m_Geometry->getVertexNormal(instanceIdx, (verticeIdx + 1)));
            btVector3 n2(m_Geometry->getVertexNormal(instanceIdx, (verticeIdx + 2)));
            
            m_Normals[i] = (n0 + n1 + n2) / 3;
            if(m_Normals[i].length2() > 0)
                m_Normals[i].normalize();
            
            m_Geometry->setVerticeTransform(instanceIdx, (verticeIdx + 0), btTransform::getIdentity());
            m_Geometry->setVerticeTransform(instanceIdx, (verticeIdx + 1), btTransform::getIdentity());
            m_Geometry->setVerticeTransform(instanceIdx, (verticeIdx + 2), btTransform::getIdentity());
            
            verticeIdx += 3;
        }
        m_Scene->getRootNode()->setOrigin(btVector3(0.0f, 0.0f, 5.0f));
    }
    
    bool World::isExploding()const
    {
        return m_IsExploding;
    }
    
    bool World::isMaxTesselations()const
    {
        return m_Geometry->isMaxSubdivisions();
    }
    
    Geometry *const World::getGeometry()const
    {
        return m_Geometry;
    }
    
    GLuint World::loadTexture(const std::string &filepath, GLuint idx, GLubyte **outData)const
    {
        GLuint      texture[1];
        glGenTextures(1, &texture[0]);
        
        glActiveTexture(GL_TEXTURE0 + idx);
        
        glBindTexture ( GL_TEXTURE_2D, texture[0] );
        
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        loadPngFile(filepath, outData);
        
        return texture[0];
    }
    
    void World::setupCubeMap(GLuint& texture) {
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_CUBE_MAP);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    void World::setupCubeMap(GLuint& texture, const std::string &filepath_xpos, const std::string &filepath_xneg, const std::string &filepath_ypos, const std::string &filepath_yneg, const std::string &filepath_zpos, const std::string &filepath_zneg)
    {
        setupCubeMap(texture);
        
        int width;
        int height;
        int components;
        
        m_Cube_xpos = (GLubyte*)stbi_load(std::string(s_BundlePath + filepath_xpos).c_str(), &width, &height, &components, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, width, height, 0, components == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, m_Cube_xpos);
        
        m_Cube_xneg = (GLubyte*)stbi_load(std::string(s_BundlePath + filepath_xneg).c_str(), &width, &height, &components, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, width, height, 0, components == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, m_Cube_xneg);
        
        m_Cube_ypos = (GLubyte*)stbi_load(std::string(s_BundlePath + filepath_ypos).c_str(), &width, &height, &components, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, width, height, 0, components == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, m_Cube_ypos);
        
        m_Cube_yneg = (GLubyte*)stbi_load(std::string(s_BundlePath + filepath_yneg).c_str(), &width, &height, &components, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, width, height, 0, components == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, m_Cube_yneg);
        
        m_Cube_zpos = (GLubyte*)stbi_load(std::string(s_BundlePath + filepath_zpos).c_str(), &width, &height, &components, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, width, height, 0, components == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, m_Cube_zpos);
        
        m_Cube_zneg = (GLubyte*)stbi_load(std::string(s_BundlePath + filepath_zneg).c_str(), &width, &height, &components, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, width, height, 0, components == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, m_Cube_zneg);
    }
    
    void World::deleteCubeMap(GLuint& texture) {
        glDeleteTextures(1, &texture);
    }
    
    
    World::World():
    m_Geometry(new MeshGeometry()),
    m_Camera(new Camera()),
    m_CameraNode(new Node()),
    m_Scene(new Scene()),
    m_NumberOfTeapots(1),
    m_Rotation(0.0f),
    m_TriangleShrapnelTransforms(NULL),
    m_ImpulseForce(NULL),
    m_CurrentVelocity(NULL),
    m_Normals(NULL),
    m_NumberOfTriangles(0),
    m_IsExploding(false),
    m_AmbientTextureImage(NULL),
    m_SpecularTextureImage(NULL),
    m_NormalTextureImage(NULL),
    m_AmbientTexture(0),
    m_DiffuseTexture(0),
    m_SpecularTexture(0),
    m_NormalTexture(0)
    {
        for (unsigned long i = 0; i < MAXIMUM_TEAPOTS; i++)
            m_TeapotNodes.push_back(new Node());
        
        unsigned long i = 0;
        const char *shaderName = SHADERNAMES[i];
        do
        {
            m_Shaders.push_back(new Shader());
            shaderName = SHADERNAMES[++i];
        }
        while (shaderName != NULL);
    }
    
    World::~World()
    {
        
        if(m_AmbientTextureImage)
            free(m_AmbientTextureImage);
        m_AmbientTextureImage = NULL;
        
        if(m_DiffuseTextureImage)
            free(m_DiffuseTextureImage);
        m_DiffuseTextureImage = NULL;
        
        if(m_SpecularTextureImage)
            free(m_SpecularTextureImage);
        m_SpecularTextureImage = NULL;
        
        if(m_NormalTextureImage)
            free(m_NormalTextureImage);
        m_NormalTextureImage = NULL;
        
        if(m_DiffuseTexture)
            glDeleteTextures(1, &m_DiffuseTexture);
        m_DiffuseTexture = 0;
        
        if(m_AmbientTexture)
            glDeleteTextures(1, &m_AmbientTexture);
        m_AmbientTexture = 0;
        
        if(m_SpecularTexture)
            glDeleteTextures(1, &m_SpecularTexture);
        m_SpecularTexture = 0;
        
        if(m_NormalTexture)
            glDeleteTextures(1, &m_NormalTexture);
        m_NormalTexture = 0;
        
        if(m_CurrentVelocity)
            delete [] m_CurrentVelocity;
        m_CurrentVelocity = NULL;
        
        if(m_ImpulseForce)
            delete [] m_ImpulseForce;
        m_ImpulseForce = NULL;
        
        if(m_TriangleShrapnelTransforms)
            delete [] m_TriangleShrapnelTransforms;
        m_TriangleShrapnelTransforms = NULL;
        
        while(!m_Shaders.empty())
        {
            Shader *shader = m_Shaders.back();
            delete shader;
            
            m_Shaders.pop_back();
        }
        
        while(!m_TeapotNodes.empty())
        {
            Node *node = m_TeapotNodes.back();
            delete node;
            
            m_TeapotNodes.pop_back();
        }
        
        delete m_Scene;
        delete m_CameraNode;
        delete m_Camera;
        delete m_Geometry;
    }
}
