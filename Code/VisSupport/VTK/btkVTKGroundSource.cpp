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

#include "btkVTKGroundSource.h"
#include "btkVTKDataObjectAdapter.h"

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPlaneSource.h>
#include <vtkVersion.h>

namespace btk
{
  /**
   * @class VTKGroundSource btkVTKGroundSource.h
   * @brief Displays ground based on the given orientation.
   * 
   * @ingroup BTKVTK
   */
  /**
   * @enum VTKGroundSource::Orientation
   * Enums used to specify the orientation of the ground.
   */
  /**
   * @var VTKGroundSource::Orientation VTKGroundSource::Automatic
   * Use the metada POINT:Y_SCREEN if available or set the orientation to PlaneXY.
   */
  /**
   * @var VTKGroundSource::Orientation VTKGroundSource::PlaneXY
   * Force the orientation to be defined by the axes X and Y.
   */
  /**
   * @var VTKGroundSource::Orientation VTKGroundSource::PlaneYZ
   * Force the orientation to be defined by the axes Y and Z.
   */
  /**
   * @var VTKGroundSource::Orientation VTKGroundSource::PlaneZX
   * Force the orientation to be defined by the axes Z and X.
   */
  
  /**
   * @fn static VTKGroundSource* VTKGroundSource::New();
   * Creates a VTKGroundSource object and return it as a pointer.
   */
  vtkStandardNewMacro(VTKGroundSource);
  
  /**
   * Prints only Superclass informations.
   */
  void VTKGroundSource::PrintSelf(ostream& os, vtkIndent indent)
  {
    this->Superclass::PrintSelf(os,indent);
  };

  /**
   * Sets the input.
   */
  void VTKGroundSource::SetInput(Acquisition::Pointer input)
  {
    VTKDataObjectAdapter* inObject = VTKDataObjectAdapter::New();
    inObject->SetBTKDataObject(input);
#if (VTK_MAJOR_VERSION >= 6)
    this->vtkPolyDataAlgorithm::SetInputData(inObject);
#else
    this->vtkPolyDataAlgorithm::SetInput(inObject);
#endif
    inObject->Delete();
  };
  
  /**
   * @fn Orientation VTKGroundSource::GetOrientation() const
   * Gets the orientation used for the ground source.
   */
  
  /**
   * Set the orientation automatically or manually. The automatic orientation use the metadata POINT:Y_SCREEN.
   */
  void VTKGroundSource::SetOrientation(Orientation o)
  {
    if (this->m_Orientation == o)
      return;
    this->m_Orientation = o;
    this->Modified();
  };
  
  /**
   * @fn void VTKGroundSource::GetNormal(double n[3]) const 
   * Returns the normal to the ground as a XYZ vector.
   */
  
  /**
   * @fn Orientation VTKGroundSource::GetAutomaticDefaultOrientation() const
   * Gets the default plane use by the automatic orientation if the metadata POINT:Y_SCREEN is not found.
   */
  
  
  /**
   * Sets the default plane use by the automatic orientation if the metadata POINT:Y_SCREEN is not found.
   *
   * The value Automatic is not useable with this method.
   */
  void VTKGroundSource::SetAutomaticDefaultOrientation(Orientation o)
  {
    if (o == Automatic)
    {
      btkErrorMacro("Only value PlaneXY, PlaneYZ and PlaneZX can be used.");
      return;
    }
    if (this->m_AutomaticDefaultOrientation == o)
      return;
    this->m_AutomaticDefaultOrientation = o;
    if (this->m_Orientation == Automatic)
      this->Modified();
  };
  
  /**
   * Constructor.
   *
   * Filter with one input and output.
   */
  VTKGroundSource::VTKGroundSource()
  : vtkPolyDataAlgorithm()
  {
    this->m_Orientation = Automatic;
    // Plane XY by default.
    this->mp_Normal[0] = 0.0; 
    this->mp_Normal[1] = 0.0;
    this->mp_Normal[2] = 1.0;
    this->m_AutomaticDefaultOrientation = PlaneXY;
  };

