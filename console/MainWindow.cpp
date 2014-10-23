#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    server(NULL),
    clearTimer(NULL)
{
    ui->setupUi(this);

    // Get user folder and create app folder
    this->userFolder = QDir::homePath() + "/.maurina/";
    QDir dir;
    if (!dir.exists(this->userFolder))
        dir.mkdir(this->userFolder);

    // Set default values
    this->serverIp = QHostAddress::LocalHost;
    this->serverPort = 1947; // Maurina's year of birth
    this->timeoutEnabled = true;
    this->timeoutValue = 2;
    this->logCount.resize(4);
    this->tabCaptions << "Log 1" << "Log 2" << "Log 3" << "Log4";

    // Load config
    this->loadConfig();

    // Set up UI
    ui->uiServerIp->setText(this->serverIp.toString());
    ui->uiServerPort->setValue(this->serverPort);
    ui->uiTimeoutEnabled->setChecked(this->timeoutEnabled);
    ui->uiTimeoutValue->setValue(this->timeoutValue);

    QString title = "Maurina v.";
    title += VERSION;
    this->setWindowTitle(title);

    ui->actionE_xit->setShortcut(QKeySequence(QKeySequence::Quit));
    ui->action_About->setShortcut(QKeySequence(QKeySequence::HelpContents));

    // UI connections
    connect(ui->uiRestart,          SIGNAL(clicked()),
            this,                   SLOT(slStartServer()));
    connect(ui->uiTimeoutEnabled,   SIGNAL(stateChanged(int)),
            this,                   SLOT(slTimeoutChanged(int)));
    connect(ui->uiTimeoutValue,     SIGNAL(valueChanged(int)),
            this,                   SLOT(slTimeoutChanged(int)));
    connect(ui->uiClearNow,         SIGNAL(clicked()),
            this,                   SLOT(slClearLogs()));
    connect(ui->actionE_xit,        SIGNAL(triggered()),
            this,                   SLOT(close()));
    connect(ui->action_About,       SIGNAL(triggered()),
            this,                   SLOT(slShowAbout()));

    // Start server
    this->slStartServer();
}

MainWindow::~MainWindow()
{
    this->saveConfig();
    delete ui;
}

void MainWindow::slStartServer()
{
    // Delete any old instances
    if (this->server != NULL)
        delete this->server;
    if (this->clearTimer != NULL)
    {
        this->clearTimer->stop();
        delete this->clearTimer;
    }

    // Set up server
    this->server = new QUdpSocket(this);
    this->serverIp = ui->uiServerIp->text();
    this->serverPort = ui->uiServerPort->value();
    if (!this->server->bind(this->serverIp, this->serverPort))
    {
        ui->uiStatusIcon->setPixmap(QPixmap(":/maurina/Resources/ledRed.png"));
        ui->uiStatusText->setText(tr("Server bind failed. "
                                      "Are you running any other instance "
                                      "of Maurina?"));
        return;
    }

    // Set up clear timer
    this->clearTimer = new QTimer(this);

    // Set up connections
    connect(this->server,       SIGNAL(readyRead()),
            this,               SLOT(slPendingDatagrams()));
    connect(this->clearTimer,   SIGNAL(timeout()),
            this,               SLOT(slClearTimeout()));

    // Update UI
    ui->uiStatusIcon->setPixmap(QPixmap(":/maurina/Resources/ledGreen.png"));
    ui->uiStatusText->setText(tr("Listening at %1:%2")
                              .arg(this->serverIp.toString())
                              .arg(this->serverPort));
    this->slClearLogs();
}

