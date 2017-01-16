/****************************************************************************
**
** Copyright (C) VCreate Logic Private Limited, Bangalore
**
** Use of this file is limited according to the terms specified by
** VCreate Logic Private Limited, Bangalore.  Details of those terms
** are listed in licence.txt included as part of the distribution package
** of this file. This file may not be distributed without including the
** licence.txt file.
**
** Contact info@vcreatelogic.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QCoreApplication>
#include <QProcess>
#include <QByteArray>
#include <QFileInfo>
#include <QtDebug>

class Runner : public QObject
{
    Q_OBJECT

public:
    Runner(const QString &exe, const QString &ts,
           int delay, QObject *parent=0)
        : QObject(parent), m_process(0), m_executable(exe),
          m_testScript(ts), m_startupDelay(delay) { }
    ~Runner() { }

    bool run() {
        if(m_process)
            return false;

        m_executable = QFileInfo(m_executable).absoluteFilePath();
        if(!QFile::exists(m_executable)) {
            qDebug("Executable '%s' doesnt exist", qPrintable(m_executable));
            return false;
        }

        m_testScript = QFileInfo(m_testScript).absoluteFilePath();
        if(!QFile::exists(m_testScript)) {
            qDebug("Test script '%s' doesnt exist", qPrintable(m_testScript));
            return false;
        }

        m_process = new QProcess(this);
        connect(m_process, SIGNAL(readyReadStandardError()),
                this, SLOT(onStandardError()));
        connect(m_process, SIGNAL(readyReadStandardOutput()),
                this, SLOT(onStandardOutput()));
        connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
                this, SLOT(onFinished(int,QProcess::ExitStatus)));

        QStringList args;
        args << "--loadComponents:Investigator/Agent";
        args << "--testScript:" + m_testScript;
        args << "--testStartDelay:" + QString::number(m_startupDelay);

        QStringList appArgs = qApp->arguments();
        int autArgs = appArgs.indexOf("--autArgs");
        while(autArgs-- >= 0 && appArgs.count())
            appArgs.takeFirst();
        args += appArgs;

        m_process->start(m_executable, args);
        if( m_process->waitForStarted() == false) {
            qDebug("Could not start '%s'", qPrintable(m_executable));
            return false;
        }

        return true;
    }

private slots:
    void onStandardError() {
        QByteArray bytes = m_process->readAllStandardError();
        printf("%s", bytes.constData());
    }

    void onStandardOutput() {
        QByteArray bytes = m_process->readAllStandardOutput();
        printf("%s", bytes.constData());
    }

    void onFinished(int code, QProcess::ExitStatus status) {
        if(status == QProcess::CrashExit) {
            qDebug("AUT crashed");
            qApp->exit(1);
        } else
            qApp->exit(code);
    }

private:
    QProcess *m_process;
    QString m_executable;
    QString m_testScript;
    int m_startupDelay;
};

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    QStringList args = a.arguments();
    int argsCount = args.indexOf("--autArgs");
    if(argsCount < 0)
        argsCount = args.count();

    if(argsCount < 3)
    {
        qDebug("%s <AUT> <test-script> [startup-delay] [--autArgs <arg1> [arg2] [arg3] ...]\n", argv[0]);
        return -1;
    }

    Runner runner( args.at(1), args.at(2), argsCount >= 4 ? args.at(3).toInt() : 0 );
    if( !runner.run() )
        return 1;

    return a.exec();
}

#include "Main.moc"
