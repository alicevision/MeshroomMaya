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
    void clear();
    void addItem(const QString&);
    void selectItems(const QList<QString>& cameraNames);

  private slots:
    void on_cameraList_itemSelectionChanged();
    void on_browseButton_clicked();
    void on_selectContextButton_clicked();
    void on_placeContextButton_clicked();
    void on_moveContextButton_clicked();

  private:
    Ui::MVGMenu ui;
};

} // mayaMVG