  /**
   * @fn VTKGroundSource::~VTKGroundSource()
   * Empty destructor.
   */
  
  /**
   * Prepare force platform geometry.
   */
  int VTKGroundSource::RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
  {
    btkNotUsed(request);
    btkNotUsed(outputVector);
    
    double n[3];
    switch (this->m_Orientation)
    {
    case Automatic:
      {
      switch (this->m_AutomaticDefaultOrientation)
      {
      default:
        btkWarningMacro("Unknown default orientation. Set to plane XY");
      case PlaneXY:
        n[0] = 0.0; n[1] = 0.0; n[2] = 1.0;
        break;
      case PlaneYZ:
        n[0] = 1.0; n[1] = 0.0; n[2] = 0.0;
        break;
      case PlaneZX:
        n[0] = 0.0; n[1] = 1.0; n[2] = 0.0;
        break;
      }
      
      vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
      VTKDataObjectAdapter* inObject;
      if (inInfo && ((inObject = VTKDataObjectAdapter::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()))) != NULL))
      {
        Acquisition::Pointer input = static_pointer_cast<Acquisition>(inObject->GetBTKDataObject());
        if (input)
        {
          btk::MetaData::ConstIterator itPoint = input->GetMetaData()->FindChild("POINT");
          if (itPoint != input->GetMetaData()->End())
          {
            btk::MetaDataInfo::Pointer yScreenInfo = (*itPoint)->ExtractChildInfo("Y_SCREEN", MetaDataInfo::Char, 1);
            if (yScreenInfo)
            {
              std::string yScreenVal = yScreenInfo->ToString(0);
              if ((yScreenVal.compare("+X") == 0) || (yScreenVal.compare("-X") == 0))
                {n[0] = 1.0; n[1] = 0.0; n[2] = 0.0;}
              else if ((yScreenVal.compare("+Y") == 0) || (yScreenVal.compare("-Y") == 0))
                {n[0] = 0.0; n[1] = 1.0; n[2] = 0.0;}
              else if ((yScreenVal.compare("+Z") == 0) || (yScreenVal.compare("-Z") == 0))
                {n[0] = 0.0; n[1] = 0.0; n[2] = 1.0;}
            }
          }
        }
      }
      break;
      }
    case PlaneXY:
      n[0] = 0.0; n[1] = 0.0; n[2] = 1.0;
      break;
    case PlaneYZ:
      n[0] = 1.0; n[1] = 0.0; n[2] = 0.0;
      break;
    case PlaneZX:
      n[0] = 0.0; n[1] = 1.0; n[2] = 0.0;
      break;
    }
    this->mp_Normal[0] = n[0]; 
    this->mp_Normal[1] = n[1];
    this->mp_Normal[2] = n[2];
    
    return 1;
  };
  
  /**
   * Generate force platform geometry.
   */
  int VTKGroundSource::RequestData(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector)
  {
    btkNotUsed(request);
    btkNotUsed(inputVector);
    
    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

    vtkPlaneSource* grid = vtkPlaneSource::New();
    grid->SetXResolution(20);
    grid->SetYResolution(20);
    grid->SetOrigin(-15.0, -15.0, 0.0);
    grid->SetPoint1(15.0, -15.0, 0.0);
    grid->SetPoint2(-15.0, 15.0, 0.0);
    grid->SetNormal(this->mp_Normal);
    grid->Update();
    output->ShallowCopy(grid->GetOutput());
    grid->Delete();
    
    return 1;
  };

  /**
   * Sets the type of object required for the input.
   */
  int VTKGroundSource::FillInputPortInformation(int port, 
                                                vtkInformation* info)
  {
    btkNotUsed(port);
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "VTKDataObjectAdapter");
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    return 1;
  }
};
