//
//  PhysicsBody.hpp
//  VerizonTest
//
//  Created by James Folk on 6/30/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#ifndef PhysicsBody_hpp
#define PhysicsBody_hpp

#include "btBroadphaseProxy.h"
#include "btTransform.h"

class btManifoldPoint;
struct btDispatcherInfo;
class btCollisionObject;
class btVector3;

namespace jamesfolk
{
    class PhysicsShape;
    class Node;
    
    class PhysicsBody
    {
        friend class Node;
    public:
        
        enum PhysicsType
        {
            PhysicsType_NONE,
            PhysicsType_Static,
            PhysicsType_Dynamic,
            PhysicsType_Kinematic,
            PhysicsType_MAX
        };
        
        enum BitCategory
        {
            BitCategory_NONE = 0,
            BitCategory_1 = 1 << 0,
            BitCategory_2 = 1 << 1,
            BitCategory_3 = 1 << 2,
            BitCategory_4 = 1 << 3,
            BitCategory_5 = 1 << 4,
            BitCategory_6 = 1 << 5,
            BitCategory_7 = 1 << 6,
            BitCategory_8 = 1 << 7,
            BitCategory_9 = 1 << 8,
            BitCategory_10 = 1 << 9,
            BitCategory_11 = 1 << 10,
            BitCategory_12 = 1 << 11,
            BitCategory_13 = 1 << 12,
            BitCategory_14 = 1 << 13,
            BitCategory_15 = 1 << 14,
            BitCategory_16 = 1 << 15,
            BitCategory_17 = 1 << 16,
            BitCategory_18 = 1 << 17,
            BitCategory_19 = 1 << 18,
            BitCategory_20 = 1 << 19,
            BitCategory_21 = 1 << 20,
            BitCategory_22 = 1 << 21,
            BitCategory_23 = 1 << 22,
            BitCategory_24 = 1 << 23,
            BitCategory_25 = 1 << 24,
            BitCategory_26 = 1 << 25,
            BitCategory_27 = 1 << 26,
            BitCategory_28 = 1 << 27,
            BitCategory_29 = 1 << 28,
            BitCategory_30 = 1 << 29,
            BitCategory_31 = 1 << 30,
            BitCategory_ALL = 0xFFFFFFFF,
            BitCategory_DefaultFilter = btBroadphaseProxy::DefaultFilter,
            BitCategory_StaticFilter = btBroadphaseProxy::StaticFilter,
            BitCategory_KinematicFilter = btBroadphaseProxy::KinematicFilter,
            BitCategory_DebrisFilter = btBroadphaseProxy::DebrisFilter,
            BitCategory_SensorTrigger = btBroadphaseProxy::SensorTrigger,
            BitCategory_CharacterFilter = btBroadphaseProxy::CharacterFilter,
            BitCategory_AllFilter = btBroadphaseProxy::AllFilter,
        };
        
        PhysicsBody();
        virtual ~PhysicsBody();
        
    public:
        void handleCollide(PhysicsBody * body, const btManifoldPoint& pt);
        void handleCollisionNear(PhysicsBody * body, const btDispatcherInfo& dispatchInfo);
        virtual void addPhysicsShape(PhysicsShape *const shape);
        void removePhysicsShape();
        PhysicsShape *const getPhysicsShape()const;
        virtual void setWorldTransform(const btTransform& transform) = 0;
        virtual btTransform getWorldTransform() const;
        virtual void setFriction(float friction);
        virtual float getFriction() const;
        virtual void setAngularFriction(float friction);
        virtual float getAngularFriction() const;
        virtual void setRestitution(float restitution);
        virtual float getRestitution() const;
        virtual void setPhysicsType(PhysicsType type);
        virtual PhysicsType getPhysicsType() const;
        virtual void setCollisionGroup(BitCategory categoryBitMask);
        virtual BitCategory getCollisionGroup() const;
        virtual void setCollisionMask(BitCategory categoryBitMask);
        virtual BitCategory getCollisionMask() const;
        virtual void setStaticPhysics();
        virtual void setKinematicPhysics();
        virtual void setDynamicPhysics();
        virtual void enableContactResponse(bool enable = true);
        virtual void enableHandleCollideCallback(bool enable = true);
        bool isStaticPhysics() const;
        bool isKinematicPhysics() const;
        bool isDynamicPhysics() const;
        virtual bool isResting() const;
        virtual void enableResting(bool enable = true);
        bool isActive()const;
        virtual const btVector3& getVelocity() const = 0;
        virtual const btVector3& getAngularVelocity() const = 0;
        virtual bool isInWorld() const = 0;
        virtual void setMass(float mass) = 0;
        virtual float getMass()const = 0;
        
    protected:
        virtual btCollisionObject *const getCollisionObject() const = 0;
        virtual void setCollisionObject(const btCollisionObject& obj, PhysicsShape *const shape) = 0;
        
        virtual bool addPhysicsBody(const btTransform& transform = btTransform::getIdentity()) = 0;
        virtual bool removePhysicsBody() = 0;
        
        Node *const getNodeOwner()const;
        void setNodeOwner(Node *const node);
        
        int getCollisionFlags() const;
        int getActivationState() const;
        
    private:
        
        PhysicsShape* m_PhysicsShape;
        BitCategory m_CategoryBitMask;
        BitCategory m_CollisionBitMask;
        
        bool m_RestingAllowed;
        bool m_IsResting;
        
        int m_CollisionFlags;
        int m_ActivationState;
        
        Node *m_NodeOwner;
    };
}

#endif /* PhysicsBody_hpp */
