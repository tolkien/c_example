#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "leftImage.inl"
#include "batteryImage.inl"

int main(int argc, char *argv[])
{
    SAFETY_IMAGE *src = &leftImage;
    int i, j, k, size;
    int r2, g2, b2, a2;
    unsigned char ch[] = "0123456789ABCDEF";
    unsigned char ch0[] = ".+#=WXYZMNOPQRST";

    if (argc < 2) {
        src = &leftImage;
    } else {
	switch (argv[1][0]) {
	case '1':
            src = &batteryImage;	break;
	case '0':
	default:
            src = &leftImage;
	}
    }

    printf("Width of the image: %d\n", src->width);
    printf("Height of image: %d\n", src->height);
    printf("Number of pixels: %d\n", src->width * src->height);
    switch (src->bpp) {
    case 2:	// RGB16
	printf("Image format: RGB16\n");	break;
    case 3:	// RGB
	printf("Image format: RGB\n");		break;
    case 4:	// RBGA
	printf("Image format: RGBA\n");		break;
    default:
	printf("Image format: unknown(%d)\n", src->bpp);
    }
    size = src->width * src->height * 4;

    for(i=0; i < 16; i++)
	printf("%c", ch[i]);
    printf("\n");

    k = 0;
    for(i=0; i < size; i += 4) {
	a2 = src->data[i+3];
	j = a2 / 16;
	if (j == 0) {
		printf("%c", ch0[a2]);
	} else
		printf("%c", ch[j]);
	if (k % src->width == (src->width - 1)) {
		printf("\n");
	}
	k++;
    }

    return EXIT_SUCCESS;
}
