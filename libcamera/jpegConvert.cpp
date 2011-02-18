/*
** Copyright 2008, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
/*
**    Version: 1.0
**    Coded by: Josebagar <joseba.gar@gmail.com> 02/2011 (linux version)
**    Revised and Recoded: KalimochoAz <calimochoazucarado@gmail.com> 02/2011 (android conversions) Done for CyanogenMOD
*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
extern "C" {
    #include <jpeglib.h>
}
#if JPEG_LIB_VERSION < 80
// The routines defined in this file have been backported from jpeglib 8.0
#include "jdatadst.h"
#endif
#include "jpegConvert.h"
#include "raw2jpeg.h"


////////////////////////////////////////////////////////////////////////////////
YuvToJpegEncoder* YuvToJpegEncoder::create(int* strides) {
    // Only ImageFormat.NV21 and ImageFormat.YUY2 are supported
    // for now.
    return new Yuv420SpToJpegEncoder(strides);
}

YuvToJpegEncoder::YuvToJpegEncoder(int* strides) : fStrides(strides) {
}

bool YuvToJpegEncoder::encode(unsigned char* dest, void* inYuv, int width,
        int height, int* offsets, int jpegQuality, uint32_t* mSize) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    long unsigned int image_size;

    // Warning, this is ONLY valid for YUV420SP (ImageFormat.NV21 in android)
    image_size = (width*height*1.5);

    // Create JPEG compression object
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    // Point it to the output file
    jpeg_mem_dest(&cinfo, &dest, &image_size);

    setJpegCompressStruct(&cinfo, width, height, jpegQuality);

    jpeg_start_compress(&cinfo, TRUE);

    compress(&cinfo, (uint8_t*) inYuv, offsets);

    jpeg_finish_compress(&cinfo);

    *mSize = (uint32_t) image_size;
    mJpegTam = (uint32_t) image_size;
    return true;
}

void YuvToJpegEncoder::setJpegCompressStruct(jpeg_compress_struct* cinfo,
        int width, int height, int quality) {
    cinfo->image_width = width;
    cinfo->image_height = height;
    cinfo->input_components = 3;
    cinfo->in_color_space = JCS_YCbCr;
    jpeg_set_defaults(cinfo);

    jpeg_set_quality(cinfo, quality, TRUE);
    jpeg_set_colorspace(cinfo, JCS_YCbCr);
    cinfo->raw_data_in = TRUE;
    cinfo->dct_method = JDCT_IFAST;
    configSamplingFactors(cinfo);
}

///////////////////////////////////////////////////////////////////
Yuv420SpToJpegEncoder::Yuv420SpToJpegEncoder(int* strides) :
        YuvToJpegEncoder(strides) {
    fNumPlanes = 2;
}

void Yuv420SpToJpegEncoder::compress(jpeg_compress_struct* cinfo,
        uint8_t* yuv, int* offsets) {
    JSAMPROW y[16];
    JSAMPROW cb[8];
    JSAMPROW cr[8];
    JSAMPARRAY planes[3];
    planes[0] = y;
    planes[1] = cb;
    planes[2] = cr;

    int width = cinfo->image_width;
    int height = cinfo->image_height;
    uint8_t* yPlanar = yuv + offsets[0];
    uint8_t* vuPlanar = yuv + offsets[1]; //width * height;
    uint8_t* uRows = new uint8_t [8 * (width >> 1)];
    uint8_t* vRows = new uint8_t [8 * (width >> 1)];


    // process 16 lines of Y and 8 lines of U/V each time.
    while (cinfo->next_scanline < cinfo->image_height) {
        //deitnerleave u and v
        deinterleave(vuPlanar, uRows, vRows, cinfo->next_scanline, width);

        for (int i = 0; i < 16; i++) {
            // y row
            y[i] = yPlanar + (cinfo->next_scanline + i) * fStrides[0];

            // construct u row and v row
            if ((i & 1) == 0) {
                // height and width are both halved because of downsampling
                int offset = (i >> 1) * (width >> 1);
                cb[i/2] = uRows + offset;
                cr[i/2] = vRows + offset;
            }
          }
        jpeg_write_raw_data(cinfo, planes, 16);
    }
    delete [] uRows;
    delete [] vRows;

}

void Yuv420SpToJpegEncoder::deinterleave(uint8_t* vuPlanar, uint8_t* uRows,
        uint8_t* vRows, int rowIndex, int width) {
    for (int row = 0; row < 8; ++row) {
        int offset = ((rowIndex >> 1) + row) * fStrides[1];
        uint8_t* vu = vuPlanar + offset;
        for (int i = 0; i < (width >> 1); ++i) {
            int index = row * (width >> 1) + i;
            uRows[index] = vu[1];
            vRows[index] = vu[0];
            vu += 2;
        }
    }
}

void Yuv420SpToJpegEncoder::configSamplingFactors(jpeg_compress_struct* cinfo) {
    // cb and cr are horizontally downsampled and vertically downsampled as well.
    cinfo->comp_info[0].h_samp_factor = 2;
    cinfo->comp_info[0].v_samp_factor = 2;
    cinfo->comp_info[1].h_samp_factor = 1;
    cinfo->comp_info[1].v_samp_factor = 1;
    cinfo->comp_info[2].h_samp_factor = 1;
    cinfo->comp_info[2].v_samp_factor = 1;
}

///////////////////////////////////////////////////////////////////////////////

int yuv420_save2jpeg(unsigned char *dest, void *src, int width, int height, int quality, uint32_t *mSize) {
    int imgStrides[2], imgOffsets[2];

    // Convert the RAW data to JPEG
    imgStrides[0] = imgStrides[1] = width;
    YuvToJpegEncoder* encoder = YuvToJpegEncoder::create(imgStrides);
    if (encoder == NULL) {
        return false;
    }

    // Guessed from frameworks/base/graphics/java/android/graphics/YuvImage.java
    // in android
    imgOffsets[0] = 0;
    imgOffsets[1] = width*height;
    encoder->encode(dest, src, width, height, imgOffsets, quality, mSize);

    delete encoder;

    return true;
}
