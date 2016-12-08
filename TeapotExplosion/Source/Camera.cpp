//
//  Camera.cpp
//  VerizonTest
//
//  Created by James Folk on 6/21/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#include "Camera.hpp"
#include "Node.hpp"
#include "Shader.hpp"

namespace jamesfolk
{
    static inline btTransform setFrom4x4Matrix(const float *m)
    {
        btMatrix3x3 basis;
        btVector3 origin;
        
        basis.setFromOpenGLSubMatrix(m);
        origin.setValue(m[12],m[13],m[14]);
        
        basis[0].setW(m[3]);
        basis[1].setW(m[7]);
        basis[2].setW(m[11]);
        origin.setW(m[15]);
        
        return btTransform(basis, origin);
    }

    btTransform Camera::makeFrustum(float *matrixBuffer, float fov, float aspect, float nearDist, float farDist, bool leftHanded )
    {
        memset(matrixBuffer, 0, sizeof(float) * 16);
        //
        // General form of the Projection Matrix
        //
        // uh = Cot( fov/2 ) == 1/Tan(fov/2)
        // uw / uh = 1/aspect
        //
        //   uw         0       0       0
        //    0        uh       0       0
        //    0         0      f/(f-n)  1
        //    0         0    -fn/(f-n)  0
        //
        // Make result to be identity first
        
        //    0         1       2       3
        //    4         5       6       7
        //    8         9      10      11
        //   12        13      14      15
        
        // check for bad parameters to avoid divide by zero:
        // if found, assert and return an identity matrix.
        if ( fov <= 0 || aspect == 0 )
        {
            return btTransform::getIdentity();
        }
        
        float frustumDepth = farDist - nearDist;
        float oneOverDepth = 1.0f / frustumDepth;
        
        matrixBuffer[5] = 1.0f / tan(0.5f * btRadians(fov));
        matrixBuffer[0] = (leftHanded ? 1.0f : -1.0f ) * matrixBuffer[5] / aspect;
        matrixBuffer[10] = farDist * oneOverDepth;
        matrixBuffer[14] = (-farDist * nearDist) * oneOverDepth;
        matrixBuffer[11] = 1.0f;
        matrixBuffer[15] = 0;
        
        return setFrom4x4Matrix(matrixBuffer);
    }
    
    btTransform Camera::makeLookAt(float *buffer,
                                   float eyeX, float eyeY, float eyeZ,
                                   float centerX, float centerY, float centerZ,
                                   float upX, float upY, float upZ)
    {
        assert(buffer);
        
        btVector3 ev(eyeX, eyeY, eyeZ );
        btVector3 cv(centerX, centerY, centerZ );
        btVector3 uv( upX, upY, upZ );
        btVector3 n((ev + -cv).normalized());
        btVector3 u((uv.cross(n).normalized()));
        btVector3 v(n.cross(u));
        
        buffer[ 0] = u.x();
        buffer[ 1] = v.x();
        buffer[ 2] = n.x();
        buffer[ 3] = 0.0f;
        buffer[ 4] = u.y();
        buffer[ 5] = v.y();
        buffer[ 6] = n.y();
        buffer[ 7] = 0.0f;
        buffer[ 8] = u.z();
        buffer[ 9] = v.z();
        buffer[10] = n.z();
        buffer[11] = 0.0f;
        buffer[12] = (-u).dot(ev);
        buffer[13] = (-v).dot(ev);
        buffer[14] = (-n).dot(ev);
        buffer[15] = 1.0f;
        
        return setFrom4x4Matrix(buffer);
    }
    
    Camera::Camera():
    m_MatrixBuffer(new float[16]),
    m_ProjectionMatrixBuffer(new GLfloat[16]),
    m_NodeOwner(NULL),
    m_Near(0.1f),
    m_Far(3000.0f),
    m_Fov(45.0f),
    m_AspectRatio(1.0f),
    m_ProjectionMatrix(new btTransform()),
    m_ModelViewDirty(true),
    m_ProjectionDirty(true)
    {
        *m_ProjectionMatrix = makeFrustum(m_ProjectionMatrixBuffer, getFov(), getAspectRatio(), getZNear(), getZFar());
        m_ProjectionDirty = true;
    }
    
