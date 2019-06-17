#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "image.h"
#include "batteryImage.inl"
#include "breakImage.inl"
#include "hazardImage.inl"
#include "engineImage.inl"
#include "oilImage.inl"
#include "warningImage.inl"
#include "leftImage.inl"
#include "rightImage.inl"

#define DOC0EXPD0(idx)             (0x3 << ((7 - (idx))*2))
#define DOC0EXPDk(idx,n)           (((n) & 0x3) << ((7 - (idx))*2))
#define DOC0EXPD(idx,n)            ((DOC0EXPD0(idx) & (n)) >> ((7 - (idx))*2))
			/* VOM 0 area reference color 0: red upper limit */
			/* VOM 0 area reference color 0: red upper limit */
#define DOC0M0CFG4_VOCAnM0RUP0     (0xf << 24)
#define DOC0M0CFG4_VOCAnMmRUP0(n)  (((n) & 0xf) << 24)
			/* VOM 0 area reference color 0: green upper limit */
#define DOC0M0CFG4_VOCAnM0GUP0     (0xf << 20)
#define DOC0M0CFG4_VOCAnMmGUP0(n)  (((n) & 0xf) << 20)
			/* VOM 0 area reference color 0: blue upper limit */
#define DOC0M0CFG4_VOCAnM0BUP0     (0xf << 16)
#define DOC0M0CFG4_VOCAnMmBUP0(n)  (((n) & 0xf) << 16)
			/* VOM 0 area reference color 0: red lower limit */
#define DOC0M0CFG4_VOCAnM0RLO0     (0xf << 8)
#define DOC0M0CFG4_VOCAnMmRLO0(n)  (((n) & 0xf) << 8)
			/* VOM 0 area reference color 0: green lower limit */
#define DOC0M0CFG4_VOCAnM0GLO0     (0xf << 4)
#define DOC0M0CFG4_VOCAnMmGLO0(n)  (((n) & 0xf) << 4)
			/* VOM 0 area reference color 0: blue lower limit */
#define DOC0M0CFG4_VOCAnM0BLO0     (0xf)
#define DOC0M0CFG4_VOCAnMmBLO0(n)  ((n) & 0xf)

FILE *_open_file(const char *filename, const char *mode);
void _handle_error(char **error, FILE *fp);
void _clean_up(FILE *fp, char **error);

struct image_map_t {
	unsigned char *name;
	SAFETY_IMAGE *src;
};
struct image_map_t img_map[] = {
	{"left",	&leftImage},
	{"battery",	&batteryImage},
	{"break",	&breakImage},
	{"hazard",	&hazardImage},
	{"engine",	&engineImage},
	{"oid",		&oilImage},
	{"warning",	&warningImage},
	{"right",	&rightImage}
};
#define IMG_MAP_SIZE	(sizeof(img_map)/sizeof(img_map[0]))

static void usage(char *progname)
{
    int i;

    printf("\n"
	"Usage: %s [options]\n"
	"\n"
	"Valid options:\n"
	"    -h          : display help\n"
	"    -v          : verbose output\n"
	"    -l <log>    : print out the map to file <log>\n"
	"    -o <output> : print out the EXP_buffer header to file <output>\n"
	"    -t <type>   : specify image type (default: %s)\n",
	progname, img_map[0].name);
    for(i=0; i < IMG_MAP_SIZE; i++)
	printf(	"                      %s\n", img_map[i].name);
    exit(0);
}

void write_img_map(const char *filename, char **error,
		unsigned long int *EXP_buffer, int cnt, int width, int size)
{
    FILE *output_ptr = _open_file(filename, "wb");
    int i, j, k;

    k=0;
    for(i=0; i < cnt + 1; i++) {
	for(j=0; j < 8; j++) {
	    if (fprintf(output_ptr, "%ld", DOC0EXPD(j,EXP_buffer[i])) < 0)
		_handle_error(error, output_ptr);
	    if (k >= size) {
		if (fprintf(output_ptr, "\n") < 0)
		    _handle_error(error, output_ptr);
		break;
	    } else if (k % width == (width-1)) {
		if (fprintf(output_ptr, "\n") < 0)
		    _handle_error(error, output_ptr);
	    }
	    k++;
	}
    }

    fclose(output_ptr);
}

void write_header_file(const char *filename, char **error,
		unsigned long int *EXP_buffer, int cnt)
{
    FILE *output_ptr = _open_file(filename, "wb");
    int i;

    for(i=0; i < cnt + 1; i++) {
	if (fprintf(output_ptr, "0x%04lx, ", EXP_buffer[i]) < 0)
	    _handle_error(error, output_ptr);
	if (i % 16 == 15) {
	    if (fprintf(output_ptr, "\n") < 0)
		_handle_error(error, output_ptr);
	}
    }
    if (fprintf(output_ptr, "\n") < 0)
	_handle_error(error, output_ptr);

    fclose(output_ptr);
}

/*
 * Open file. In case of error, print message and exit.
 */
FILE *_open_file(const char *filename, const char *mode)
{
    FILE *fp = fopen(filename, mode);
    if (fp == NULL)
    {
        fprintf(stderr, "Could not open file %s", filename);

        exit(EXIT_FAILURE);
    } 

    return fp;
}

/*
 * Print error message and clean up resources.
 */
void _handle_error(char **error, FILE *fp)
{
    fprintf(stderr, "ERROR: %s\n", *error);
    _clean_up(fp, error);

    exit(EXIT_FAILURE);
}

