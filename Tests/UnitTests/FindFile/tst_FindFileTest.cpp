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

#include <QString>
#include <QtTest>
#include <QProcessEnvironment>

#include <GCF3/GCFGlobal>
#include <GCF3/Version>

class FindFileTest : public QObject
{
    Q_OBJECT
    
public:
    FindFileTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testSearchPaths();
    void testSearchPathsForLibs();

    void testFindLibrary_data();
    void testFindLibrary();

    void testFindFile_data();
    void testFindFile();
};

FindFileTest::FindFileTest()
{
}

void FindFileTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void FindFileTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void FindFileTest::testSearchPaths()
{
    QStringList expectedPaths;
    expectedPaths << qApp->applicationDirPath() << QDir::currentPath() << QDir::homePath();

    expectedPaths << QLibraryInfo::location( QLibraryInfo::BinariesPath );
#if QT_VERSION >= 0x050000
    expectedPaths << QLibraryInfo::location( QLibraryInfo::LibraryExecutablesPath );
#endif
    expectedPaths << QLibraryInfo::location( QLibraryInfo::SettingsPath );
    expectedPaths << QLibraryInfo::location( QLibraryInfo::HeadersPath );
    expectedPaths << QLibraryInfo::location( QLibraryInfo::PrefixPath );

#ifdef Q_OS_WIN32
    QString pathEnv = QProcessEnvironment::systemEnvironment().value("PATH");
    expectedPaths += pathEnv.split(";", QString::SkipEmptyParts);
#else
    QString pathEnv = QProcessEnvironment::systemEnvironment().value("PATH");
    expectedPaths += pathEnv.split(":", QString::SkipEmptyParts);
#endif

    expectedPaths.removeDuplicates();

    QStringList searchPaths = GCF::searchPaths();

    QVERIFY(expectedPaths.count() == searchPaths.count());
    for(int i=0; i<expectedPaths.count(); i++)
    {
        QString msg = QString("Mismatch at %1").arg(i);
        QVERIFY2(searchPaths.at(i) == expectedPaths.at(i), msg.toLatin1());
    }
}

void FindFileTest::testSearchPathsForLibs()
{
    QStringList expectedPaths;
    expectedPaths << qApp->applicationDirPath() << QDir::currentPath() << QDir::homePath();
    expectedPaths << qApp->libraryPaths();

#ifdef Q_OS_WIN32
    // Just paths, nothing else
#endif
#ifdef Q_OS_LINUX
    QString libsEnv = QProcessEnvironment::systemEnvironment().value("LD_LIBRARY_PATH");
    expectedPaths += libsEnv.split(":", QString::SkipEmptyParts);
    expectedPaths << "/usr/lib" << "/lib" << "/usr/local/lib";
#endif
#ifdef Q_OS_MAC
    QString libsEnv = QProcessEnvironment::systemEnvironment().value("DYLD_LIBRARY_PATH");
    expectedPaths += libsEnv.split(":", QString::SkipEmptyParts);
    expectedPaths << "/usr/lib" << "/lib" << "/usr/local/lib";
#endif

    expectedPaths << QLibraryInfo::location( QLibraryInfo::LibrariesPath );
    expectedPaths << QLibraryInfo::location( QLibraryInfo::PluginsPath );
    expectedPaths << QLibraryInfo::location( QLibraryInfo::ImportsPath );

    expectedPaths << QLibraryInfo::location( QLibraryInfo::BinariesPath );
#if QT_VERSION >= 0x050000
    expectedPaths << QLibraryInfo::location( QLibraryInfo::LibraryExecutablesPath );
#endif
    expectedPaths << QLibraryInfo::location( QLibraryInfo::SettingsPath );
    expectedPaths << QLibraryInfo::location( QLibraryInfo::HeadersPath );
    expectedPaths << QLibraryInfo::location( QLibraryInfo::PrefixPath );

#ifdef Q_OS_WIN32
    QString pathEnv = QProcessEnvironment::systemEnvironment().value("PATH");
    expectedPaths += pathEnv.split(";", QString::SkipEmptyParts);
#else
    QString pathEnv = QProcessEnvironment::systemEnvironment().value("PATH");
    expectedPaths += pathEnv.split(":", QString::SkipEmptyParts);
#endif

    expectedPaths.removeDuplicates();

    QStringList searchPaths = GCF::searchPathsForLibs();

    QVERIFY(expectedPaths.count() == searchPaths.count());
    for(int i=0; i<expectedPaths.count(); i++)
    {
        QString msg = QString("Mismatch at %1").arg(i);
        QVERIFY2(searchPaths.at(i) == expectedPaths.at(i), msg.toLatin1());
    }
}

