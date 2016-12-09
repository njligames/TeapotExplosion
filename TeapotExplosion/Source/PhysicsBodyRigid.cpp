//
//  PhysicsBodyRigid.cpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#include "PhysicsBodyRigid.hpp"

#include "Node.hpp"
#include "PhysicsWorld.hpp"
#include "Scene.hpp"

#include "PhysicsShape.hpp"

#include <iostream>

#include "btDefaultMotionState.h"
#include "btRigidBody.h"
#include "btCollisionObject.h"

namespace jamesfolk
{
    PhysicsBodyRigid::PhysicsBodyRigid():
    PhysicsBody(),
    m_Mass(1.0f),
    m_btDefaultMotionState(new btDefaultMotionState()),
    m_btRigidBody(NULL),
    m_LinearForceAndPositionArrayIndex(0),
    m_AngularForceAndPositionArrayIndex(0),
    m_Charge(0),
    m_Scene(NULL)
    {
    }
    
    
    PhysicsBodyRigid::~PhysicsBodyRigid()
    {
        removePhysicsBody();
        
        if(m_btRigidBody)
            delete m_btRigidBody;
        m_btRigidBody = NULL;
        
        delete m_btDefaultMotionState;
        m_btDefaultMotionState = NULL;
    }
    
    void PhysicsBodyRigid::setWorldTransform(const btTransform &transform)
    {
        m_btDefaultMotionState->setWorldTransform(transform);
        if(getBody())
            getBody()->setMotionState(m_btDefaultMotionState);
    }
    
    btTransform PhysicsBodyRigid::getWorldTransform()const
    {
        btTransform t;
        getBody()->getMotionState()->getWorldTransform(t);
        return t;
    }
    
    void PhysicsBodyRigid::setLinearFactor(const btVector3 &factor)
    {
        getBody()->setLinearFactor(factor);
    }
    
    const btVector3 &PhysicsBodyRigid::getLinearFactor()const
    {
        return getBody()->getLinearFactor();
    }
    
    void PhysicsBodyRigid::setAngularFactor(const btVector3 &factor)
    {
        getBody()->setAngularFactor(factor);
    }
    
    void PhysicsBodyRigid::setAngularFactor(float factor)
    {
        getBody()->setAngularFactor(factor);
    }
    
    const btVector3 &PhysicsBodyRigid::getAngularFactor()const
    {
        return getBody()->getAngularFactor();
    }
    
    void PhysicsBodyRigid::setMass(float mass)
    {
        m_Mass = mass;
        
        if(PhysicsBody::getNodeOwner() &&
           getPhysicsShape())
        {
            btTransform t = getWorldTransform();
            addPhysicsBody(t);
        }
    }
    
    float PhysicsBodyRigid::getMass()const
    {
        return m_Mass;
    }
    
    void PhysicsBodyRigid::setCharge(float charge)
    {
        m_Charge = charge;
    }
    
    float PhysicsBodyRigid::getCharge()const
    {
        return m_Charge;
    }
    
    void PhysicsBodyRigid::setDamping(float damping)
    {
        getBody()->setDamping(damping, getAngularDamping());
    }
    
    float PhysicsBodyRigid::getDamping()const
    {
        return getBody()->getLinearDamping();
    }
    
    void PhysicsBodyRigid::setAngularDamping(float damping)
    {
        getBody()->setDamping(getDamping(), damping);
    }
    
    float PhysicsBodyRigid::getAngularDamping()const
    {
        return getBody()->getAngularDamping();
    }
    
    void PhysicsBodyRigid::applyForce(const btVector3 &direction, bool impulse)
    {
        if(impulse)
        {
            getBody()->applyCentralImpulse(direction);
        }
        else
        {
            assert(m_LinearForceAndPositionArrayIndex < (20 - 1));
            
            ForceAndPosition *fp = m_LinearForceAndPositionArray.at(m_LinearForceAndPositionArrayIndex);
            fp->set(direction, getNodeOwner()->getOrigin());
            ++m_LinearForceAndPositionArrayIndex;
        }
    }
    
