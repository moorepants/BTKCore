/* 
 * The Biomechanical ToolKit
 * Copyright (c) 2009-2013, Arnaud Barré
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *     * Redistributions of source code must retain the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *     * Neither the name(s) of the copyright holders nor the names
 *       of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written
 *       permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __btkVTKOpenGLContextDevice2D_p_h
#define __btkVTKOpenGLContextDevice2D_p_h

// WARNING: This is exaclty the same file than vtkOpenGL2ContextDevice2DPrivate.h 
//          which is distributed with the sources of VTK 5.10, but it is not installed.
//          There is no choice than copy the whole file to implement the classes
//          btk::VTKOpenGLContextDevice2D and btk::VTKOpenGL2ContextDevice2D used
//          to fix the issue #13199 (http://www.vtk.org/Bug/view.php?id=13199)

#include <vtkOpenGL2ContextDevice2D.h>
#include <vtkColor.h>
#include <vtkTextProperty.h>
#include <vtkFreeTypeTools.h>
#include <vtkVersion.h>

#include <algorithm>
#include <list>
#include <utility>

#include <assert.h> // Arnaud Barré - For MSVC 2008

// .NAME vtkTextureImageCache - store vtkTexture and vtkImageData identified by
// a unique key.
// .SECTION Description
// Creating and initializing a texture can be time consuming,
// vtkTextureImageCache offers the ability to reuse them as much as possible.
template <class Key>
class vtkTextureImageCache
{
public:
  struct CacheData
  {
    vtkSmartPointer<vtkImageData> ImageData;
    vtkSmartPointer<vtkTexture>   Texture;
  };

  // Description:
  // CacheElement associates a unique key to some cache.
  struct CacheElement: public std::pair<Key, CacheData>
  {
    // Default constructor
    CacheElement()
      : std::pair<Key, CacheData>(Key(), CacheData()){}
    // Construct a partial CacheElement with no CacheData
    // This can be used for temporary CacheElement used to search a given
    // key into the cache list.
    CacheElement(const Key& key)
      : std::pair<Key, CacheData>(key, CacheData()){}
    // Standard constructor of CacheElement
    CacheElement(const Key& key, const CacheData& cacheData)
      : std::pair<Key, CacheData>(key, cacheData){}
    // Operator tuned to be used when searching into the cache list using
    // std::find()
    bool operator==(const CacheElement& other)const
    {
      // Here we cheat and make the comparison only on the key, this allows
      // us to use std::find() to search for a given key.
      return this->first == other.first;
    }
  };

  // Description:
  // Construct a texture image cache with a maximum number of texture of 50.
  vtkTextureImageCache()
  {
    this->MaxSize = 50;
  }

  // Description:
  // Search the cache list to see if a given key already exists. Returns true
  // if the key is found, false otherwise.
  bool IsKeyInCache(const Key& key)const
  {
    return std::find(this->Cache.begin(), this->Cache.end(), key) != this->Cache.end();
  }

  // Description:
  // Return the cache associated to a key. If the key doesn't exist yet in the
  // cache list, create a new cache.
  // The returned cache is moved at the begining of the cache list for faster
  // search next time. The most use cache is faster to be searched.
  CacheData& GetCacheData(const Key& key);

  // Description:
  // Release all the OpenGL Pixel Buffer Object(PBO) associated with the
  // textures of the cache list.
  void ReleaseGraphicsResources(vtkWindow* window)
  {
    typename std::list<CacheElement >::iterator it;
    for (it = this->Cache.begin(); it != this->Cache.end(); ++it)
      {
      it->second.Texture->ReleaseGraphicsResources(window);
      }
  }

protected:
  // Description:
  // Add a new cache entry into the cache list. Enforce the MaxSize size of the
  // list by removing the least used cache if needed.
  CacheData& AddCacheData(const Key& key, const CacheData& cacheData)
  {
    assert(!this->IsKeyInCache(key));
    if (this->Cache.size() >= this->MaxSize)
      {
      this->Cache.pop_back();
      }
    this->Cache.push_front(CacheElement(key, cacheData));
    return this->Cache.begin()->second;
  }

  // Description:
  // List of a pair of key and cache data.
  std::list<CacheElement > Cache;
  // Description:
  // Maximum size the cache list can be.
  size_t MaxSize;
};

template<class Key>
typename vtkTextureImageCache<Key>::CacheData& vtkTextureImageCache<Key>
::GetCacheData(const Key& key)
{
  typename std::list<CacheElement>::iterator it =
    std::find(this->Cache.begin(), this->Cache.end(), CacheElement(key));
  if (it != this->Cache.end())
    {
    return it->second;
    }
  CacheData cacheData;
  cacheData.ImageData = vtkSmartPointer<vtkImageData>::New();
  cacheData.Texture = vtkSmartPointer<vtkTexture>::New();
#if (VTK_MAJOR_VERSION >= 6)
  cacheData.Texture->SetInputData(cacheData.ImageData);
#else
  cacheData.Texture->SetInput(cacheData.ImageData);
#endif
  return this->AddCacheData(key, cacheData);
}

// .NAME TextPropertyKey - unique key for a vtkTextProperty and text
// .SECTION Description
// Uniquely describe a pair of vtkTextProperty and text.
struct TextPropertyKey
{
  // Description:
  // Transform a text property into an unsigned long
  static unsigned int GetIdFromTextProperty(vtkTextProperty* textProperty)
  {
    unsigned long id;
    vtkFreeTypeTools::GetInstance()->MapTextPropertyToId(textProperty, &id);
    return static_cast<unsigned int>(id);
  }

  // Description:
  // Creates a TextPropertyKey.
  TextPropertyKey(vtkTextProperty* textProperty, const vtkStdString& text)
  {
    this->TextPropertyId = GetIdFromTextProperty(textProperty);
#if (((VTK_MAJOR_VERSION == 5) && (VTK_MINOR_VERSION >= 10)) || (VTK_MAJOR_VERSION >= 6))
    this->FontSize = textProperty->GetFontSize();
    double color[3];
    textProperty->GetColor(color);
    this->Color.Set(static_cast<unsigned char>(color[0] * 255),
                    static_cast<unsigned char>(color[1] * 255),
                    static_cast<unsigned char>(color[2] * 255),
                    static_cast<unsigned char>(textProperty->GetOpacity() * 255));
#endif
    this->Text = text;
  }

  // Description:
  // Compares two TextPropertyKeys with each other. Returns true if they are
  // identical: same text and text property
  bool operator==(const TextPropertyKey& other)const
  {
    return this->TextPropertyId == other.TextPropertyId &&
      this->Text == other.Text
#if (((VTK_MAJOR_VERSION == 5) && (VTK_MINOR_VERSION >= 10)) || (VTK_MAJOR_VERSION >= 6))
      &&
      this->FontSize == other.FontSize &&
      this->Color[0] == other.Color[0] &&
      this->Color[1] == other.Color[1] &&
      this->Color[2] == other.Color[2] &&
      this->Color[3] == other.Color[3]
#endif
      ;
  }
#if (((VTK_MAJOR_VERSION == 5) && (VTK_MINOR_VERSION >= 10)) || (VTK_MAJOR_VERSION >= 6))
  unsigned short FontSize;
  vtkColor4ub Color;
#endif
  // States in the function not to use more than 32 bits - int works fine here.
  unsigned int TextPropertyId;
  vtkStdString Text;
};

class vtkOpenGLContextDevice2D::Private
{
public:
  Private()
  {
    this->Texture = NULL;
    this->TextureProperties = vtkContextDevice2D::Linear |
        vtkContextDevice2D::Stretch;
    this->SpriteTexture = NULL;
    this->SavedLighting = GL_TRUE;
    this->SavedDepthTest = GL_TRUE;
    this->SavedAlphaTest = GL_TRUE;
    this->SavedStencilTest = GL_TRUE;
    this->SavedBlend = GL_TRUE;
    this->SavedDrawBuffer = 0;
    this->SavedClearColor[0] = this->SavedClearColor[1] =
                               this->SavedClearColor[2] =
                               this->SavedClearColor[3] = 0.0f;
    this->TextCounter = 0;
    this->GLExtensionsLoaded = false;
    this->OpenGL15 = false;
    this->OpenGL20 = false;
    this->GLSL = false;
    this->PowerOfTwoTextures = true;
  }

  ~Private()
  {
    if (this->Texture)
      {
      this->Texture->Delete();
      this->Texture = NULL;
      }
    if (this->SpriteTexture)
      {
      this->SpriteTexture->Delete();
      this->SpriteTexture = NULL;
      }
  }

  void SaveGLState(bool colorBuffer = false)
  {
    this->SavedLighting = glIsEnabled(GL_LIGHTING);
    this->SavedDepthTest = glIsEnabled(GL_DEPTH_TEST);

    if (colorBuffer)
      {
      this->SavedAlphaTest = glIsEnabled(GL_ALPHA_TEST);
      this->SavedStencilTest = glIsEnabled(GL_STENCIL_TEST);
      this->SavedBlend = glIsEnabled(GL_BLEND);
      glGetFloatv(GL_COLOR_CLEAR_VALUE, this->SavedClearColor);
      glGetIntegerv(GL_DRAW_BUFFER, &this->SavedDrawBuffer);
      }
  }

  void RestoreGLState(bool colorBuffer = false)
  {
    this->SetGLCapability(GL_LIGHTING, this->SavedLighting);
    this->SetGLCapability(GL_DEPTH_TEST, this->SavedDepthTest);

    if (colorBuffer)
      {
      this->SetGLCapability(GL_ALPHA_TEST, this->SavedAlphaTest);
      this->SetGLCapability(GL_STENCIL_TEST, this->SavedStencilTest);
      this->SetGLCapability(GL_BLEND, this->SavedBlend);

      if(this->SavedDrawBuffer != GL_BACK_LEFT)
        {
        glDrawBuffer(this->SavedDrawBuffer);
        }

      int i = 0;
      bool colorDiffer = false;
      while(!colorDiffer && i < 4)
        {
        colorDiffer=this->SavedClearColor[i++] != 0.0;
        }
      if(colorDiffer)
        {
        glClearColor(this->SavedClearColor[0],
                     this->SavedClearColor[1],
                     this->SavedClearColor[2],
                     this->SavedClearColor[3]);
        }
      }
  }

  void SetGLCapability(GLenum capability, GLboolean state)
  {
    if (state)
      {
      glEnable(capability);
      }
    else
      {
      glDisable(capability);
      }
  }

  float* TexCoords(float* f, int n)
  {
    float* texCoord = new float[2*n];
    float minX = f[0]; float minY = f[1];
    float maxX = f[0]; float maxY = f[1];
    float* fptr = f;
    for(int i = 0; i < n; ++i)
      {
      minX = fptr[0] < minX ? fptr[0] : minX;
      maxX = fptr[0] > maxX ? fptr[0] : maxX;
      minY = fptr[1] < minY ? fptr[1] : minY;
      maxY = fptr[1] > maxY ? fptr[1] : maxY;
      fptr+=2;
      }
    fptr = f;
    if (this->TextureProperties & vtkContextDevice2D::Repeat)
      {
      double* textureBounds = this->Texture->GetInput()->GetBounds();
      float rangeX = (textureBounds[1] - textureBounds[0]) ?
        textureBounds[1] - textureBounds[0] : 1.;
      float rangeY = (textureBounds[3] - textureBounds[2]) ?
        textureBounds[3] - textureBounds[2] : 1.;
      for (int i = 0; i < n; ++i)
        {
        texCoord[i*2] = (fptr[0]-minX) / rangeX;
        texCoord[i*2+1] = (fptr[1]-minY) / rangeY;
        fptr+=2;
        }
      }
    else // this->TextureProperties & vtkContextDevice2D::Stretch
      {
      float rangeX = (maxX - minX)? maxX - minX : 1.f;
      float rangeY = (maxY - minY)? maxY - minY : 1.f;
      for (int i = 0; i < n; ++i)
        {
        texCoord[i*2] = (fptr[0]-minX)/rangeX;
        texCoord[i*2+1] = (fptr[1]-minY)/rangeY;
        fptr+=2;
        }
      }
    return texCoord;
  }

  vtkVector2i FindPowerOfTwo(const vtkVector2i& size)
    {
    vtkVector2i pow2(1, 1);
    for (int i = 0; i < 2; ++i)
      {
      while (pow2[i] < size[i])
        {
        pow2[i] *= 2;
        }
      }
    return pow2;
    }

  GLuint TextureFromImage(vtkImageData *image, vtkVector2f& texCoords)
    {
    if (image->GetScalarType() != VTK_UNSIGNED_CHAR)
      {
      cout << "Error = not an unsigned char..." << endl;
      return 0;
      }
    int bytesPerPixel = image->GetNumberOfScalarComponents();
    int size[3];
    image->GetDimensions(size);
    vtkVector2i newImg = this->FindPowerOfTwo(vtkVector2i(size[0], size[1]));

    for (int i = 0; i < 2; ++i)
      {
      texCoords[i] = size[i] / float(newImg[i]);
      }

    unsigned char *dataPtr =
        new unsigned char[newImg[0] * newImg[1] * bytesPerPixel];
    unsigned char *origPtr =
        static_cast<unsigned char*>(image->GetScalarPointer());

    for (int i = 0; i < newImg[0]; ++i)
      {
      for (int j = 0; j < newImg[1]; ++j)
        {
        for (int k = 0; k < bytesPerPixel; ++k)
          {
          if (i < size[0] && j < size[1])
            {
            dataPtr[i * newImg[0] * bytesPerPixel + j * bytesPerPixel + k] =
                origPtr[i * size[0] * bytesPerPixel + j * bytesPerPixel + k];
            }
          else
            {
            dataPtr[i * newImg[0] * bytesPerPixel + j * bytesPerPixel + k] =
                k == 3 ? 0 : 255;
            }
          }
        }
      }

    GLuint tmpIndex(0);
    GLint glFormat = bytesPerPixel == 3 ? GL_RGB : GL_RGBA;
    GLint glInternalFormat = bytesPerPixel == 3 ? GL_RGB8 : GL_RGBA8;

    glGenTextures(1, &tmpIndex);
    glBindTexture(GL_TEXTURE_2D, tmpIndex);

    glTexEnvf(GL_TEXTURE_ENV, vtkgl::COMBINE_RGB, GL_REPLACE);
    glTexEnvf(GL_TEXTURE_ENV, vtkgl::COMBINE_ALPHA, GL_REPLACE);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                     vtkgl::CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                     vtkgl::CLAMP_TO_EDGE );

    glTexImage2D(GL_TEXTURE_2D, 0 , glInternalFormat,
                 newImg[0], newImg[1], 0, glFormat,
                 GL_UNSIGNED_BYTE, static_cast<const GLvoid *>(dataPtr));
    glAlphaFunc(GL_GREATER, static_cast<GLclampf>(0));
    glEnable(GL_ALPHA_TEST);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_TEXTURE_2D);
    delete [] dataPtr;
    return tmpIndex;
    }

  GLuint TextureFromImage(vtkImageData *image)
  {
    if (image->GetScalarType() != VTK_UNSIGNED_CHAR)
      {
      cout << "Error = not an unsigned char..." << endl;
      return 0;
      }
    int bytesPerPixel = image->GetNumberOfScalarComponents();
    int size[3];
    image->GetDimensions(size);

    unsigned char *dataPtr =
        static_cast<unsigned char*>(image->GetScalarPointer());
    GLuint tmpIndex(0);
    GLint glFormat = bytesPerPixel == 3 ? GL_RGB : GL_RGBA;
    GLint glInternalFormat = bytesPerPixel == 3 ? GL_RGB8 : GL_RGBA8;

    glGenTextures(1, &tmpIndex);
    glBindTexture(GL_TEXTURE_2D, tmpIndex);

    glTexEnvf(GL_TEXTURE_ENV, vtkgl::COMBINE_RGB, GL_REPLACE);
    glTexEnvf(GL_TEXTURE_ENV, vtkgl::COMBINE_ALPHA, GL_REPLACE);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                     vtkgl::CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                     vtkgl::CLAMP_TO_EDGE );

    glTexImage2D(GL_TEXTURE_2D, 0 , glInternalFormat,
                 size[0], size[1], 0, glFormat,
                 GL_UNSIGNED_BYTE, static_cast<const GLvoid *>(dataPtr));
    glAlphaFunc(GL_GREATER, static_cast<GLclampf>(0));
    glEnable(GL_ALPHA_TEST);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_TEXTURE_2D);
    return tmpIndex;
  }

  vtkTexture *Texture;
  unsigned int TextureProperties;
  vtkTexture *SpriteTexture;
  // Store the previous GL state so that we can restore it when complete
  GLboolean SavedLighting;
  GLboolean SavedDepthTest;
  GLboolean SavedAlphaTest;
  GLboolean SavedStencilTest;
  GLboolean SavedBlend;
  GLint SavedDrawBuffer;
  GLfloat SavedClearColor[4];

  int TextCounter;
  vtkVector2i Dim;
  vtkVector2i Offset;
  bool GLExtensionsLoaded;
  bool OpenGL15;
  bool OpenGL20;
  bool GLSL;
  bool PowerOfTwoTextures;

  // Description:
  // Cache for text images. Generating texture for strings is expensive,
  // we cache the textures here for a faster reuse.
  mutable vtkTextureImageCache<TextPropertyKey> TextTextureCache;
};

#endif // __btkVTKOpenGLContextDevice2D_p_h
