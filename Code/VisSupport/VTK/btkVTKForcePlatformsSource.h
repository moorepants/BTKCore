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

#ifndef __btkVTKForcePlatformsSource_h
#define __btkVTKForcePlatformsSource_h

#include "btkForcePlatformCollection.h"

#include <vtkPolyDataAlgorithm.h>

namespace btk
{
  class VTKForcePlatformsSource : public vtkPolyDataAlgorithm 
  {
  public:
    BTK_VTK_EXPORT static VTKForcePlatformsSource* New();
    vtkTypeMacro(VTKForcePlatformsSource, vtkPolyDataAlgorithm);
    BTK_VTK_EXPORT void PrintSelf(ostream& os, vtkIndent indent);

    BTK_VTK_EXPORT void SetInput(ForcePlatformCollection::Pointer input);
    
    double GetScaleUnit() {return this->m_Scale;};
    void SetScaleUnit(double s) {this->m_Scale = s;};
    
    bool GetShowAxes() const {return this->m_ShowAxes;};
    BTK_VTK_EXPORT void SetShowAxes(bool actived);
    
    bool GetShowIndex() const {return this->m_ShowIndex;};
    BTK_VTK_EXPORT void SetShowIndex(bool actived);
    
  protected:
    BTK_VTK_EXPORT VTKForcePlatformsSource();
    ~VTKForcePlatformsSource() {};
    
    BTK_VTK_EXPORT virtual int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);

    BTK_VTK_EXPORT virtual int FillInputPortInformation(int port, vtkInformation* info);
    
  private:
    VTKForcePlatformsSource(const VTKForcePlatformsSource& );  // Not implemented.
    VTKForcePlatformsSource& operator=(const VTKForcePlatformsSource& );  // Not implemented.
    
    double m_Scale;
    bool m_ShowAxes;
    bool m_ShowIndex;
    bool m_PropertiesModified; // FIXME: Really necessary to add this parameter? 
  };
};
#endif // __btkVTKForcePlatformsSource_h
