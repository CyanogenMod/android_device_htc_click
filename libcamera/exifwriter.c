#include "exifwriter.h"

#include "jhead.h"
#define LOG_TAG "ExifWriterCamera"
// #define LOG_NDEBUG 0

#include <utils/Log.h>

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define TAG_ORIENTATION            0x0112
#define TAG_MAKE                   0x010F
#define TAG_MODEL                  0x0110
#define TAG_IMAGE_WIDTH            0x0100
#define TAG_IMAGE_LENGTH           0x0101
#define TAG_EXIF_VERSION           0x9000
#define EXIF_TOTAL_DATA 2


float *float2degminsec( float deg )
{
  float *res = malloc( sizeof(float)*3 );
  res[0] =  floorf( deg );
  float min = ( deg - res[0] ) * 60.;
  res[1] = floorf( min );
  res[2] = ( min - res[1] ) * 60.;
  return res;
}


//
// original source from
// http://stackoverflow.com/questions/95727/how-to-convert-floats-to-human-readable-fractions
//
char * float2rationnal( float src )
{
  long m[2][2];
  float x, startx;
  long maxden = 1000;
  long ai;

  startx = x = src;

  LOGV("float2rationnal: Convertir %f", src);

  /* initialize matrix */
  m[0][0] = m[1][1] = 1;
  m[0][1] = m[1][0] = 0;

  /* loop finding terms until denom gets too big */
  while (m[1][0] *  ( ai = (long)x ) + m[1][1] <= maxden) {
      long t;
      t = m[0][0] * ai + m[0][1];
      m[0][1] = m[0][0];
      m[0][0] = t;
      t = m[1][0] * ai + m[1][1];
      m[1][1] = m[1][0];
      m[1][0] = t;
      if(x==(float)ai) break;     // AF: division by zero
      x = 1/(x - (float) ai);
      if(x>(float)0x7FFFFFFF) break;  // AF: representation failure
  }


  /* now remaining x is between 0 and 1/ai */
  /* approx as either 0 or 1/m where m is max that will fit in maxden */
  /* first try zero */

  /* now try other possibility */
  ai = (maxden - m[1][1]) / m[1][0];
  m[0][0] = m[0][0] * ai + m[0][1];
  m[1][0] = m[1][0] * ai + m[1][1];

  char *res = (char *)malloc( 256 * sizeof(char) );

  snprintf( res, 256, "%ld/%ld", m[0][0], m[1][0] );
  return res;
}

char * coord2degminsec( float src )
{
    char *res = (char *)malloc( 256 * sizeof(char) );
    LOGV("coord2degminsec: Convertir %f", src);
    float *dms = float2degminsec( fabs(src) );
    LOGV("coord2degminsec: paso1 (float) %f %f %f", dms[0], dms[1], dms[2]);
    strcpy( res, float2rationnal(dms[0]) );
    strcat( res , "," );
    strcat( res , float2rationnal(dms[1]) );
    strcat( res , "," );
    strcat( res , float2rationnal(dms[2]) );
    LOGV("coord2degminsec: Convertido en %s", res);
    free( dms );
    return res;
}