void MainWindow::slPendingDatagrams()
{
    while (this->server->hasPendingDatagrams())
    {
        // Clear logs if needed
        if (this->resetLogs)
        {
            this->resetLogs = false;
            this->slClearLogs();
        }
        // Read datagram
        QByteArray datagram;
        datagram.resize(this->server->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        this->server->readDatagram(datagram.data(), datagram.size(),
                                   &sender, &senderPort);

        // Parse data
        QVariantMap data =
                        QJsonDocument::fromJson(datagram).toVariant().toMap();

        this->tabCaptions = data["tabs"].toStringList();

        // Update tabs using datagram info
        short tabCount = this->tabCaptions.length();
        for (int x = 0; x < tabCount; ++x)
            ui->tabWidget->setTabText(x, this->tabCaptions.at(x));

        // Update log windows
        this->addDataToLog(0, data["log1"].toString());
        this->addDataToLog(1, data["log2"].toString());
        this->addDataToLog(2, data["log3"].toString());
        this->addDataToLog(3, data["log4"].toString());

        // Launch clear timer
        this->clearTimer->start(this->timeoutValue * 1000);
    }
}

void MainWindow::slTimeoutChanged(int dummy)
{
    /* We use this slot for the check box and spin box change events
     * so the parameter received is not used, we'll get the data directly
     * from the UI elements. */
    dummy *= 0; // This is just to remove the "unused parameter" compile warning
    this->timeoutEnabled = (ui->uiTimeoutEnabled->checkState() == Qt::Checked);
    this->timeoutValue = ui->uiTimeoutValue->value();
}

void MainWindow::slClearTimeout()
{
    this->resetLogs = this->timeoutEnabled;
}

void MainWindow::slClearLogs()
{
    ui->uiLog1->clear();
    ui->uiLog2->clear();
    ui->uiLog3->clear();
    ui->uiLog4->clear();

    this->logCount.fill(0);
    for (short x = 0; x < 4; ++x)
        ui->tabWidget->setTabText(x, this->tabCaptions.at(x));
}

void MainWindow::slShowAbout()
{
    aboutWindow about;
    about.exec();
}

void MainWindow::addDataToLog(int index, QString data)
{
    if (data.isEmpty())
        return;

    // Append data to UI
    switch (index)
    {
        case 0: ui->uiLog1->append(data); break;
        case 1: ui->uiLog2->append(data); break;
        case 2: ui->uiLog3->append(data); break;
        case 3: ui->uiLog4->append(data); break;
    }

    // Update message count and tab caption
    ++this->logCount[index];

    for (short x = 0; x < 4; ++x)
    {
        QString caption(this->tabCaptions.at(x));
        if (this->logCount.at(x) > 0)
            caption += " (" + QString::number(this->logCount.at(x)) + ")";
        ui->tabWidget->setTabText(x, caption);
    }

}

void MainWindow::loadConfig()
{
    // Default geometry values
    QRect windowGeometry(50, 50, 700, 400);

    // Load and parse config file if exists
    bool configExists = QFile(this->userFolder + CONFIG_FILE).exists();
    if (configExists)
    {
        QFile file(this->userFolder + CONFIG_FILE);
        if (file.open(QFile::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&file);
            QString data("");
            while (!in.atEnd())
            {
                QString line(in.readLine().trimmed());

                if (line.startsWith("#") || line.isEmpty())
                    continue;

                QStringList parts = line.split("=");
                if (parts.count() != 2)
                    continue;
                QString key = parts.at(0).trimmed().toLower();
                QString value = parts.at(1).trimmed();

                if (key == "serverip")
                    this->serverIp.setAddress(value);
                if (key == "serverport")
                    this->serverPort = value.toInt();
                if (key == "timeoutvalue")
                    this->timeoutValue = value.toInt();
                if (key == "timeoutenabled")
                    this->timeoutEnabled = (value == "1") ? true : false;
                if (key == "windowx")
                    windowGeometry.setX(value.toInt());
                if (key == "windowy")
                    windowGeometry.setY(value.toInt());
                if (key == "windoww")
                    windowGeometry.setWidth(value.toInt());
                if (key == "windowh")
                    windowGeometry.setHeight(value.toInt());
                if (key == "tab1caption")
                    this->tabCaptions[0] = value;
                if (key == "tab2caption")
                    this->tabCaptions[1] = value;
                if (key == "tab3caption")
                    this->tabCaptions[2] = value;
                if (key == "tab4caption")
                    this->tabCaptions[3] = value;
            }
            file.close();
        }
    }

    // Apply geometry values
    this->setGeometry(windowGeometry);
}

void MainWindow::saveConfig()
{
    QFile configFile(this->userFolder + CONFIG_FILE);
    if (configFile.open(QIODevice::WriteOnly))
    {
        QString data("# Maurina config file.\n"
                     "# Default values:\n# serverIp = 127.0.0.1\n"
                     "# serverPort = 1947\n# timeoutEnabled = 1\n"
                     "# timeoutValue = 2\n# windowX = 50\n"
                     "# windowY = 50\n# windowW = 700\n# windowH = 400\n"
                     "# tab1caption = Log 1\n# tab2caption = Log 2\n"
                     "# tab3caption = Log 3\n# tab4caption = Log 4\n\n");

        data += "serverIp = " + this->serverIp.toString() + "\n";
        data += "serverPort = " + QString::number(this->serverPort)+"\n";
        data += "timeoutEnabled = ";
        data += (this->timeoutEnabled) ? "1" : "0";
        data += "\ntimeoutValue = " + QString::number(this->timeoutValue)+"\n";

        QRect wGeometry = this->geometry();
        data += "windowX = " + QString::number(wGeometry.x())+"\n";
        data += "windowY = " + QString::number(wGeometry.y())+"\n";
        data += "windowW = " + QString::number(wGeometry.width())+"\n";
        data += "windowH = " + QString::number(wGeometry.height())+"\n";

        for (short x = 0; x < 4; ++x)
        {
            data += "tab" + QString::number(x + 1) + "caption = " +
                    this->tabCaptions.at(x)+"\n";
        }

        QTextStream out(&configFile);
        out << data;
        configFile.close();
    }
}
