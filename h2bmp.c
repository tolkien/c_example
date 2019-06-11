#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leftImage.h"
#include "bmp.h"

bool _check(bool condition, char **error, const char *error_message);
BMPImage *create_bmp_from_h(char **error);

void write_image(const char *filename, BMPImage *image, char **error);
FILE *_open_file(const char *filename, const char *mode);
void _handle_error(char **error, FILE *fp, BMPImage *image);
void _clean_up(FILE *fp, BMPImage *image, char **error);

int main(void)
{
    char *error = NULL;
    BMPImage *image = create_bmp_from_h(&error);

    printf("Magic identifier: 0x%x (==0x4d42)\n", image->header.type);
    printf("File size in bytes: %d\n", image->header.size);
    printf("Offset to image data in bytes from beginning of file: %d\n", image->header.offset);
    printf("DIB Header size in bytes: %d\n", image->header.dib_header_size);
    printf("Width of the image: %d\n", image->header.width_px);
    printf("Height of image: %d\n", image->header.height_px);
    printf("Number of color planes: %d\n", image->header.num_planes);
    printf("Bits per pixel: %d\n", image->header.bits_per_pixel);
    printf("Compression type: %d\n", image->header.compression);
    printf("Image size in bytes: %d\n", image->header.image_size_bytes);
    printf("Pixels per meter: %d\n", image->header.x_resolution_ppm);
    printf("Pixels per meter: %d\n", image->header.y_resolution_ppm);
    printf("Number of colors: %d\n", image->header.num_colors);
    printf("Important colors: %d\n", image->header.important_colors);

    write_image("copy.bmp", image, &error);
    _clean_up(NULL, image, &error);

    return EXIT_SUCCESS;
}

/*
 * Create a BMP image from c header file.
 * 
 * https://en.wikipedia.org/wiki/BMP_file_format
 * 
 * - Return: the image as a BMPImage on the heap.
 */
BMPImage *create_bmp_from_h(char **error)
{    
    BMPImage *image = malloc(sizeof(*image));
    if (!_check(image != NULL, error, "Not enough memory"))
    {
        return NULL;
    }

    memset(image, 0x0, sizeof(*image));

    SAFETY_IMAGE *src = &leftImage;
    image->header.type = 0x4d42;
    image->header.size = sizeof(BMPHeader);	// File size in bytes
    image->header.offset = sizeof(BMPHeader);
    image->header.dib_header_size = 40;
    image->header.width_px = src->width;
    image->header.height_px = src->height;
    image->header.num_planes = 1;
    switch (src->bpp) {
    case 2:	// RGB16
	image->header.bits_per_pixel = 16;	break;
    case 3:	// RGB
	image->header.bits_per_pixel = 24;	break;
    case 4:	// RBGA
	image->header.bits_per_pixel = 32;
	image->header.compression = 3;
	break;
    default:
        image->header.bits_per_pixel = 24;
    }
    image->header.image_size_bytes = sizeof(*src) - 3*sizeof(unsigned int);
    image->header.size += image->header.image_size_bytes;

    // Allocate memory for image data
    image->data = malloc(sizeof(*image->data) * image->header.image_size_bytes);
    if (!_check(image->data != NULL, error, "Not enough memory"))
    {
        return NULL;
    }
    // Read image data
    memcpy(image->data, src->data, image->header.image_size_bytes);

    return image;
}

void write_image(const char *filename, BMPImage *image, char **error)
{
    FILE *output_ptr = _open_file(filename, "wb");

    if (!write_bmp(output_ptr, image, error))
    {
        _handle_error(error, output_ptr, image);
    }    
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
void _handle_error(char **error, FILE *fp, BMPImage *image)
{
    fprintf(stderr, "ERROR: %s\n", *error);
    _clean_up(fp, image, error);

    exit(EXIT_FAILURE);
}

/*
 * Close file and release memory.
 */
void _clean_up(FILE *fp, BMPImage *image, char **error)
{
    if (fp != NULL)
    {
        fclose(fp);
    }    
    free_bmp(image);
    free(*error);
}