#ifdef Q_OS_MAC
/*
 * For testing GCF::findLibrary() we depend on the fact that certain
 * files are available on standard paths in a typical Mac OSX system.
 */
void FindFileTest::testFindLibrary_data()
{
    QTest::addColumn<QString>("library");
    QTest::addColumn<QString>("libraryPath");

    {
        QDir dir = qApp->applicationDirPath();
        dir.cdUp();
        dir.cdUp();
        QString gcf3Path = dir.absoluteFilePath("libGCF3.dylib");
        QTest::newRow("GCF3") << "GCF3" << gcf3Path;
    }

    QTest::newRow("NonExistingLib") << "NonExistingLib" << QString();
    QTest::newRow("c++") << "c++" << "/usr/lib/libc++.dylib";
    QTest::newRow("sqldrivers/qsqlite") << "sqldrivers/qsqlite"
        << QLibraryInfo::location( QLibraryInfo::PluginsPath ) + "/sqldrivers/libqsqlite.dylib";
    QTest::newRow("Qt/labs/folderlistmodel/qmlfolderlistmodelplugin")
            << "Qt/labs/folderlistmodel/qmlfolderlistmodelplugin"
            << QLibraryInfo::location(QLibraryInfo::ImportsPath) + "/Qt/labs/folderlistmodel/libqmlfolderlistmodelplugin.dylib";
}

void FindFileTest::testFindLibrary()
{
    QFETCH(QString, library);
    QFETCH(QString, libraryPath);

    QString gLibPath = GCF::findLibrary(library);
    QCOMPARE(gLibPath, libraryPath.replace("//", "/"));
}
#endif

#ifdef Q_OS_LINUX
void FindFileTest::testFindLibrary_data()
{
    QTest::addColumn<QString>("library");
    QTest::addColumn<QString>("libraryPath");

    {
        QDir dir = qApp->applicationDirPath();
        dir.cdUp();
        dir.cdUp();
        QString gcf3Path = dir.absoluteFilePath("libGCF3.so");
        QTest::newRow("GCF3") << "GCF3" << gcf3Path;
    }

    QTest::newRow("xorg/modules/fb") << "xorg/modules/fb" << "/usr/lib/xorg/modules/libfb.so";
    QTest::newRow("NonExistingLib") << "NonExistingLib" << QString();
    QTest::newRow("sqldrivers/qsqlite") << "sqldrivers/qsqlite"
        << QLibraryInfo::location( QLibraryInfo::PluginsPath ) + "/sqldrivers/libqsqlite.so";
}

void FindFileTest::testFindLibrary()
{
    QFETCH(QString, library);
    QFETCH(QString, libraryPath);

    QString gLibPath = GCF::findLibrary(library);
    QCOMPARE(gLibPath, libraryPath.replace("//", "/"));
}
#endif

