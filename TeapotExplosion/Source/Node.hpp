//
//  Node.hpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#ifndef Node_hpp
#define Node_hpp

#include "btTransform.h"
#include "btVector2.h"
#include "btQuaternion.h"

#include <vector>
#include <string>

namespace jamesfolk
{
    class Camera;
    class Geometry;
    class PhysicsBody;
    
    class TornadoData;
    
    class Node
    {
        friend class Geometry;
        friend class Scene;
        
    public:
        /* members */
        Node();
        Node(const Node &rhs);
        const Node &operator=(const Node &rhs);
        ~Node();
        
        void setName(const std::string &name);
        const std::string &getName()const;
        
        void addCamera(Camera *camera);
        void removeCamera();
        Camera* getCamera();
        
        void addGeometry(Geometry * body);
        void removeGeometry();
        Geometry *const getGeometry()const;
        
        void enableHideGeometry(bool hidden = true);
        bool isHiddenGeometry()const;
        
        void setOpacity(float opacity);
        float getOpacity()const;
        void setNormalMatrix(const btMatrix3x3 &mtx);
        const btMatrix3x3 &getNormalMatrix()const;
        
        void setColorBase(const btVector4 &color);
        const btVector4 &getColorBase()const;
    public:
        Node* getParentNode();
        const Node* getParentNode() const;
        bool hasParentNode() const;
        void setParentNode(Node * parent);
        void removeParentNode();
        bool removeFromParentNode();
        Node* findChildNode(const std::string &name);
        const Node* findChildNode(const std::string &name) const;
        Node* getChildNode(const unsigned long index);
        const Node* getChildNode(const unsigned long index) const;
        void getChildrenNodes(std::vector<Node*> & children) const;
        unsigned long getChildNodeIndex(Node * object) const;
        bool hasChildNode(Node * object) const;
        bool hasChildrenNodes() const;
        void addChildNode(Node * object);
        void removeChildNode(const unsigned long index);
        void removeChildNode(Node * object);
        void removeChildrenNodes();
        unsigned long numberOfChildrenNodes() const;
        void replaceChildNode(Node * oldChild, Node * newChild);
    public:
        btTransform getWorldTransform() const;
        
//        const btTransform& getColorTransform() const;
//        void setColorTransform(const btTransform& transform);
        const btTransform& getTransform() const;
        void setTransform(const btTransform& transform);
        btVector3 getOrigin() const;
        void setOrigin(const btVector3& origin);
        void setOrigin(const btVector2& origin);
        btQuaternion getRotation() const;
        void setRotation(const btQuaternion& rotation);
        btVector3 getEulerAngles() const;
        void setEulerAngles(const btVector3& angles);
        const btQuaternion& getOrientation() const;
        void setOrientation(const btQuaternion& orientation);
        
        const btVector3 &getScale()const;
        void setScale(const btVector3 &scale);
        void setScale(const float scale);
    public:
        bool addPhysicsBody(PhysicsBody *const body);
        void removePhysicsBody();
        PhysicsBody *const getPhysicsBody()const;
        
        TornadoData *const getTornadoData()const;
        
        void setGravity(const btVector3 &vec);
        void setVelocity(const btVector3 &vec);
        const btVector3 &getVelocity()const;
        void addImpulseForce(const btVector3 &vec);
        void setMaxSpeed(float speed);
        float getMaxSpeed()const;
    protected:
        void setGeometryIndex(unsigned long index);
        unsigned long getGeometryIndex() const;
        void clearGeometryIndex();
        
        bool isTransformDirty()const;
        void resetTransformDirty();
        
        void update(float timestep);
        void render(Geometry *const geometry);
    private:
        std::string m_Name;
        btVector3 *m_Scale;
        btTransform* m_Transform;
//        btTransform* m_ColorTransform;
        btQuaternion* m_Orientation;
        
        Node* m_ParentNode;
        std::vector<Node*> m_ChildrenNodes;
        
        Camera *m_Camera;
        Geometry *m_Geometry;
        unsigned long m_GeometryIndex;
        
        bool m_HideGeometry;
        float m_Opacity;
        btMatrix3x3 *m_NormalMatrix;
        btVector4 *m_Colorbase;
        
        PhysicsBody *m_PhysicsBody;
        
        bool m_TransformDirty;
        bool m_NormalMatrixDirty;
//        bool m_ColorTransformDirty;
        bool m_OpacityDirty;
        bool m_HiddenDirty;
        bool m_ColorBaseDirty;
        
        btVector3 *m_GravityForce;
        btVector3 *m_ImpulseForce;
        btVector3 *m_CurrentVelocity;
        btVector3 *m_HeadingVector;
        float m_MaxSpeed;
        
    };
}

#endif /* Node_hpp */
