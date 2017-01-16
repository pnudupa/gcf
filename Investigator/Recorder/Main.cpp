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

class Recorder : public QObject
{
    Q_OBJECT

public:
    Recorder(const QString &exe, const QString &ts, QObject *parent=0)
        : QObject(parent), m_process(0), m_executable(exe),
          m_testScript(ts) { }
    ~Recorder() { }

    bool run() {
        if(m_process)
            return false;

        m_executable = QFileInfo(m_executable).absoluteFilePath();
        if(!QFile::exists(m_executable)) {
            qDebug("Executable '%s' doesnt exist", qPrintable(m_executable));
            return false;
        }

        if(!m_testScript.isEmpty())
            m_testScript = QFileInfo(m_testScript).absoluteFilePath();
        if(!m_testScript.isEmpty() && QFile::exists(m_testScript))
            qDebug("Test script '%s' exists. It will be overwritten!", qPrintable(m_testScript));

        m_process = new QProcess(this);
        connect(m_process, SIGNAL(readyReadStandardError()),
                this, SLOT(onStandardError()));
        connect(m_process, SIGNAL(readyReadStandardOutput()),
                this, SLOT(onStandardOutput()));
        connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
                this, SLOT(onFinished(int,QProcess::ExitStatus)));

        QStringList args;
        args << "--loadComponents:Investigator/Agent";
        if(m_testScript.isEmpty())
            args << "--record";
        else
            args << "--record:" + m_testScript;

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
};

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    QStringList args = a.arguments();
    int argsCount = args.indexOf("--autArgs");
    if(argsCount < 0)
        argsCount = args.count();

    if(argsCount < 2)
    {
        qDebug("%s <AUT> [output-test-script] [--autArgs <arg1> [arg2] [arg3] ...]\n", argv[0]);
        return -1;
    }

    Recorder recorder( args.at(1), argsCount >= 3 ? args.at(2) : QString() );
    if( !recorder.run() )
        return 1;

    return a.exec();
}

#include "Main.moc"
