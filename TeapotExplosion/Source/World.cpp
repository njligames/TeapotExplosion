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

static unsigned long MAXIMUM_TEAPOTS = 10000;

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
        std::string vertexShaderData = loadASCIIFile("Shaders/StandardShader.vert");
        std::string fragmentShaderData = loadASCIIFile("Shaders/StandardShader.frag");
        
        glClearColor(0.7,0.7,0.7,1);
        
        glEnable(GL_DEPTH_TEST);
        glFrontFace(GL_CW);
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
        
        srand((unsigned int)time(0));
        
        m_CameraNode->addCamera(m_Camera);
        m_CameraNode->setOrigin(btVector3(0.0f, 0.0f, 0.0f));
        
        m_Scene->addActiveNode(m_CameraNode);
        m_Scene->addActiveCamera(m_Camera);
        m_Scene->getRootNode()->setOrigin(btVector3(0.0f, 0.0f, 10.0f));
        
        assert(m_Shader->load(vertexShaderData, fragmentShaderData));
        
        m_Geometry->load(m_Shader, objFileData);
        
        for (std::vector<Node*>::iterator i = m_TeapotNodes.begin();
             i != m_TeapotNodes.end();
             i++)
        {
            Node *node = *i;
            
            m_Scene->addActiveNode(node);
            m_Scene->getRootNode()->addChildNode(node);
            
            node->addGeometry(m_Geometry);
        }
        
        m_ShaderMap.insert(ShaderMapPair("Default", m_Shader));
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
        for (std::vector<Node*>::iterator i = m_TeapotNodes.begin();
             i != m_TeapotNodes.end();
             i++)
        {
            Node *node = *i;
            
            node->setNormalMatrix(node->getWorldTransform().getBasis().inverse().transpose());
        }
    }
    
    void World::render()
    {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        
        m_Scene->render();
    }
    
    void World::touch(World::TouchState state, float x, float y)
    {
        
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
    
    World::World():
    m_Shader(new Shader()),
    m_Geometry(new MeshGeometry()),
    m_Camera(new Camera()),
    m_CameraNode(new Node()),
    m_Scene(new Scene())
    {
        for (unsigned long i = 0; i < MAXIMUM_TEAPOTS; i++)
            m_TeapotNodes.push_back(new Node());
    }
    
    World::~World()
    {
        delete m_Scene;
        delete m_CameraNode;
        delete m_Camera;
        delete m_Geometry;
        delete m_Shader;
    }
}
