#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Get system language and load translations
    if (QLocale().name().split("_").at(0) == "es")
    {
        QTranslator *translator = new QTranslator();
        if (translator->load("maurina_es", ":/maurina/languages"))
            a.installTranslator(translator);
    }

    // Launch main window
    MainWindow w;
    w.show();
    
    return a.exec();
}
