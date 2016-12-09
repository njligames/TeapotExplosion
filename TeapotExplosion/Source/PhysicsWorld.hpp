//
//  PhysicsWorld.hpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#ifndef PhysicsWorld_hpp
#define PhysicsWorld_hpp

#include "btOverlappingPairCache.h"
#include "btDynamicsWorld.h"

#include <vector>

class btVector3;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;

class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
struct btOverlapFilterCallback;
class btGhostPairCallback;
class btPairCachingGhostObject;
struct btBroadphaseProxy;
class btVoronoiSimplexSolver;
class btMinkowskiPenetrationDepthSolver;

namespace jamesfolk
{
    class Camera;
    class Node;
    class PhysicsBodyRigid;
    
    class PhysicsWorld
    {
    public:
        PhysicsWorld();
        virtual ~PhysicsWorld();
        
        class CustomFilterCallback : public btOverlapFilterCallback {
        public:
            /**
             *  <#Description#>
             */
            CustomFilterCallback();
            /**
             *  <#Description#>
             *
             *  @return <#return value description#>
             */
            virtual ~CustomFilterCallback();
            
            /**
             *  <#Description#>
             *
             *  @param proxy0 <#proxy0 description#>
             *  @param proxy1 <#proxy1 description#>
             *
             *  @return <#return value description#>
             */
            virtual bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const;
        };
        
        void update(float timeStep,int maxSubSteps=1, float fixedTimeStep=float(1.)/float(60.));
        void render(Camera * camera);
        void setGravity(const btVector3& gravity);
        btVector3 getGravity() const;
        void setSimulationSpeed(const float speed);
        float getSimulationSpeed() const;
        float getTimeStep() const;
        void setPreTickCallback(btInternalTickCallback callback);
        void setPostTickCallback(btInternalTickCallback callback);
//        bool rayTestClosest(const btVector3& rayFromWorld,
//                            const btVector3& rayToWorld,
//                            PhysicsRayContact& rayContact,
//                            njliBitCategories collisionGroup = JLI_BIT_CATEGORY_DefaultFilter,
//                            njliBitCategories collisionMask = JLI_BIT_CATEGORY_AllFilter) const;
//        bool rayTestAll(const btVector3& rayFromWorld,
//                        const btVector3& rayToWorld,
//                        btAlignedObjectArray<PhysicsRayContact*>& rayContacts,
//                        s32& numContacts,
//                        njliBitCategories collisionGroup = JLI_BIT_CATEGORY_DefaultFilter,
//                        njliBitCategories collisionMask = JLI_BIT_CATEGORY_AllFilter) const;
        
        void enablePause(bool enable = true);
        bool isPaused() const;
        
        void ghostObjectCollisionTest();
    protected:
        void debugDrawWorld();
        
    public:
        bool addRigidBody(PhysicsBodyRigid *const body);
        bool removeRigidBody(PhysicsBodyRigid *const body);
        
        
        
    private:
        
        
        float m_SimulationSpeed;
        float m_TimeStep;
        
        btDefaultCollisionConfiguration* m_collisionConfiguration;
        btCollisionDispatcher* m_dispatcher;
        
        btBroadphaseInterface* m_overlappingPairCache;
        
        btSequentialImpulseConstraintSolver* m_solver;
        btDiscreteDynamicsWorld* m_dynamicsWorld;
        btOverlapFilterCallback* m_btOverlapFilterCallback;
        btGhostPairCallback* m_btGhostPairCallback;
        
        btAlignedObjectArray<btPairCachingGhostObject*>* m_GhostObjects;
        
        btBroadphaseInterface* m_broadphase;
        btAlignedObjectArray<struct btBroadphaseProxy*> m_proxies;
        
        std::vector<Node*> m_collisionNodes;
        
        bool m_Paused;
        
        btVoronoiSimplexSolver* m_simplexSolver;
        btMinkowskiPenetrationDepthSolver* m_pdSolver;
    };
}

#endif /* PhysicsWorld_hpp */
