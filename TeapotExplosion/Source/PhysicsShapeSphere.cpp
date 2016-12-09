//
//  PhysicsShapeSphere.cpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#include "PhysicsShapeSphere.hpp"

#include "btSphereShape.h"

namespace jamesfolk
{
    PhysicsShapeSphere::PhysicsShapeSphere():
    PhysicsShape(),
    m_btSphereShape(new btSphereShape(1.0f))
    {
        m_btSphereShape->setUserPointer(static_cast<void*>(this));
    }
    
    PhysicsShapeSphere::~PhysicsShapeSphere()
    {
        m_btSphereShape->setUserPointer(NULL);
        delete m_btSphereShape;
        m_btSphereShape=NULL;
    }
    
    int	PhysicsShapeSphere::getNumVertices() const
    {
        return 0;
    }
    
    int PhysicsShapeSphere::getNumEdges() const
    {
        return 0;
    }
    
    void PhysicsShapeSphere::getEdge(int i,btVector3& pa,btVector3& pb) const
    {
        
    }
    
    void PhysicsShapeSphere::getVertex(int i,btVector3& vtx) const
    {
        
    }
    
    int	PhysicsShapeSphere::getNumPlanes() const
    {
        return 0;
    }
    
    void PhysicsShapeSphere::getPlane(btVector3& planeNormal,btVector3& planeSupport,int i ) const
    {
        
    }
    
    bool PhysicsShapeSphere::isInside(const btVector3& pt,float tolerance) const
    {
        return false;
    }
    
    
    void PhysicsShapeSphere::setRadius(float radius)
    {
        *m_btSphereShape = btSphereShape(radius);
    }
    
    void PhysicsShapeSphere::calculateLocalInertia(float mass,btVector3& inertia) const
    {
        const btSphereShape *sphere = dynamic_cast<const btSphereShape*>(getCollisionShape());
        sphere->calculateLocalInertia(mass, inertia);
    }
    
    float PhysicsShapeSphere::getRadius() const
    {
        const btSphereShape *sphere = dynamic_cast<const btSphereShape*>(getCollisionShape());
        return sphere->getRadius();
    }
    
    void PhysicsShapeSphere::setUnscaledRadius(float	radius)
    {
        btSphereShape *sphere = dynamic_cast<btSphereShape*>(getCollisionShape());
        sphere->setUnscaledRadius(radius);
    }
    
    void PhysicsShapeSphere::setMargin(float margin)
    {
        btSphereShape *sphere = dynamic_cast<btSphereShape*>(getCollisionShape());
        sphere->setMargin(margin);
    }
    
    float PhysicsShapeSphere::getMargin() const
    {
        const btSphereShape *sphere = dynamic_cast<const btSphereShape*>(getCollisionShape());
        
        return sphere->getMargin();
    }
    
    btCollisionShape *const PhysicsShapeSphere::getCollisionShape() const
    {
        return m_btSphereShape;
    }
    
    void PhysicsShapeSphere::setCollisionShape(const btCollisionShape &shape)
    {
        assert(shape.getShapeType() == m_btSphereShape->getShapeType());
        const btSphereShape *_btSphereShape = dynamic_cast<const btSphereShape*>(&shape);
        setRadius(_btSphereShape->getRadius());
    }
}
