//
//  PhysicsShape.hpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#ifndef PhysicsShape_hpp
#define PhysicsShape_hpp

#include "btVector3.h"
#include "btTransform.h"

class btRigidBody;
class btMotionState;
class btCollisionShape;

namespace jamesfolk
{
    class PhysicsBody;
    
    class PhysicsShape
    {
        friend class PhysicsBodyRigid;
        
    public:
        PhysicsShape();
        virtual ~PhysicsShape();
        
        btRigidBody* createRigidBodyWithShape(float mass, btMotionState * motionState, const btVector3& localInertia = btVector3(0, 0, 0));
        btVector3 calculateLocalInertia(float mass) const;
        virtual int getNumVertices() const = 0;
        virtual int getNumEdges() const = 0;
        virtual void getEdge(int i, btVector3& pa, btVector3& pb) const = 0;
        virtual void getVertex(int i, btVector3& vtx) const = 0;
        virtual int getNumPlanes() const = 0;
        virtual void getPlane(btVector3 & planeNormal, btVector3 & planeSupport, int i) const = 0;
        virtual bool isInside(const btVector3& pt, btScalar tolerance) const = 0;
        virtual void setLocalScaling(const btVector3& scaling);
        virtual void getAabb(const btTransform& t, btVector3& aabbMin, btVector3& aabbMax) const;
        
    protected:
        virtual btCollisionShape *const getCollisionShape() const = 0;
        virtual void setCollisionShape(const btCollisionShape& shape) = 0;
        
        PhysicsBody *const getPhysicsBodyOwner()const;
        void setPhysicsBodyOwner(PhysicsBody *const body);
    private:
        PhysicsBody *m_PhysicsBody;
    };
}

#endif /* PhysicsShape_hpp */
