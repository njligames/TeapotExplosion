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

namespace jamesfolk
{
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
        
        void resize(int x, int y, int width, int height);
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
        
        
    protected:
        
    private:
        static World *s_Instance;
        static std::string s_BundlePath;
        
        World();
        World(const World &world);
        virtual ~World();
        const World &operator=(const World &rhs);
    };
}

#endif /* World_hpp */
