//
//  PhysicsShape.cpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#include "PhysicsShape.hpp"

#include "btRigidBody.h"
#include "btCollisionShape.h"

namespace jamesfolk
{
    PhysicsShape::PhysicsShape():
    m_PhysicsBody(NULL)
    {
        
    }
    
    PhysicsShape::~PhysicsShape()
    {
        
    }
    
    btRigidBody *PhysicsShape::createRigidBodyWithShape(float mass, btMotionState* motionState, const btVector3& localInertia)
    {
        btRigidBody *rb = new btRigidBody(mass, motionState, getCollisionShape(), localInertia);
        return rb;
    }
    
    btVector3 PhysicsShape::calculateLocalInertia(btScalar mass)const
    {
        btVector3 localInertia(0,0,0);
        if(mass > 0.0f)
        {
            getCollisionShape()->calculateLocalInertia(mass, localInertia);
        }
        return localInertia;
    }
    
    void PhysicsShape::setLocalScaling(const btVector3& scaling)
    {
        getCollisionShape()->setLocalScaling(scaling);
    }
    
    void PhysicsShape::getAabb(const btTransform& t,btVector3& aabbMin,btVector3& aabbMax) const
    {
        getCollisionShape()->getAabb(t, aabbMin, aabbMax);
    }
    
    PhysicsBody *const PhysicsShape::getPhysicsBodyOwner()const
    {
        return m_PhysicsBody;
    }
    
    void PhysicsShape::setPhysicsBodyOwner(PhysicsBody *const body)
    {
        m_PhysicsBody = body;
    }
}
