#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

#include <QDialog>
#include <QHostAddress>

namespace Ui {
class configWindow;
}

class configWindow : public QDialog
{
    Q_OBJECT

public:
    explicit configWindow(QWidget *parent = 0);
    ~configWindow();

    void setServerAdress(QHostAddress ip);
    void setServerPort(quint16 port);
    void setStyles(QHash<QString, QString> styles);

    QHostAddress getServerAddress();
    quint16 getServerPort();
    QHash<QString, QString> getStyles();

private:
    Ui::configWindow *ui;
};

#endif // CONFIGWINDOW_H
