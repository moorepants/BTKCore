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
 * LOSS OF USE, DATA, OR PROFITS;OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __btkVTKInteractorStyleTrackballFixedUpCamera_h
#define __btkVTKInteractorStyleTrackballFixedUpCamera_h

#include "btkConfigure.h"

#include <vtkInteractorStyleTrackballCamera.h>

#define btk_VTKISTC_RUBBER 99

class vtkUnsignedCharArray;
class vtkRenderWindow;

namespace btk
{
  class VTKInteractorStyleTrackballFixedUpCamera : public vtkInteractorStyleTrackballCamera
  {
  public:
    BTK_VTK_EXPORT static VTKInteractorStyleTrackballFixedUpCamera* New();
    vtkTypeMacro(VTKInteractorStyleTrackballFixedUpCamera, vtkInteractorStyleTrackballCamera);
    BTK_VTK_EXPORT void PrintSelf(ostream& os, vtkIndent indent);
    
    BTK_VTK_EXPORT void GetGlobalUp(double n[3]) const {n[0] = this->mp_GlobalUp[0]; n[1] = this->mp_GlobalUp[1]; n[2] = this->mp_GlobalUp[2];};
    BTK_VTK_EXPORT void SetGlobalUp(const double n[3]);
    
    BTK_VTK_EXPORT virtual void Rotate();
    
    vtkGetMacro(RotationEnabled,int);
    vtkSetMacro(RotationEnabled,int);
    vtkBooleanMacro(RotationEnabled,int);
    vtkGetMacro(SpinEnabled,int);
    vtkSetMacro(SpinEnabled,int);
    vtkBooleanMacro(SpinEnabled,int);
    vtkGetMacro(PanEnabled,int);
    vtkSetMacro(PanEnabled,int);
    vtkBooleanMacro(PanEnabled,int);
    vtkGetMacro(DollyEnabled,int);
    vtkSetMacro(DollyEnabled,int);
    vtkBooleanMacro(DollyEnabled,int);
    
    vtkGetMacro(CharEventEnabled,int);
    vtkSetMacro(CharEventEnabled,int);
    vtkBooleanMacro(CharEventEnabled,int);
    
    vtkGetMacro(RightButtonEnabled,int);
    vtkSetMacro(RightButtonEnabled,int);
    vtkBooleanMacro(RightButtonEnabled,int);
    
    BTK_VTK_EXPORT virtual void OnLeftButtonDown();
    BTK_VTK_EXPORT virtual void OnLeftButtonUp();
    BTK_VTK_EXPORT virtual void OnRightButtonDown();
    BTK_VTK_EXPORT virtual void OnRightButtonUp();
    BTK_VTK_EXPORT virtual void OnMouseMove();
    BTK_VTK_EXPORT virtual void OnChar();
    
    vtkGetMacro(ForceRubberBandDrawing,int);
    vtkSetMacro(ForceRubberBandDrawing,int);
    vtkBooleanMacro(ForceRubberBandDrawing,int);
    BTK_VTK_EXPORT void GetRubberBandPoints(int pts[4]) const;
    BTK_VTK_EXPORT void Rubber();
    BTK_VTK_EXPORT void RedrawRubberBand();
    BTK_VTK_EXPORT void UpdateRubberBackground();
    BTK_VTK_EXPORT virtual void StartRubberBand();
    BTK_VTK_EXPORT virtual void EndRubberBand();
    
    BTK_VTK_EXPORT void ActivateFixForRubberBandDrawing_MacOS1050(bool activated);
    
  protected:
    BTK_VTK_EXPORT VTKInteractorStyleTrackballFixedUpCamera();
    BTK_VTK_EXPORT ~VTKInteractorStyleTrackballFixedUpCamera();
    
  private:
    static int GetForegroundFrameBufferIndex(vtkRenderWindow* renWin, int forceRubberBandDrawing);
    static int GetForegroundFrameBufferIndex_FixMacOS1050(vtkRenderWindow* renWin, int forceRubberBandDrawing);
    
    double mp_GlobalUp[3];
    int RotationEnabled;
    int SpinEnabled;
    int PanEnabled;
    int DollyEnabled;
    int CharEventEnabled;
    int RightButtonEnabled;
    int mp_RubberBandCorners[2][2];
    vtkUnsignedCharArray* mp_RubberBandHorizontalLinesForeground[4];
    vtkUnsignedCharArray* mp_RubberBandHorizontalLinesBackground[4];
    int ForceRubberBandDrawing;
  };
};

#endif // __btkVTKInteractorStyleTrackballFixedUpCamera_h