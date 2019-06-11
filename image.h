#ifndef __RCARH3_DRM_IMAGE_H__
#define __RCARH3_DRM_IMAGE_H__

typedef struct image {
  unsigned int   width;
  unsigned int   height;
  unsigned int   bpp; /* 2:RGB16, 3:RGB, 4:RGBA */
  unsigned char  data[110 * 110 * 4];
} SAFETY_IMAGE;

#endif /* __RCARH3_DRM_IMAGE_H__ */
