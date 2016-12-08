//
//  PhysicsBodyRigid.hpp
//  VerizonTest
//
//  Created by James Folk on 6/30/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#ifndef PhysicsBodyRigid_hpp
#define PhysicsBodyRigid_hpp

#include <vector>

#include "PhysicsBody.hpp"

class btRigidBody;
struct btDefaultMotionState;

namespace jamesfolk
{
    class PhysicsBodyRigid;
    class Scene;
    
    class PhysicsBodyRigid : public PhysicsBody
    {
        friend class PhysicsWorld;
        
    public:
        PhysicsBodyRigid();
        virtual ~PhysicsBodyRigid();
        
        virtual void setWorldTransform(const btTransform &transform);
        virtual btTransform getWorldTransform()const;
        virtual void setLinearFactor(const btVector3 &factor);
        virtual const btVector3 &getLinearFactor()const;
        virtual void setAngularFactor(const btVector3 &factor);
        virtual void setAngularFactor(float factor);
        virtual const btVector3 &getAngularFactor()const;
        virtual void setMass(float mass);
        virtual float getMass()const;
        
        virtual void setCharge(float charge);
        virtual float getCharge()const;
        
        virtual void setDamping(float damping);
        virtual float getDamping()const;
        
        virtual void setAngularDamping(float damping);
        virtual float getAngularDamping()const;
        
        virtual void applyForce(const btVector3 &direction, bool impulse = false);
        virtual void applyForce(const btVector3 &direction, const btVector3 &position, bool impulse = false);
        
        virtual void applyAngularForce(const btVector3 &direction, bool impulse = false);
        
        virtual void clearAllForces();
        
        virtual void setVelocity(const btVector3 &velocity);
        virtual const btVector3 &getVelocity()const;
        
        virtual void setAngularVelocity(const btVector3 &velocity);
        virtual const btVector3 &getAngularVelocity()const;
        
        virtual bool isInWorld()const;
        
        void setScene(Scene *const scene);
        Scene *const getScene()const;
    protected:
        virtual btCollisionObject *const getCollisionObject() const;
        virtual void setCollisionObject(const btCollisionObject &obj, PhysicsShape *const shape);
        
        virtual btRigidBody *const getBody() const;
        
        virtual void applyFinalForce();
        virtual void applyFinalAngularForce();
        
        virtual bool addPhysicsBody(const btTransform &transform = btTransform::getIdentity());
        virtual bool removePhysicsBody();
    private:
        float m_Mass;
        btDefaultMotionState *m_btDefaultMotionState;
        btRigidBody *m_btRigidBody;
        
        
        
        ATTRIBUTE_ALIGNED16(class) ForceAndPosition
        {
        public:
            ForceAndPosition();
            ForceAndPosition(const ForceAndPosition &copy);
            BT_DECLARE_ALIGNED_ALLOCATOR();
            ForceAndPosition &operator=(const ForceAndPosition &rhs);
            
            void set(const btVector3 &force, const btVector3 &position);
            const btVector3 &getForce()const;
            const btVector3 &getPosition()const;
            void clear();
        private:
            btVector3 *m_Force;
            btVector3 *m_Position;
        };
        
        std::vector<ForceAndPosition*> m_LinearForceAndPositionArray;
        unsigned long m_LinearForceAndPositionArrayIndex;
        
        std::vector<ForceAndPosition*> m_AngularForceAndPositionArray;
        unsigned long m_AngularForceAndPositionArrayIndex;
        
        btTransform *m_btTransform;
        float m_Charge;
        
        Scene *m_Scene;
    };
}

#endif /* PhysicsBodyRigid_hpp */
