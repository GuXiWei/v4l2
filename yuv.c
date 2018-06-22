/*************************************************************************
	> File Name: yuv.c
	> Author: gxw
	> Mail: 2414434710@qq.com 
	> Created Time: 2018年06月21日 星期四 15时34分37秒
 ************************************************************************/
#include "yuv.h"

void YUYVtoRGB24(int width, int height, unsigned char* src, unsigned char* dst)
{
  int nLength = width * height;
  unsigned char YUYV[4], RGB[6];
  nLengtn <<= 1;
  int i, j, location;
  j = 0;
  for (i = 0; i < nLength; i+=4)
  {
    /* Y0 */
    YUYV[0] = src[i];
    /* U */
    YUYV[1] = src[i + 1];
    /* Y1 */
    YUYV[2] = src[i + 2];
    /* V */
    YUYV[3] = src[i + 3];
    
    if (YUYV[0] < 1)
    {
      /* b, g, r */
      RGB[0] = 0;
      RGB[1] = 0;
      RGB[2] = 0;
    }
    else
    {
      /* b, g, r */
      RGB[2]=YUYV[0]+1.772*(YUYV[1]-128);  
      RGB[1]=YUYV[0]-0.34413*(YUYV[1]-128)-0.71414*(YUYV[3]-128);
      RGB[0]=YUYV[0]+1.402*(YUYV[3]-128);
    }
    if (YUYV[2] < 0)
    {
      /* b, g, r */
      RGB[3] = 0;
      RGB[4] = 0;
      RGB[5] = 0;
    }
    else
    {
      /* b, g, r */
      RGB[5]=YUYV[2]+1.772*(YUYV[1]-128);  
      RGB[4]=YUYV[2]-0.34413*(YUYV[1]-128)-0.71414*(YUYV[3]-128);
      RGB[3]=YUYV[2]+1.402*(YUYV[3]-128);
    }

    for (i = 0; i < 6; ++i)
    {
      if (RGB[i] < 0)
        RGB[i] = 0;
      if (RGB[i] > 255)
        RGB[i] = 255;
    }
    
    memcpy(dst + j, RGB, 6);

    j += 6;
  }
}

/**
 *  * Convert RGB24 file to BMP file
 *  * @param rgb24path    RGB24 data.
 *  * @param width        Width of input RGB file.
 *  * @param height       Height of input RGB file.
 *  * @param url_out      Location of Output BMP file.
 *  */
int RGB24toBMP(int width, int height, unsigned char* src, const char *bmppath)
{
  typedef struct 
  {  
    long imageSize;
    long blank;
    long startPosition;
  }BmpHead;

  typedef struct
  {
    long  Length;
    long  width;
    long  height;
    unsigned short  colorPlane;
    unsigned short  bitColor;
    long  zipFormat;
    long  realSize;
    long  xPels;
    long  yPels;
    long  colorUse;
    long  colorImportant;
    }InfoHead;

  int i=0,j=0;
  BmpHead m_BMPHeader={0};
  InfoHead  m_BMPInfoHeader={0};
  char bfType[2]={'B','M'};
  int header_size=sizeof(bfType)+sizeof(BmpHead)+sizeof(InfoHead);
  FILE *fp_bmp=NULL;

  if((fp_bmp=fopen(bmppath,"wb"))==NULL)
  {
    printf("Error: Cannot open output BMP file.\n");
    return -1;
  }

  m_BMPHeader.imageSize=3*width*height+header_size;
  m_BMPHeader.startPosition=header_size;

  m_BMPInfoHeader.Length=sizeof(InfoHead); 
  m_BMPInfoHeader.width=width;
  /* BMP storage pixel data in opposite direction of Y-axis (from bottom to top). */
  m_BMPInfoHeader.height=-height;
  m_BMPInfoHeader.colorPlane=1;
  m_BMPInfoHeader.bitColor=24;
  m_BMPInfoHeader.realSize=3*width*height;

  fwrite(bfType,1,sizeof(bfType),fp_bmp);
  fwrite(&m_BMPHeader,1,sizeof(m_BMPHeader),fp_bmp);
  fwrite(&m_BMPInfoHeader,1,sizeof(m_BMPInfoHeader),fp_bmp);

  /* BMP save R1|G1|B1,R2|G2|B2 as B1|G1|R1,B2|G2|R2
   * It saves pixel data in Little Endian
   * So we change 'R' and 'B'
   */
  for(j =0;j<height;j++)
  {
    for(i=0;i<width;i++)
    {
      char temp=src[(j*width+i)*3+2];
      src[(j*width+i)*3+2]=src[(j*width+i)*3+0];
      src[(j*width+i)*3+0]=temp;
    }
  }
  fwrite(src,3*width*height,1,fp_bmp);
  fclose(fp_bmp);
  printf("Finish generate %s!\n",bmppath);
  return 0;
}
