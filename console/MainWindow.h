#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QTimer>
#include <QLabel>
#include <QTranslator>

#include "aboutWindow.h"

#define CONFIG_FILE "config"
#define VERSION "1.0"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void slStartServer();
    void slPendingDatagrams();
    void slTimeoutChanged(int state);
    void slClearTimeout();
    void slClearLogs();
    void slShowAbout();
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;

    QString userFolder;
    QHostAddress serverIp;
    quint16 serverPort;
    bool timeoutEnabled;
    int timeoutValue;
    QUdpSocket *server;
    QTimer *clearTimer;
    bool resetLogs;
    QVector<int> logCount;
    QStringList tabCaptions;

    void loadConfig();
    void saveConfig();
    void addDataToLog(int index, QString data);
};

#endif // MAINWINDOW_H
