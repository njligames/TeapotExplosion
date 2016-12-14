//
//  World.hpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#ifndef World_hpp
#define World_hpp

#include <string>
#include <vector>
#include <map>

#import <OpenGLES/ES2/glext.h>
#import <OpenGLES/ES2/gl.h>

#include "btVector2.h"

class btTransform;
class btVector3;

namespace jamesfolk
{
    class Shader;
    class Geometry;
    class Camera;
    class Node;
    class Scene;
    
    class World
    {
    public:
        static World *const getInstance();
        static void createInstance();
        static void destroyInstance();
        static std::string loadASCIIFile(const std::string &filepath);
        static bool loadPngImage(const std::string &filepath, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte **outData);
        static bool readPngFile(const std::string &filepath, int &width, int &height, bool &outHasAlpha, GLubyte **row_pointers);
        static void setBundlePath(const std::string &path);
        
        void create();
        void destroy();
        
        void resize(float x, float y, float width, float height);
        void update(float step);
        void render();
        
        enum TouchState
        {
            TouchState_Up,
            TouchState_Down,
            TouchState_Move,
            TouchState_Cancelled
        };
        
        void addTouch(TouchState state, const btVector2 &touch, unsigned long taps);
        
        void setShader(const std::string &shader);
        
        void setNumberOfTeapots(const int num);
        int numberOfTeapots()const;
        
        void explodeTeapots();
        void subdivideTeapots();
        void resetTeapots();
        
        bool isExploding()const;
        bool isMaxTesselations()const;
        
        Geometry *const getGeometry()const;
        
    protected:
        
        GLuint loadTexture(const std::string &filepath, GLuint idx, GLubyte **outData)const;
    private:
        static World *s_Instance;
        static std::string s_BundlePath;
        
        World();
        World(const World &world);
        virtual ~World();
        const World &operator=(const World &rhs);
        
        std::vector<Node*> m_TeapotNodes;
        std::vector<Shader*> m_Shaders;
        
        Geometry *m_Geometry;
        Camera *m_Camera;
        Node *m_CameraNode;
        Scene *m_Scene;
        
        typedef std::map<std::string, Shader*> ShaderMap;
        typedef std::pair<std::string, Shader*> ShaderMapPair;
        
        ShaderMap m_ShaderMap;
        int m_NumberOfTeapots;
        
        float m_Rotation;
        
        btTransform *m_TriangleShrapnelTransforms;
        btVector3 *m_ImpulseForce;
        btVector3 *m_CurrentVelocity;
        btVector3 *m_Normals;
        GLsizei m_NumberOfTriangles;
        
        struct Touch
        {
            TouchState state;
            btVector2 touch;
            unsigned long taps;
        };
        std::vector<Touch> m_Touches;
        
        bool m_IsExploding;
        
        GLubyte *m_EarthTextureImage;
        GLubyte *m_NormalTextureImage;
        
        GLuint m_EarthTexture;
        GLuint m_NormalTexture;
        
    };
}

#endif /* World_hpp */
