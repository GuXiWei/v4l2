/*************************************************************************
	> File Name: yuv.h
	> Author: gxw
	> Mail: 2414434710@qq.com 
	> Created Time: 2018年06月21日 星期四 15时14分53秒
 ************************************************************************/
#ifndef _YUV_H_
#define _YUV_H_

#include<stdio.h>
#include<string.h>

/**
 * YUYV to RGB24
 */
void YUYVtoRGB24(int width, int height, unsigned char* src, unsigned char* dst);

/**
 * Convert RGB24 file to BMP file
 * @param rgb24path    RGB24 data.
 * @param width        Width of input RGB file.
 * @param height       Height of input RGB file.
 * @param url_out      Location of Output BMP file.
 */
int RGB24toBMP(int width, int height, unsigned char* src, const char *bmppath);

#endif
