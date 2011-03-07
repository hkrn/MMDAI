/*
 *
 * QMAARToolKit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * QMAARToolKit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QMAARToolKit; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef QMAARTOOLKITPLUGIN_H
#define QMAARTOOLKITPLUGIN_H

#include "QMAPlugin.h"

#include <MMDAI/SceneController.h>

#include <AR/gsub_lite.h>
#include <AR/video.h>
#include <AR/param.h>
#include <AR/ar.h>

class QMAARToolKitPlugin : public QMAPlugin
{
  Q_OBJECT
  Q_INTERFACES(QMAPlugin);

public:
  QMAARToolKitPlugin();
  ~QMAARToolKitPlugin();

public slots:
  void initialize(MMDAI::SceneController *controller);
  void start();
  void stop();
  void receiveCommand(const QString &command, const QStringList &arguments);
  void receiveEvent(const QString &type, const QStringList &arguments);
  void update(const QRect &rect, const QPoint &pos, const double delta);
  void prerender();
  void postrender();

signals:
  void commandPost(const QString &command, const QStringList &arguments);
  void eventPost(const QString &type, const QStringList &arguments);

private:
  MMDAI::SceneController *m_controller;
  ARUint8 *m_image;
  ARParam m_cameraParam;
  ARGL_CONTEXT_SETTINGS_REF m_settings;
  int m_patternID;
  int m_threshold;
  double m_patternCenter[2];
  double m_patternWidth;
  double m_patternTransform[3][4];
  bool m_enabled;
};

#endif // QMAARTOOLKITPLUGIN_H

