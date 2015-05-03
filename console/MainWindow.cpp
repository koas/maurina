#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    server(NULL), clearTimer(NULL), scControls(NULL), scAbout(NULL),
    scLayout(NULL), scExit(NULL), scPreferences(NULL)
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
    this->logCount.resize(5);
    this->tabCaptions << "Log 1" << "Log 2" << "Log 3" << "Log4" << "Log5";
    this->controlsVisible = true;
    this->layoutType = DetailedLayout;

    // Set default styles
    QString defaultSize("font-size : 12px;");

    QString defaultFont("font-family : monospace;");
    #ifdef Q_OS_WIN32
    defaultFont = "font-family : Consolas;";
    #endif
    #ifdef Q_OS_MAC
    defaultFont = "font-family : Monaco;";
    #endif
    #ifdef Q_OS_LINUX
    defaultFont = "font-family : \"Liberation Mono\";";
    #endif

    this->styles["time"] = defaultFont + " color : #aaaaaa; " + defaultSize;
    this->styles["pre"]  = defaultFont + " color : #000000; " + defaultSize;
    this->styles["var"]  = defaultFont + " color : #000000; " + defaultSize;
    this->styles["h1"]   = defaultFont + " color : #b50000; " + defaultSize;
    this->styles["h2"]   = defaultFont + " color : #009C39; " + defaultSize;
    this->styles["h3"]   = defaultFont + " color : #000000; " + defaultSize;
    this->styles["h4"]   = defaultFont + " color : #000000; " + defaultSize;
    this->styles["h5"]   = defaultFont + " color : #000000; " + defaultSize;
    this->styles["h6"]   = defaultFont + " color : #000000; " + defaultSize;

    // Load config
    this->loadConfig();

    // Set up UI
    ui->uiTimeoutEnabled->setChecked(this->timeoutEnabled);
    ui->uiTimeoutValue->setValue(this->timeoutValue);

    QString title = "Maurina v.";
    title += VERSION;
    this->setWindowTitle(title);

    ui->actionE_xit->setShortcut(QKeySequence::Quit);
    ui->action_About->setShortcut(QKeySequence::HelpContents);
    ui->action_Preferences->setShortcut(QKeySequence(tr("Ctrl+P")));
    ui->action_HideCntrls->setShortcut(QKeySequence(tr("Ctrl+H")));
    ui->actionChangeLayout->setShortcut(QKeySequence(tr("Ctrl+L")));

    ui->frameCompactLayoutTop->hide();
    ui->frameCompactLayoutBottom->hide();
    ui->tabWidgetA->tabBar()->hide();
    ui->tabWidgetB->tabBar()->hide();
    ui->tabWidgetC->tabBar()->hide();
    ui->tabWidgetD->tabBar()->hide();
    ui->tabWidgetE->tabBar()->hide();

    // UI connections
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
    connect(ui->action_Preferences, SIGNAL(triggered()),
            this,                   SLOT(slShowPreferences()));
    connect(ui->action_HideCntrls,  SIGNAL(triggered()),
            this,                   SLOT(slToggleControls()));
    connect(ui->actionChangeLayout, SIGNAL(triggered()),
            this,                   SLOT(slChangeLayout()));

    this->updateControls();
    this->updateLayout();

    // Start server
    this->startServer();
}

MainWindow::~MainWindow()
{
    this->saveConfig();
    delete ui;
}

void MainWindow::startServer()
{
    // Delete any old instances
    if (this->server != NULL)
        delete this->server;
    if (this->clearTimer != NULL)
    {
        this->clearTimer->stop();

        delete this->clearTimer;
        this->clearTimer = NULL;
    }

    // Set up server
    this->server = new QUdpSocket(this);
    if (!this->server->bind(this->serverIp, this->serverPort))
    {
        // If controls are hidden show them so the user sees the message
        if (!this->controlsVisible)
            this->slToggleControls();

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
        this->setTabCaptions();

        // Update log windows
        this->addDataToLog(0, data["log1"].toString());
        this->addDataToLog(1, data["log2"].toString());
        this->addDataToLog(2, data["log3"].toString());
        this->addDataToLog(3, data["log4"].toString());
        this->addDataToLog(4, data["log5"].toString());

        // Launch clear timer
        this->clearTimer->start(this->timeoutValue * 1000);
    }
}

