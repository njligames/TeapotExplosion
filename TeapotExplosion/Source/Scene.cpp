//
//  Scene.cpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#include "Scene.hpp"
#include "Node.hpp"
#include "Geometry.hpp"
#include "PhysicsWorld.hpp"

#include <assert.h>

namespace jamesfolk
{
    Scene::Scene():
    m_RootNode(new Node()),
    m_PhysicsWorld(new PhysicsWorld())
    {
        addActiveNode(m_RootNode);
    }
    
    Scene::~Scene()
    {
        delete m_PhysicsWorld;
        delete m_RootNode;
    }
    
    void Scene::update(float timeStep,int maxSubSteps, float fixedTimeStep)
    {
        for(std::vector<Node*>::iterator j = m_ActiveNodes.begin();
            j != m_ActiveNodes.end();
            j++)
        {
            Node *node = *j;
            node->update(timeStep);
        }
//        getPhysicsWorld()->update(timeStep, maxSubSteps, fixedTimeStep);
    }
    
    void Scene::render()
    {
        for(std::vector<Camera*>::iterator i = m_ActiveCameras.begin();
            i != m_ActiveCameras.end();
            i++)
        {
            m_ActiveGeometries.clear();
            
            Camera *camera = *i;
            
            for(std::vector<Node*>::iterator j = m_ActiveNodes.begin();
                j != m_ActiveNodes.end();
                j++)
            {
                Node *node = *j;
                
                Geometry *geometry = node->getGeometry();
                
                if(geometry)
                {
                    node->render(geometry);
                    
                    if(m_ActiveGeometries.end() == std::find(m_ActiveGeometries.begin(), m_ActiveGeometries.end(), geometry))
                    {
                        m_ActiveGeometries.push_back(geometry);
                    }
                }
            }
            
            for(std::vector<Geometry*>::iterator j = m_ActiveGeometries.begin();
                j != m_ActiveGeometries.end();
                j++)
            {
                Geometry *geometry = *j;
                
                geometry->render(camera);
            }
            for(std::vector<Node*>::iterator j = m_ActiveNodes.begin();
                j != m_ActiveNodes.end();
                j++)
                (*j)->resetTransformDirty();
        }
    }
    
    void Scene::addActiveCamera(Camera * camera)
    {
        assert(camera);
        
        std::vector<Camera*>::iterator i = std::find(m_ActiveCameras.begin(), m_ActiveCameras.end(), camera);
        if(i == m_ActiveCameras.end())
            m_ActiveCameras.push_back(camera);
    }
    
    void Scene::removeActiveCamera(Camera * camera)
    {
        assert(camera);
        
        std::vector<Camera*>::iterator i = std::find(m_ActiveCameras.begin(), m_ActiveCameras.end(), camera);
        
        m_ActiveCameras.erase(i);
    }
    
    void Scene::addActiveNode(Node * node)
    {
        assert(node);
        
        std::vector<Node*>::iterator i = std::find(m_ActiveNodes.begin(), m_ActiveNodes.end(), node);
        if(i == m_ActiveNodes.end())
            m_ActiveNodes.push_back(node);
    }
    
    void Scene::removeActiveNode(Node * node)
    {
        assert(node);
        
        std::vector<Node*>::iterator i = std::find(m_ActiveNodes.begin(), m_ActiveNodes.end(), node);
        
        m_ActiveNodes.erase(i);
    }
    
    Node *const Scene::getRootNode()const
    {
        return m_RootNode;
    }
    
    PhysicsWorld *const Scene::getPhysicsWorld()const
    {
        return m_PhysicsWorld;
    }
}

