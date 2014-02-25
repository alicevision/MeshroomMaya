#pragma once

#include "ui_menu.h"
#include <QWidget>

namespace mayaMVG {

class MVGMenu: public QWidget
{
    Q_OBJECT

public:
   MVGMenu(QWidget * parent = 0);
   ~MVGMenu();

public:
  void addCamera(const QString& text);
  void clear();
  void selectCameras(const QList<QString>& cameraNames);

  void populateMVGMenu();

private slots:
  void on_cameraList_itemSelectionChanged();

  void clearSelectedView(const QString& view);

  void on_cameraImportButton_clicked();
  void on_pointCloudImportButton_clicked();
  void on_densePointCloudImportButton_clicked();
  void on_projectBrowseButton_clicked();
  void on_activeContextButton_clicked();

private:
  void importPointCloud( const std::string& sPathToPly,
                         const std::string& sParticleName );
  Ui::MVGMenu ui;
  bool _updateEnabled;
};

} // mayaMVG