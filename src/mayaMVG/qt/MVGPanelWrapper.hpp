#pragma once

#include <QObject>
#include <maya/MCallbackIdArray.h>
#include <maya/MEventMessage.h>
#include <QColor>

namespace mayaMVG
{

class MVGPanelWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(QString label READ getLabel NOTIFY labelChanged)
    Q_PROPERTY(bool isVisible READ getIsVisible)
    Q_PROPERTY(bool isPointCloudDisplayed READ isPointCloudDisplayed WRITE displayPointCloud NOTIFY
                   isPointCloudDisplayedChanged)
    Q_PROPERTY(QColor color READ getColor WRITE setColor NOTIFY colorChanged)
    
public:
    MVGPanelWrapper(const QString& name);
    MVGPanelWrapper(const QString& name, const QString& label, const QColor& color);
    ~MVGPanelWrapper();

public:
    QColor getColor() const;
    void setColor(const QColor& color);
    void emitIsPointCloudDisplayedChanged();
    /// Update panel's color according to the locator's attribute it relies on
    void onColorAttributeChanged();

public Q_SLOTS:
    inline QString getName() const { return _name; }
    inline QString getLabel() const { return _label; }
    void setLabel(QString label);
    inline const bool getIsVisible() const { return _isVisible; }
    bool isPointCloudDisplayed() const;
    void displayPointCloud(const bool display);

Q_SIGNALS:
    void isPointCloudDisplayedChanged();
    void labelChanged();
    void colorChanged();
    
private:
    /**
     * Update container's stylesheet to add a border which color matches the result
     * of getColor().
     */
    void updateStylesheet();
    
private:
    QString _name;
    QString _label;
    QColor _defaultColor;
    bool _isVisible;
};

} // namespace
