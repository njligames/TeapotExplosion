//
//  PhysicsShapeSphere.hpp
//  VerizonTest
//
//  Created by James Folk on 7/1/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#ifndef PhysicsShapeSphere_hpp
#define PhysicsShapeSphere_hpp

#include "PhysicsShape.hpp"

class btSphereShape;

namespace jamesfolk
{
    class PhysicsShapeSphere:
    public PhysicsShape
    {
    public:
        PhysicsShapeSphere();
        virtual ~PhysicsShapeSphere();
        
        virtual int	getNumVertices() const;
        virtual int getNumEdges() const;
        virtual void getEdge(int i,btVector3& pa,btVector3& pb) const;
        virtual void getVertex(int i,btVector3& vtx) const;
        virtual int	getNumPlanes() const;
        virtual void getPlane(btVector3& planeNormal,btVector3& planeSupport,int i ) const;
        virtual	bool isInside(const btVector3& pt,float tolerance) const;
        void setRadius(float radius);
        void calculateLocalInertia(float mass,btVector3& inertia) const;
        float getRadius() const;
        void setUnscaledRadius(float	radius);
        void setMargin(float margin);
        float getMargin() const;
    protected:
        virtual btCollisionShape *const getCollisionShape() const;
        virtual void setCollisionShape(const btCollisionShape &shape);
        
    private:
        btSphereShape *m_btSphereShape;
    };
}

#endif /* PhysicsShapeSphere_hpp */
