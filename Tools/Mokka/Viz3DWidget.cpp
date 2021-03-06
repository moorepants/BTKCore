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

#include "Viz3DWidget.h"
#include "Acquisition.h"

#include <btkVTKAxesWidget.h>
#include <btkVTKMarkerPickerInteractionCallback.h>
#include <btkVTKRubberRenderInteractionCallback.h>
#include <btkVTKInteractorStyleTrackballFixedUpCamera.h>

#include <vtkCellPicker.h>
#include <vtkCallbackCommand.h>
#include <vtkAxesActor.h>
#include <vtkIdList.h>
#include <vtkCamera.h>
#include <vtkCollectionIterator.h>
#include <vtkCaptionActor2D.h>
#include <vtkTextProperty.h>
#include <vtkTextActor.h>
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkTIFFWriter.h>

#include <QKeyEvent>
#include <QToolTip>
#include <QSysInfo>

static const double CameraZoom = 1.6;

Viz3DWidget::Viz3DWidget(QWidget* parent)
: VizRendererWidget(parent)
{
  // Member
  this->mp_Acquisition = 0;
  this->mp_Renderer = vtkRenderer::New();
  this->mp_AxesWidget = btk::VTKAxesWidget::New();
  this->mp_EventQtSlotConnections = vtkEventQtSlotConnect::New();
  this->m_CameraConfigurationSaved = false;
  this->mp_CamFocalPoint[0] = 0.0; this->mp_CamFocalPoint[1] = 0.0; this->mp_CamFocalPoint[2] = 0.0;
  this->mp_CamPosition[0] = 0.0; this->mp_CamPosition[1] = 0.0; this->mp_CamPosition[2] = 0.0;
  this->mp_CamViewUp[0] = 0.0; this->mp_CamViewUp[1] = 0.0; this->mp_CamViewUp[2] = 0.0;
  
  // No need to send mouse events to VTK when a mouse button isn't down
  this->setMouseTracking(false);
  
#if defined(Q_OS_WIN) && (QT_VERSION == 0x040803)
  // Drag and drop set manually for Qt 4.8.3 as it introduced a bug under Windows for the drag'n drop action.
  this->setAcceptDrops(true);
#endif
}

Viz3DWidget::~Viz3DWidget()
{
  this->mp_Renderer->Delete();
  this->mp_AxesWidget->Delete();
  this->mp_EventQtSlotConnections->Delete();
};

