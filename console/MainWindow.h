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
#include <QShortcut>

#include "aboutWindow.h"
#include "configWindow.h"

#define CONFIG_FILE "config"
#define STYLES_FILE "styles"
#define VERSION "1.1"

namespace Ui
{
class MainWindow;
}

/**
* Layout types
*/
enum LayoutType
{
    DetailedLayout = 0, /**< Only one tab visible	*/
    CompactLayout  = 1,	/**< All tabs visible	*/
    };

class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void slPendingDatagrams();
    void slTimeoutChanged(int state);
    void slClearTimeout();
    void slClearLogs();
    void slShowAbout();
    void slShowPreferences();
    void slToggleControls();
    void slChangeLayout();
    
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
    bool controlsVisible;
    QShortcut *scControls, *scAbout, *scLayout, *scExit, *scPreferences;
    LayoutType layoutType;
    QHash<QString, QString> styles;

    void loadConfig();
    void saveConfig();
    void startServer();
    void addDataToLog(int index, QString data);
    void setTabCaptions();
    void setTabCaption(int index, QString caption);
    void updateControls();
    void updateLayout();
    QString formatData(QString data);
};

#endif // MAINWINDOW_H
