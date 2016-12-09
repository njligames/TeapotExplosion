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
        void touch(TouchState state, float x, float y);
        
        void setShader(const std::string &shader);
        
        
    protected:
        
    private:
        static World *s_Instance;
        static std::string s_BundlePath;
        
        World();
        World(const World &world);
        virtual ~World();
        const World &operator=(const World &rhs);
        
        std::vector<Node*> m_TeapotNodes;
        
        Shader *m_Shader;
        Geometry *m_Geometry;
        Camera *m_Camera;
        Node *m_CameraNode;
        Scene *m_Scene;
        
        typedef std::map<std::string, Shader*> ShaderMap;
        typedef std::pair<std::string, Shader*> ShaderMapPair;
        
        ShaderMap m_ShaderMap;
        
    };
}

#endif /* World_hpp */
