#include <QApplication>
#include <QTextCodec>
#include "tvwindow.h"
#ifdef Q_WS_X11
 #include <X11/Xlib.h>
#endif

int main(int argc, char *argv[])
{
    #ifdef Q_WS_X11
     XInitThreads();
    #endif
    QCoreApplication::setApplicationName("QtTvTime");
    QApplication app(argc, argv);
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF8")); // Для строковых констант
    //QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF8")); // Для функций перевода tr()
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF8"));
    TVWindow tvwindow;   
    tvwindow.show();
    return app.exec();
}