static void dump_to_file(const char *fname,
                         uint8_t *buf, uint32_t size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    LOGV("opening file [%s]\n", fname);
    int fd = open(fname, O_RDWR | O_CREAT);
    if (fd < 0) {
        LOGE("failed to create file [%s]: %s", fname, strerror(errno));
        return;
    }

    LOGV("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            LOGE("failed to write to file [%s]: %s",
                 fname, strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    LOGV("done writing %d bytes to file [%s] in %d passes\n",
         size, fname, cnt);
    close(fd);
}

void writeExif( void *origData, void *destData , int origSize , uint32_t *resultSize, int orientation,camera_position_type  *pt ) {
  const char *filename = "/cache/tmp/temp.jpg";

    dump_to_file( filename, (uint8_t *)origData, origSize );
    LOGV("WRITE EXIF Filename %s", filename);
    chmod( filename, S_IRWXU );
    ResetJpgfile();


    memset(&ImageInfo, 0, sizeof(ImageInfo));
    ImageInfo.FlashUsed = -1;
    ImageInfo.MeteringMode = -1;
    ImageInfo.Whitebalance = -1;

    int gpsTag = 0;
    if( pt != NULL ) {
        LOGV("EXIF ADD GPS DATA ........");
        gpsTag = 6;
    } else{
        LOGV("EXIF NO GPS ........");
    }


    ExifElement_t *t = (ExifElement_t *)malloc( sizeof(ExifElement_t)*(EXIF_TOTAL_DATA+gpsTag) );

    ExifElement_t *it = t;
    // Store file date/time.
    (*it).Tag = TAG_ORIENTATION;
    (*it).Format = FMT_USHORT;
    (*it).DataLength = 1;
    unsigned short v;
    LOGV("EXIF Orientation %d º", orientation);
    if( orientation == 90 ) {
        (*it).Value = "6\0";
    } else if( orientation == 180 ) {
        (*it).Value = "3\0";
    } else {
        (*it).Value = "1\0";
    }
    (*it).GpsTag = FALSE;

    it++;
    (*it).Tag = TAG_MODEL;
    (*it).Format = FMT_STRING;
    (*it).Value = "Tattoo with CyanogenMOD\0";
    (*it).DataLength = 24;
    (*it).GpsTag = FALSE;

    if( pt != NULL ) {
        LOGV("pt->latitude == %f", pt->latitude );
        LOGV("pt->longitude == %f", pt->longitude );
        LOGV("pt->altitude == %d", pt->altitude );

        it++;
        (*it).Tag = 0x01;
        (*it).Format = FMT_STRING;
        if( pt->latitude > 0 ) {
            (*it).Value = "N\0";
        } else {
            (*it).Value = "S\0";
        }
        (*it).DataLength = 2;
        (*it).GpsTag = TRUE;

        it++;
        (*it).Value = coord2degminsec( pt->latitude );
        LOGV("writeExif: La latitud queda en: %s", (*it).Value);

        (*it).Tag = 0x02;
        (*it).Format = FMT_URATIONAL;
        (*it).DataLength = 3;
        (*it).GpsTag = TRUE;

        it++;
        (*it).Tag = 0x03;
        (*it).Format = FMT_STRING;
        if( (*pt).longitude > 0 ) {
            (*it).Value = "E\0";
        } else {
            (*it).Value = "W\0";
        }
        (*it).DataLength = 2;
        (*it).GpsTag = TRUE;

        it++;
        (*it).Value = coord2degminsec( pt->longitude );
        LOGV("writeExif: La longitud queda en: %s", (*it).Value);

        (*it).Tag = 0x04;
        (*it).Format = FMT_URATIONAL;
        (*it).DataLength = 3;
        (*it).GpsTag = TRUE;

        it++;
        (*it).Tag = 0x05;
        (*it).Format = FMT_USHORT;
        if( (*pt).altitude > 0 ) {
            (*it).Value = "0\0";
        } else {
            (*it).Value = "1\0";
        }
        (*it).DataLength = 1;
        (*it).GpsTag = TRUE;

        it++;
        (*it).Value = float2rationnal( fabs( pt->altitude ) );
        LOGV("writeExif: La altitud queda en: %s", (*it).Value);

        (*it).Tag = 0x06;
        (*it).Format = FMT_SRATIONAL;
        (*it).DataLength = 1;
        (*it).GpsTag = TRUE;
    }

   {
        struct stat st;
        if (stat(filename, &st) >= 0) {
            ImageInfo.FileDateTime = st.st_mtime;
            ImageInfo.FileSize = st.st_size;
        }
    }
    strncpy(ImageInfo.FileName, filename, PATH_MAX);
    LOGV("Image EXIF Filename %s", filename);

    ReadMode_t ReadMode;
    ReadMode = READ_METADATA;
    ReadMode |= READ_IMAGE;
    int res = ReadJpegFile(filename, (ReadMode_t)ReadMode );
    LOGV("READ EXIF Filename %s", filename);

    create_EXIF( t, EXIF_TOTAL_DATA, gpsTag);

    WriteJpegFile(filename);
    chmod( filename, S_IRWXU );
    DiscardData();

    FILE *src;
    src = fopen( filename, "r");

    fseek( src, 0L, SEEK_END );
    (*resultSize) = ftell(src);
    fseek( src, 0L, SEEK_SET );

    int read = fread( destData, 1, (*resultSize), src );

    free( t );

    unlink( filename );
}
