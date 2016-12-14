//
//  World.cpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#include "World.hpp"
#include <stdlib.h>
//#import "png.h"
//#include <png.h>
#import <libpng/png.h>

#include "Shader.hpp"
#include "Geometry.hpp"
#include "MeshGeometry.hpp"
#include "Camera.hpp"
#include "Node.hpp"
#include "Scene.hpp"


static unsigned int MAXIMUM_TEAPOTS = 1;
static unsigned int MAXIMUM_SUBDIVISIONS = 2;

static float randomFloat(float min, float max)
{
    float r = (float)rand() / (float)RAND_MAX;
    return min + r * (max - min);
}

static const char *SHADERNAMES[] =
{
    "StandardShader",
    "PassThrough",
    0
};

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
    
    bool World::loadPngImage(const std::string &filepath, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte **outData)
    {
        png_structp png_ptr;
        png_infop info_ptr;
        unsigned int sig_read = 0;
        int color_type, interlace_type;
        FILE *fp;
        
        std::string fullPath = s_BundlePath + filepath;
        
        if ((fp = fopen(fullPath.c_str(), "rb")) == NULL)
            return false;
        
        /* Create and initialize the png_struct
         * with the desired error handler
         * functions.  If you want to use the
         * default stderr and longjump method,
         * you can supply NULL for the last
         * three parameters.  We also supply the
         * the compiler header file version, so
         * that we know if the application
         * was compiled with a compatible version
         * of the library.  REQUIRED
         */
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                         NULL, NULL, NULL);
        
        if (png_ptr == NULL) {
            fclose(fp);
            return false;
        }
        
        /* Allocate/initialize the memory
         * for image information.  REQUIRED. */
        info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL) {
            fclose(fp);
            png_destroy_read_struct(&png_ptr, NULL, NULL);
            return false;
        }
        
        /* Set error handling if you are
         * using the setjmp/longjmp method
         * (this is the normal method of
         * doing things with libpng).
         * REQUIRED unless you  set up
         * your own error handlers in
         * the png_create_read_struct()
         * earlier.
         */
        if (setjmp(png_jmpbuf(png_ptr))) {
            /* Free all of the memory associated
             * with the png_ptr and info_ptr */
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            fclose(fp);
            /* If we get here, we had a
             * problem reading the file */
            return false;
        }
        
        /* Set up the output control if
         * you are using standard C streams */
        png_init_io(png_ptr, fp);
        
        /* If we have already
         * read some of the signature */
        png_set_sig_bytes(png_ptr, sig_read);
        
        /*
         * If you have enough memory to read
         * in the entire image at once, and
         * you need to specify only
         * transforms that can be controlled
         * with one of the PNG_TRANSFORM_*
         * bits (this presently excludes
         * dithering, filling, setting
         * background, and doing gamma
         * adjustment), then you can read the
         * entire image (including pixels)
         * into the info structure with this
         * call
         *
         * PNG_TRANSFORM_STRIP_16 |
         * PNG_TRANSFORM_PACKING  forces 8 bit
         * PNG_TRANSFORM_EXPAND forces to
         *  expand a palette into RGB
         */
        png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);
        
        png_uint_32 width, height;
        int bit_depth;
        png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
                     &interlace_type, NULL, NULL);
        outWidth = width;
        outHeight = height;
        
        unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
        *outData = (unsigned char*) malloc(row_bytes * outHeight);
        
        png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
        
        for (int i = 0; i < outHeight; i++) {
            // note that png is ordered top to
            // bottom, but OpenGL expect it bottom to top
            // so the order or swapped
            memcpy(*outData+(row_bytes * (outHeight-1-i)), row_pointers[i], row_bytes);
        }
        
        /* Clean up after the read,
         * and free any memory allocated */
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        
        /* Close the file */
        fclose(fp);
        
        /* That's it */
        return true;
    }
    
    bool World::readPngFile(const std::string &filepath, int &width, int &height, bool &outHasAlpha, GLubyte **row_pointers)
