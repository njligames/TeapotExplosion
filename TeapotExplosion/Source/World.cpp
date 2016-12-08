//
//  World.cpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#include "World.hpp"
#include <stdlib.h>

namespace jamesfolk
{
    World *World::s_Instance = NULL;
    std::string World::s_BundlePath = "";
    
    World *const World::getInstance()
    {
        if(NULL == s_Instance)
            s_Instance = new World();
        return s_Instance;
    }
    
    void World::createInstance()
    {
        if(NULL == s_Instance)
            s_Instance = new World();
    }
    
    void World::destroyInstance()
    {
        if(NULL != s_Instance)
            delete s_Instance;
        s_Instance = NULL;
    }
    
    std::string World::loadASCIIFile(const std::string &filepath)
    {
        std::string filedata("");
        
        std::string fullPath = s_BundlePath + filepath;
        
        FILE *file = fopen(fullPath.c_str(), "r");
        
        if(file)
        {
            fseek(file, 0, SEEK_END);
            long fileSize = ftell(file);
            fseek(file, 0, SEEK_SET);
            
            char* buffer = (char*)malloc(fileSize);
            buffer[fileSize] = '\0';
            
            fread(buffer, 1, fileSize, file);
            filedata = std::string(buffer);
            
            fclose(file);
            free(buffer);
        }
        
        return filedata;
    }
    
    void World::setBundlePath(const std::string &path)
    {
        s_BundlePath = path;
    }

    void World::create()
    {
        std::string objFileData = loadASCIIFile("Models/utah-teapot.obj");
        std::string vertexShaderData = loadASCIIFile("Shaders/StandardShader.vert");
        std::string fragmentShaderData = loadASCIIFile("Shaders/StandardShader.frag");
        
        int i = 0;
    }
    
    void World::destroy()
    {
        
    }

    void World::resize(int x, int y, int width, int height)
    {
        
    }
    
    void World::update(float step)
    {
        
    }
    
    void World::render()
    {
        
    }
    
    void World::touch(World::TouchState state, float x, float y)
    {
        
    }
    
    World::World()
    {
        
    }
    
    World::~World()
    {
        
    }
}