void MainWindow::slTimeoutChanged(int dummy)
{
    Q_UNUSED(dummy);
    
    /* We use this slot for the check box and spin box change events
     * so the parameter received is not used, we'll get the data directly
     * from the UI elements. */
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
    ui->uiLog5->clear();

    this->logCount.fill(0);
    this->setTabCaptions();
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
    data = this->formatData(data);

    switch (index)
    {
        case 0: ui->uiLog1->append(data); break;
        case 1: ui->uiLog2->append(data); break;
        case 2: ui->uiLog3->append(data); break;
        case 3: ui->uiLog4->append(data); break;
        case 4: ui->uiLog5->append(data); break;
    }

    // Update message count and tab captions
    ++this->logCount[index];
    this->setTabCaptions();
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
                if (key == "tab5caption")
                    this->tabCaptions[4] = value;
                if (key == "controlsvisible")
                    this->controlsVisible = (value == "1") ? true : false;
                if (key == "layout")
                    this->layoutType = (LayoutType) value.toInt();
            }
            file.close();
        }
    }

    // Load and parse styles file if exists
    bool stylesExists = QFile(this->userFolder + STYLES_FILE).exists();
    if (stylesExists)
    {
        QFile file(this->userFolder + STYLES_FILE);
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

                this->styles[key] = value;
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
                     "# tab3caption = Log 3\n# tab4caption = Log 4\n"
                     "# tab5caption = Log 5\n# controlsVisible = 1\n"
                     "# layout = 0\n\n");

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

        short numCaptions = this->tabCaptions.count();
        for (short x = 0; x < numCaptions; ++x)
        {
            data += "tab" + QString::number(x + 1) + "caption = " +
                    this->tabCaptions.at(x)+"\n";
        }

        data += "controlsVisible = ";
        data += (this->controlsVisible) ? "1" : "0";
        data += "\nlayout = ";
        data += QString::number(this->layoutType);

        QTextStream out(&configFile);
        out << data;
        configFile.close();

        data += "\n# Message styles. Additional styles may be added in";
        data += "\n# styles file.\n";
    }

    QFile stylesFile(this->userFolder + STYLES_FILE);
    if (stylesFile.open(QIODevice::WriteOnly))
    {
        QString data("# Maurina styles definition. You can add styles for "
                     "any other HTML tag.");

        foreach(QString key, this->styles.keys())
            data += "\n" + key + " = " + this->styles[key];

        QTextStream out(&stylesFile);
        out << data;
        stylesFile.close();
    }
}

void MainWindow::slToggleControls()
{
    this->controlsVisible = !this->controlsVisible;
    this->saveConfig();
    this->updateControls();
}

void MainWindow::updateControls()
{
    ui->frameControls1->setVisible(this->controlsVisible);
    ui->frameControls2->setVisible(this->controlsVisible);
    ui->menuBar->setVisible(this->controlsVisible);

    // If menu bar is hidden the actions are also hidden, and
    // shortcuts won't work. In that case we create new shortcuts so the
    // user can still use them.
    if (!this->controlsVisible)
    {
        this->scControls = new QShortcut(QKeySequence(tr("Ctrl+H")), this);
        this->scControls->setContext(Qt::ApplicationShortcut);
        connect(this->scControls, SIGNAL(activated()),
                this,             SLOT(slToggleControls()));

        this->scAbout = new QShortcut(QKeySequence::HelpContents, this);
        this->scAbout->setContext(Qt::ApplicationShortcut);
        connect(this->scAbout, SIGNAL(activated()),
                this,          SLOT(slShowAbout()));

        this->scExit = new QShortcut(QKeySequence::Quit, this);
        this->scExit->setContext(Qt::ApplicationShortcut);
        connect(this->scExit, SIGNAL(activated()),
                this,         SLOT(close()));

        this->scLayout = new QShortcut(QKeySequence(tr("Ctrl+L")), this);
        this->scLayout->setContext(Qt::ApplicationShortcut);
        connect(this->scLayout, SIGNAL(activated()),
                this,           SLOT(slChangeLayout()));

        this->scPreferences = new QShortcut(QKeySequence(tr("Ctrl+P")), this);
        this->scPreferences->setContext(Qt::ApplicationShortcut);
        connect(this->scPreferences, SIGNAL(activated()),
                this,                SLOT(slShowPreferences()));

        // We also remove all margins for the central widget
        ui->centralWidget->layout()->setContentsMargins(0, 0, 0, 0);
    }
    else // If controls are back, delete the shortcuts we created to avoid a
         // shortcut overload error
    {
        if (this->scControls != NULL)
        {
            delete this->scControls;
            this->scControls = NULL;
        }
        if (this->scAbout!= NULL)
        {
            delete this->scAbout;
            this->scAbout = NULL;
        }
        if (this->scExit!= NULL)
        {
            delete this->scExit;
            this->scExit= NULL;
        }
        if (this->scLayout!= NULL)
        {
            delete this->scLayout;
            this->scLayout= NULL;
        }
        if (this->scPreferences!= NULL)
        {
            delete this->scPreferences;
            this->scPreferences = NULL;
        }

        // And restore the central widget margins
        ui->centralWidget->layout()->setContentsMargins(9, 9, 9, 9);
    }
}

