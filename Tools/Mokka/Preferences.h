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

#ifndef Preferences_h
#define Preferences_h

#if !defined(Q_OS_MAC)

  #include "ui_Preferences.h"
  #include "ChartCycleSettingsManager.h"

  #include <QDialog>
  #include <QMap>

  class Preferences : public QDialog, public Ui::Preferences
  {
    Q_OBJECT

  public:
    enum {DefaultConfigurationUse = 0, DefaultConfigurationPath, EventEditorWhenInserting, DefaultGroundOrientation, DefaultTimeBarEventDisplay,
          DefaultBackgroundColor, DefaultGridFrontColor, DefaultGridBackColor, DefaultSegmentColor, DefaultMarkerColor, DefaultMarkerRadius, DefaultTrajectoryLength, ForcePlatformAxesDisplay,
          ForcePlatformIndexDisplay, DefaultForcePlateColor, DefaultForceVectorColor, UserLayoutIndex, UserLayouts, AutomaticCheckUpdateUse, DevelopmentChannelSubscriptionUsed,
          DefaultGRFButterflyActivation, ForcePathDisplay, DefaultPlotLineWidth, ChartEventDisplay, chartUnitAxisX};
  
    Preferences(QWidget* parent = 0);
    // ~Preferences(); // Implicit
  
    void setPreference(int preference, QVariant data) {this->m_Data[preference] = data;};
  
    void saveSettings();
    void resetSettings();
    
    void showGeneralPreferences() {this->tabWidget->setCurrentWidget(this->generalTab);};
    void showVisualisationPreferences() {this->tabWidget->setCurrentWidget(this->visualisationTab);};
    void showChartPreferences() {this->tabWidget->setCurrentWidget(this->chartTab);};
    void showLayoutsPreferences() {this->tabWidget->setCurrentWidget(this->layoutsTab);};
    void showAdvancedPreferences() {this->tabWidget->setCurrentWidget(this->advancedTab);};
    
    void setChartCycleSettingsManager(ChartCycleSettingsManager* manager);
    void setUserLayouts(QList<QVariant>* layouts) {this->layoutTable->setUserLayouts(layouts);};
    void refreshUserLayouts() {this->layoutTable->refresh();};
  
    QString lastDirectory;
  
  public slots:
    void setDefaultConfiguration();
    void setDefaultBackgroundColor();
    void setDefaultGridFrontColor();
    void setDefaultGridBackColor();
    void setDefaultSegmentColor();
    void setDefaultMarkerColor();
    void setDefaultForcePlateColor();
    void setDefaultForceVectorColor();
    void enableChartCycleButtons(int index);
    void addChartCycleSetting();
    void removeChartCycleSetting();
    void editChartCycleSetting();
  
  signals:
    void useDefaultConfigurationStateChanged(bool isUsed);
    void defaultConfigurationPathChanged(const QString& path);
    void defaultGroundOrientationChanged(int index);
    void defaultTimeBarEventDisplayChanged(int index);
    void useEventEditorWhenInsertingStateChanged(bool isChecked);
    void defaultBackgroundColorChanged(const QColor& color);
    void defaultGridFrontColorChanged(const QColor& color);
    void defaultGridBackColorChanged(const QColor& color);
    void defaultSegmentColorChanged(const QColor& color);
    void defaultMarkerColorChanged(const QColor& color);
    void defaultMarkerRadiusChanged(double radius);
    void defaultMarkerTrajectoryLengthChanged(int index);
    void showForcePlatformAxesChanged(int index);
    void showForcePlatformIndexChanged(int index);
    void defaultForcePlateColorChanged(const QColor& color);
    void defaultForceVectorColorChanged(const QColor& color);
    void automaticCheckUpdateStateChanged(bool isChecked);
    void subscribeDevelopmentChannelStateChanged(bool isChecked);
    void userLayoutsChanged(const QList<QVariant>& layouts, int index);
    void defaultGRFButterflyActivationChanged(int index);
    void showForcePathChanged(int index);
    void defaultPlotLineWidthChanged(double width);
    void showChartEventChanged(int index);
    void chartHorizontalAxisUnitChanged(int index);
    
  private slots:
    void removeUserLayout(int index);
    void relabelUserLayout(int index, const QString& label);
    void updateDroppedUserLayouts(int newRow, int oldRow);
    void forceChartUnitAxisX(int index);
  
  private:
    void conditionalTemporaryCycleSettingsDataInit();
    void resetTemporaryCycleSettingsData();
    
    QMap<int, QVariant> m_Data;
    ChartCycleSettingsManager* mp_ChartCycleSettingsManager;
    QList<ChartCycleSetting> m_TemporaryCycleSettings;
    int m_TemporaryCurrentCycleSetting;
  };

  inline void colorizeButton(QPushButton* button, const QColor& color)
  {
    button->setStyleSheet("QPushButton {border: 1px solid darkgray;background: " + color.name() + ";} QPushButton:pressed {border: 1px solid rgb(100,100,100);}");
    button->setProperty("backgroundColor", color);
  };
#else
  #include "Preferences_mac.h"
#endif

#endif // Preferences_h