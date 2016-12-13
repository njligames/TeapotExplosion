//
//  PhysicsWorld.cpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#include "PhysicsWorld.hpp"

#include "btVector3.h"
#include "btDefaultCollisionConfiguration.h"
#include "btCollisionDispatcher.h"
#include "btDbvtBroadphase.h"
#include "btSequentialImpulseConstraintSolver.h"
#include "btDiscreteDynamicsWorld.h"
#include "btBox2dBox2dCollisionAlgorithm.h"
#include "btConvex2dConvex2dAlgorithm.h"
#include "btMinkowskiPenetrationDepthSolver.h"
#include "btGhostObject.h"
#ifdef BT_DEBUG
#include "btQuickprof.h"
#endif
#include "btCollisionAlgorithm.h"
#include "btCollisionWorld.h"
#include "btQuickprof.h"

#include "PhysicsBody.hpp"
#include "Camera.hpp"
#include "Node.hpp"
#include "PhysicsBodyRigid.hpp"

namespace jamesfolk
{
    void DefaultCustomPreTickCallback (btDynamicsWorld *world, btScalar timeStep)
    {
        btDiscreteDynamicsWorld *discreteDynamicsWorld = reinterpret_cast<btDiscreteDynamicsWorld*>(world);
        PhysicsWorld *physicsWorld = reinterpret_cast<PhysicsWorld*>(discreteDynamicsWorld->getWorldUserInfo());
        physicsWorld->ghostObjectCollisionTest();
    }
    
    
    void DefaultCustomPostTickCallback (btDynamicsWorld *world, btScalar timeStep)
    {
    }
    
