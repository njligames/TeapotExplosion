//
//  PhysicsBody.cpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#include "PhysicsBody.hpp"

#include <vector>

#include "btCollisionObject.h"

#include "Node.hpp"

namespace jamesfolk
{
    inline char IsOn(const int value, const int bit)
    {
        int v = value & bit;
        char ret = (v == 0) ? 0 : 1;
        return ret;
    }
    
    inline unsigned int On(const int value, const int bit)//, bool on = true)
    {
        return value | bit;
    }
    
    inline unsigned int Off(const int value, const int bit)
    {
        if (IsOn(value, bit)) {
            return value ^ bit;
        }
        return value;
    }
    
    PhysicsBody::PhysicsBody():
    m_CategoryBitMask(BitCategory_ALL),
    m_CollisionBitMask(BitCategory_ALL),
    //    m_TotalForce(new btVector3()),
    //    m_TotalAngularForce(new btVector3()),
    m_RestingAllowed(true),
    m_IsResting(false),
    m_CollisionFlags(btCollisionObject::CF_STATIC_OBJECT),
    m_ActivationState(ACTIVE_TAG),
    m_NodeOwner(NULL)
    {
        
    }
    
    PhysicsBody::~PhysicsBody()
    {
        
    }
    
    void PhysicsBody::handleCollide(PhysicsBody *body, const btManifoldPoint&pt)
    {
        if (body->getPhysicsShape() && m_PhysicsShape)
        {
//            char buffer[BUFFER_SIZE] = "NodeCollide";
//            jamesfolk::World::getInstance()->getWorldLuaVirtualMachine()->execute(buffer, getParent(), body->getParent(), pt);
        }
    }
    
    void PhysicsBody::handleCollisionNear(PhysicsBody *body, const btDispatcherInfo& dispatchInfo)
    {
        if (m_PhysicsShape)
        {
//            char buffer[BUFFER_SIZE] = "NodeNear";
//            jamesfolk::World::getInstance()->getWorldLuaVirtualMachine()->execute(buffer, getParent(), body->getParent(), dispatchInfo);
        }
        
    }
    //    void PhysicsBody::addPhysicsWorld(PhysicsWorld *world)
    //    {
    //        assert(world != NULL);
    //
    //        if(getPhysicsWorld())
    //            removeChild(getPhysicsWorld());
    //
    //        m_PhysicsWorld = world;
    //
    //        addChild(m_PhysicsWorld);
    //    }
    //
    //    void PhysicsBody::removePhysicsWorld()
    //    {
    //        if(getPhysicsWorld())
    //            removeChild(getPhysicsWorld());
    //
    //        m_PhysicsWorld = NULL;
    //    }
    //
    //    PhysicsWorld *PhysicsBody::getPhysicsWorld()
    //    {
    //        int idx = getChildIndex(m_PhysicsWorld);
    //        if(idx != -1)
    //            return dynamic_cast<PhysicsWorld*>(getChild(idx));
    //        return NULL;
    //    }
    //
    //    const PhysicsWorld *PhysicsBody::getPhysicsWorld() const
    //    {
    //        int idx = getChildIndex(m_PhysicsWorld);
    //        if(idx != -1)
    //            return dynamic_cast<const PhysicsWorld*>(getChild(idx));
    //        return NULL;
    //    }
    
    void PhysicsBody::addPhysicsShape(PhysicsShape *const shape)
    {
        assert(shape != NULL);
        
        m_PhysicsShape = shape;
    
    }
    
    void PhysicsBody::removePhysicsShape()
    {
        m_PhysicsShape = NULL;
    }
    
    PhysicsShape *const PhysicsBody::getPhysicsShape()const
    {
        return m_PhysicsShape;
    }
    
    btTransform PhysicsBody::getWorldTransform()const
    {
        return getCollisionObject()->getWorldTransform();
    }
    
    void PhysicsBody::setFriction(float friction)
    {
        getCollisionObject()->setFriction(friction);
    }
    
    float PhysicsBody::getFriction()const
    {
        return getCollisionObject()->getFriction();
    }
    
    void PhysicsBody::setAngularFriction(float friction)
    {
        getCollisionObject()->setRollingFriction(friction);
    }
    
    float PhysicsBody::getAngularFriction()const
    {
        return getCollisionObject()->getRollingFriction();
    }
    
    void PhysicsBody::setRestitution(float restitution)
    {
        getCollisionObject()->setRestitution(restitution);
    }
    
    float PhysicsBody::getRestitution()const
    {
        return getCollisionObject()->getRestitution();
    }
    
    void PhysicsBody::setPhysicsType(PhysicsType type)
    {
        assert(type > PhysicsType_NONE &&
                     type < PhysicsType_MAX);
        
        switch (type)
        {
            case PhysicsType_Dynamic:
                setDynamicPhysics();
                break;
            case PhysicsType_Kinematic:
                setKinematicPhysics();
                break;
            case PhysicsType_Static:
                setStaticPhysics();
                break;
            default:
                break;
        }
        
        if(PhysicsBody::getNodeOwner() && getPhysicsShape())
        {
            btTransform t = getWorldTransform();
            addPhysicsBody(t);
        }
    }
    
    PhysicsBody::PhysicsType PhysicsBody::getPhysicsType()const
    {
        if(isKinematicPhysics())
            return PhysicsType_Kinematic;
        
        if(isStaticPhysics())
            return PhysicsType_Static;
        
        if(isDynamicPhysics())
            return PhysicsType_Dynamic;
        
        return PhysicsType_NONE;
    }
    