#ifdef Q_OS_WIN32
void FindFileTest::testFindLibrary_data()
{
    QTest::addColumn<QString>("library");
    QTest::addColumn<QString>("libraryPath");

    {
        QDir dir = qApp->applicationDirPath();
        dir.cdUp();
        dir.cdUp();
        QString gcf3Path = dir.absoluteFilePath("GCF3.dll");
        QTest::newRow("GCF3") << "GCF3" << gcf3Path;
    }

#ifdef Q_OS_WIN32
#if QT_VERSION >= 0x050000
    QString library = "sqldrivers/qsqlite";
#else
    QString library = "sqldrivers/qsqlite4";
#endif
#else
    QString library = "sqldrivers/qsqlite";
#endif

    QTest::newRow("gdi32") << "gdi32" << "C:/Windows/System32/gdi32.dll";
    QTest::newRow("NonExistingLib") << "NonExistingLib" << QString();
    QTest::newRow(library.toLatin1().constData()) << library
                                                  << QLibraryInfo::location( QLibraryInfo::PluginsPath ) + '/' + library + ".dll";
}

void FindFileTest::testFindLibrary()
{
    QFETCH(QString, library);
    QFETCH(QString, libraryPath);

    QString gLibPath = GCF::findLibrary(library).toLower();
    gLibPath = gLibPath.replace("/", "\\");
    gLibPath = gLibPath.replace("\\\\", "\\");

    libraryPath = libraryPath.toLower();
    libraryPath = libraryPath.replace("/", "\\");
    libraryPath = libraryPath.replace("\\\\", "\\");

    QCOMPARE(gLibPath, libraryPath);
}
#endif

#ifdef Q_OS_MAC
/*
 * For testing GCF::findFile() we depend on the fact that certain
 * files are available on standard paths in a typical Mac OSX system.
 */
void FindFileTest::testFindFile_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("filePath");

    QTest::newRow("tst_FindFileTest") << "tst_FindFileTest" << qApp->applicationFilePath();
    QTest::newRow("Downloads/About Downloads.lpdf")
            << "Downloads/About Downloads.lpdf"
            << QDir::homePath() + "/Downloads/About Downloads.lpdf";
    QTest::newRow("make") << "make" << "/usr/bin/make";
    QTest::newRow("fsck") << "fsck" << "/sbin/fsck";
    QTest::newRow("qmake") << "qmake" << QLibraryInfo::location(QLibraryInfo::BinariesPath) + "/qmake";
}

void FindFileTest::testFindFile()
{
    QFETCH(QString, file);
    QFETCH(QString, filePath);

    QString gFilePath = GCF::findFile(file);
    QCOMPARE(gFilePath, filePath.replace("//", "/"));
}
#endif

#ifdef Q_OS_LINUX
void FindFileTest::testFindFile_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("filePath");

    QTest::newRow("tst_FindFileTest") << "tst_FindFileTest" << qApp->applicationFilePath();
    QTest::newRow("make") << "make" << "/usr/bin/make";
    QTest::newRow("fsck") << "fsck" << "/sbin/fsck";
    QTest::newRow("qmake") << "qmake" << QLibraryInfo::location(QLibraryInfo::BinariesPath) + "/qmake";
}

void FindFileTest::testFindFile()
{
    QFETCH(QString, file);
    QFETCH(QString, filePath);

    QString gFilePath = GCF::findFile(file);
    QCOMPARE(gFilePath, filePath.replace("//", "/"));
}
#endif

#ifdef Q_OS_WIN32
void FindFileTest::testFindFile_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("filePath");

    QTest::newRow("notepad.exe") << "notepad.exe" << "C:\\Windows\\System32\\notepad.exe";
    QTest::newRow("explorer.exe") << "explorer.exe" << "C:\\Windows\\System32\\explorer.exe";
    QTest::newRow("qmake.exe") << "qmake.exe" << QLibraryInfo::location(QLibraryInfo::BinariesPath) + "/qmake.exe";
}

void FindFileTest::testFindFile()
{
    QFETCH(QString, file);
    QFETCH(QString, filePath);

    QString gFilePath = GCF::findFile(file).toLower();
    filePath = filePath.toLower();

    filePath = filePath.replace("/", "\\");
    filePath = filePath.replace("\\\\", "\\");
    gFilePath = gFilePath.replace("/", "\\");
    gFilePath = gFilePath.replace("\\\\", "\\");

    QCOMPARE(gFilePath, filePath);
}
#endif

QTEST_MAIN(FindFileTest)

#include "tst_FindFileTest.moc"
