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

#ifndef __btkVTKOpenGLContextDevice2D_h
#define __btkVTKOpenGLContextDevice2D_h

#include <vtkOpenGLContextDevice2D.h>
#include <vtkVersion.h>

namespace btk
{
#if (((VTK_MAJOR_VERSION == 5) && (VTK_MINOR_VERSION >= 8)) || (VTK_MAJOR_VERSION >= 6))
  class VTKOpenGLContextDevice2D : public vtkOpenGLContextDevice2D
  {
  public:
    vtkTypeMacro(VTKOpenGLContextDevice2D, vtkOpenGLContextDevice2D);
    
    static VTKOpenGLContextDevice2D* New();
    
    // ~VTKOpenGLContextDevice2D(); // Implicit.
    
    using vtkOpenGLContextDevice2D::DrawString;
    virtual void DrawString(float *point, const vtkStdString& string);
    
  protected:
    VTKOpenGLContextDevice2D();
    
  private:
    void AlignText(double orientation, float width, float height, float *p);
    
    VTKOpenGLContextDevice2D(const VTKOpenGLContextDevice2D& ); // Not implemented.
    void operator=(const VTKOpenGLContextDevice2D& ); // Not implemented.
  };
#else
  /**
   * Use to fix the issue #13199 (http://www.vtk.org/Bug/view.php?id=13199).
   */
  typedef vtkOpenGLContextDevice2D VTKOpenGLContextDevice2D;
#endif
};

#endif // __btkVTKOpenGLContextDevice2D_h