void Viz3DWidget::initialize()
{
  // VTK UI
  //this->mp_Renderer->TwoSidedLightingOn();
  VizRendererWindow* renwin = VizRendererWindow::New();
  renwin->AddRenderer(this->mp_Renderer);
  renwin->LineSmoothingOn();
  //renwin->PolygonSmoothingOn();
  this->SetRenderWindow(renwin);
  renwin->Delete();
  // VTK cell picker
  vtkCellPicker* cellPicker = vtkCellPicker::New();
  cellPicker->SetTolerance(0.005);
  this->GetRenderWindow()->GetInteractor()->SetPicker(cellPicker);
  cellPicker->Delete();
  // VTK interaction style
  btk::VTKInteractorStyleTrackballFixedUpCamera* style = btk::VTKInteractorStyleTrackballFixedUpCamera::New();
  style->CharEventEnabledOff();
  style->RightButtonEnabledOff(); // Conflict with the right click button for the contextual menu.
  vtkCallbackCommand* pickerMouseInteraction = vtkCallbackCommand::New();
  pickerMouseInteraction->SetClientData(style);
  pickerMouseInteraction->SetCallback(&btk::VTKPickerInteractionCallback);
  style->AddObserver(vtkCommand::LeftButtonPressEvent, pickerMouseInteraction);
  style->AddObserver(vtkCommand::MouseMoveEvent, pickerMouseInteraction);
  style->AddObserver(vtkCommand::LeftButtonReleaseEvent, pickerMouseInteraction);
#ifdef Q_OS_MAC
  if (QSysInfo::MacintoshVersion == QSysInfo::MV_LEOPARD)
    style->ActivateFixForRubberBandDrawing_MacOS1050(true);
#endif
  this->GetRenderWindow()->GetInteractor()->SetInteractorStyle(style);
  pickerMouseInteraction->Delete();
  style->Delete();
  // Rubber callback
  vtkCallbackCommand* rubberRenderInteraction = vtkCallbackCommand::New();
  rubberRenderInteraction->SetClientData(style);
  rubberRenderInteraction->SetCallback(&btk::VTKRubberRenderInteractionCallback);
  this->mp_Renderer->AddObserver(vtkCommand::EndEvent, rubberRenderInteraction);
  rubberRenderInteraction->Delete();
  
  // VTK WIDGET
  this->mp_AxesWidget->SetParentRenderer(this->mp_Renderer);
  this->mp_AxesWidget->SetInteractor(this->GetRenderWindow()->GetInteractor());
  this->mp_AxesWidget->SetZoomFactor(1.25);
  vtkAxesActor* axesActor = this->mp_AxesWidget->GetAxesActor();
  axesActor->SetShaftTypeToCylinder();
  axesActor->SetNormalizedLabelPosition(1.25, 1.25, 1.25);
  axesActor->SetCylinderRadius(0.500 * axesActor->GetCylinderRadius());
  axesActor->SetConeRadius(1.025 * axesActor->GetConeRadius());
  axesActor->GetXAxisCaptionActor2D()->GetTextActor()->SetTextScaleModeToNone();
  axesActor->GetYAxisCaptionActor2D()->GetTextActor()->SetTextScaleModeToNone();
  axesActor->GetZAxisCaptionActor2D()->GetTextActor()->SetTextScaleModeToNone();
  axesActor->GetXAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(10);
  axesActor->GetYAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(10);
  axesActor->GetZAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(10);
  this->mp_AxesWidget->SetEnabled(1);
  
  // Links between VTK & Qt
  this->mp_EventQtSlotConnections->Connect(
      this->GetRenderWindow()->GetInteractor(), 
      btk::VTKMarkerPickedEvent,
      this, 
      SLOT(selectPickedMarker(vtkObject*, unsigned long, void*, void*)));
  this->mp_EventQtSlotConnections->Connect(
      this->GetRenderWindow()->GetInteractor(), 
      btk::VTKToggleMarkerPickedEvent,
      this, 
      SLOT(togglePickedMarker(vtkObject*, unsigned long, void*, void*)));
  this->mp_EventQtSlotConnections->Connect(
      this->GetRenderWindow()->GetInteractor(), 
      btk::VTKToggleMarkersSelectedEvent,
      this, 
      SLOT(toggleSelectedMarkers(vtkObject*, unsigned long, void*, void*)));
  this->mp_EventQtSlotConnections->Connect(
      this->GetRenderWindow()->GetInteractor(), 
      btk::VTKToggleMarkerTrajectoryPickedEvent,
      this, 
      SLOT(toggleTrajectoryMarker(vtkObject*, unsigned long, void*, void*)));
      
  this->mp_Renderer->GetActiveCamera()->SetClippingRange(1.0,1000.0);
};

void Viz3DWidget::restoreProjectionCameraConfiguration()
{
  if (!this->m_CameraConfigurationSaved) // No configuration saved
    return;
  vtkCamera* cam = this->mp_Renderer->GetActiveCamera();
  cam->SetPosition(this->mp_CamPosition);
  cam->SetFocalPoint(this->mp_CamFocalPoint);
  cam->SetViewUp(this->mp_CamViewUp);
  this->mp_Renderer->ResetCameraClippingRange();
  this->m_CameraConfigurationSaved = false;
};

void Viz3DWidget::saveProjectionCameraConfiguration()
{
  if (this->m_CameraConfigurationSaved) // Configuration already saved
    return;
  vtkCamera* cam = this->mp_Renderer->GetActiveCamera();
  cam->GetPosition(this->mp_CamPosition);
  cam->GetViewUp(this->mp_CamViewUp);
  cam->GetFocalPoint(this->mp_CamFocalPoint);
  this->m_CameraConfigurationSaved = true;
};

