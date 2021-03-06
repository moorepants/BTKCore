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

#ifndef __btkVTKChartLayout_h
#define __btkVTKChartLayout_h

#include "btkConfigure.h"

#include "vtkAbstractContextItem.h"
#include "vtkVector.h" // For ivars

namespace btk
{
  class VTKChartTimeSeries;

  class VTKChartLayout : public vtkAbstractContextItem
  {
  public:
    vtkTypeMacro(VTKChartLayout, vtkAbstractContextItem);
    
    BTK_VTK_EXPORT static VTKChartLayout* New();

    BTK_VTK_EXPORT virtual bool Paint(vtkContext2D *painter);
    
    BTK_VTK_EXPORT virtual void SetSize(const vtkVector2i& size);
    BTK_VTK_EXPORT virtual vtkVector2i GetSize() const {return this->Size;}

    BTK_VTK_EXPORT virtual void SetGutter(const vtkVector2f& gutter);
    BTK_VTK_EXPORT virtual vtkVector2f GetGutter() const {return this->Gutter;}

    BTK_VTK_EXPORT virtual void SetBorders(int left, int bottom, int right, int top);
    BTK_VTK_EXPORT virtual void GetBorders(int borders[4]);

    BTK_VTK_EXPORT virtual bool SetChart(const vtkVector2i& position, VTKChartTimeSeries* chart);
    BTK_VTK_EXPORT virtual VTKChartTimeSeries* GetChart(const vtkVector2i& position);
    BTK_VTK_EXPORT virtual VTKChartTimeSeries* TakeChart(const vtkVector2i &position);
    
    BTK_VTK_EXPORT virtual bool SetChartSpan(const vtkVector2i& position, const vtkVector2i& span);
    BTK_VTK_EXPORT virtual vtkVector2i GetChartSpan(const vtkVector2i& position);
    
    BTK_VTK_EXPORT virtual void UpdateLayout();
    
    BTK_VTK_EXPORT virtual void PrintSelf(ostream& os, vtkIndent indent);

  protected:
    BTK_VTK_EXPORT VTKChartLayout();
    BTK_VTK_EXPORT ~VTKChartLayout();

    // The number of charts in x and y.
    vtkVector2i Size;
    // The gutter between each chart.
    vtkVector2f Gutter;
    int Borders[4];
    bool LayoutIsDirty;

  private:
    class PIMPL;
    PIMPL *Private;
    
    VTKChartLayout(const VTKChartLayout &); // Not implemented.
    void operator=(const VTKChartLayout &); // Not implemented.
  };
};
#endif //__btkVTKChartLayout_h
