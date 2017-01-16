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

#include "GCFGlobal.h"
#include "Log.h"
#include "Version.h"

#include <QDir>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#endif
#include <QCoreApplication>

#ifdef QT_GUI_LIB
#include <QDesktopServices>
#endif

#include <QLibraryInfo>
#include <QProcessEnvironment>

/*******
\namespace GCF
\brief Contains all class and global-methods in GCF

\htmlonly
<pre>#include &lt;GCF3/GCFGlobal&gt;</pre>
\endhtmlonly
*/

/**
\ingroup gcf_core
\return version of GCF library
 */
const GCF::Version &GCF::version()
{
    static const GCF::Version ver(GCF_VERSION_MAJOR, GCF_VERSION_MINOR, GCF_VERSION_REVISION);
    return ver;
}

/**
\ingroup gcf_core
\return "VCreate Logic Pvt. Ltd"
 */
QString GCF::vendor()
{
    static QString ret("VCreate Logic Pvt. Ltd.");
    return ret;
}

/**
\ingroup gcf_core
\return "http://gcf.vcreatelogic.com"
 */
QString GCF::url()
{
    static QString ret("http://gcf.vcreatelogic.com");
    return ret;
}

/**
\ingroup gcf_core
\return date and time when GCF was built
 */
QString GCF::buildTimestamp()
{
    return __TIMESTAMP__;
}

Q_GLOBAL_STATIC(QString,GCFContentDirectory)

/**
\ingroup gcf_core

Use this function to specify the directory where GCF should look for content-files.

@param path Path (absolute or relative to application executable's path) in which
GCF should look for content files.

\note If a content directory has already been set, then this function is a no-op.
 */
void GCF::setContentDirectory(const QString &path)
{
    if( ::GCFContentDirectory()->isEmpty() )
        *(::GCFContentDirectory()) = path;

    GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT, "Cannot modify content directory, once set!");
}

/**
\ingroup gcf_core
\return the content directory path where GCF will look for content-files by default.

\note Unless the user specifies a content-directory via \ref setContentDirectory(), the
GCF will look for content-files in \ref applicationDataDirectoryPath() /content.
 */
QString GCF::contentDirectory()
{
    if(::GCFContentDirectory()->isEmpty())
        return QString("%1/content").arg(applicationDataDirectoryPath());

    return *(::GCFContentDirectory());
}

Q_GLOBAL_STATIC(QString,GCFSettingsDirectory)

/**
  \ingroup gcf_core
Use this function to specify the directory where GCF should look for settings-files.

@param path Path (absolute or relative to application executable's path) in which
GCF should look for settings files.

\note If a settings directory has already been set, then this function is a no-op.
 */
void GCF::setSettingsDirectory(const QString &path)
{
    if( ::GCFSettingsDirectory()->isEmpty() )
        *(::GCFSettingsDirectory()) = path;

    GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT, "Cannot modify settings directory, once set!");
}

/**
  \ingroup gcf_core
\return the settings directory path where GCF will look for settings-files by default.

\note Unless the user specifies a settings-directory via \ref setSettingsDirectory(), the
GCF will look for content-files in \ref applicationDataDirectoryPath() /content.
 */
QString GCF::settingsDirectory()
{
    if(::GCFSettingsDirectory()->isEmpty())
        return QString("%1/settings").arg(applicationDataDirectoryPath());

    return *(::GCFSettingsDirectory());
}

/**
  \ingroup gcf_core
\return the directory for loading and saving of user-specific data.
*/
QString GCF::applicationDataDirectoryPath()
{
    static QString path;
    if(path.isEmpty())
    {
        // If application name has not already been set, then we better
        // set it now.
        if(qApp->applicationName().isEmpty())
        {
            if(qApp)
                qApp->setApplicationName( QFileInfo(qApp->arguments().first()).baseName() );
            else
                qApp->setApplicationName("GCF Library");
        }

        // If organizationName has not already been set, then we better
        // set it now.
        if(qApp->organizationName().isEmpty())
            qApp->setOrganizationName("VCreate Logic");

#if QT_VERSION >= 0x050000
        path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
    #ifdef QT_GUI_LIB
        path = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    #else
        path = QDir::homePath() + "/" + qApp->organizationName() + "/" + qApp->applicationName();
    #endif
#endif

        QDir pathDir(path);
        if(!pathDir.exists())
            QDir().mkpath(path);
    }

    return path;
}