void Viz3DWidget::copyProjectionCameraConfiguration(Viz3DWidget* source)
{
  this->m_CameraConfigurationSaved = source->m_CameraConfigurationSaved;
  memcpy(this->mp_CamPosition, source->mp_CamPosition, 3*sizeof(double));
  memcpy(this->mp_CamViewUp, source->mp_CamViewUp, 3*sizeof(double));
  memcpy(this->mp_CamFocalPoint, source->mp_CamFocalPoint, 3*sizeof(double));
};

void Viz3DWidget::projectionCamera(double focalPoint[3], double position[3], double viewUp[3])
{
  vtkCamera* cam = this->mp_Renderer->GetActiveCamera();
  cam->GetPosition(position);
  cam->GetViewUp(viewUp);
  cam->GetFocalPoint(focalPoint);
};

void Viz3DWidget::setProjectionCamera(double focalPoint[3], double position[3], double viewUp[3])
{
  vtkCamera* cam = this->mp_Renderer->GetActiveCamera();
  cam->SetPosition(position);
  cam->SetViewUp(viewUp);
  cam->SetFocalPoint(focalPoint);
  this->mp_Renderer->ResetCameraClippingRange();
};

void Viz3DWidget::setOrthogonalView(int view)
{
  double n[3];
  static_cast<btk::VTKInteractorStyleTrackballFixedUpCamera*>(this->GetRenderWindow()->GetInteractor()->GetInteractorStyle())->GetGlobalUp(n);
  vtkCamera* cam = this->mp_Renderer->GetActiveCamera();
  cam->SetFocalPoint(0.0,0.0,0.0);
  if (n[0] == 1.0)
  {
    cam->SetViewUp(0.0,1.0,0.0);
    cam->SetRoll(0.0);
    switch (view)
    {
    case Top:
      cam->SetPosition(1.0,0.0,0.0);
      break;
    case Bottom:
      cam->SetPosition(-1.0,0.0,0.0);
      cam->SetRoll(180.0);
      break;
    case Left:
      cam->SetPosition(0.0,0.0,1.0);
      cam->SetRoll(90.0);
      break;
    case Right:
      cam->SetPosition(0.0,0.0,-1.0);
      cam->SetRoll(-90.0);
      break;
    case Back:
      cam->SetPosition(0.0,1.0,0.0);
      cam->SetViewUp(1.0,0.0,0.0);
      cam->SetRoll(90.0);
      break;
    case Front:
      cam->SetPosition(0.0,-1.0,0.0);
      cam->SetViewUp(1.0,0.0,0.0);
      cam->SetRoll(90.0);
      break;
    default:
      qDebug("Unknown orthogonal view.");
    }
  }
  else if (n[1] == 1.0)
  {
    cam->SetViewUp(0.0,1.0,0.0);
    cam->SetRoll(0.0);
    switch (view)
    {
    case Top:
      cam->SetViewUp(0.0,0.0,-1.0);
      cam->SetPosition(0.0,1.0,0.0);
      break;
    case Bottom:
      cam->SetViewUp(0.0,0.0,1.0);
      cam->SetPosition(0.0,-1.0,0.0);
      break;
    case Left:
      cam->SetPosition(-1.0,0.0,0.0);
      break;
    case Right:
      cam->SetPosition(1.0,0.0,0.0);
      break;
    case Back:
      cam->SetPosition(0.0,0.0,-1.0);
      break;
    case Front:
      cam->SetPosition(0.0,0.0,1.0);
      break;
    default:
      qDebug("Unknown orthogonal view.");
    }
  }
  else if (n[2] == 1.0)
  {
    cam->SetViewUp(0.0,1.0,0.0);
    cam->SetRoll(0.0);
    switch (view)
    {
    case Top:
      cam->SetPosition(0.0,0.0,1.0);
      break;
    case Bottom:
      cam->SetPosition(0.0,0.0,-1.0);
      cam->SetRoll(180.0);
      break;
    case Left:
      cam->SetPosition(-1.0,0.0,0.0);
      cam->SetRoll(90.0);
      break;
    case Right:
      cam->SetPosition(1.0,0.0,0.0);
      cam->SetRoll(-90.0);
      break;
    case Back:
      cam->SetPosition(0.0,1.0,0.0);
      cam->SetViewUp(0.0,0.0,1.0);
      break;
    case Front:
      cam->SetPosition(0.0,-1.0,0.0);
      cam->SetViewUp(0.0,0.0,1.0);
      break;
    default:
      qDebug("Unknown orthogonal view.");
    }
  }
  else
    qDebug("Unsupported global up vector.");
  this->mp_Renderer->ResetCamera();
  cam->Zoom(CameraZoom);
  this->render();
}

