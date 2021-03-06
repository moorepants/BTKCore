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

#ifndef ChartOptionsWidget_h
#define ChartOptionsWidget_h

#include "ui_ChartOptionsWidget.h"

#include <QWidget>

class ChartWidget;

class ChartOptionsWidget : public QWidget, public Ui::ChartOptionsWidget
{
  Q_OBJECT
  
public:
  enum {LineColor = Qt::UserRole + 1, LineWidth, ItemId, ItemEnabled};
  
  ChartOptionsWidget(QWidget* parent = 0);
  
  void setPlot(int rowIdx, const QString& label, const QColor& color, double width, bool visible, bool discarded, bool disabled);
  void clear();
  void setPlotOptionEnabled(bool enabled);
  QList<int> selectedPlots() const;
  void setSelectedPlots(const QList<int>& indices);
  
public slots:
  void displayPlotOption();
  void removePlot();
  void setLineColor();
  void setLineWidth(double value);
  void togglePlotVisibility();
  
signals:
  void lineColorChanged(const QList<int>& indices, const QColor& color);
  void lineWidthChanged(const QList<int>& indices, double value);
  void plotRemoved(int id);
  void chartTitleChanged(const QString& title);
  void pausePlaybackRequested(bool paused);
  void plotHidden(int id, bool isHidden);
  
protected:
  virtual bool event(QEvent* event);
  virtual bool eventFilter(QObject* object, QEvent* event);
  virtual void paintEvent(QPaintEvent* event);
  
private slots:
  void emitChartTitleChanged();
  
private:
  friend class ChartWidget;

  QPixmap createLineIcon(const QColor& color, double width);
  void setLineColorButtonColor(const QColor& color);
  
  int shiftArrow;
#ifdef Q_OS_WIN
  bool m_FixUpdateWindowsXP;
#endif
};

#endif // ChartOptionsWidget_h