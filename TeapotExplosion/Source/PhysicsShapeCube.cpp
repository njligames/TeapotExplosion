//
//  PhysicsShapeCube.cpp
//  VerizonTest
//
//  Created by James Folk on 6/30/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#include "PhysicsShapeCube.hpp"

#include "btBoxShape.h"

namespace jamesfolk
{
    PhysicsShapeCube::PhysicsShapeCube():
    PhysicsShape(),
    m_btBoxShape(new btBoxShape(btVector3(1,1,1)))
    {
        m_btBoxShape->setUserPointer(static_cast<void*>(this));
    }
    
    PhysicsShapeCube::~PhysicsShapeCube()
    {
        m_btBoxShape->setUserPointer(NULL);
        delete m_btBoxShape;
        m_btBoxShape=NULL;
    }
    
    void PhysicsShapeCube::setHalfExtends(const btVector3 &extends)
    {
        *m_btBoxShape = btBoxShape(extends);
    }
    
    btVector3 PhysicsShapeCube::getHalfExtentsWithMargin() const
    {
        return getShape()->getHalfExtentsWithMargin();
    }
    
    const btVector3& PhysicsShapeCube::getHalfExtentsWithoutMargin() const
    {
        return getShape()->getHalfExtentsWithoutMargin();
    }
    
    btVector3 PhysicsShapeCube::localGetSupportingVertex(const btVector3& vec) const
    {
        return getShape()->localGetSupportingVertex(vec);
    }
    
    btVector3 PhysicsShapeCube::localGetSupportingVertexWithoutMargin(const btVector3& vec)const
    {
        return getShape()->localGetSupportingVertexWithoutMargin(vec);
    }
    
    void PhysicsShapeCube::batchedUnitVectorGetSupportingVertexWithoutMargin(const btVector3* vectors,btVector3* supportVerticesOut,int numVectors) const
    {
        getShape()->batchedUnitVectorGetSupportingVertexWithoutMargin(vectors, supportVerticesOut, numVectors);
    }
    
    void PhysicsShapeCube::setMargin(btScalar collisionMargin)
    {
        getShape()->setMargin(collisionMargin);
    }
    
    void PhysicsShapeCube::calculateLocalInertia(btScalar mass,btVector3& inertia) const
    {
        getShape()->calculateLocalInertia(mass, inertia);
    }
    
    void PhysicsShapeCube::getPlane(btVector3& planeNormal,btVector3& planeSupport,int i ) const
    {
        getShape()->getPlane(planeNormal, planeSupport, i);
    }
    
    btVector3 PhysicsShapeCube::getPlane(int i)const
    {
        btVector3 v,dummy;
        getPlane(v, dummy, i);
        return v;
    }
    
    int PhysicsShapeCube::getNumPlanes() const
    {
        return getShape()->getNumPlanes();
    }
    
    int PhysicsShapeCube::getNumVertices() const
    {
        return getShape()->getNumVertices();
    }
    
    int PhysicsShapeCube::getNumEdges() const
    {
        return getShape()->getNumEdges();
    }
    
    void PhysicsShapeCube::getVertex(int i,btVector3& vtx) const
    {
        getShape()->getVertex(i, vtx);
    }
    
    void PhysicsShapeCube::getPlaneEquation(btVector4& plane,int i) const
    {
        getShape()->getPlaneEquation(plane, i);
    }
    
    void PhysicsShapeCube::getEdge(int i,btVector3& pa,btVector3& pb) const
    {
        getShape()->getEdge(i, pa, pb);
    }
    
    bool PhysicsShapeCube::isInside(const btVector3& pt,btScalar tolerance) const
    {
        return getShape()->isInside(pt, tolerance);
    }
    
    btCollisionShape *const PhysicsShapeCube::getCollisionShape() const
    {
        return m_btBoxShape;
    }
    
    void PhysicsShapeCube::setCollisionShape(const btCollisionShape &shape)
    {
        assert(shape.getShapeType() == m_btBoxShape->getShapeType());
        
        const btBoxShape *_btBoxShape = dynamic_cast<const btBoxShape*>(&shape);
        
        setHalfExtends(_btBoxShape->getHalfExtentsWithoutMargin() / _btBoxShape->getLocalScaling());
    }
    
    btBoxShape *const PhysicsShapeCube::getShape() const
    {
        return dynamic_cast<btBoxShape *const>(m_btBoxShape);
    }
}
