#include "mainwindow.h"
#include "guids.h"
#include "vcl_utils.h"
#include <QStyleFactory>

Q_DECLARE_METATYPE(std::shared_ptr<v8catalog>)
Q_DECLARE_METATYPE(std::unique_ptr<OneC::Metadata::Configuration>)


#include <QApplication>

int main(int argc, char *argv[])
{

    qRegisterMetaType<std::shared_ptr<v8catalog>>();
    qRegisterMetaType<std::shared_ptr<OneC::Metadata::Configuration>>();

    QString Test_String = "Парсер конфигураций 1С (*.cf)";
    QApplication app(argc, argv);

    app.setApplicationName("v8 parser");
    // Стиль Fusion рисует соединительные линии
    //app.setStyle(QStyleFactory::create("Windows"));
    app.setStyle(QStyleFactory::create("Fusion"));   // общий стиль

    MainWindow window;

    window.resize(1100, 700);
    window.setWindowTitle(Test_String);
    window.show();
    return QApplication::exec();
}
