/*************************************************************************
        > File Name: main_rgb_to_bmp.c
        > Author:
        > Mail:
        > Created Time: 2018年06月23日 星期六 22时19分29秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "yuv.h"

int simplest_rgb24_to_bmp(const char *rgb24path, int width, int height,
                          const char *bmppath) {
#pragma pack(1)
  typedef struct {
    int imageSize;
    int blank;
    int startPosition;
  } BmpHead;
#pragma pack()

#pragma pack(1)
  typedef struct {
    int Length;
    int width;
    int height;
    unsigned short colorPlane;
    unsigned short bitColor;
    int zipFormat;
    int realSize;
    int xPels;
    int yPels;
    int colorUse;
    int colorImportant;

  } InfoHead;
#pragma pack()

  int i = 0, j = 0;
  BmpHead m_BMPHeader = {0};
  InfoHead m_BMPInfoHeader = {0};
  char bfType[2] = {'B', 'M'};
  int header_size = sizeof(bfType) + sizeof(BmpHead) + sizeof(InfoHead);
  printf("%d\n", sizeof(InfoHead));
  unsigned char *rgb24_buffer = NULL;
  FILE *fp_rgb24 = NULL, *fp_bmp = NULL;

  if ((fp_rgb24 = fopen(rgb24path, "rb")) == NULL) {
    printf("Error: Cannot open input RGB24 file.\n");
    return -1;
  }
  if ((fp_bmp = fopen(bmppath, "wb")) == NULL) {
    printf("Error: Cannot open output BMP file.\n");
    return -1;
  }

  rgb24_buffer = (unsigned char *)malloc(width * height * 3);
  fread(rgb24_buffer, 1, width * height * 3, fp_rgb24);

  m_BMPHeader.imageSize = 3 * width * height + header_size;
  m_BMPHeader.startPosition = header_size;

  m_BMPInfoHeader.Length = sizeof(InfoHead);
  m_BMPInfoHeader.width = width;
  m_BMPInfoHeader.height = -height;
  m_BMPInfoHeader.colorPlane = 1;
  m_BMPInfoHeader.bitColor = 24;
  m_BMPInfoHeader.realSize = 3 * width * height;

  fwrite(bfType, 1, sizeof(bfType), fp_bmp);
  fwrite(&m_BMPHeader, 1, sizeof(m_BMPHeader), fp_bmp);
  fwrite(&m_BMPInfoHeader, 1, sizeof(m_BMPInfoHeader), fp_bmp);

  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++) {
      char temp = rgb24_buffer[(j * width + i) * 3 + 2];
      rgb24_buffer[(j * width + i) * 3 + 2] =
          rgb24_buffer[(j * width + i) * 3 + 0];
      rgb24_buffer[(j * width + i) * 3 + 0] = temp;
    }
  }
  fwrite(rgb24_buffer, 3 * width * height, 1, fp_bmp);
  fclose(fp_rgb24);
  fclose(fp_bmp);
  free(rgb24_buffer);
  printf("Finish generate %s!\n", bmppath);
  return 0;
}
int main() {
  /*
  FILE* file_rgb = NULL;
  file_rgb = fopen("lena_256x256_rgb24.rgb", "rb");
  if (!file_rgb)
  {
    printf("open file failed!\n");
    return -1;
  }

  unsigned char* rgb24_buff = NULL;
  rgb24_buff = (unsigned char*)malloc(256*256*3);
  if (!rgb24_buff)
    return -1;

  fread(rgb24_buff, 1, 256*256*3, file_rgb);
  RGB24toBMP(256, 256, rgb24_buff, "rgb24.bmp");
  fclose(file_rgb);
  free(rgb24_buff);
  */
  simplest_rgb24_to_bmp("lena_256x256_rgb24.rgb", 256, 256, "rgb24.bmp");
}
