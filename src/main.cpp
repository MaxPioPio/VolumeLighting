#include <iostream>
#include <QtWidgets/QApplication>

#include "mainwindow.hpp"

static const QString logFilePath = "log.txt";

#define LOG_TO_TEXTFILE


void myMessageHandler(QtMsgType /*type*/, const QMessageLogContext& /*context*/, const QString &msg) {
    // write to file
    QFile outFile(logFilePath);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << msg << "\r\n" << flush;
    outFile.close();
}

int main(int argc, char *argv[]) {

#ifdef LOG_TO_TEXTFILE
    // install the message file logger
    QFile outFile(logFilePath);
    outFile.remove();
    qInstallMessageHandler(myMessageHandler);
#endif

    // start the application
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