    void PhysicsBodyRigid::applyForce(const btVector3 &direction, const btVector3 &position, bool impulse)
    {
        if(impulse)
        {
            getBody()->applyImpulse(direction, position);
        }
        else
        {
            assert(m_LinearForceAndPositionArrayIndex < (20 - 1));
            
            ForceAndPosition *fp = m_LinearForceAndPositionArray.at(m_LinearForceAndPositionArrayIndex);
            fp->set(direction, position);
            ++m_LinearForceAndPositionArrayIndex;
        }
    }
    
    void PhysicsBodyRigid::applyAngularForce(const btVector3 &direction, bool impulse)
    {
        if(impulse)
        {
            getBody()->applyTorqueImpulse(direction);
        }
        else
        {
            assert(m_AngularForceAndPositionArrayIndex < (20 - 1));
            
            ForceAndPosition *fp = m_AngularForceAndPositionArray.at(m_AngularForceAndPositionArrayIndex);
            fp->set(direction, getNodeOwner()->getOrigin());
            ++m_AngularForceAndPositionArrayIndex;
        }
    }
    
    void PhysicsBodyRigid::clearAllForces()
    {
        getBody()->clearForces();
        m_LinearForceAndPositionArrayIndex = 0;
        m_AngularForceAndPositionArrayIndex = 0;
    }
    
    
    
    void PhysicsBodyRigid::setVelocity(const btVector3 &velocity)
    {
        getBody()->setLinearVelocity(velocity);
    }
    
    const btVector3 &PhysicsBodyRigid::getVelocity()const
    {
        return getBody()->getLinearVelocity();
    }
    
    void PhysicsBodyRigid::setAngularVelocity(const btVector3 &velocity)
    {
        getBody()->setAngularVelocity(velocity);
    }
    
    const btVector3 &PhysicsBodyRigid::getAngularVelocity()const
    {
        return getBody()->getAngularVelocity();
    }
    
    btCollisionObject *const PhysicsBodyRigid::getCollisionObject() const
    {
        return dynamic_cast<btCollisionObject*const>(m_btRigidBody);
    }
    
    void PhysicsBodyRigid::setCollisionObject(const btCollisionObject &obj, PhysicsShape *const shape)
    {
        assert(shape);
        assert(obj.getInternalType()&btCollisionObject::CO_RIGID_BODY);
        
        assert(false);
//        const btCollisionObject *_collisionObject = dynamic_cast<const btRigidBody *>(&obj);
        
        //TODO: Implement....
    }
    
    btRigidBody *const PhysicsBodyRigid::getBody() const
    {
        return m_btRigidBody;
    }
    
    void PhysicsBodyRigid::applyFinalForce()
    {
        for(unsigned long i = 0; i < m_LinearForceAndPositionArrayIndex; ++i)
        {
            getBody()->applyForce(m_LinearForceAndPositionArray[i]->getForce(), m_LinearForceAndPositionArray[i]->getPosition());
        }
        m_LinearForceAndPositionArrayIndex = 0;
        
    }
    
    void PhysicsBodyRigid::applyFinalAngularForce()
    {
        for(unsigned long i = 0; i < m_AngularForceAndPositionArrayIndex; ++i)
        {
            getBody()->applyTorque(m_AngularForceAndPositionArray[i]->getForce());
        }
        m_AngularForceAndPositionArrayIndex = 0;
    }
    
