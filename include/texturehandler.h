#pragma once

#include "glew.h"
#include "texture.h"
#include "cubemap.h"
#include "list.hpp"
#include "sharedpointer.hpp"

// =================================================================================================
// Very simply class for texture tracking
// Main purpose is to keep track of all texture objects in the game and return them to OpenGL in
// a well defined and controlled way at program termination without having to bother about releasing
// textures at a dozen places in the game

class TextureHandler 
{
    public:
        TextureList m_textures;

        typedef Texture* (*tGetter) (void);

        TextureHandler() {}

        ~TextureHandler () {
            Destroy ();
        }

        void Destroy(void);

        Texture* GetTexture(void);

        Cubemap* GetCubemap(void);

        bool Remove(Texture* texture);

        TextureList Create(String textureFolder, List<String>& textureNames, GLenum textureType);

        TextureList CreateTextures(String textureFolder, List<String>& textureNames);

        TextureList CreateCubemaps(String textureFolder, List<String>& textureNames);

        TextureList CreateByType(String textureFolder, List<String>& textureNames, GLenum textureType);

};

extern TextureHandler* textureHandler;

// =================================================================================================