void Viz3DWidget::setGlobalFrameVisible(bool visible)
{
  this->mp_AxesWidget->SetEnabled(visible ? 1 : 0);
};

void Viz3DWidget::copy(Viz3DWidget* source)
{
  // Clean the 3D view
  this->mp_Renderer->RemoveAllViewProps();
  
  // Copy the acquisition pointer
  this->setAcquisition(source->acquisition());
  // Add vtkViewProp to the new QVtkDialogWidget
  vtkRenderer* sourceRenderer = source->mp_Renderer;
  vtkCollectionIterator* it = sourceRenderer->GetViewProps()->NewIterator();
  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
  {
    this->mp_Renderer->AddViewProp(static_cast<vtkProp*>(it->GetCurrentObject()));
    it->GoToNextItem();
  }
  it->Delete();
  this->mp_Renderer->SetBackground(sourceRenderer->GetBackground());
  this->copyProjectionCameraConfiguration(source);
  // Copy camera orientation
  vtkCamera* sourceCamera = sourceRenderer->GetActiveCamera();
  vtkCamera* targetCamera = this->mp_Renderer->GetActiveCamera();
  targetCamera->SetPosition(sourceCamera->GetPosition()); 
  targetCamera->SetFocalPoint(sourceCamera->GetFocalPoint()); 
  targetCamera->SetViewUp(sourceCamera->GetViewUp());
  this->mp_Renderer->ResetCameraClippingRange();
  targetCamera->Zoom(CameraZoom);
};

void Viz3DWidget::renderToPng(const QString& filename)
{
  vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
  windowToImageFilter->SetInput(this->GetRenderWindow());
  windowToImageFilter->SetMagnification(1); 
  // windowToImageFilter->SetInputBufferTypeToRGBA(); //also record the alpha (transparency) channel
  windowToImageFilter->Update();
  vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
  writer->SetFileName(qPrintable(filename));
  writer->SetInputConnection(windowToImageFilter->GetOutputPort());
  writer->Write();
}

void Viz3DWidget::renderToJpeg(const QString& filename)
{
  vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
  windowToImageFilter->SetInput(this->GetRenderWindow());
  windowToImageFilter->SetMagnification(1); 
  // windowToImageFilter->SetInputBufferTypeToRGBA(); //also record the alpha (transparency) channel
  windowToImageFilter->Update();
  vtkSmartPointer<vtkJPEGWriter> writer = vtkSmartPointer<vtkJPEGWriter>::New();
  writer->ProgressiveOn();
  writer->SetQuality(95);
  writer->SetFileName(qPrintable(filename));
  writer->SetInputConnection(windowToImageFilter->GetOutputPort());
  writer->Write();
}

void Viz3DWidget::renderToTiff(const QString& filename)
{
  vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
  windowToImageFilter->SetInput(this->GetRenderWindow());
  windowToImageFilter->SetMagnification(1); 
  // windowToImageFilter->SetInputBufferTypeToRGBA(); //also record the alpha (transparency) channel
  windowToImageFilter->Update();
  vtkSmartPointer<vtkTIFFWriter> writer = vtkSmartPointer<vtkTIFFWriter>::New();
  writer->SetCompressionToNoCompression();
  writer->SetFileName(qPrintable(filename));
  writer->SetInputConnection(windowToImageFilter->GetOutputPort());
  writer->Write();
}

void Viz3DWidget::selectPickedMarker(vtkObject* /* caller */, unsigned long /* vtk_event */, void* /* client_data */, void* call_data)
{
  int id = *static_cast<int*>(call_data);
  emit pickedMarkerChanged(id);
};

