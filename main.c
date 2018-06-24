/*************************************************************************
        > File Name: main.c
        > Author: gxw
        > Mail: 2414434710@qq.com
        > Created Time: 2018年06月18日 星期一 13时55分22秒
 ************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <librtmp/rtmp.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "yuv.h"

struct buffer {
  void* start;
  size_t length;
};
struct buffer* buffers = NULL;
static int cameraFD;

static int read_frame(void) {
  struct v4l2_buffer buf;
  memset(&buf, 0, sizeof(buf));
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  if (-1 == ioctl(cameraFD, VIDIOC_DQBUF, &buf)) return -1;
  // fputc('.', stdout);
  // fflush(stdout);
  unsigned char* rgb24 = malloc(640 * 480 * 3 * sizeof(unsigned char));
  const char* bmppath = "rgb24.bmp";
  if (!rgb24) return -1;
  YUYVtoRGB24(640, 480, buffers[buf.index].start, rgb24);
  RGB24toBMP(640, 480, rgb24, bmppath);
  if (-1 == ioctl(cameraFD, VIDIOC_QBUF, &buf)) return -1;
  free(rgb24);
  return 0;
}

int main() {
  cameraFD = open("/dev/video0", O_RDWR);
  if (cameraFD == -1)
    return -1;
  else
    printf("success!\n");
  v4l2_std_id std;
  int ret;
  struct v4l2_capability cap;
  /*
  do
    ret = ioctl(cameraFD, VIDIOC_QUERYSTD, &std);
  while (ret == -1 &&  errno == EAGAIN);
  printf("%d\n", std);
  switch (std)
  {
    case V4L2_STD_NTSC_M:
      printf("1 typt\n");
      break;
    case V4L2_STD_PAL_B:
      printf("2 type\n");
      break;
    default:
      printf("none\n");
      break;
 }*/
  if (ioctl(cameraFD, VIDIOC_QUERYCAP, &cap) == -1) printf("error\n");
  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) printf("error\n");
  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
    printf("error\n");
    return -1;
  };
  struct v4l2_fmtdesc fmt1;
  memset(&fmt1, 0, sizeof(fmt1));
  fmt1.index = 0;
  fmt1.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  while ((ioctl(cameraFD, VIDIOC_ENUM_FMT, &fmt1)) ==
         0)  //获取当前视频设备的帧格式 比如：YUYV
  {
    fmt1.index++;
    printf("{ pixelformat = '%c%c%c%c', description = '%s' }\n",
           fmt1.pixelformat & 0xFF, (fmt1.pixelformat >> 8) & 0xFF,
           (fmt1.pixelformat >> 16) & 0xFF, (fmt1.pixelformat >> 24) & 0xFF,
           fmt1.description);
  };

  struct v4l2_format fmt;
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = 640;
  fmt.fmt.pix.height = 480;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
  if (-1 == ioctl(cameraFD, VIDIOC_S_FMT, &fmt)) {
    printf("VIDIOC_S_FMT error\n");
    return -1;
  }
  if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV) {
    printf("pixelformat error\n");
    return -1;
  }

  struct v4l2_requestbuffers req;
  memset(&req, 0, sizeof(req));
  req.count = 4;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if (-1 == ioctl(cameraFD, VIDIOC_REQBUFS, &req)) {
    printf("VIDIOC_REQBUFS\n");
    return -1;
  }
  if (req.count < 2) {
    printf("Insufficient buffer memory\n");
    return -2;
  }
  buffers = calloc(req.count, sizeof(*buffers));
  if (!buffers) {
    printf("calloc buffers failed!\n");
    return -1;
  }
  int n_buffers;
  for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = n_buffers;
    if (-1 == ioctl(cameraFD, VIDIOC_QUERYBUF, &buf)) {
      printf("VIDIOC_QUERYBUF\n");
      return -1;
    }
    buffers[n_buffers].length = buf.length;
    buffers[n_buffers].start = mmap(NULL,  // start anywhere
                                    buf.length, PROT_READ | PROT_WRITE,
                                    MAP_SHARED, cameraFD, buf.m.offset);

    if (MAP_FAILED == buffers[n_buffers].start) {
      printf("mmap error\n");
      return -1;
    }
  }
  int i = 0;
  for (; i < n_buffers; ++i) {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    if (-1 == ioctl(cameraFD, VIDIOC_QBUF, &buf)) {
      printf("VIDIOC_QBUF error\n");
      return -1;
    }
  }
  // 开启摄像头
  enum v4l2_buf_type type;
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == ioctl(cameraFD, VIDIOC_STREAMON, &type)) {
    printf("VIDIOC_STREAMON error\n");
    return -1;
  }
  // 采集循环
  uint32_t count = 1;
  while (count--) {
    for (;;) {
      fd_set fds;
      struct timeval tv;
      int r;
      FD_ZERO(&fds);
      FD_SET(cameraFD, &fds);
      /* timeout */
      tv.tv_sec = 4;
      tv.tv_usec = 0;
      r = select(cameraFD + 1, &fds, NULL, NULL, &tv);
      if (r == -1) {
        if (EINTR == errno) continue;
        return -1;
      }
      if (r == 0) {
        printf("select timeout!\n");
        return -1;
      }
      if (read_frame() == -1)
        return -1;
      else
        break;
    }
  }
  /* stop capture */
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == ioctl(cameraFD, VIDIOC_STREAMOFF, &type)) return -1;
  /* munmap */
  for (i = 0; i < n_buffers; ++i)
    if (-1 == munmap(buffers[i].start, buffers[i].length)) return -1;
  free(buffers);
  /* close device */
  if (-1 == close(cameraFD)) return -1;
}
