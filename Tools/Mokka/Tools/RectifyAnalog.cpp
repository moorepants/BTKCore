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

#include "RectifyAnalog.h"
#include "AnalogToolOptionDialog.h"
#include "UndoCommands.h"

void RectifyAnalog::RegisterTool(ToolsManager* manager)
{
  manager->addAnalogTool(tr("Rectification"), ToolFactory<RectifyAnalog>);
};

RectifyAnalog::RectifyAnalog(QWidget* parent)
: AbstractTool("Rectify Analog", parent)
{};
  
AbstractTool::RunState RectifyAnalog::run(ToolCommands* cmds, ToolsData* const data)
{
  AnalogToolOptionDialog dialog("Rectify Analog", this->parentWidget());
  dialog.initialize(data);
  if (dialog.exec() == QDialog::Accepted)
  {
    btk::AnalogCollection::Pointer analogs = btk::AnalogCollection::New();
    QList<int> ids = dialog.extractSelectedAnalogChannels(analogs, "_Rfied", "Rectified", data, cmds); // Rfied: Rectified
    
    QString log;
    QString log_;
    if (ids.count() == 1)
    {
      log = "The channel ";
      log_ = " was rectified.";
    }
    else
    {
      log = "The channels ";
      log_ = " were rectified.";
    }
    int inc = 0;
    QSharedPointer< QList<btk::Analog::Values> > values(new QList<btk::Analog::Values>());
    for (btk::AnalogCollection::ConstIterator it = analogs->Begin() ; it != analogs->End() ; ++it)
    {
      values->push_back((*it)->GetValues().array().abs());
      log += (inc != 0 ? ", '" : "'") + QString::fromStdString((*it)->GetLabel()) + "'";
      ++inc;
    }
    new SetAnalogsValues(data->acquisition(), ids, values, cmds->acquisitionCommand());
    TOOL_LOG_INFO(log + log_);
    return Success;
  }

  return Cancel;
};