void Viz3DWidget::togglePickedMarker(vtkObject* /* caller */, unsigned long /* vtk_event */, void* /* client_data */, void* call_data)
{
  int id = *static_cast<int*>(call_data);
  emit pickedMarkerToggled(id);
};

void Viz3DWidget::toggleSelectedMarkers(vtkObject* /* caller */, unsigned long /* vtk_event */, void* /* client_data */, void* call_data)
{
  QList<int> ids;
  vtkIdList* selectionIds = static_cast<vtkIdList*>(call_data);
  for (int i = 0 ; i < selectionIds->GetNumberOfIds() ; ++i)
    ids << selectionIds->GetId(i);
  emit selectedMarkersToggled(ids);
};

void Viz3DWidget::toggleTrajectoryMarker(vtkObject* /* caller */, unsigned long /* vtk_event */, void* /* client_data */, void* call_data)
{
  int id = *static_cast<int*>(call_data);
  emit trajectoryMarkerToggled(id);
};

void Viz3DWidget::render()
{
  // FIXME: A simple ChartWidget::update() is enough under MacOS X but not Windows XP...
  //        The use of the method vtkRenderWindow:::Render could slowdown the display...
  //        Should we need a special case for Windows XP? Same for Windows 7?
  if (this->updatesEnabled())
    this->GetRenderWindow()->Render();
};

void Viz3DWidget::show(bool s)
{
  vtkActorCollection* actors = this->mp_Renderer->GetActors();
  actors->InitTraversal();
  vtkActor* actor = actors->GetNextItem();
  actor = actors->GetNextItem(); // Ground
  while (actor)
  {
    actor->SetVisibility(s ? 1 : 0);
    actor = actors->GetNextItem();
  }
  if (this->isVisible())
    this->render();
};

bool Viz3DWidget::event(QEvent* event)
{
  if ((event->type() == QEvent::ToolTip) && this->mp_Acquisition)
  {
    QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
    vtkCellPicker* picker = static_cast<vtkCellPicker*>(this->GetRenderWindow()->GetInteractor()->GetPicker());
    picker->Pick((double)helpEvent->x(), (double)(this->height()-helpEvent->y()-1), 0.0, this->mp_Renderer);
    if (picker->GetCellId() >= 0 )
    {
      vtkActor* pickedActor = picker->GetActor();
      if (pickedActor)
      {
        vtkPolyData* pd = vtkPolyData::SafeDownCast(pickedActor->GetMapper()->GetInput());
        if (pd)
        { 
          vtkCell* pickedCell = pd->GetCell(picker->GetCellId());
          int pnt = pickedCell->GetPointId(0);
          vtkIdTypeArray* inputPointIds = vtkIdTypeArray::SafeDownCast(pd->GetPointData()->GetArray("MarkersIds"));
          if (inputPointIds)
          { 
            int id = static_cast<int>(inputPointIds->GetTuple1(pnt));
            QToolTip::showText(helpEvent->globalPos(), this->mp_Acquisition->pointLabel(id));
            return true;
          } 
        }
      }
    }
    QToolTip::hideText();
    event->ignore();
    return true;
  }
  return this->VizRendererWidget::event(event);
};

void Viz3DWidget::keyPressEvent(QKeyEvent* event)
{
  event->accept(); // Keyboard events are not sent to VTK
  //this->QVTKWidget::keyPressEvent(event);
};

void Viz3DWidget::keyReleaseEvent(QKeyEvent* event)
{
  event->accept(); // Keyboard events are not sent to VTK
  //this->QVTKWidget::keyReleaseEvent(event);
};

void Viz3DWidget::mousePressEvent(QMouseEvent* event)
{
  this->GetRenderWindow()->GetInteractor()->SetAltKey(event->modifiers() == Qt::AltModifier ? 1 : 0);
  this->VizRendererWidget::mousePressEvent(event);
};

void Viz3DWidget::resizeEvent(QResizeEvent* event)
{
  this->VizRendererWidget::resizeEvent(event);
  QSize size = event->size();
  const double l = 100.0;
  double width = l / static_cast<double>(size.width());
  double height = l / static_cast<double>(size.height());
  this->mp_AxesWidget->SetViewport(0, 0, width, height);
};