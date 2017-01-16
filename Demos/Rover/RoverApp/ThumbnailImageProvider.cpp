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

#include "ThumbnailImageProvider.h"

#include <QFile>
#include <QImage>
#include <QBuffer>
#include <QByteArray>
#include <QImageReader>

// Exif defines
#define JPEG_SOI 0xffd8
#define JPEG_SOS 0xffda
#define JPEG_EOI 0xffd9
#define JPEG_APP1 0xffe1

struct ThumbnailImageProviderData
{
    QMap<QString, QImage> imageList;
};

ThumbnailImageProvider::ThumbnailImageProvider():
    QQuickImageProvider(QQuickImageProvider::Image)
{
    d = new ThumbnailImageProviderData;
}

ThumbnailImageProvider::~ThumbnailImageProvider()
{
    delete d;
}

QImage ThumbnailImageProvider::requestImage(const QString &fileName, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(size);

    if(d->imageList.contains(fileName))
        return d->imageList[fileName];

    d->imageList[fileName] = fetchThumbnail(fileName, requestedSize);
    return d->imageList[fileName];
}

bool readWord( QIODevice &sdevice, unsigned short *target, bool invert ) {
    unsigned short t;
    if (sdevice.read((char*)&t, 2) != 2) return false;
    if (invert)
        *target = ((t&255) << 8) | ((t>>8)&255);
    else
        *target = t;
    return true;
}

/**
 * Scans though exif-chunks, finds the app1-chunk and processes it.
 */
bool exifScanloop( QIODevice &jpegFile, unsigned int &tnOffset, unsigned int &tnLength ) {
    // LOOP THROUGH TAGS
    while (1) {
        unsigned short tagid, tagLength;
        if (!readWord( jpegFile, &tagid, true )) return 0; // updated
        if (tagid == JPEG_EOI || tagid == JPEG_SOS) break; // Data ends
        if (!readWord( jpegFile, &tagLength, true )) return 0; // updated

        if (tagid == JPEG_APP1) {
            char str[6];
            jpegFile.read(str,6 );

            // Store the current position for offset calculation
            int basepos = jpegFile.pos();

            // read tiff - header
            unsigned short tifhead[2];
            for (int h=0; h<2; h++)
                if (!readWord(jpegFile, &tifhead[h], true)) return false; // updated

            if (tifhead[0] != 0x4949)
                return false; // invalid byte order

            while (1) {
                unsigned int offset;
                jpegFile.read( (char*)&offset, 4);
                if (offset==0) break;
                jpegFile.seek( basepos + offset );

                unsigned short fields;
                if (!readWord(jpegFile, &fields, false)) return false;
                while (fields>0) {
                    char ifdentry[12];
                    jpegFile.read( ifdentry, 12 );
                    unsigned short tagnumber = (((unsigned short)ifdentry[0]) | (unsigned short)ifdentry[1]<<8);
                    // Offset of the thumbnaildata
                    if (tagnumber == 0x0201) {
                        memcpy( &tnOffset, ifdentry+8, 4 );
                        tnOffset += basepos;

                    } else  // Length of the thumbnaildata
                        if (tagnumber == 0x0202) {
                            memcpy( &tnLength, ifdentry+8, 4 );
                        };
                    fields--;
                    if (tnOffset != 0 && tnLength!=0) return true;
                }
            }
            return false;
        }
        jpegFile.seek( jpegFile.pos() + tagLength-2 );
    }
    return false;
}

QImage ThumbnailImageProvider::fetchThumbnail(const QString &fileName, const QSize &requiredSize)
{
    QFile jpegFile(fileName);

    if (!jpegFile.open( QIODevice::ReadOnly	)) return fetchScaledImage(fileName, requiredSize);
    unsigned short jpegId;
    if (!readWord( jpegFile, &jpegId, true )) return fetchScaledImage(fileName, requiredSize); // updated
    if (jpegId!= JPEG_SOI) return fetchScaledImage(fileName, requiredSize); // JPEG SOI must be here

    unsigned int tnOffset = 0;
    unsigned int tnLength = 0;
    if (exifScanloop( jpegFile, tnOffset, tnLength)) {
        // Goto the thumbnail offset in the file
        jpegFile.seek( tnOffset );
        // Use image reader to decode jpeg-encoded thumbnail
        QByteArray tnArray = jpegFile.read( tnLength );
        QBuffer buf( &tnArray, 0 );
        QImageReader reader(&buf);
        reader.setAutoDetectImageFormat( false );
        reader.setFormat("jpg");
        QSize imageSize = reader.size();
        reader.setClipRect(QRect(0, 7, imageSize.width(), imageSize.height() - 14));
        return reader.read();
    }

    return fetchScaledImage(fileName, requiredSize);
}

QImage ThumbnailImageProvider::fetchScaledImage(const QString& fileName, const QSize &requiredSize)
{
    QImageReader imageReader(fileName);
    QSize imageSize = imageReader.size();
    double actialScaleFactor = qMax(imageSize.height(), imageSize.width());
    double requiredScaleFactor = qMax(requiredSize.height(), requiredSize.width());
    double scaleMultiplier = actialScaleFactor / requiredScaleFactor;
    imageReader.setScaledSize(QSize(imageSize.width() / scaleMultiplier, imageSize.height() / scaleMultiplier));
    return imageReader.read();
}
