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

#ifndef IPCCOMMON_H
#define IPCCOMMON_H

#include "../Core/GCFGlobal.h"
#include "../Core/Version.h"

#ifdef GCF_IPC_STATIC_BUILD
    #define GCF_IPC_EXPORT
#else
    #ifdef GCF_IPC_DLL_BUILD
        #define GCF_IPC_EXPORT Q_DECL_EXPORT
    #else
        #define GCF_IPC_EXPORT Q_DECL_IMPORT
    #endif
#endif

#define GCF_IPC_VERSION_MAJOR GCF_VERSION_MAJOR
#define GCF_IPC_VERSION_MINOR GCF_VERSION_MINOR
#define GCF_IPC_VERSION_REVISION GCF_VERSION_REVISION

#endif // IPCCOMMON_H