//    void read_png_file(char *filename)
    {
        png_byte color_type;
        png_byte bit_depth;
        
//        png_bytep *row_pointers;

        std::string fullPath = s_BundlePath + filepath;
        
        FILE *fp = fopen(fullPath.c_str(), "rb");
        
        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if(!png) abort();
        
        png_infop info = png_create_info_struct(png);
        if(!info) abort();
        
        if(setjmp(png_jmpbuf(png))) abort();
        
        png_init_io(png, fp);
        
        png_read_info(png, info);
        
        width      = png_get_image_width(png, info);
        height     = png_get_image_height(png, info);
        color_type = png_get_color_type(png, info);
        bit_depth  = png_get_bit_depth(png, info);
        
        // Read any color_type into 8bit depth, RGBA format.
        // See http://www.libpng.org/pub/png/libpng-manual.txt
        
        if(bit_depth == 16)
            png_set_strip_16(png);
        
        if(color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_palette_to_rgb(png);
        
        // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
        if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
            png_set_expand_gray_1_2_4_to_8(png);
        
        if(png_get_valid(png, info, PNG_INFO_tRNS))
            png_set_tRNS_to_alpha(png);
        
        // These color_type don't have an alpha channel then fill it with 0xff.
        if(color_type == PNG_COLOR_TYPE_RGB ||
           color_type == PNG_COLOR_TYPE_GRAY ||
           color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
        
        if(color_type == PNG_COLOR_TYPE_GRAY ||
           color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png);
        
        png_read_update_info(png, info);
        
        row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
        for(int y = 0; y < height; y++) {
            row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
        }
        
        png_read_image(png, row_pointers);
        
        fclose(fp);
        
        outHasAlpha = false;
        if(color_type == PNG_COLOR_TYPE_RGBA ||
           color_type == PNG_COLOR_TYPE_GA)
            outHasAlpha = true;
        
        return true;
    }
    
    void World::setBundlePath(const std::string &path)
    {
        s_BundlePath = path;
    }

    void World::create()
    {
        m_EarthTexture = loadTexture("Images/4096_earth.png", 0, &m_EarthTextureImage);
        m_NormalTexture = loadTexture("Images/4096_normal.png", 0, &m_NormalTextureImage);
        
        std::string objFileData = loadASCIIFile("Models/utah-teapot-lowpoly.obj");
//        std::string objFileData = loadASCIIFile("Models/triangle.obj");
//        std::string objFileData = loadASCIIFile("Models/triangle_subdivide.obj");
        
        glClearColor(0.0, 0.0, 0.0, 1.0);
        
        glEnable(GL_DEPTH_TEST);
        glFrontFace(GL_CW);
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
        
//        glEnable(GL_TEXTURE_2D);
//        glEnable(GL_BLEND);
//        glBlendFunc(GL_ONE, GL_SRC_COLOR);
        
        srand((unsigned int)time(0));
        
        m_CameraNode->addCamera(m_Camera);
        m_CameraNode->setOrigin(btVector3(0.0f, 0.0f, 0.0f));
        
        m_Scene->addActiveNode(m_CameraNode);
        m_Scene->addActiveCamera(m_Camera);
        
        unsigned long i = 0;
        for (std::vector<Shader*>::iterator iter = m_Shaders.begin();
             iter != m_Shaders.end();
             iter++)
        {
            Shader *shader = *iter;
            std::string shaderName = SHADERNAMES[i++];
            std::string vertexShader = loadASCIIFile(std::string("Shaders/") + shaderName + std::string(".vert"));
            std::string fragmentShader = loadASCIIFile(std::string("Shaders/") + shaderName + std::string(".frag"));
            
            assert(shader->load(vertexShader, fragmentShader));
            m_ShaderMap.insert(ShaderMapPair(shaderName, shader));
            
        }
        
        m_Scene->getRootNode()->setOrigin(btVector3(0.0f, 0.0f, 5.0f));
        
        m_Geometry->load(m_Shaders[0], objFileData, MAXIMUM_TEAPOTS, MAXIMUM_SUBDIVISIONS);
        
        float y = 0.0f;
        float z = -3.0f;
        for (std::vector<Node*>::iterator i = m_TeapotNodes.begin();
             i != m_TeapotNodes.end();
             i++)
        {
            Node *node = *i;
            
            m_Scene->addActiveNode(node);
            
            m_Scene->getRootNode()->addChildNode(node);
            
            node->addGeometry(m_Geometry);
            
            node->setOrigin(btVector3(0.0f, y, z));
            y += 1.1f;
            z += 3.0;
            
//            node->setColorBase(btVector4(randomFloat(0.9f, 1.0f), randomFloat(0.9f, 1.0f), randomFloat(0.9f, 1.0f), 1.0f));
        }
        
        m_Geometry->setAmbientTexture(m_EarthTexture);
        m_Geometry->setDiffuseTexture(m_EarthTexture);
        m_Geometry->setNormalTexture(m_NormalTexture);
        
        resetTeapots();
        m_Touches.clear();
    }
    
    void World::destroy()
    {
        for (std::vector<Node*>::iterator i = m_TeapotNodes.begin();
             i != m_TeapotNodes.end();
             i++)
        {
            Node *node = *i;
            
            node->removeGeometry();
            
            m_Scene->removeActiveNode(node);
            m_Scene->getRootNode()->removeChildNode(node);
        }
    }

    void World::resize(float x, float y, float width, float height)
    {
        GLint params[4];
        glGetIntegerv(GL_VIEWPORT, params);
        
        if(params[2] != width && params[3] != height)
        {
            glViewport(x, y, width, height);
            
            m_Camera->setAspectRatio(fabs(width / height));
        }
    }
    
    void World::update(float step)
    {
        for (std::vector<Touch>::iterator i = m_Touches.begin();
             i != m_Touches.end();
             i++)
        {
            Touch t = *i;
        }
        m_Touches.clear();
        
        btQuaternion rotX(btVector3(1.0, 0.0, 0.0), m_Rotation);
        btQuaternion rotY(btVector3(0.0, 1.0, 0.0), m_Rotation);
        
        for (std::vector<Node*>::iterator i = m_TeapotNodes.begin();
             i != m_TeapotNodes.end();
             i++)
        {
            Node *node = *i;
            
            
            node->setNormalMatrix(node->getWorldTransform().getBasis().inverse().transpose());
            
            if(!isExploding())
            {
                node->setRotation(rotX * rotY);
                m_Rotation += step;
            }
        }
        
//        m_Scene->update(step);
        
        GLsizei verticeIdx = 0;
        GLsizei instanceIdx = 0;
        float mass = 1.0f;
        float maxSpeed = 1.0f;
        for (GLsizei i = 0; i < m_NumberOfTriangles; i++)
        {
            btVector3 acceleration(m_ImpulseForce[i] / mass);
            
            m_CurrentVelocity[i] = ((m_CurrentVelocity[i] + acceleration) * step);
            if(m_CurrentVelocity[i].length() > maxSpeed)
                m_CurrentVelocity[i] = m_CurrentVelocity[i].normalized() * maxSpeed;
            
            m_TriangleShrapnelTransforms[i].setOrigin(m_TriangleShrapnelTransforms[i].getOrigin() + m_CurrentVelocity[i] * step);
            
            m_Geometry->transformVertice(instanceIdx, (verticeIdx + 0), m_TriangleShrapnelTransforms[i]);
            m_Geometry->transformVertice(instanceIdx, (verticeIdx + 1), m_TriangleShrapnelTransforms[i]);
            m_Geometry->transformVertice(instanceIdx, (verticeIdx + 2), m_TriangleShrapnelTransforms[i]);
            
            verticeIdx += 3;
            
            m_ImpulseForce[i] = btVector3(0,0,0);
        }
    }
    
    void World::render()
    {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        
        m_Scene->render();
    }
    
    void World::addTouch(World::TouchState state, const btVector2 &touch, unsigned long taps)
    {
        World::Touch t;
        t.state = state;
        t.touch = touch;
        t.taps = taps;
        m_Touches.push_back(t);
    }
    
    void World::setShader(const std::string &shader)
    {
        ShaderMap::iterator i = m_ShaderMap.find(shader);
        if(i != m_ShaderMap.end())
        {
            Shader *shader = (*i).second;
            
            m_Geometry->setShader(shader);
        }
    }
    
    void World::setNumberOfTeapots(const int num)
    {
        if(num < MAXIMUM_TEAPOTS)
            m_NumberOfTeapots = num;
    }
    
    int World::numberOfTeapots()const
    {
        return m_NumberOfTeapots;
    }
    
    void World::explodeTeapots()
    {
        if(!m_IsExploding)
        {
            float min = 0.1;
            float max = 10.0;
            for (GLsizei i = 0; i < m_NumberOfTriangles; i++)
            {
                btVector3 force(m_Normals[i] * btVector3(randomFloat(min, max),
                                                         randomFloat(min, max),
                                                         randomFloat(min, max)));
                
                m_ImpulseForce[i] = force;
            }
            m_IsExploding = true;
        }
    }
    
    void World::subdivideTeapots()
    {
        m_Geometry->subdivide();
        
        resetTeapots();
    }
    
    void World::resetTeapots()
    {
        m_IsExploding = false;
        
        if(m_Normals)
            delete [] m_Normals;
        m_Normals = NULL;
        
        if(m_CurrentVelocity)
            delete [] m_CurrentVelocity;
        if(m_ImpulseForce)
            delete [] m_ImpulseForce;
        if(m_TriangleShrapnelTransforms)
            delete [] m_TriangleShrapnelTransforms;
        
        m_NumberOfTriangles = m_Geometry->numberOfVertices() / 3;
        
        m_TriangleShrapnelTransforms = new btTransform[m_NumberOfTriangles];
        m_ImpulseForce = new btVector3[m_NumberOfTriangles];
        m_CurrentVelocity = new btVector3[m_NumberOfTriangles];
        m_Normals = new btVector3[m_NumberOfTriangles];
        
        GLsizei instanceIdx = 0;
        GLsizei verticeIdx = 0;
        for (GLsizei i = 0; i < m_NumberOfTriangles; i++)
        {
            m_TriangleShrapnelTransforms[i] = btTransform::getIdentity();
            
            m_ImpulseForce[i] = btVector3(0.0, 0.0, 0.0);
            m_CurrentVelocity[i] = btVector3(0.0, 0.0, 0.0);
            
            btVector3 n0(m_Geometry->getVertexNormal(instanceIdx, (verticeIdx + 0)));
            btVector3 n1(m_Geometry->getVertexNormal(instanceIdx, (verticeIdx + 1)));
            btVector3 n2(m_Geometry->getVertexNormal(instanceIdx, (verticeIdx + 2)));
            
            m_Normals[i] = (n0 + n1 + n2) / 3;
            if(m_Normals[i].length2() > 0)
                m_Normals[i].normalize();
            
            m_Geometry->setVerticeTransform(instanceIdx, (verticeIdx + 0), btTransform::getIdentity());
            m_Geometry->setVerticeTransform(instanceIdx, (verticeIdx + 1), btTransform::getIdentity());
            m_Geometry->setVerticeTransform(instanceIdx, (verticeIdx + 2), btTransform::getIdentity());
            
            verticeIdx += 3;
        }
        m_Scene->getRootNode()->setOrigin(btVector3(0.0f, 0.0f, 5.0f));
    }
    
    bool World::isExploding()const
    {
        return m_IsExploding;
    }
    
    bool World::isMaxTesselations()const
    {
        return m_Geometry->isMaxSubdivisions();
    }
    
    Geometry *const World::getGeometry()const
    {
        return m_Geometry;
    }
    
    GLuint World::loadTexture(const std::string &filepath, GLuint idx, GLubyte **outData)const
    {
        GLuint      texture[1];
        glGenTextures(1, &texture[0]);
        
        glActiveTexture(GL_TEXTURE0 + idx);
        
        glBindTexture ( GL_TEXTURE_2D, texture[0] );
        
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        int width, height;
        bool hasAlpha;
        
        assert( readPngFile(filepath, width, height, hasAlpha, outData) );
        
//        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//        glTexImage2D(GL_TEXTURE_2D, 0, hasAlpha ? GL_RGBA : GL_RGB, width, height, 0, hasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, outData);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, *outData);
        
        
        
//        glEnable(GL_TEXTURE_2D);
        
        return texture[0];
    }
    
    World::World():
    m_Geometry(new MeshGeometry()),
    m_Camera(new Camera()),
    m_CameraNode(new Node()),
    m_Scene(new Scene()),
    m_NumberOfTeapots(1),
    m_Rotation(0.0f),
    m_TriangleShrapnelTransforms(NULL),
    m_ImpulseForce(NULL),
    m_CurrentVelocity(NULL),
    m_Normals(NULL),
    m_NumberOfTriangles(0),
    m_IsExploding(false),
    m_EarthTextureImage(NULL),
    m_NormalTextureImage(NULL),
    m_EarthTexture(0),
    m_NormalTexture(0)
    {
        for (unsigned long i = 0; i < MAXIMUM_TEAPOTS; i++)
            m_TeapotNodes.push_back(new Node());
        
        unsigned long i = 0;
        const char *shaderName = SHADERNAMES[i];
        do
        {
            m_Shaders.push_back(new Shader());
            shaderName = SHADERNAMES[++i];
        }
        while (shaderName != NULL);
    }
    
    World::~World()
    {
        if(m_EarthTexture)
            glDeleteTextures(1, &m_EarthTexture);
        if(m_NormalTexture)
            glDeleteTextures(1, &m_NormalTexture);
        
        if(m_EarthTextureImage)
            free(m_EarthTextureImage);
        m_EarthTextureImage = NULL;
        
        if(m_CurrentVelocity)
            delete [] m_CurrentVelocity;
        m_CurrentVelocity = NULL;
        
        if(m_ImpulseForce)
            delete [] m_ImpulseForce;
        m_ImpulseForce = NULL;
        
        if(m_TriangleShrapnelTransforms)
            delete [] m_TriangleShrapnelTransforms;
        m_TriangleShrapnelTransforms = NULL;
        
        while(!m_Shaders.empty())
        {
            Shader *shader = m_Shaders.back();
            delete shader;
            
            m_Shaders.pop_back();
        }
        
        while(!m_TeapotNodes.empty())
        {
            Node *node = m_TeapotNodes.back();
            delete node;
            
            m_TeapotNodes.pop_back();
        }
        
        delete m_Scene;
        delete m_CameraNode;
        delete m_Camera;
        delete m_Geometry;
    }
}
