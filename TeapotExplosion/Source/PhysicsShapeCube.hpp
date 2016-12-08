//
//  PhysicsShapeCube.hpp
//  VerizonTest
//
//  Created by James Folk on 6/30/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#ifndef PhysicsShapeCube_hpp
#define PhysicsShapeCube_hpp

#include "PhysicsShape.hpp"

class btBoxShape;

namespace jamesfolk
{
    class PhysicsShapeCube:
    public PhysicsShape
    {
    public:
        PhysicsShapeCube();
        virtual ~PhysicsShapeCube();
        
        void setHalfExtends(const btVector3 &extends);
        btVector3 getHalfExtentsWithMargin() const;
        const btVector3& getHalfExtentsWithoutMargin() const;
        btVector3 localGetSupportingVertex(const btVector3& vec) const;
        btVector3 localGetSupportingVertexWithoutMargin(const btVector3& vec)const;
        void batchedUnitVectorGetSupportingVertexWithoutMargin(const btVector3* vectors,btVector3* supportVerticesOut,int numVectors) const;
        void setMargin(btScalar collisionMargin);
        void calculateLocalInertia(btScalar mass,btVector3& inertia) const;
        void getPlane(btVector3& planeNormal,btVector3& planeSupport,int i ) const;
        btVector3 getPlane(int i)const;
        int getNumPlanes() const;
        int getNumVertices() const;
        int getNumEdges() const;
        void getVertex(int i,btVector3& vtx) const;
        void getPlaneEquation(btVector4& plane,int i) const;
        void getEdge(int i,btVector3& pa,btVector3& pb) const;
        bool isInside(const btVector3& pt,btScalar tolerance) const;
        
    protected:
        virtual btCollisionShape *const getCollisionShape() const;
        virtual void setCollisionShape(const btCollisionShape &shape);
        
        virtual btBoxShape *const getShape() const;
    private:
        btBoxShape *m_btBoxShape;
    };
}

#endif /* PhysicsShapeCube_hpp */