/**
\internal
*/
QStringList searchPaths(bool forLibs=false)
{
    /*
     * This function returns a list of directory paths for finding
     * files and libraries. If the forLibs parameter is true; then
     * the returned list contains qApp->libraryPaths(), $LD_LIBRARY_PATH first
     * and then $PATH
     */

    QStringList paths;
    paths << qApp->applicationDirPath() << QDir::currentPath() << QDir::homePath();

    if( forLibs )
    {
        paths += qApp->libraryPaths();
#ifdef Q_OS_MAC
        {
            QString env = QProcessEnvironment::systemEnvironment().value("DYLD_LIBRARY_PATH");
            paths += env.split(":",QString::SkipEmptyParts);
            paths << "/usr/lib" << "/lib" << "/usr/local/lib";
        }
#endif
#ifdef Q_OS_LINUX
        {
            QString env = QProcessEnvironment::systemEnvironment().value("LD_LIBRARY_PATH");
            paths += env.split(":",QString::SkipEmptyParts);
            paths << "/usr/lib" << "/lib" << "/usr/local/lib";
        }
#endif
#ifdef Q_OS_WIN32
        // We are going to consider PATH environment variable anyway; so nothing additional to do here
#endif

        paths << QLibraryInfo::location( QLibraryInfo::LibrariesPath );
        paths << QLibraryInfo::location( QLibraryInfo::PluginsPath );
        paths << QLibraryInfo::location( QLibraryInfo::ImportsPath );
    }

    paths << QLibraryInfo::location( QLibraryInfo::BinariesPath );
#if QT_VERSION >= 0x050000
    paths << QLibraryInfo::location( QLibraryInfo::LibraryExecutablesPath );
#endif
    paths << QLibraryInfo::location( QLibraryInfo::SettingsPath );
    paths << QLibraryInfo::location( QLibraryInfo::HeadersPath );
    paths << QLibraryInfo::location( QLibraryInfo::PrefixPath );

    QString env = QProcessEnvironment::systemEnvironment().value("PATH");
#ifdef Q_OS_MAC
    paths += env.split(":",QString::SkipEmptyParts);
#endif
#ifdef Q_OS_LINUX
    paths += env.split(":",QString::SkipEmptyParts);
#endif
#ifdef Q_OS_WIN32
    paths += env.split(";",QString::SkipEmptyParts);
#endif

    paths.removeDuplicates();
    return paths;
}

/**
  \ingroup gcf_core
Returns a list of paths where GCF will look for shared libraries and components.
The returned list include
\li Application's directory path ( \c QCoreApplication::applicationDirPath() )
\li Current working directory ( \c QDir::currentPath() )
\li The current user's home directory ( \c QDir::homePath() )
\li Libary paths searched by \c QLibrary ( \c QCoreApplication::libraryPaths() )
\li Directories in \c DYLD_LIBRARY_PATH (Mac), \c LD_LIBRARY_PATH (Linux)
\li \c QLibraryInfo::LibrariesPath
\li \c QLibraryInfo::PluginsPath
\li \c QLibraryInfo::ImportsPath
\li \c QLibraryInfo::BinariesPath
\li \c QLibraryInfo::LibraryExecutablesPath
\li \c QLibraryInfo::SettingsPath
\li \c QLibraryInfo::HeadersPath
\li \c QLibraryInfo::PrefixPath
\li Directories in the \c PATH environment variable
*/
QStringList GCF::searchPathsForLibs()
{
    return ::searchPaths(true);
}

/**
  \ingroup gcf_core
Returns a list of paths where GCF will look for files (not shared libraries)
The returned list include
\li Application's directory path ( \c QCoreApplication::applicationDirPath() )
\li Current working directory ( \c QDir::currentPath() )
\li The current user's home directory ( \c QDir::homePath() )
\li Libary paths searched by \c QLibrary ( \c QCoreApplication::libraryPaths() )
\li \c QLibraryInfo::LibraryExecutablesPath
\li \c QLibraryInfo::SettingsPath
\li \c QLibraryInfo::HeadersPath
\li \c QLibraryInfo::PrefixPath
\li Directories in the \c PATH environment variable
*/
QStringList GCF::searchPaths()
{
    return ::searchPaths(false);
}

/**
  \ingroup gcf_core
This function searches for a library by name \c name and returns complete
path to the library file. If no such library was found the function returns
an empty string.

Example:
\code
// On a Mac OS machine

QString libFile = GCF::findLibrary("c++");
// libFile will contain /usr/lib/libc++.dylib
\endcode
*/
QString GCF::findLibrary(const QString &name)
{
    QStringList nameComps = name.split("/");

#ifdef Q_OS_MAC
    nameComps.last() = QString("lib%1.dylib").arg(nameComps.last());
#endif
#ifdef Q_OS_LINUX
    nameComps.last() = QString("lib%1.so").arg(nameComps.last());
#endif
#ifdef Q_OS_WIN32
    nameComps.last() = QString("%1.dll").arg(nameComps.last());
#endif

    QString libFile = nameComps.join("/");

    QStringList libPaths = ::searchPaths(true);
    Q_FOREACH(QString libPath, libPaths)
    {
        QDir libDir(libPath);
        QString libFilePath = libDir.absoluteFilePath(libFile);
        QFileInfo fi(libFilePath);
        if(fi.exists() && fi.isReadable())
            return libFilePath;
    }

    return QString();
}

/**
  \ingroup gcf_core
This function looks for a file by name \c name and returns complete path to the
file on the system. If no such file was found, then the function returns an empty
string.

Example:
\code
// On a Windows machine

QString path = GCF::findFile("notepad.exe");
// path will contain C:\\Windows\\System32\\notepad.exe
\endcode
*/
QString GCF::findFile(const QString &name)
{
    QStringList paths = ::searchPaths(false);
    Q_FOREACH(QString path, paths)
    {
        QDir dir(path);
        QString filePath = dir.absoluteFilePath(name);
        QFileInfo fi(filePath);
        if(fi.exists() && fi.isReadable())
            return filePath;
    }

    return QString();
}

/**
  \ingroup gcf_core
This function returns all the paths where a file by name \c name was found.
*/
QStringList GCF::findFiles(const QString &name)
{
    QStringList retList;
    QStringList paths = ::searchPaths(false);
    Q_FOREACH(QString path, paths)
    {
        QDir dir(path);
        QString filePath = dir.absoluteFilePath(name);
        QFileInfo fi(filePath);
        if(fi.exists() && fi.isReadable())
            retList << filePath;
    }

    return retList;
}