    static void CustomNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo)
    {
        //        BT_PROFILE("CustomNearCallback");
        btCollisionObject *pCollisionObject0 = (btCollisionObject *)collisionPair.m_pProxy0->m_clientObject;
        btCollisionObject *pCollisionObject1 = (btCollisionObject *)collisionPair.m_pProxy1->m_clientObject;
        
        if (dispatcher.needsCollision(pCollisionObject0,pCollisionObject1))
        {
            const btCollisionObjectWrapper* parent = 0;
            btCollisionObjectWrapper obj0Wrap(parent,
                                              pCollisionObject0->getCollisionShape(),
                                              pCollisionObject0,
                                              pCollisionObject0->getWorldTransform(), 0, 0);
            btCollisionObjectWrapper obj1Wrap(parent,
                                              pCollisionObject1->getCollisionShape(),
                                              pCollisionObject1,
                                              pCollisionObject1->getWorldTransform(), 0, 0);
            
            
            //dispatcher will keep algorithms persistent in the collision pair
            if (!collisionPair.m_algorithm)
            {
                collisionPair.m_algorithm = dispatcher.findAlgorithm(&obj0Wrap,&obj1Wrap);
            }
            
            if (collisionPair.m_algorithm)
            {
                btManifoldResult contactPointResult(&obj0Wrap,&obj1Wrap);
                
                if (dispatchInfo.m_dispatchFunc == 		btDispatcherInfo::DISPATCH_DISCRETE)
                {
                    //discrete collision detection query
                    
                    if(collisionPair.m_algorithm != NULL)
                        collisionPair.m_algorithm->processCollision(&obj0Wrap,&obj1Wrap,dispatchInfo,&contactPointResult);
                } else
                {
                    //continuous collision detection query, time of impact (toi)
                    btScalar toi = collisionPair.m_algorithm->calculateTimeOfImpact(pCollisionObject0,pCollisionObject1,dispatchInfo,&contactPointResult);
                    if (dispatchInfo.m_timeOfImpact > toi)
                        dispatchInfo.m_timeOfImpact = toi;
                    
                }
                
                Node *node0 = static_cast<Node*>(pCollisionObject0->getUserPointer());
                Node *node1 = static_cast<Node*>(pCollisionObject1->getUserPointer());
                PhysicsBody *body0 = NULL;
                PhysicsBody *body1 = NULL;
                
                if (node0)
                    body0 = node0->getPhysicsBody();
                
                if (node1)
                    body1 = node1->getPhysicsBody();
                
                
                if(body0 && body1)
                {
                    body0->handleCollisionNear(body1, dispatchInfo);
                    body1->handleCollisionNear(body0, dispatchInfo);
                }
            }
        }
        //        dispatcher.defaultNearCallback(collisionPair, dispatcher, dispatchInfo);
    }
    
    static inline btScalar	CustomCalculateCombinedFriction(float friction0,float friction1)
    {
        btScalar friction = friction0 * friction1;
        
        const btScalar MAX_FRICTION  = 10.f;
        if (friction < -MAX_FRICTION)
            friction = -MAX_FRICTION;
        if (friction > MAX_FRICTION)
            friction = MAX_FRICTION;
        return friction;
        
    }
    
    static inline btScalar	CustomCalculateCombinedRestitution(float restitution0,float restitution1)
    {
        return restitution0 * restitution1;
    }
    
    static bool CustomContactAddedCallback(btManifoldPoint& cp,
                                           const btCollisionObjectWrapper* colObj0Wrap,
                                           int partId0,
                                           int index0,
                                           const btCollisionObjectWrapper* colObj1Wrap,
                                           int partId1,
                                           int index1)
    {
        btCollisionObject *pCollisionObject0 = const_cast<btCollisionObject *>(colObj0Wrap->getCollisionObject());
        btCollisionObject *pCollisionObject1 = const_cast<btCollisionObject *>(colObj1Wrap->getCollisionObject());
        
        cp.m_combinedFriction = CustomCalculateCombinedFriction(pCollisionObject0->getFriction(),pCollisionObject1->getFriction());
        cp.m_combinedRestitution = CustomCalculateCombinedRestitution(pCollisionObject0->getRestitution(),pCollisionObject1->getRestitution());
        cp.m_combinedRollingFriction = CustomCalculateCombinedFriction(pCollisionObject0->getRollingFriction(), pCollisionObject1->getRollingFriction());
        
        //        Node *node0 = static_cast<Node*>(pCollisionObject0->getUserPointer());
        //        Node *node1 = static_cast<Node*>(pCollisionObject1->getUserPointer());
        //        PhysicsBody *body0 = node0->getPhysicsBody();
        //        PhysicsBody *body1 = node1->getPhysicsBody();
        //
        //        if (node0 && node1 && body0 && body1)
        //        {
        //            body0->handleCollide(body1, cp);
        //            body1->handleCollide(body0, cp);
        //        }
        
        Node *node0 = static_cast<Node*>(pCollisionObject0->getUserPointer());
        Node *node1 = static_cast<Node*>(pCollisionObject1->getUserPointer());
        PhysicsBody *body0 = NULL;
        PhysicsBody *body1 = NULL;
        
        if (node0)
            body0 = node0->getPhysicsBody();
        
        if (node1)
            body1 = node1->getPhysicsBody();
        
        
        if(body0 && body1)
        {
            body0->handleCollide(body1, cp);
            body1->handleCollide(body0, cp);
        }
        
        
        
        
        //this return value is currently ignored, but to be on the safe side: return false if you don't calculate friction
        return true;
    }
    
    static bool CustomContactProcessedCallback(btManifoldPoint& cp,
                                               void* body0,void* body1)
    {
        return true;
    }
    
    static bool CustomContactDestroyedCallback(void* userPersistentData)
    {
        return true;
    }
    
    
    
    PhysicsWorld::CustomFilterCallback::CustomFilterCallback()
    {
        
    }
    PhysicsWorld::CustomFilterCallback::~CustomFilterCallback()
    {
        
    }
    
    // return true when pairs need collision
    bool PhysicsWorld::CustomFilterCallback::needBroadphaseCollision(btBroadphaseProxy* proxy0,btBroadphaseProxy* proxy1) const
    {
        bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
        collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);
        
        return collides;
    }
    
    PhysicsWorld::PhysicsWorld():
    m_SimulationSpeed(1.0f),
    m_TimeStep(0.0f),
    m_collisionConfiguration(new btDefaultCollisionConfiguration()),
    m_dispatcher(new btCollisionDispatcher(m_collisionConfiguration)),
    m_overlappingPairCache(new btDbvtBroadphase()),
    m_solver(new btSequentialImpulseConstraintSolver),
    m_dynamicsWorld(new btDiscreteDynamicsWorld(m_dispatcher,
                                                m_overlappingPairCache,
                                                m_solver,
                                                m_collisionConfiguration)),
    m_btOverlapFilterCallback(new CustomFilterCallback()),
    m_btGhostPairCallback(new btGhostPairCallback()),
    m_GhostObjects(new btAlignedObjectArray<btPairCachingGhostObject*>()),
    m_Paused(false)
    {
        m_dynamicsWorld->setGravity(btVector3(0,0,0));
        
        m_dispatcher->setNearCallback(CustomNearCallback);
        m_dynamicsWorld->getPairCache()->setOverlapFilterCallback(m_btOverlapFilterCallback);
        m_dynamicsWorld->setInternalTickCallback(DefaultCustomPreTickCallback,NULL,true);
        m_dynamicsWorld->setInternalTickCallback(DefaultCustomPostTickCallback,NULL,false);
        m_dynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(m_btGhostPairCallback);
        
        m_dynamicsWorld->setWorldUserInfo(this);
        
//        m_dynamicsWorld->setDebugDrawer(jamesfolk::World::getInstance()->getDebugDrawer());
    }
    
    PhysicsWorld::~PhysicsWorld()
    {
        delete m_pdSolver;
        m_pdSolver = NULL;
        
        delete m_simplexSolver;
        m_simplexSolver = NULL;
        
        delete m_GhostObjects;
        m_GhostObjects = NULL;
        
        delete m_btGhostPairCallback;
        m_btGhostPairCallback = NULL;
        
        delete m_btOverlapFilterCallback;
        m_btOverlapFilterCallback = NULL;
        
        for (int i=m_dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
        {
            btCollisionObject* obj =
            m_dynamicsWorld->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if (body && body->getMotionState())
            {
                btMotionState *ms = body->getMotionState();
                delete ms;ms=NULL;
            }
            m_dynamicsWorld->removeCollisionObject( obj );
            delete obj;obj=NULL;
        }
        
        delete m_dynamicsWorld;m_dynamicsWorld=NULL;
        delete m_solver;m_solver=NULL;
        delete m_overlappingPairCache;m_overlappingPairCache=NULL;
        delete m_dispatcher;m_dispatcher=NULL;
        delete m_collisionConfiguration;m_collisionConfiguration=NULL;
    }
    
    void PhysicsWorld::update(float timeStep,int maxSubSteps, float fixedTimeStep)
    {
        BT_PROFILE("PhysicsWorld::update");
        
        gContactAddedCallback = CustomContactAddedCallback;
        gContactProcessedCallback = CustomContactProcessedCallback;
        gContactDestroyedCallback = CustomContactDestroyedCallback;
        
        m_TimeStep = timeStep;
        
        if(!m_Paused)
            m_dynamicsWorld->stepSimulation(m_TimeStep * m_SimulationSpeed, maxSubSteps, fixedTimeStep);
    }
    
    void PhysicsWorld::render(Camera *camera)
    {
//        camera->cull((btDbvtBroadphase*)m_overlappingPairCache);
    }
    
    void PhysicsWorld::setGravity(const btVector3 &gravity)
    {
        m_dynamicsWorld->setGravity(gravity);
    }
    btVector3 PhysicsWorld::getGravity()const
    {
        return m_dynamicsWorld->getGravity();
    }
    
    void PhysicsWorld::setSimulationSpeed(const float speed)
    {
        m_SimulationSpeed = speed;
    }
    
    float PhysicsWorld::getSimulationSpeed()const
    {
        return m_SimulationSpeed;
    }
    
    float PhysicsWorld::getTimeStep()const
    {
        return m_TimeStep;
    }
    
    void PhysicsWorld::setPreTickCallback(btInternalTickCallback callback)
    {
        m_dynamicsWorld->setInternalTickCallback(callback,NULL,true);
    }
    
    void PhysicsWorld::setPostTickCallback(btInternalTickCallback callback)
    {
        m_dynamicsWorld->setInternalTickCallback(callback,NULL,false);
    }
    
//    bool PhysicsWorld::rayTestClosest(const btVector3 &rayFromWorld,
//                                      const btVector3 &rayToWorld,
//                                      PhysicsRayContact &rayContact,
//                                      njliBitCategories collisionGroup,
//                                      njliBitCategories collisionMask)const
//    {
//        btCollisionWorld::ClosestRayResultCallback rayCallback(rayFromWorld, rayToWorld);
//        rayCallback.m_collisionFilterGroup = collisionGroup;
//        rayCallback.m_collisionFilterMask = collisionMask;
//        rayCallback.m_flags = 0;
//        
//        m_dynamicsWorld->rayTest(rayFromWorld, rayToWorld, rayCallback);
//        
//        if(rayCallback.hasHit())
//        {
//            //            Node *😀 = NULL;
//            Node *node = static_cast<Node*>(rayCallback.m_collisionObject->getUserPointer());
//            node->enableTouched();
//            rayContact.set(rayCallback.m_closestHitFraction,
//                           rayCallback.m_hitNormalWorld,
//                           rayCallback.m_hitPointWorld,
//                           node);
//            return true;
//        }
//        return false;
//    }
//    
//    bool PhysicsWorld::rayTestAll(const btVector3 &rayFromWorld,
//                                  const btVector3 &rayToWorld,
//                                  btAlignedObjectArray<PhysicsRayContact*> &rayContacts,
//                                  int &numContacts,
//                                  njliBitCategories collisionGroup,
//                                  njliBitCategories collisionMask)const
//    {
//        btAlignedObjectArray<Node*> collisionNodes;
//        
//        btCollisionWorld::AllHitsRayResultCallback rayCallback(rayFromWorld, rayToWorld);
//        rayCallback.m_collisionFilterGroup = collisionGroup;
//        rayCallback.m_collisionFilterMask = collisionMask;
//        rayCallback.m_flags = 0;
//        
//        m_dynamicsWorld->rayTest(rayFromWorld, rayToWorld, rayCallback);
//        
//        if(rayCallback.hasHit())
//        {
//            numContacts = rayCallback.m_collisionObjects.size();
//            for (int i = 0; i < numContacts; ++i)
//            {
//                if (i < rayContacts.size())
//                {
//                    rayContacts.at(i)->set(rayCallback.m_hitFractions[i],
//                                           rayCallback.m_hitNormalWorld[i],
//                                           rayCallback.m_hitPointWorld[i],
//                                           static_cast<Node*>(rayCallback.m_collisionObjects[i]->getUserPointer()));
//                }
//                else
//                {
//                    DEBUG_LOG_PRINT_E(TAG, "The btAlignedObjectArray is not large enough (%d of %d)", i, rayContacts.size());
//                }
//            }
//            return true;
//        }
//        return false;
//    }
    
    void PhysicsWorld::enablePause(bool enable)
    {
        m_Paused = enable;
    }
    
    bool PhysicsWorld::isPaused()const
    {
        return m_Paused;
    }
    
    void PhysicsWorld::debugDrawWorld()
    {
        m_dynamicsWorld->debugDrawWorld();
    }
    
    bool PhysicsWorld::addRigidBody(PhysicsBodyRigid *const body)
    {
        if(body)
        {
            if(!body->isInWorld())
            {
                
                assert(body->getBody());
//                assert(body->getNodeOwner());
                
                m_dynamicsWorld->addRigidBody(body->getBody(),
                                              body->getCollisionGroup(),
                                              body->getCollisionMask());
                
//                body->getCollisionObject()->setUserPointer(body->getNodeOwner());
            }
            return true;
        }
        return false;
        
    }
    bool PhysicsWorld::removeRigidBody(PhysicsBodyRigid *const body)
    {
        if(body && body->getBody())
        {
            if (body->isInWorld())
            {
                assert(body->getBody());
                
                m_dynamicsWorld->removeRigidBody(body->getBody());
                body->getCollisionObject()->setUserPointer(NULL);
                
                return true;
            }
        }
        return false;
    }
    
    void PhysicsWorld::ghostObjectCollisionTest()
    {
        if(NULL == m_GhostObjects)
            return;
        
        int size = m_GhostObjects->size();
        
        for(int ghostIndex = 0; ghostIndex < size; ghostIndex++)
        {
            btPairCachingGhostObject *pCurrentGhostObject = (*m_GhostObjects)[ghostIndex];
            
            // Prepare for getting all the contact manifolds for one Overlapping Pair
            btManifoldArray   manifoldArray;
            // Get all the Overlapping Pair
            btBroadphasePairArray& pairArray = pCurrentGhostObject->getOverlappingPairCache()->getOverlappingPairArray();
            int numPairs = pairArray.size();
            
            for (int pairIndex=0;pairIndex<numPairs;pairIndex++)
            {
                manifoldArray.clear();
                
                const btBroadphasePair& pair = pairArray[pairIndex];
                
                //unless we manually perform collision detection on this pair, the contacts are in the dynamics world paircache:
                //The next line fetches the collision information for this Pair
                btBroadphasePair* collisionPair = m_dynamicsWorld->getPairCache()->findPair(pair.m_pProxy0,pair.m_pProxy1);
                if (!collisionPair)
                    continue;
                
                // Read out the all contact manifolds for this Overlapping Pair
                if (collisionPair->m_algorithm)
                    collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);
                
                for (int j=0;j<manifoldArray.size();j++)
                {
                    btPersistentManifold* manifold = manifoldArray[j];
                    
                    const btCollisionObject *pCollisionObject0 = (const btCollisionObject *)manifold->getBody0();
                    const btCollisionObject *pCollisionObject1 = (const btCollisionObject *)manifold->getBody1();
                    
                    Node *node0 = static_cast<Node*>(pCollisionObject0->getUserPointer());
                    Node *node1 = static_cast<Node*>(pCollisionObject1->getUserPointer());
                    
                    for (int p=0;p<manifold->getNumContacts();p++)
                    {
                        const btManifoldPoint&pt = manifold->getContactPoint(p);
                        if (pt.getDistance()<0.f)
                        {
                            if(pCollisionObject0 == pCurrentGhostObject)
                                node0->getPhysicsBody()->handleCollide(node1->getPhysicsBody(),
                                                                       pt);
                            else
                                node1->getPhysicsBody()->handleCollide(node0->getPhysicsBody(),
                                                                       pt);
                        }
                    }
                }
            }
        }
    }
}
