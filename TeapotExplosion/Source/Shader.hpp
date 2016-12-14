//
//  Shader.hpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#ifndef Shader_hpp
#define Shader_hpp

#include <string>
#include <vector>
#include <map>

#import <OpenGLES/ES2/glext.h>
#import <OpenGLES/ES2/gl.h>

#include "btTransform.h"

namespace jamesfolk
{
    class Shader
    {
    public:
        /* members */
        Shader();
        Shader(const Shader &rhs);
        const Shader &operator=(const Shader &rhs);
        ~Shader();

        bool load(const std::string &vertexSource,
                  const std::string &fragmentSource);
        
        void unLoad();
        bool isLoaded()const;
        
        bool use()const;
        
        int getAttributeLocation(const std::string &attributeName)const;
        
        int getUniformLocation(const std::string &uniformName);
        
        bool setUniformValue(const std::string &uniformName, const btTransform &value, bool transpose = false);
        bool setUniformValue(const std::string &uniformName, GLfloat *matrix4x4, bool transpose = false);
        bool getUniformValue(const std::string &uniformName, btTransform &value);
        
        bool setUniformValue(const char *uniformName, GLuint value);
        bool getUniformValue(const char *uniformName, GLuint &value);
        
        bool setUniformValue(const char *uniformName, const btVector3 &value);
        bool getUniformValue(const char *uniformName, btVector3 &value);
        
        bool setUniformValue(const char *uniformName, float value);
        bool getUniformValue(const char *uniformName, float &value);
        
        bool setUniformValue(const char *uniformName, const btVector4 &value);
        bool getUniformValue(const char *uniformName, btVector4 &value);
    protected:
        GLuint compileShader(const std::string &source, GLenum type);
        bool compileStatus(GLuint shader);
        
        bool linkProgram(GLuint program);
        
    private:
        GLuint m_Program;
        
        GLfloat *m_mat4Buffer;
        GLfloat *m_vec3Buffer;
        GLfloat *m_vec4Buffer;
        
        typedef std::map<std::string, int> UniformMap;
        typedef std::pair<std::string, int> UniformPair;
        
        UniformMap m_UniformMap;
        
    };
}

#endif /* Shader_hpp */