/*
 * Close file and release memory.
 */
void _clean_up(FILE *fp, char **error)
{
    if (fp != NULL)
    {
        fclose(fp);
    }    
    free(*error);
}

int main(int argc, char *argv[])
{
    char *error = NULL;
    SAFETY_IMAGE *src = NULL;
    int i, j, k, cnt, size;
    int r[256], g[256], b[256];
    int r2, g2, b2, a2;
#define CLUT_SIZE	8
    int CLUT[CLUT_SIZE][3];
    unsigned long int EXP_buffer[4096];

    int opt, verbose;
    char *log_file, *h_file, img_no;

    log_file = NULL;
    h_file = NULL;
    img_no = 0;
    while (1) {
	opt = getopt(argc, argv, "hvl:o:t:");
	if (opt == -1)
	    break;

	switch (opt) {
	    case 'v':
		verbose = 1;
		break;
	    case 'l':
		log_file = optarg;
		break;
	    case 'o':
		h_file = optarg;
		break;
	    case 't':
		img_no = atoi(optarg);
		break;
	    case 'h':
	    default:
		usage(argv[0]);
		break;
	}
    }
#if 0
    if (optind != argc-1)
	usage(argv[0]);
#endif
    if (img_no >= IMG_MAP_SIZE)
	img_no = 0;
    src = img_map[img_no].src;

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

    memset(r, 0x0, sizeof(r));
    memset(g, 0x0, sizeof(g));
    memset(b, 0x0, sizeof(b));

    memset(CLUT, 0x0, sizeof(CLUT));
    memset(EXP_buffer, 0x0, sizeof(EXP_buffer));

    k = 0;
    cnt = 0;
    for(i=0; i < size; i += 4) {
	r2 = src->data[i];	g2 = src->data[i+1];	b2 = src->data[i+2];
	r[r2] += 1;		g[g2] += 1;		b[b2] += 1;

	/* convert RGBA to RGB444 */
	a2 = src->data[i+3] / 16;
	r2 /= 16;		g2 /= 16;		b2 /= 16;

	if ((r2 + g2 + b2 == 0) || (a2 < 0xF)) {
	    /* do nothing
	    EXP_buffer[cnt] |= DOC0EXPDk(k,0);
	    */
	} else {
	    for(j=1; j < CLUT_SIZE; j++) {
		if ((CLUT[j][0] == r2) &&
		    (CLUT[j][1] == g2) &&
		    (CLUT[j][2] == b2)) {
			EXP_buffer[cnt] |= DOC0EXPDk(k,j);
			break;
		}
		if (CLUT[j][0] + CLUT[j][1] + CLUT[j][2] > 0) {
		    continue;
		}
		if (verbose)
		printf("%d, %4d : CLUT[%d] %02x, %02x, %02x != %02x, %02x, %02x/%d, %d, %d\n",
	    			i, CLUT[j][0] + CLUT[j][1] + CLUT[j][2],
				j, CLUT[j][0], CLUT[j][1], CLUT[j][2],
				r2, g2, b2,
				(CLUT[j][0] == r2), (CLUT[j][1] == g2), (CLUT[j][2] == b2));
		CLUT[j][0] = r2;	CLUT[j][1] = g2;	CLUT[j][2] = b2;
		EXP_buffer[cnt] |= DOC0EXPDk(k,j);
		if (verbose)
		printf("CLUT[%d] is registered (%d)\n", j, i);
		break;
	    }
	    if (j >= CLUT_SIZE) {
		printf("number of color pallet is greater than 4\n");
		for(j=1; j < CLUT_SIZE; j++) {
		    if (CLUT[j][0] + CLUT[j][1] + CLUT[j][2] > 0)
			printf("CLUT[%d] %02x, %02x, %02x\n",
				j, CLUT[j][0], CLUT[j][1], CLUT[j][2]);
    		}
    		printf("count %d\n", i);
		_clean_up(NULL, &error);
		return EXIT_FAILURE;
	    }
	}

	k++;
	if (k >= 8) {
	    cnt++;
	    k = 0;
	}
    }

    for(i=0; i < 256; i++) {
	if ((r[i] + g[i] + b[i]) == 0)
	    continue;
	if (r[i] != 0)
	    printf("R[%02x] %5d", i, r[i]);
	else
	    printf("           ");
	if (g[i] != 0)
	    printf(" G[%02x] %5d", i, g[i]);
	else
	    printf("            ");
	if (b[i] != 0)
	    printf(" R[%02x] %5d\n", i, b[i]);
	else
	    printf("            \n");
    }

    printf("CLUT[%d] %02x, %02x, %02x\n",
				0, CLUT[0][0], CLUT[0][1], CLUT[0][2]);
    for(j=1; j < CLUT_SIZE; j++) {
	if (CLUT[j][0] + CLUT[j][1] + CLUT[j][2] > 0)
	    printf("CLUT[%d] %02x, %02x, %02x\n",
				j, CLUT[j][0], CLUT[j][1], CLUT[j][2]);
    }
    printf("size of EXP_buffer: %d.%d\n", cnt, k);
    if (h_file != NULL)
	write_header_file(h_file, &error,  EXP_buffer, cnt);
    if (log_file != NULL)
	write_img_map(log_file, &error,  EXP_buffer, cnt, src->width, size);

    _clean_up(NULL, &error);

    return EXIT_SUCCESS;
}
