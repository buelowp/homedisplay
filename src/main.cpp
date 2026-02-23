/*
    This file is part of MythClock.

    MythClock is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MythClock is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MythClock.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <QApplication>
#include "primarydisplay.h"

static QFile debugFile;
static bool enableFileOutput;
static QString dbgFile;
static bool enableDbgOutput;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QDateTime now = QDateTime::currentDateTime();
    QTextStream out(&debugFile);
    QTextStream cerr(stderr);

    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
        case QtDebugMsg:
            if (debugFile.isOpen() && enableFileOutput)
                out << "[" << now.toString("dd.MM.yyyy hh:mm:ss.zzz") << "]" << " Debug: " << localMsg.constData() << " (" << context.function << ":" << context.line << ")" << Qt::endl;
            cerr << "[" << now.toString("dd.MM.yyyy hh:mm:ss.zzz") << "]" << " Debug: " << localMsg.constData() << " (" << context.function << ":" << context.line << ")" << Qt::endl;
        break;
        case QtInfoMsg:
            if (debugFile.isOpen() && enableFileOutput)
                out << "[" << now.toString("dd.MM.yyyy hh:mm:ss.zzz") << "]" << " Info: " << localMsg.constData() << " (" << context.function << ":" << context.line << ")" << Qt::endl;
            cerr << "[" << now.toString("dd.MM.yyyy hh:mm:ss.zzz") << "]" << " Info: " << localMsg.constData() << " (" << context.function << ":" << context.line << ")" << Qt::endl;
        break;
        case QtWarningMsg:
            if (debugFile.isOpen() && enableFileOutput)
                out << "[" << now.toString("dd.MM.yyyy hh:mm:ss.zzz") << "]" << " Warn: " << localMsg.constData() << " (" << context.function << ":" << context.line << ")" << Qt::endl;
            cerr << "[" << now.toString("dd.MM.yyyy hh:mm:ss.zzz") << "]" << " Warn: " << localMsg.constData() << " (" << context.function << ":" << context.line << ")" << Qt::endl;
        break;
        case QtCriticalMsg:
            if (debugFile.isOpen() && enableFileOutput)
                out << "[" << now.toString("dd.MM.yyyy hh:mm:ss.zzz") << "]" << " Critical: " << localMsg.constData() << " (" << context.function << ":" << context.line << ")" << Qt::endl;
            cerr << "[" << now.toString("dd.MM.yyyy hh:mm:ss.zzz") << "]" << " Critical: " << localMsg.constData() << " (" << context.function << ":" << context.line << ")" << Qt::endl;
        break;
        case QtFatalMsg:
            if (debugFile.isOpen() && enableFileOutput)
                out << "[" << now.toString("dd.MM.yyyy hh:mm:ss.zzz") << "]" << " Fatal: " << localMsg.constData() << " (" << context.function << ":" << context.line << ")" << Qt::endl;
            cerr << "[" << now.toString("dd.MM.yyyy hh:mm:ss.zzz") << "]" << " Fatal: " << localMsg.constData() << " (" << context.function << ":" << context.line << ")" << Qt::endl;
        abort();
    }
}

int main(int argc, char **argv)
{
    qInstallMessageHandler(myMessageOutput);

    QApplication app (argc, argv);
    PrimaryDisplay frame;
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "home", "homedisplay");

    enableDbgOutput = true;
    enableFileOutput = false;

    if (settings.contains("enabledebugoutput")) {
        enableDbgOutput = settings.value("enabledebugoutput").toBool();
        if (settings.contains("debugfile")) {
            QString f = settings.value("debugfile").toString();
            debugFile.setFileName(f);
            if (settings.contains("enabledebugoutput")) {
                enableFileOutput = settings.value("enablefileoutput").toBool();
            }
            if (enableFileOutput) {
                if (!debugFile.open(QIODevice::ReadOnly | QIODevice::Append)) {
                    qWarning() << __PRETTY_FUNCTION__ << ": Error opening debug file:" << debugFile.errorString();
                    enableFileOutput = false;
                }
            }
        }
        qDebug() << __PRETTY_FUNCTION__ << ": debug is" << settings.value("debug").toBool();
    }
    if (!settings.value("debug").toBool()) {
        frame.showFullScreen();
        app.setOverrideCursor(QCursor(Qt::BlankCursor));
    }
    else {
        frame.setFixedSize(800, 480);
        frame.show();
    }

    return app.exec();
}
