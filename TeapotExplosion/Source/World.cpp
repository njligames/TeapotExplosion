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

static unsigned int MAXIMUM_TEAPOTS = 6;
static unsigned int MAXIMUM_SUBDIVISIONS = 3;

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
    
    void World::setBundlePath(const std::string &path)
    {
        s_BundlePath = path;
    }

    void World::create()
    {
        std::string objFileData = loadASCIIFile("Models/utah-teapot-lowpoly.obj");
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
        
        float y = 0.2f;
        float z = 0.0f;
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
            
            node->setColorBase(btVector4(randomFloat(0.9f, 1.0f), randomFloat(0.9f, 1.0f), randomFloat(0.9f, 1.0f), 1.0f));
        }
        
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
            
            if(t.state == World::TouchState_Down)
            {
                //            subdivideTeapots();
                explodeTeapots();
            }
            if(t.state == World::TouchState_Up)
            {
                //            explodeTeapots();
            }
        }
        m_Touches.clear();
//        btQuaternion rotX(btVector3(1.0, 0.0, 0.0), m_Rotation);
//        btQuaternion rotY(btVector3(0.0, 1.0, 0.0), m_Rotation);
//        btQuaternion rotZ(btVector3(0.0, 0.0, 1.0), m_Rotation);
        
//        int ii = 0;
        for (std::vector<Node*>::iterator i = m_TeapotNodes.begin();
             i != m_TeapotNodes.end();
             i++)
        {
            Node *node = *i;
            
            
            node->setNormalMatrix(node->getWorldTransform().getBasis().inverse().transpose());
            
//            if(ii % 3 == 0)
//            {
//                node->setRotation(rotX);
//            }
//            else if(ii % 3 == 1)
//            {
//                node->setRotation(rotY);
//            }
//            else if(ii % 3 == 2)
//            {
//                node->setRotation(rotZ);
//            }
//            ii++;
        }
        
//        m_Rotation += step;
//        
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
    
//    void World::touch(World::TouchState state, float x, float y)
//    {
//        if(state == World::TouchState_Down)
//        {
////            subdivideTeapots();
//            explodeTeapots();
//        }
//        if(state == World::TouchState_Up)
//        {
////            explodeTeapots();
//        }
//    }
    
    void World::addTouch(World::TouchState state, const btVector2 &touch)
    {
        World::Touch t;
        t.state = state;
        t.touch = touch;
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
        float min = 0.1;
        float max = 3.0;
        for (GLsizei i = 0; i < m_NumberOfTriangles; i++)
        {
            btVector3 force(m_Normals[i] * btVector3(randomFloat(min, max),
                                                     randomFloat(min, max),
                                                     randomFloat(min, max)));
            
            m_ImpulseForce[i] = force;
        }
    }
    
    void World::subdivideTeapots()
    {
        m_Geometry->subdivide();
        m_Scene->getRootNode()->setOrigin(btVector3(0.0f, 0.0f, 5.0f));
        resetTeapots();
    }
    
    void World::resetTeapots()
    {
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
            
            verticeIdx += 3;
        }
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
    m_NumberOfTriangles(0)
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
