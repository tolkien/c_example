#include <stdio.h>
#include <stdlib.h>  // for EXIT_SUCCESS and EXIT_FAILURE
#include "bmp.h"

BMPImage *read_image(const char *filename, char **error);
FILE *_open_file(const char *filename, const char *mode);
void _handle_error(char **error, FILE *fp, BMPImage *image);
void _clean_up(FILE *fp, BMPImage *image, char **error);

int main(void)
{
    char *error = NULL;
    BMPImage *image = read_image("160x160_24bit.bmp", &error);

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

    _clean_up(NULL, image, &error);

    return EXIT_SUCCESS;
}

BMPImage *read_image(const char *filename, char **error)
{
    FILE *input_ptr = _open_file(filename, "rb");

    BMPImage *image = read_bmp(input_ptr, error);
    if (*error != NULL)
    {
        _handle_error(error, input_ptr, image);
    }
    fclose(input_ptr);

    return image;
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
