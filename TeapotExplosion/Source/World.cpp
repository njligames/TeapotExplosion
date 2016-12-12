//
//  World.cpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#include "World.hpp"
#include <stdlib.h>

#include "Shader.hpp"
#include "Geometry.hpp"
#include "MeshGeometry.hpp"
#include "Camera.hpp"
#include "Node.hpp"
#include "Scene.hpp"

static unsigned int MAXIMUM_TEAPOTS = 1;
static unsigned int MAXIMUM_SUBDIVISIONS = 3;

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
    
    void World::setBundlePath(const std::string &path)
    {
        s_BundlePath = path;
    }

    void World::create()
    {
        std::string objFileData = loadASCIIFile("Models/utah-teapot.obj");
//        std::string objFileData = loadASCIIFile("Models/triangle.obj");
//        std::string objFileData = loadASCIIFile("Models/triangle_subdivide.obj");
        
        glClearColor(0.0, 0.0, 0.0, 1.0);
        
        glEnable(GL_DEPTH_TEST);
        glFrontFace(GL_CW);
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
        
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
        for (std::vector<Node*>::iterator i = m_TeapotNodes.begin();
             i != m_TeapotNodes.end();
             i++)
        {
            Node *node = *i;
            
            m_Scene->addActiveNode(node);
            
            m_Scene->getRootNode()->addChildNode(node);
            
            node->addGeometry(m_Geometry);
            
            node->setOrigin(btVector3(0.0f, y, 0.0f));
            y += 0.1f;
        }
        
        
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
        glViewport(x, y, width, height);
        
        m_Camera->setAspectRatio(fabs(width / height));
    }
    
    void World::update(float step)
    {
//        int ii = 0;
//        for (std::vector<Node*>::iterator i = m_TeapotNodes.begin();
//             i != m_TeapotNodes.end();
//             i++)
//        {
//            Node *node = *i;
//            
//            node->setNormalMatrix(node->getWorldTransform().getBasis().inverse().transpose());
//            
//            btQuaternion rotX(btVector3(1.0, 0.0, 0.0), m_RotationX);
//            btQuaternion rotY(btVector3(0.0, 1.0, 0.0), m_RotationY);
//            btQuaternion rotZ(btVector3(0.0, 0.0, 1.0), m_RotationZ);
//            
//            btQuaternion q(rotX * rotY * rotZ);
//            q = rotX;
//            node->setRotation(q);
//            
//            float s = step * (ii + 1);
//            m_RotationX += s;
//            m_RotationY += s;
//            m_RotationZ += s;
//            
//            ii++;
//        }
    }
    
    void World::render()
    {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        
        m_Scene->render();
    }
    
    void World::touch(World::TouchState state, float x, float y)
    {
        if(state == World::TouchState_Down)
        {
            m_Geometry->subdivide();
            m_Scene->getRootNode()->setOrigin(btVector3(0.0f, 0.0f, 5.0f));
        }
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
    
    World::World():
    m_Geometry(new MeshGeometry()),
    m_Camera(new Camera()),
    m_CameraNode(new Node()),
    m_Scene(new Scene()),
    m_NumberOfTeapots(1),
    m_RotationX(0.0f),
    m_RotationY(0.0f),
    m_RotationZ(0.0f)
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