    bool PhysicsBodyRigid::addPhysicsBody(const btTransform &transform)
    {
        if (m_btRigidBody && m_btRigidBody->getNumConstraintRefs() != 0)
        {
            //            assert(TAG, "The PhysicsBodyRigid has constraints so it cannot be added.");
            return false;
        }
        
        if(m_Scene)
        {
            removePhysicsBody();
            
            PhysicsWorld *physicsWorld = m_Scene->getPhysicsWorld();
            
            if(physicsWorld)
            {
                Node *parent = PhysicsBody::getNodeOwner();
                
                if(!parent)
                    std::cout << "The PhysicsBodyRigid is not attached to a Node.";
                
                PhysicsShape *physicsShape = getPhysicsShape();
                
                if(!physicsShape)
                    std::cout <<  "The PhysicsBodyRigid does not have a shape.";
                
                if(physicsShape && parent)
                {
                    //                m_btDefaultMotionState->setWorldTransform(transform);
                    setWorldTransform(transform);
                    
                    float mass = 0;
                    if(isDynamicPhysics())
                    {
                        mass = getMass();
                    }
                    
                    /*
                     m_linearVelocity.setValue(btScalar(0.0), btScalar(0.0), btScalar(0.0));
                     m_angularVelocity.setValue(btScalar(0.),btScalar(0.),btScalar(0.));
                     m_angularFactor.setValue(1,1,1);
                     m_linearFactor.setValue(1,1,1);
                     */
                    
                    btVector3 localInertia(physicsShape->calculateLocalInertia(mass));
                    
                    btRigidBody::btRigidBodyConstructionInfo cinfo(getMass(), m_btDefaultMotionState, physicsShape->getCollisionShape(), localInertia);
                    
                    if (m_btRigidBody)
                    {
                        btVector3 linearVelocity(getVelocity());
                        btVector3 angularVelocity(getAngularVelocity());
                        btVector3 angularFactor(getAngularFactor());
                        btVector3 linearFactor(getLinearFactor());
                        
                        
                        *m_btRigidBody = btRigidBody(cinfo);
                        
                        m_btRigidBody->setLinearFactor(linearFactor);
                        m_btRigidBody->setAngularVelocity(angularVelocity);
                        m_btRigidBody->setAngularFactor(angularFactor);
                        m_btRigidBody->setLinearVelocity(linearVelocity);
                    }
                    else
                    {
                        m_btRigidBody = new btRigidBody(cinfo);
                    }
                    
                    m_btRigidBody->setCollisionFlags(getCollisionFlags());
                    m_btRigidBody->setActivationState(getActivationState());
                    
                    return physicsWorld->addRigidBody(this);
                }
            }
            else
            {
                std::cout << "The physics world is NULL";
            }
        }
        
        return false;
    }
    
    bool PhysicsBodyRigid::removePhysicsBody()
    {
        if(m_Scene)
        {
            PhysicsWorld *physicsWorld = m_Scene->getPhysicsWorld();
            if(physicsWorld)
                return physicsWorld->removeRigidBody(this);
        }
        
        return false;
    }
    
    bool PhysicsBodyRigid::isInWorld()const
    {
        if(m_btRigidBody)
            return m_btRigidBody->isInWorld();
        return false;
    }
    
    void PhysicsBodyRigid::setScene(Scene *const scene)
    {
        m_Scene = scene;
    }
    
    Scene *const PhysicsBodyRigid::getScene()const
    {
        return m_Scene;
    }
    
    PhysicsBodyRigid::ForceAndPosition::ForceAndPosition() :
    m_Force(new btVector3(0,0,0)),
    m_Position(new btVector3(0,0,0))
    {
    }
    
    PhysicsBodyRigid::ForceAndPosition::ForceAndPosition(const PhysicsBodyRigid::ForceAndPosition &copy) :
    m_Force(new btVector3(*(copy.m_Force))),
    m_Position(new btVector3(*(copy.m_Position)))
    {
    }
    
    PhysicsBodyRigid::ForceAndPosition &PhysicsBodyRigid::ForceAndPosition::operator=(const PhysicsBodyRigid::ForceAndPosition &rhs)
    {
        if(this != &rhs)
        {
            *m_Force = *(rhs.m_Force);
            *m_Position = *(rhs.m_Position);
        }
        return *this;
    }
    
    void PhysicsBodyRigid::ForceAndPosition::set(const btVector3 &force, const btVector3 &position)
    {
        *m_Force = force;
        *m_Position = position;
    }
    
    const btVector3 &PhysicsBodyRigid::ForceAndPosition::getForce()const
    {
        return *m_Force;
    }
    
    const btVector3 &PhysicsBodyRigid::ForceAndPosition::getPosition()const
    {
        return *m_Position;
    }
    
    void PhysicsBodyRigid::ForceAndPosition::clear()
    {
        *m_Position = btVector3(0,0,0);
        *m_Force = btVector3(0,0,0);
    }
}
