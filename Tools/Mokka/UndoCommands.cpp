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
 
#include "UndoCommands.h"

// ----------------------------------------------- //
//                     GENERAL                     //
// ----------------------------------------------- //

// --------------- EditRegionOfInterest ---------------
EditRegionOfInterest::EditRegionOfInterest(Acquisition* acq, int lb, int rb, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(), m_Events()
{
  this->mp_Acquisition = acq;
  this->mp_ROI[0] = lb;
  this->mp_ROI[1] = rb;
  
  for (QMap<int,Event*>::const_iterator it = this->mp_Acquisition->events().begin() ; it != this->mp_Acquisition->events().end() ; ++it)
  {
    if ((it.value()->frame < this->mp_ROI[0]) || (it.value()->frame > this->mp_ROI[1]))
      this->m_Ids << it.key();
  }
};

EditRegionOfInterest::~EditRegionOfInterest()
{
  for (int i = 0 ; i < this->m_Events.count() ; ++i)
    delete this->m_Events[i];
};

void EditRegionOfInterest::action()
{
  int temp[2];
  this->mp_Acquisition->regionOfInterest(temp[0], temp[1]);
  this->mp_Acquisition->setRegionOfInterest(this->mp_ROI[0], this->mp_ROI[1]);
  this->mp_ROI[0] = temp[0];
  this->mp_ROI[1] = temp[1];
};

void EditRegionOfInterest::undo()
{
  this->action();
  this->mp_Acquisition->insertEvents(this->m_Ids, this->m_Events);
  this->m_Events.clear();
}

void EditRegionOfInterest::redo()
{
  this->action();
  this->m_Events = this->mp_Acquisition->takeEvents(this->m_Ids);
}

// --------------- ReframeAcquisition ---------------
ReframeAcquisition::ReframeAcquisition(Acquisition* acq, int ff, QUndoCommand* parent)
: AcquisitionUndoCommand(parent)
{
  this->mp_Acquisition = acq;
  this->m_FirstFrame = ff;
};

void ReframeAcquisition::action()
{
  int temp[2]; this->mp_Acquisition->regionOfInterest(temp[0], temp[1]);
  this->mp_Acquisition->setFirstFrame(this->m_FirstFrame);
  this->m_FirstFrame = temp[0];
};

// ----------------------------------------------- //
//               POINT/MARKER EDITION              //
// ----------------------------------------------- //

//  --------------- CreateAveragedMarker  ---------------
CreateAveragedMarker::CreateAveragedMarker(Acquisition* acq, const QList<int>& markers, QUndoCommand* parent)
: AcquisitionUndoCommand(parent)
{
  this->m_Id = -1;
  this->mp_Acquisition = acq;
  int btkidx = this->mp_Acquisition->createAveragedMarker(markers);
  for (QMap<int,Point*>::const_iterator it = acq->points().begin() ; it != acq->points().end() ; ++it)
  {
    if ((*it)->btkidx == btkidx)
    {
      this->m_Id = it.key();
      break;
    }
  }
  if (this->m_Id == -1)
    qDebug("Impossible to retrieve the created markers: ID not found.");
  
  this->mp_Marker = 0;
};

CreateAveragedMarker::~CreateAveragedMarker()
{
  if (this->mp_Marker != 0)
    delete this->mp_Marker;
};

void CreateAveragedMarker::undo()
{
  if (this->m_Id != -1)
    this->mp_Marker = this->mp_Acquisition->takePoints(QList<int>() << this->m_Id).first();
};

void CreateAveragedMarker::redo()
{
  // Second condition is to avoid the inserting of the marker a second time during the initial redo
  if ((this->m_Id != -1) && (this->mp_Marker != 0))
  {
    this->mp_Acquisition->insertPoints(QList<int>() << this->m_Id , QList<Point*>() << this->mp_Marker);
    this->mp_Marker = 0;
  }
};

// --------------- EditPointLabel ---------------
EditPointLabel::EditPointLabel(Acquisition* acq, int id, const QString& label, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Label(label)
{
  this->mp_Acquisition = acq;
  this->m_Id = id;
};

void EditPointLabel::action()
{
  QString temp = this->mp_Acquisition->pointLabel(this->m_Id);
  this->mp_Acquisition->setPointLabel(this->m_Id, this->m_Label);
  this->m_Label = temp;
};

// --------------- EditPointDescription ---------------
EditPointDescription::EditPointDescription(Acquisition* acq, const QVector<int>& ids, const QString& desc, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(ids), m_Descriptions(ids.count())
{
  this->mp_Acquisition = acq;
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    this->m_Descriptions[i] = desc;
};

void EditPointDescription::action()
{
  QVector<QString> temp(this->m_Ids.count());
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp[i] = this->mp_Acquisition->pointDescription(this->m_Ids[i]);
  this->mp_Acquisition->setPointsDescription(this->m_Ids, this->m_Descriptions);
  this->m_Descriptions = temp;
};

// --------------- EditMarkersRadius ---------------
EditMarkersRadius::EditMarkersRadius(Acquisition* acq, const QVector<int>& ids, double radius, QUndoCommand* parent)
: ConfigurationUndoCommand(parent), m_Ids(ids), m_Radii(ids.count())
{
  this->mp_Acquisition = acq;
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    this->m_Radii[i] = radius;
};

void EditMarkersRadius::action()
{
  QVector<double> temp(this->m_Ids.count());
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp[i] = this->mp_Acquisition->markerRadius(this->m_Ids[i]);
  this->mp_Acquisition->setMarkersRadius(this->m_Ids, this->m_Radii);
  this->m_Radii = temp;
};

// --------------- EditMarkersColor ---------------
EditMarkersColor::EditMarkersColor(Acquisition* acq, const QVector<int>& ids, const QColor& color, QUndoCommand* parent)
: ConfigurationUndoCommand(parent), m_Ids(ids), m_Colors(ids.count())
{
  this->mp_Acquisition = acq;
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    this->m_Colors[i] = color;
};

void EditMarkersColor::action()
{
  QVector<QColor> temp(this->m_Ids.count());
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp[i] = this->mp_Acquisition->markerColor(this->m_Ids[i]);
  this->mp_Acquisition->setMarkersColor(this->m_Ids, this->m_Colors);
  this->m_Colors = temp;
};

// --------------- EditMarkersVisibility ---------------
EditMarkersVisibility::EditMarkersVisibility(Acquisition* acq, const QVector<int>& ids, bool visible, QUndoCommand* parent)
: ConfigurationUndoCommand(parent), m_Ids(ids), m_Visibles(ids.count())
{
  this->mp_Acquisition = acq;
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    this->m_Visibles[i] = visible;
};

void EditMarkersVisibility::action()
{
  QVector<bool> temp(this->m_Ids.count());
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp[i] = this->mp_Acquisition->markerVisible(this->m_Ids[i]);
  this->mp_Acquisition->setMarkersVisible(this->m_Ids, this->m_Visibles);
  this->m_Visibles = temp;
};

// --------------- EditMarkersTrajectoryVisibility ---------------
EditMarkersTrajectoryVisibility::EditMarkersTrajectoryVisibility(Acquisition* acq, const QVector<int>& ids, bool visible, QUndoCommand* parent)
: ConfigurationUndoCommand(parent), m_Ids(ids), m_Visibles(ids.count())
{
  this->mp_Acquisition = acq;
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    this->m_Visibles[i] = visible;
};

void EditMarkersTrajectoryVisibility::action()
{
  QVector<bool> temp(this->m_Ids.count());
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp[i] = this->mp_Acquisition->markerTrajectoryVisible(this->m_Ids[i]);
  this->mp_Acquisition->setMarkersTrajectoryVisible(this->m_Ids, this->m_Visibles);
  this->m_Visibles = temp;
};

// --------------- RemovePoints ---------------
RemovePoints::RemovePoints(Acquisition* acq, const QList<int>& ids, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(ids), m_Points()
{
  this->mp_Acquisition = acq;
};

RemovePoints::~RemovePoints()
{
  for (int i = 0 ; i < this->m_Points.count() ; ++i)
    delete this->m_Points[i];
};

void RemovePoints::undo()
{
  this->mp_Acquisition->insertPoints(this->m_Ids, this->m_Points);
  this->m_Points.clear();
};

void RemovePoints::redo()
{
  this->m_Points = this->mp_Acquisition->takePoints(this->m_Ids);
};

// ----------------------------------------------- //
//                  ANALOG EDITION                 //
// ----------------------------------------------- //

// --------------- EditAnalogLabel ---------------
EditAnalogLabel::EditAnalogLabel(Acquisition* acq, int id, const QString& label, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Label(label)
{
  this->mp_Acquisition = acq;
  this->m_Id = id;
};

void EditAnalogLabel::action()
{
  QString temp = this->mp_Acquisition->analogLabel(this->m_Id);
  this->mp_Acquisition->setAnalogLabel(this->m_Id, this->m_Label);
  this->m_Label = temp;
};

// --------------- EditAnalogDescription ---------------
EditAnalogDescription::EditAnalogDescription(Acquisition* acq, const QVector<int>& ids, const QString& desc, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(ids), m_Descriptions(ids.count())
{
  this->mp_Acquisition = acq;
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    this->m_Descriptions[i] = desc;
};

void EditAnalogDescription::action()
{
  QVector<QString> temp(this->m_Ids.count());
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp[i] = this->mp_Acquisition->analogDescription(this->m_Ids[i]);
  this->mp_Acquisition->setAnalogsDescription(this->m_Ids, this->m_Descriptions);
  this->m_Descriptions = temp;
};

// --------------- EditAnalogsUnit ---------------
EditAnalogsUnit::EditAnalogsUnit(Acquisition* acq, const QVector<int>& ids, const QString& unit, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(ids), m_Units(ids.count())
{
  this->mp_Acquisition = acq;
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    this->m_Units[i] = unit;
};

void EditAnalogsUnit::action()
{
  QVector<QString> temp(this->m_Ids.count());
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp[i] = this->mp_Acquisition->analogUnit(this->m_Ids[i]);
  this->mp_Acquisition->setAnalogsUnit(this->m_Ids, this->m_Units);
  this->m_Units = temp;
};

// --------------- EditAnalogsGain ---------------
EditAnalogsGain::EditAnalogsGain(Acquisition* acq, const QVector<int>& ids, Analog::Gain gain, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(ids), m_Gains(ids.count())
{
  this->mp_Acquisition = acq;
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    this->m_Gains[i] = gain;
};

void EditAnalogsGain::action()
{
  QVector<Analog::Gain> temp(this->m_Ids.count());
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp[i] = this->mp_Acquisition->analogGain(this->m_Ids[i]);
  this->mp_Acquisition->setAnalogsGain(this->m_Ids, this->m_Gains);
  this->m_Gains = temp;
};

// --------------- EditAnalogsOffset ---------------
EditAnalogsOffset::EditAnalogsOffset(Acquisition* acq, const QVector<int>& ids, int offset, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(ids), m_Offsets(ids.count())
{
  this->mp_Acquisition = acq;
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    this->m_Offsets[i] = offset;
};

void EditAnalogsOffset::action()
{
  QVector<int> temp(this->m_Ids.count());
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp[i] = this->mp_Acquisition->analogOffset(this->m_Ids[i]);
  this->mp_Acquisition->setAnalogsOffset(this->m_Ids, this->m_Offsets);
  this->m_Offsets = temp;
};

// --------------- EditAnalogsScale ---------------
EditAnalogsScale::EditAnalogsScale(Acquisition* acq, const QVector<int>& ids, double scale, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(ids), m_Scales(ids.count())
{
  this->mp_Acquisition = acq;
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    this->m_Scales[i] = scale;
};

void EditAnalogsScale::action()
{
  QVector<double> temp(this->m_Ids.count());
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp[i] = this->mp_Acquisition->analogScale(this->m_Ids[i]);
  this->mp_Acquisition->setAnalogsScale(this->m_Ids, this->m_Scales);
  this->m_Scales = temp;
};

// --------------- ShiftAnalogsValues ---------------
ShiftAnalogsValues::ShiftAnalogsValues(Acquisition* acq, const QList<int>& ids, const QList<double>& offsets, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(ids.toVector()), m_Offsets(offsets.toVector())
{
  this->mp_Acquisition = acq;
};

void ShiftAnalogsValues::action()
{
  QVector<double> temp(this->m_Ids.count());
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp[i] = -1.0 * this->m_Offsets[i];
  int numAnalogs = this->mp_Acquisition->analogCount();
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
  {
    QMap<int, Analog*>::const_iterator it = this->mp_Acquisition->analogs().find(this->m_Ids[i]);
    if (it != this->mp_Acquisition->analogs().end())
    {
      if ((*it)->btkidx < numAnalogs)
      {
        btk::AnalogCollection::Iterator itA = this->mp_Acquisition->btkAcquisition()->BeginAnalog();
        std::advance(itA, (*it)->btkidx);
        (*itA)->GetValues().array() += this->m_Offsets[i];
        (*itA)->Modified();
      }
      else
        qDebug("Invalid BTK analog index. Impossible to shift analog's values.");
    }
    else
      qDebug("Invalid analog ID. Impossible to shift analog's values.");
  }  
  this->m_Offsets = temp;
  this->mp_Acquisition->emitAnalogsValuesChanged(this->m_Ids);
};

// --------------- ScaleAnalogsValues ---------------
ScaleAnalogsValues::ScaleAnalogsValues(Acquisition* acq, const QList<int>& ids, const QList<double>& scales, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(ids.toVector()), m_Scales(scales.toVector())
{
  this->mp_Acquisition = acq;
};

void ScaleAnalogsValues::action()
{
  // WARNING: All the scales are assumed to be not null!
  QVector<double> temp(this->m_Ids.count());
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp[i] = 1.0 / this->m_Scales[i];
  int numAnalogs = this->mp_Acquisition->analogCount();
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
  {
    QMap<int, Analog*>::const_iterator it = this->mp_Acquisition->analogs().find(this->m_Ids[i]);
    if (it != this->mp_Acquisition->analogs().end())
    {
      if ((*it)->btkidx < numAnalogs)
      {
        btk::AnalogCollection::Iterator itA = this->mp_Acquisition->btkAcquisition()->BeginAnalog();
        std::advance(itA, (*it)->btkidx);
        (*itA)->GetValues() *= this->m_Scales[i];
        (*itA)->Modified();
      }
      else
        qDebug("Invalid BTK analog index. Impossible to scale analog's values.");
    }
    else
      qDebug("Invalid analog ID. Impossible to scale analog's values.");
  }  
  this->m_Scales = temp;
  this->mp_Acquisition->emitAnalogsValuesChanged(this->m_Ids);
};

// --------------- SetAnalogsValues ---------------
SetAnalogsValues::SetAnalogsValues(Acquisition* acq, const QList<int>& ids, QSharedPointer< QList<btk::Analog::Values> > values, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(ids.toVector()), mp_Values(values)
{
  this->mp_Acquisition = acq;
};

void SetAnalogsValues::action()
{
  int numAnalogs = this->mp_Acquisition->analogCount();
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
  {
    QMap<int, Analog*>::const_iterator it = this->mp_Acquisition->analogs().find(this->m_Ids[i]);
    if (it != this->mp_Acquisition->analogs().end())
    {
      if ((*it)->btkidx < numAnalogs)
      {
        btk::AnalogCollection::Iterator itA = this->mp_Acquisition->btkAcquisition()->BeginAnalog();
        std::advance(itA, (*it)->btkidx);
        qSwap((*itA)->GetValues(), this->mp_Values->operator[](i));
        (*itA)->Modified();
      }
      else
        qDebug("Invalid BTK analog index. Impossible to set analog's values.");
    }
    else
      qDebug("Invalid analog ID. Impossible to set analog's values.");
  }
  this->mp_Acquisition->emitAnalogsValuesChanged(this->m_Ids);
}

// --------------- RemoveAnalogs ---------------
RemoveAnalogs::RemoveAnalogs(Acquisition* acq, const QList<int>& ids, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(ids), m_Analogs()
{
  this->mp_Acquisition = acq;
};

RemoveAnalogs::~RemoveAnalogs()
{
  for (int i = 0 ; i < this->m_Analogs.count() ; ++i)
    delete this->m_Analogs[i];
};

void RemoveAnalogs::undo()
{
  this->mp_Acquisition->insertAnalogs(this->m_Ids, this->m_Analogs);
  this->m_Analogs.clear();
};

void RemoveAnalogs::redo()
{
  this->m_Analogs = this->mp_Acquisition->takeAnalogs(this->m_Ids);
};

// --------------- CreateAnalogs  ---------------
CreateAnalogs::CreateAnalogs(Acquisition* acq, const QList<int>& ids, btk::AnalogCollection::Pointer analogs, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(ids), m_Analogs(), mp_BTKAnalogs(analogs)
{
  this->mp_Acquisition = acq;
  int num = acq->btkAcquisition()->GetAnalogs()->GetItemNumber();
  for (btk::AnalogCollection::Iterator it = this->mp_BTKAnalogs->Begin() ; it != this->mp_BTKAnalogs->End() ; ++it)
  {
    Analog* a = Analog::fromBtkAnalog(*it);
    a->btkidx = num++;
    this->m_Analogs.push_back(a);
  }
};

CreateAnalogs::~CreateAnalogs()
{
  for (int i = 0 ; i < this->m_Analogs.count() ; ++i)
    delete this->m_Analogs[i];
};

void CreateAnalogs::undo()
{
  for (btk::AnalogCollection::Iterator it = this->mp_BTKAnalogs->Begin() ; it != this->mp_BTKAnalogs->End() ; ++it)
  {
    int idx = this->mp_Acquisition->btkAcquisition()->GetAnalogs()->GetIndexOf(*it);
    if (idx != -1)
      this->mp_Acquisition->btkAcquisition()->GetAnalogs()->RemoveItem(idx);
  }
  this->m_Analogs = this->mp_Acquisition->takeAnalogs(this->m_Ids);
};

void CreateAnalogs::redo()
{
  for (btk::AnalogCollection::Iterator it = this->mp_BTKAnalogs->Begin() ; it != this->mp_BTKAnalogs->End() ; ++it)
    this->mp_Acquisition->btkAcquisition()->GetAnalogs()->InsertItem(*it);
  this->mp_Acquisition->insertAnalogs(this->m_Ids, this->m_Analogs);
  this->m_Analogs.clear();
};

// ----------------------------------------------- //
//                   EVENT EDITION                 //
// ----------------------------------------------- //

// --------------- EditEventFrame ---------------
EditEventFrame::EditEventFrame(Acquisition* acq, int id, int frame, QUndoCommand* parent)
: AcquisitionUndoCommand(parent)
{
  this->mp_Acquisition = acq;
  this->m_Id = id;
  this->m_Frame = frame;
};

void EditEventFrame::action()
{
  int temp = this->mp_Acquisition->eventFrame(this->m_Id);
  this->mp_Acquisition->setEventFrame(this->m_Id, this->m_Frame);
  this->m_Frame = temp;
};

// --------------- SetEvents  ---------------
SetEvents::SetEvents(Acquisition* acq, const QList<int>& ids, const QList<Event*>& events, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(ids), m_Events(events)
{
  this->mp_Acquisition = acq;
};

SetEvents::~SetEvents()
{
  for (int i = 0 ; i < this->m_Events.count() ; ++i)
    delete this->m_Events[i];
};

void SetEvents::action()
{
  QList<Event*> temp;
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp << this->mp_Acquisition->events().value(this->m_Ids[i], NULL);
  this->mp_Acquisition->setEvents(this->m_Ids, this->m_Events);
  this->m_Events = temp;
};

// --------------- RemoveEvents  ---------------
RemoveEvents::RemoveEvents(Acquisition* acq, const QList<int>& ids, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(ids), m_Events()
{
  this->mp_Acquisition = acq;
};

RemoveEvents::~RemoveEvents()
{
  for (int i = 0 ; i < this->m_Events.count() ; ++i)
    delete this->m_Events[i];
};

void RemoveEvents::undo()
{
  this->mp_Acquisition->insertEvents(this->m_Ids, this->m_Events);
  this->m_Events.clear();
};

void RemoveEvents::redo()
{
  this->m_Events = this->mp_Acquisition->takeEvents(this->m_Ids);
};

// --------------- InsertEvents  ---------------
InsertEvents::InsertEvents(Acquisition* acq, Event* e, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(), m_Events()
{
  this->mp_Acquisition = acq;
  this->m_Ids.push_back(acq->generateNewEventId());
  this->m_Events.push_back(e);
};

InsertEvents::InsertEvents(Acquisition* acq, const QList<int>& ids, const QList<Event*>& events, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(ids), m_Events(events)
{
  this->mp_Acquisition = acq;
};

InsertEvents::~InsertEvents()
{
  for (int i = 0 ; i < this->m_Events.count() ; ++i)
    delete this->m_Events[i];
};

void InsertEvents::undo()
{
  this->m_Events = this->mp_Acquisition->takeEvents(this->m_Ids);
};

void InsertEvents::redo()
{
  this->mp_Acquisition->insertEvents(this->m_Ids, this->m_Events);
  this->m_Events.clear();
};

// --------------- EditSegmentLabel ---------------
EditSegmentLabel::EditSegmentLabel(Model* m, int id, const QString& label, QUndoCommand* parent)
: ConfigurationUndoCommand(parent), m_Label(label)
{
  this->mp_Model = m;
  this->m_Id = id;
};

void EditSegmentLabel::action()
{
  QString temp = this->mp_Model->segmentLabel(this->m_Id);
  this->mp_Model->setSegmentLabel(this->m_Id, this->m_Label);
  this->m_Label = temp;
};

// --------------- EditSegmentsDescription ---------------
EditSegmentsDescription::EditSegmentsDescription(Model* m, const QVector<int>& ids, const QString& desc, QUndoCommand* parent)
: ConfigurationUndoCommand(parent), m_Ids(ids), m_Descriptions(ids.count())
{
  this->mp_Model = m;
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    this->m_Descriptions[i] = desc;
};

void EditSegmentsDescription::action()
{
  QVector<QString> temp(this->m_Ids.count());
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp[i] = this->mp_Model->segmentDescription(this->m_Ids[i]);
  this->mp_Model->setSegmentsDescription(this->m_Ids, this->m_Descriptions);
  this->m_Descriptions = temp;
};

// --------------- EditSegmentsColor ---------------
EditSegmentsColor::EditSegmentsColor(Model* m, const QVector<int>& ids, const QColor& color, QUndoCommand* parent)
: ConfigurationUndoCommand(parent), m_Ids(ids), m_Colors(ids.count())
{
  this->mp_Model = m;
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    this->m_Colors[i] = color;
};

void EditSegmentsColor::action()
{
  QVector<QColor> temp(this->m_Ids.count());
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp[i] = this->mp_Model->segmentColor(this->m_Ids[i]);
  this->mp_Model->setSegmentsColor(this->m_Ids, this->m_Colors);
  this->m_Colors = temp;
};

// --------------- EditSegmentDefinition ---------------
EditSegmentDefinition::EditSegmentDefinition(Model* m, int id, const QVector<int>& markerIds, const QVector<Pair>& links, const QVector<Triad>& faces, QUndoCommand* parent)
: ConfigurationUndoCommand(parent), m_MarkerIds(markerIds), m_Links(links), m_Faces(faces)
{
  this->mp_Model = m;
  this->m_Id = id;
};

void EditSegmentDefinition::action()
{
  QVector<int> tempMarkerIds = this->mp_Model->segmentMarkerIds(this->m_Id);
  QVector<Pair> tempLinks = this->mp_Model->segmentLinks(this->m_Id);
  QVector<Triad> tempFaces = this->mp_Model->segmentFaces(this->m_Id);
  this->mp_Model->setSegmentDefinition(this->m_Id, this->m_MarkerIds, this->m_Links, this->m_Faces);
  this->m_MarkerIds = tempMarkerIds;
  this->m_Links = tempLinks;
  this->m_Faces = tempFaces;
};

// --------------- EditSegmentsVisibility ---------------
EditSegmentsVisibility::EditSegmentsVisibility(Model* m, const QVector<int>& ids, bool visible, QUndoCommand* parent)
: ConfigurationUndoCommand(parent), m_Ids(ids), m_Visibles(ids.count())
{
  this->mp_Model = m;
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    this->m_Visibles[i] = visible;
};

void EditSegmentsVisibility::action()
{
  QVector<bool> temp(this->m_Ids.count());
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp[i] = this->mp_Model->segmentVisible(this->m_Ids[i]);
  this->mp_Model->setSegmentsVisible(this->m_Ids, this->m_Visibles);
  this->m_Visibles = temp;
};

// --------------- EditSegmentsSurfaceVisibility ---------------
EditSegmentsSurfaceVisibility::EditSegmentsSurfaceVisibility(Model* m, const QVector<int>& ids, bool visible, QUndoCommand* parent)
: ConfigurationUndoCommand(parent), m_Ids(ids), m_Visibles(ids.count())
{
  this->mp_Model = m;
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    this->m_Visibles[i] = visible;
};

void EditSegmentsSurfaceVisibility::action()
{
  QVector<bool> temp(this->m_Ids.count());
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp[i] = this->mp_Model->segmentSurfaceVisible(this->m_Ids[i]);
  this->mp_Model->setSegmentsSurfaceVisible(this->m_Ids, this->m_Visibles);
  this->m_Visibles = temp;
};

// --------------- RemoveSegments ---------------
RemoveSegments::RemoveSegments(Model* m, const QList<int>& ids, QUndoCommand* parent)
: ConfigurationUndoCommand(parent), m_Ids(ids), m_Segments()
{
  this->mp_Model = m;
};

RemoveSegments::~RemoveSegments()
{
  for (int i = 0 ; i < this->m_Segments.count() ; ++i)
    delete this->m_Segments[i];
};

void RemoveSegments::undo()
{
  this->mp_Model->insertSegments(this->m_Ids, this->m_Segments);
  this->m_Segments.clear();
};

void RemoveSegments::redo()
{
  this->m_Segments = this->mp_Model->takeSegments(this->m_Ids);
};

// --------------- InsertSegment  ---------------
InsertSegment::InsertSegment(Model* m, Segment* seg, QUndoCommand* parent)
: ConfigurationUndoCommand(parent), m_Ids(), m_Segments()
{
  this->mp_Model = m;
  this->m_Ids.push_back(m->generateNewSegmentId());
  this->m_Segments.push_back(seg);
};

InsertSegment::~InsertSegment()
{
  for (int i = 0 ; i < this->m_Segments.count() ; ++i)
    delete this->m_Segments[i];
};

void InsertSegment::undo()
{
  this->m_Segments = this->mp_Model->takeSegments(this->m_Ids);
};

void InsertSegment::redo()
{
  this->mp_Model->insertSegments(this->m_Ids, this->m_Segments);
  this->m_Segments.clear();
};

// ----------------------------------------------- //
//                   VIDEO EDITION                 //
// ----------------------------------------------- //

// --------------- EditVideosDelay ---------------
EditVideosDelay::EditVideosDelay(Acquisition* acq, const QVector<int>& ids, qint64 delay, QUndoCommand* parent)
: AcquisitionUndoCommand(parent) , m_Ids(ids), m_Delays(ids.count())
{
  this->mp_Acquisition = acq;
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    this->m_Delays[i] = delay;
};

void EditVideosDelay::action()
{
  QVector<qint64> temp(this->m_Ids.count());
  for (int i = 0 ; i < this->m_Ids.count() ; ++i)
    temp[i] = this->mp_Acquisition->videoDelay(this->m_Ids[i]);
  this->mp_Acquisition->setVideoDelay(this->m_Ids, this->m_Delays);
  this->m_Delays = temp;
};

// --------------- RemoveVideos ---------------
RemoveVideos::RemoveVideos(Acquisition* acq, const QList<int>& ids, QUndoCommand* parent)
: AcquisitionUndoCommand(parent), m_Ids(ids), m_Videos()
{
  this->mp_Acquisition = acq;
};

RemoveVideos::~RemoveVideos()
{
  for (int i = 0 ; i < this->m_Videos.count() ; ++i)
    delete this->m_Videos[i];
};

void RemoveVideos::undo()
{
  this->mp_Acquisition->insertVideos(this->m_Ids, this->m_Videos);
  this->m_Videos.clear();
};

void RemoveVideos::redo()
{
  this->m_Videos = this->mp_Acquisition->takeVideos(this->m_Ids);
};