    Camera::~Camera()
    {
        delete m_ProjectionMatrix;
        m_ProjectionMatrix = NULL;
        
        delete [] m_ProjectionMatrixBuffer;
        m_ProjectionMatrixBuffer = NULL;
        
        delete [] m_MatrixBuffer;
        m_MatrixBuffer = NULL;
    }
    
    void Camera::setZNear(const float val)
    {
        bool changed = (val != getZNear());
        
        m_Near = val;
        
        if(changed)
        {
            *m_ProjectionMatrix = makeFrustum(m_ProjectionMatrixBuffer, getFov(), getAspectRatio(), getZNear(), getZFar());
            m_ProjectionDirty = true;
        }
    }
    
    float Camera::getZNear() const
    {
        return m_Near;
    }
    
    void Camera::setZFar(const float val)
    {
        bool changed = (val != getZFar());
        
        m_Far = val;
        
        if(changed)
        {
            *m_ProjectionMatrix = makeFrustum(m_ProjectionMatrixBuffer, getFov(), getAspectRatio(), getZNear(), getZFar());
            m_ProjectionDirty = true;
        }
    }
    
    float Camera::getZFar() const
    {
        return m_Far;
    }
    
    void Camera::setFov(const float val)
    {
        bool changed = (val != getFov());
        
        m_Fov = val;
        
        if(changed)
        {
            *m_ProjectionMatrix = makeFrustum(m_ProjectionMatrixBuffer, getFov(), getAspectRatio(), getZNear(), getZFar());
            m_ProjectionDirty = true;
        }
    }
    
    float Camera::getFov() const
    {
        return m_Fov;
    }
    
    void Camera::setAspectRatio(const float val)
    {
        bool changed = (val != getAspectRatio());
        
        m_AspectRatio = val;
        
        if(changed)
        {
            *m_ProjectionMatrix = makeFrustum(m_ProjectionMatrixBuffer, getFov(), getAspectRatio(), getZNear(), getZFar());
            m_ProjectionDirty = true;
        }
    }
    
    float Camera::getAspectRatio()const
    {
        return m_AspectRatio;
    }
    
    btTransform Camera::getModelView()const
    {
        if(getNodeOwner())
            return getNodeOwner()->getWorldTransform();
        return btTransform::getIdentity();
    }
    
    btTransform Camera::getProjectionMatrix()const
    {
        return *m_ProjectionMatrix;
    }
    
    Node *const Camera::getNodeOwner()const
    {
        return m_NodeOwner;
    }
    
    void Camera::setNodeOwner(Node *const node)
    {
        m_NodeOwner = node;
    }
    
    void Camera::lookAt(const btVector3 &target, const btVector3 &up)
    {
        btTransform _btTransform(makeLookAt(m_MatrixBuffer,
                                            getNodeOwner()->getOrigin().getX(),
                                            getNodeOwner()->getOrigin().getY(),
                                            getNodeOwner()->getOrigin().getZ(),
                                            target.getX(),
                                            target.getY(),
                                            target.getZ(),
                                            up.getX(),
                                            up.getY(),
                                            up.getZ()));
        btQuaternion q;
        _btTransform.inverse().getBasis().getRotation(q);
        
        getNodeOwner()->setOrientation(q);
    }
    
    void Camera::render(Shader *const shader, bool shouldRedraw)
    {
        if(m_ModelViewDirty || shouldRedraw)
        {
            assert(shader->setUniformValue("modelView", getModelView()));
            m_ModelViewDirty = false;
        }
        
        if(m_ProjectionDirty || shouldRedraw)
        {
            assert(shader->setUniformValue("projection", m_ProjectionMatrixBuffer));
            m_ProjectionDirty = false;
        }
    }
}