    void PhysicsBody::setCollisionGroup(BitCategory categoryBitMask)
    {
        m_CategoryBitMask = categoryBitMask;
        if(PhysicsBody::getNodeOwner() && getPhysicsShape())
        {
            btTransform t = getWorldTransform();
            addPhysicsBody(t);
        }
    }
    
    PhysicsBody::BitCategory PhysicsBody::getCollisionGroup()const
    {
        return m_CategoryBitMask;
    }
    
    void PhysicsBody::setCollisionMask(BitCategory categoryBitMask)
    {
        m_CollisionBitMask = categoryBitMask;
        if(PhysicsBody::getNodeOwner() && getPhysicsShape())
        {
            btTransform t = getWorldTransform();
            addPhysicsBody(t);
        }
    }
    
    PhysicsBody::BitCategory PhysicsBody::getCollisionMask()const
    {
        return m_CollisionBitMask;
    }
    
    
    void PhysicsBody::setStaticPhysics()
    {
        m_CollisionFlags = On(m_CollisionFlags, btCollisionObject::CF_STATIC_OBJECT);
        m_CollisionFlags = Off(m_CollisionFlags, btCollisionObject::CF_KINEMATIC_OBJECT);
        m_ActivationState = ACTIVE_TAG;
        
        if(getCollisionObject())
        {
            getCollisionObject()->setCollisionFlags(m_CollisionFlags);
            getCollisionObject()->setActivationState(m_ActivationState);
        }
        
        if(PhysicsBody::getNodeOwner() && getPhysicsShape())
        {
            btTransform t = getWorldTransform();
            addPhysicsBody(t);
        }
    }
    void PhysicsBody::setKinematicPhysics()
    {
        m_CollisionFlags = On(m_CollisionFlags, btCollisionObject::CF_KINEMATIC_OBJECT);
        m_CollisionFlags = Off(m_CollisionFlags, btCollisionObject::CF_STATIC_OBJECT);
        m_ActivationState = DISABLE_DEACTIVATION;
        
        if(getCollisionObject())
        {
            getCollisionObject()->setCollisionFlags(m_CollisionFlags);
            getCollisionObject()->setActivationState(m_ActivationState);
        }
        
        if(PhysicsBody::getNodeOwner() && getPhysicsShape())
        {
            btTransform t = getWorldTransform();
            addPhysicsBody(t);
        }
    }
    void PhysicsBody::setDynamicPhysics()
    {
        m_CollisionFlags = Off(m_CollisionFlags, btCollisionObject::CF_KINEMATIC_OBJECT);
        m_CollisionFlags = Off(m_CollisionFlags, btCollisionObject::CF_STATIC_OBJECT);
        m_ActivationState = ACTIVE_TAG;
        
        if(getCollisionObject())
        {
            getCollisionObject()->setCollisionFlags(m_CollisionFlags);
            getCollisionObject()->setActivationState(m_ActivationState);
        }
        
        if(PhysicsBody::getNodeOwner() && getPhysicsShape())
        {
            btTransform t = getWorldTransform();
            addPhysicsBody(t);
        }
    }
    void PhysicsBody::enableContactResponse(bool enable)
    {
        if(enable)
        {
            m_CollisionFlags = Off(m_CollisionFlags, btCollisionObject::CF_NO_CONTACT_RESPONSE);
        }
        else
        {
            m_CollisionFlags = On(m_CollisionFlags, btCollisionObject::CF_NO_CONTACT_RESPONSE);
        }
        
        if(getCollisionObject())
        {
            getCollisionObject()->setCollisionFlags(m_CollisionFlags);
        }
    }
    void PhysicsBody::enableHandleCollideCallback(bool enable)
    {
        if(enable)
        {
            m_CollisionFlags = On(m_CollisionFlags, btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
        }
        else
        {
            m_CollisionFlags = Off(m_CollisionFlags, btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
        }
        
        if(getCollisionObject())
        {
            getCollisionObject()->setCollisionFlags(m_CollisionFlags);
        }
    }
    
    bool PhysicsBody::isStaticPhysics()const
    {
        return IsOn(m_CollisionFlags, btCollisionObject::CF_STATIC_OBJECT);
    }
    
    bool PhysicsBody::isKinematicPhysics()const
    {
        return IsOn(m_CollisionFlags, btCollisionObject::CF_KINEMATIC_OBJECT);
    }
    
    bool PhysicsBody::isDynamicPhysics()const
    {
        return (!(IsOn(m_CollisionFlags, btCollisionObject::CF_STATIC_OBJECT)) &&
                !(IsOn(m_CollisionFlags, btCollisionObject::CF_KINEMATIC_OBJECT)));
    }
    
    bool PhysicsBody::isResting()const
    {
        return m_IsResting;
    }
    
    void PhysicsBody::enableResting(bool enable)
    {
        m_RestingAllowed = enable;
    }
    
    bool PhysicsBody::isActive()const
    {
        return getCollisionObject()->isActive();
    }
    
    Node *const PhysicsBody::getNodeOwner()const
    {
        return m_NodeOwner;
    }
    
    void PhysicsBody::setNodeOwner(Node *const node)
    {
        m_NodeOwner = node;
    }
    
    int PhysicsBody::getCollisionFlags()const
    {
        return m_CollisionFlags;
    }
    
    int PhysicsBody::getActivationState()const
    {
        return m_ActivationState;
    }
}