void MainWindow::slChangeLayout()
{
    if (this->layoutType == DetailedLayout)
        this->layoutType = CompactLayout;
    else this->layoutType = DetailedLayout;

    this->saveConfig();
    this->updateLayout();
}

void MainWindow::updateLayout()
{
    if (this->layoutType == CompactLayout)
    {
        // Show / hide elements
        ui->frameCompactLayoutTop->show();
        ui->frameCompactLayoutBottom->show();
        ui->tabWidget->hide();

        // Move tabs to bottom widgets
        ui->tabWidgetA->addTab(ui->tabWidget->widget(0), "");
        ui->tabWidgetB->addTab(ui->tabWidget->widget(0), "");
        ui->tabWidgetC->addTab(ui->tabWidget->widget(0), "");
        ui->tabWidgetD->addTab(ui->tabWidget->widget(0), "");
        ui->tabWidgetE->addTab(ui->tabWidget->widget(0), "");
    }
    else
    {
        // Move tabs back to first tab widget
        ui->tabWidget->addTab(ui->tabWidgetA->widget(0),
                               ui->tabWidgetA->tabText(0));
        ui->tabWidget->addTab(ui->tabWidgetB->widget(0),
                               ui->tabWidgetB->tabText(0));
        ui->tabWidget->addTab(ui->tabWidgetC->widget(0),
                               ui->tabWidgetC->tabText(0));
        ui->tabWidget->addTab(ui->tabWidgetD->widget(0),
                               ui->tabWidgetD->tabText(0));
        ui->tabWidget->addTab(ui->tabWidgetE->widget(0),
                               ui->tabWidgetE->tabText(0));

        // Hide / show elements
        ui->frameCompactLayoutTop->hide();
        ui->frameCompactLayoutBottom->hide();
        ui->tabWidget->show();
    }

    this->setTabCaptions();
}

void MainWindow::setTabCaptions()
{
    QTabBar* bar = ui->tabWidget->tabBar();

    short tabCount = this->tabCaptions.length();
    for (int x = 0; x < tabCount; ++x)
    {
        this->setTabCaption(x, this->tabCaptions.at(x));

        if (this->logCount.at(x) > 0)
            bar->setTabTextColor(x, QColor(0, 0, 0));
        else bar->setTabTextColor(x, QColor(180, 180, 180));
    }
}

void MainWindow::setTabCaption(int index, QString caption)
{
    if (this->logCount.at(index) > 0)
        caption += " (" + QString::number(this->logCount.at(index)) + ")";

    if (this->layoutType == DetailedLayout)
        ui->tabWidget->setTabText(index, caption);
    else
    {
        caption = caption.replace("&", "");
        if (index == 0)
            ui->labelA->setText(caption);
        if (index == 1)
            ui->labelB->setText(caption);
        if (index == 2)
            ui->labelC->setText(caption);
        if (index == 3)
            ui->labelD->setText(caption);
        if (index == 4)
            ui->labelE->setText(caption);
    }
}

QString MainWindow::formatData(QString data)
{
    foreach(QString key, this->styles.keys())
    {
        QString openTag("<" + key + ">");
        QString closeTag("</" + key + ">");
        QString value(this->styles[key].replace('"', "'"));
        QString newOpenTag("<span style=\"" + value + "\">");
        QString newCloseTag("</span>");
        if (key == "pre")
        {
            newOpenTag = "<pre style=\"" + value + "\">";
            newCloseTag = "</pre>";
        }

        data = data.replace(openTag, newOpenTag, Qt::CaseInsensitive);
        data = data.replace(closeTag, newCloseTag, Qt::CaseInsensitive);
    }

    return data;
}

void MainWindow::slShowPreferences()
{
    configWindow config;

    config.setServerAdress(this->serverIp);
    config.setServerPort(this->serverPort);
    config.setStyles(this->styles);

    if (config.exec() == QDialog::Accepted)
    {
        this->serverIp = config.getServerAddress();
        this->serverPort = config.getServerPort();
        this->styles = config.getStyles();
        this->saveConfig();
        this->startServer();
    }
}
