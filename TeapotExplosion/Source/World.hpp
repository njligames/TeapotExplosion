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

#define PI 3.14159265358979323846264338327f

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
        
        static void loadPngFile(const std::string &filepath, GLubyte **row_pointers);
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
        
        // Robert Penner's easing functions in GLSL
        // https://github.com/stackgl/glsl-easings
        static float linear(float t) {
            assert(t >= 0 && t <= 1.0);
            return t;
        }
        
        static float exponentialIn(float t) {
            assert(t >= 0 && t <= 1.0);
            return t == 0.0 ? t : pow(2.0, 10.0 * (t - 1.0));
        }
        
        static float exponentialOut(float t) {
            assert(t >= 0 && t <= 1.0);
            return t == 1.0 ? t : 1.0 - pow(2.0, -10.0 * t);
        }
        
        static float exponentialInOut(float t) {
            assert(t >= 0 && t <= 1.0);
            return t == 0.0 || t == 1.0
            ? t
            : t < 0.5
            ? +0.5 * pow(2.0, (20.0 * t) - 10.0)
            : -0.5 * pow(2.0, 10.0 - (t * 20.0)) + 1.0;
        }
        
        static float sineIn(float t) {
            assert(t >= 0 && t <= 1.0);
            return sin((t - 1.0) * SIMD_HALF_PI) + 1.0;
        }
        
        static float sineOut(float t) {
            assert(t >= 0 && t <= 1.0);
            return sin(t * SIMD_HALF_PI);
        }
        
        static float sineInOut(float t) {
            assert(t >= 0 && t <= 1.0);
            return -0.5 * (cos(PI * t) - 1.0);
        }
        
        static float qinticIn(float t) {
            assert(t >= 0 && t <= 1.0);
            return pow(t, 5.0);
        }
        
        static float qinticOut(float t) {
            assert(t >= 0 && t <= 1.0);
            return 1.0 - (pow(t - 1.0, 5.0));
        }
        
        static float qinticInOut(float t) {
            assert(t >= 0 && t <= 1.0);
            return t < 0.5
            ? +16.0 * pow(t, 5.0)
            : -0.5 * pow(2.0 * t - 2.0, 5.0) + 1.0;
        }
        
        static float quarticIn(float t) {
            assert(t >= 0 && t <= 1.0);
            return pow(t, 4.0);
        }
        
        static float quarticOut(float t) {
            assert(t >= 0 && t <= 1.0);
            return pow(t - 1.0, 3.0) * (1.0 - t) + 1.0;
        }
        
        static float quarticInOut(float t) {
            assert(t >= 0 && t <= 1.0);
            return t < 0.5
            ? +8.0 * pow(t, 4.0)
            : -8.0 * pow(t - 1.0, 4.0) + 1.0;
        }
        
        static float quadraticInOut(float t) {
            assert(t >= 0 && t <= 1.0);
            float p = 2.0 * t * t;
            return t < 0.5 ? p : -p + (4.0 * t) - 1.0;
        }
        
        static float quadraticIn(float t) {
            assert(t >= 0 && t <= 1.0);
            return t * t;
        }
        
        static float quadraticOut(float t) {
            assert(t >= 0 && t <= 1.0);
            return -t * (t - 2.0);
        }
        
        static float cubicIn(float t) {
            assert(t >= 0 && t <= 1.0);
            return t * t * t;
        }
        
        static float cubicOut(float t) {
            assert(t >= 0 && t <= 1.0);
            float f = t - 1.0;
            return f * f * f + 1.0;
        }
        
        static float cubicInOut(float t) {
            assert(t >= 0 && t <= 1.0);
            return t < 0.5
            ? 4.0 * t * t * t
            : 0.5 * pow(2.0 * t - 2.0, 3.0) + 1.0;
        }
        
        static float elasticIn(float t) {
            assert(t >= 0 && t <= 1.0);
            return sin(13.0 * t * SIMD_HALF_PI) * pow(2.0, 10.0 * (t - 1.0));
        }
        
        static float elasticOut(float t) {
            assert(t >= 0 && t <= 1.0);
            return sin(-13.0 * (t + 1.0) * SIMD_HALF_PI) * pow(2.0, -10.0 * t) + 1.0;
        }
        
        static float elasticInOut(float t) {
            assert(t >= 0 && t <= 1.0);
            return t < 0.5
            ? 0.5 * sin(+13.0 * SIMD_HALF_PI * 2.0 * t) * pow(2.0, 10.0 * (2.0 * t - 1.0))
            : 0.5 * sin(-13.0 * SIMD_HALF_PI * ((2.0 * t - 1.0) + 1.0)) * pow(2.0, -10.0 * (2.0 * t - 1.0)) + 1.0;
        }
        
        static float circularIn(float t) {
            assert(t >= 0 && t <= 1.0);
            return 1.0 - sqrt(1.0 - t * t);
        }
        
        static float circularOut(float t) {
            assert(t >= 0 && t <= 1.0);
            return sqrt((2.0 - t) * t);
        }
        
        static float circularInOut(float t) {
            assert(t >= 0 && t <= 1.0);
            return t < 0.5
            ? 0.5 * (1.0 - sqrt(1.0 - 4.0 * t * t))
            : 0.5 * (sqrt((3.0 - 2.0 * t) * (2.0 * t - 1.0)) + 1.0);
        }
        
        static float bounceOut(float t) {
            assert(t >= 0 && t <= 1.0);
            const float a = 4.0 / 11.0;
            const float b = 8.0 / 11.0;
            const float c = 9.0 / 10.0;
            
            const float ca = 4356.0 / 361.0;
            const float cb = 35442.0 / 1805.0;
            const float cc = 16061.0 / 1805.0;
            
            float t2 = t * t;
            
            return t < a
            ? 7.5625 * t2
            : t < b
            ? 9.075 * t2 - 9.9 * t + 3.4
            : t < c
            ? ca * t2 - cb * t + cc
            : 10.8 * t * t - 20.52 * t + 10.72;
        }
        
        static float bounceIn(float t) {
            assert(t >= 0 && t <= 1.0);
            return 1.0 - bounceOut(1.0 - t);
        }
        
        static float bounceInOut(float t) {
            assert(t >= 0 && t <= 1.0);
            return t < 0.5
            ? 0.5 * (1.0 - bounceOut(1.0 - t * 2.0))
            : 0.5 * bounceOut(t * 2.0 - 1.0) + 0.5;
        }
        
        static float backIn(float t) {
            assert(t >= 0 && t <= 1.0);
            return pow(t, 3.0) - t * sin(t * PI);
        }
        
        static float backOut(float t) {
            assert(t >= 0 && t <= 1.0);
            float f = 1.0 - t;
            return 1.0 - (pow(f, 3.0) - f * sin(f * PI));
        }
        
        static float backInOut(float t) {
            assert(t >= 0 && t <= 1.0);
            float f = t < 0.5
            ? 2.0 * t
            : 1.0 - (2.0 * t - 1.0);
            
            float g = pow(f, 3.0) - f * sin(f * PI);
            
            return t < 0.5
            ? 0.5 * g
            : 0.5 * (1.0 - g) + 0.5;
        }
    protected:
        
        GLuint loadTexture(const std::string &filepath, GLuint idx, GLubyte **outData)const;
        
        void setupCubeMap(GLuint& texture);
        void setupCubeMap(GLuint& texture, const std::string &filepath_xpos, const std::string &filepath_xneg, const std::string &filepath_ypos, const std::string &filepath_yneg, const std::string &filepath_zpos, const std::string &filepath_zneg);
        
        void deleteCubeMap(GLuint& texture);
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
        
//        GLubyte *m_EarthTextureImage;
//        GLubyte *m_NormalTextureImage;
        
//        GLuint m_EarthTexture;
//        GLuint m_NormalTexture;
        
        GLubyte *m_AmbientTextureImage;
        GLubyte *m_DiffuseTextureImage;
        GLubyte *m_SpecularTextureImage;
        GLubyte *m_NormalTextureImage;
        
        GLuint m_AmbientTexture;
        GLuint m_DiffuseTexture;
        GLuint m_SpecularTexture;
        GLuint m_NormalTexture;
        
        GLubyte *m_Cube_xpos;
        GLubyte *m_Cube_xneg;
        GLubyte *m_Cube_ypos;
        GLubyte *m_Cube_yneg;
        GLubyte *m_Cube_zpos;
        GLubyte *m_Cube_zneg;
        
    };
}

#endif /* World_hpp */
