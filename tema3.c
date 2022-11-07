#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "bmp_header.h"
#define DIM_CMD 100
#define DIM_TYPE 10


typedef struct {
    char blue;
    char green;
    char red;
}pixel;

typedef struct {
    bmp_fileheader *file_header;
    bmp_infoheader *info_header;
    pixel **bitmap;
}bmp;

// salvarea imaginii
void save(char* fileName, bmp *image) {
    FILE *filePtr = fopen(fileName, "wb");

    fwrite(image->file_header, 1, sizeof(bmp_fileheader), filePtr);
    fwrite(image->info_header, 1, sizeof(bmp_infoheader), filePtr);

    int height = image->info_header->height;
    int width = image->info_header->width;

    int offset = width % 4;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++)
            fwrite(&image->bitmap[i][j], 1, sizeof(pixel), filePtr);
        if (offset != 0)
            fwrite(&image->bitmap[i][width], 1, offset, filePtr);
    }

    fclose(filePtr);
}
// incarcarea imaginii in memorie pentru editare
bmp* edit(char* fileName) {
    char *path = malloc(DIM_TYPE);
    FILE *filePtr = fopen(fileName, "rb");

    free(path);

    bmp *image = malloc(sizeof(bmp));

    image->file_header = malloc(sizeof(bmp_fileheader));
    image->info_header = malloc(sizeof(bmp_infoheader));

    fread(image->file_header, 1, sizeof(bmp_fileheader), filePtr);
    fread(image->info_header, 1, sizeof(bmp_infoheader), filePtr);

    int height = image->info_header->height;
    int width = image->info_header->width;

    int offset = width % 4;

    image->bitmap = malloc(height * sizeof(pixel*));

    for (int i = 0; i < height; i++)
        image->bitmap[i] = malloc((width + (offset != 0)) * sizeof(pixel));


    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++)
            fread(&image->bitmap[i][j], 1, sizeof(pixel), filePtr);
        if (offset != 0)
            fread(&image->bitmap[i][width], 1, offset, filePtr);
    }

    fclose(filePtr);
    return image;
}

// inserarea unei noi imagini (overlap) peste cea din memorie de la pixelul de pozitie[x][y]
void insert(bmp* image, bmp* overlap_image, int y, int x) {
    int min_height = 0, min_width = 0;
    int height1 = image->info_header->height;
    int height2 = overlap_image->info_header->height;
    int width1 = image->info_header->width;
    int width2 = overlap_image->info_header->width;

    if (height1 - x < height2) {
        min_height = height1 - x;
    } else {
        min_height = height2;
    }

    if (width1 - y < width2) {
        min_width = width1 - y;
    } else {
        min_width = width2;
    }

    for (int i = 0; i < min_height; i++)
        for (int j = 0; j < min_width; j++) {
            image->bitmap[i + x][j + y] = overlap_image->bitmap[i][j];
        }
    free(overlap_image);
}

// desenarea unui punct de grosime size
void draw_point(pixel **bitmap, int y, int x, int size, pixel color, int max_height, int max_width) {
    if (!(x >= 0 && y >= 0 && x < max_height && y < max_width))
        return;
    for (int i = x - size/2; i <= x + size/2; i++)
        for (int j = y - size/2; j <= y + size/2; j++)
            if (i >= 0 && j >= 0 && i < max_height && j < max_width)
                bitmap[i][j] = color;
}

// desenarea unei linii intre punctele (x1,y1) si (x2,y2)
void draw_line(pixel **bitmap, int y1, int x1, int y2, int x2, int size, pixel color, int max_height, int max_width) {
    int min_x = x1 * (x1 < x2) + x2 * (x1 >= x2);
    int max_x = x1 + x2 - min_x;
    int min_y = y1 * (y1 < y2) + y2 * (y1 >= y2);
    int max_y = y1 + y2 - min_y;
    draw_point(bitmap, y1, x1, size, color, max_height, max_width);
    draw_point(bitmap, y2, x2, size, color, max_height, max_width);
    if (y1 == y2) {
        for (int j = min_x; j <= max_x; j++)
            draw_point(bitmap, y1, j, size, color, max_height, max_width);
        return;
    }
    if (x1 == x2) {
        for (int i = min_y; i <= max_y; i++)
            draw_point(bitmap, i, x1, size, color, max_height, max_width);
        return;
    }
    int x_length = max_x - min_x;
    int y_length = max_y - min_y;
    if (x_length < y_length) {
        int aux_x = 0;
        for (int i = min_y; i <= max_y; i++) {
            aux_x = ((i - y1) * (x2 - x1) + x1 * (y2 - y1)) / (y2 - y1);
            draw_point(bitmap, i, aux_x, size, color, max_height, max_width);
        }
        return;
    } else if (x_length >= y_length) {
        int aux_y = 0;
        for (int i = min_x; i <= max_x; i++) {
            aux_y = ((i - x1) * (y2 - y1) + y1 * (x2 - x1)) / (x2 - x1);
            draw_point(bitmap, aux_y, i, size, color, max_height, max_width);
        }
        return;
    }
}

void prep_line(pixel **bitmap, int size, pixel color, int max_height, int max_width) {
    int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    y1 = atoi(strtok(NULL, " "));
    x1 = atoi(strtok(NULL, " "));
    y2 = atoi(strtok(NULL, " "));
    x2 = atoi(strtok(NULL, " "));
    draw_line(bitmap, y1, x1, y2, x2, size, color, max_height, max_width);
}

// desenarea unui dreptunghi
void draw_rectangle(pixel **bitmap, int size, pixel color, int max_height, int max_width) {
    int y = atoi(strtok(NULL, " "));
    int x = atoi(strtok(NULL, " "));
    int width = atoi(strtok(NULL, " "));
    int height = atoi(strtok(NULL, " "));

    draw_line(bitmap, y, x, y + width, x, size, color, max_height, max_width);
    draw_line(bitmap, y, x, y, x + height, size, color, max_height, max_width);
    draw_line(bitmap, y + width, x, y + width, x + height, size, color, max_height, max_width);
    draw_line(bitmap, y, x + height, y + width, x + height, size, color, max_height, max_width);
}

// desenarea unui triunghi
void draw_triangle(pixel **bitmap, int size, pixel color, int max_height, int max_width) {
    int y1 = atoi(strtok(NULL, " "));
    int x1 = atoi(strtok(NULL, " "));
    int y2 = atoi(strtok(NULL, " "));
    int x2 = atoi(strtok(NULL, " "));
    int y3 = atoi(strtok(NULL, " "));
    int x3 = atoi(strtok(NULL, " "));

    draw_line(bitmap, y1, x1, y2, x2, size, color, max_height, max_width);
    draw_line(bitmap, y1, x1, y3, x3, size, color, max_height, max_width);
    draw_line(bitmap, y2, x2, y3, x3, size, color, max_height, max_width);
}

// verificare daca pixelul nostru are aceeasi culoare cu variabila color
int check(pixel **bitmap, int y, int x, pixel color) {
    return (bitmap[x][y].blue != color.blue) + (bitmap[x][y].green != color.green) + (bitmap[x][y].red != color.red);
}

// algoritm recursiv de flood fill
void fill(pixel **bitmap, int y, int x, pixel prevCol, pixel newCol, int max_height, int max_width) {
    if (x < 0 || x >= max_height || y < 0 || y >= max_width)
        return;
    if (check(bitmap, y, x, prevCol) != 0)
        return;
    if (check(bitmap, y, x, newCol) == 0)
        return;

    bitmap[x][y] = newCol;

    fill(bitmap, y, x + 1, prevCol, newCol, max_height, max_width);
    fill(bitmap, y, x - 1, prevCol, newCol, max_height, max_width);
    fill(bitmap, y + 1, x, prevCol, newCol, max_height, max_width);
    fill(bitmap, y - 1, x, prevCol, newCol, max_height, max_width);
}

// tratarea tipurilor de comenzi ale editorului de imagini
int main() {
    char* cmd = malloc(DIM_CMD * sizeof(char));
    char* type = NULL;
    bmp *image = NULL;
    pixel color;
    int line_size = 1;

    do {
        fgets(cmd, DIM_CMD, stdin);
        int len = (int)(strlen(cmd) - 1);
        if (cmd[len] == '\n')
            cmd[len] = '\0';

        type = strtok(cmd, " ");
        if (strcmp(type, "save") == 0) {
            save(strtok(NULL, " "), image);
        } else if (strcmp(type, "edit") == 0) {
            image = edit(strtok(NULL, " "));
        } else if (strcmp(type, "insert") == 0) {
            int y = 0, x = 0;
            bmp *overlap_image = NULL;
            overlap_image = edit(strtok(NULL, " "));

            y = atoi(strtok(NULL, " "));
            x = atoi(strtok(NULL, " "));
            insert(image, overlap_image, y, x);
        } else if (strcmp(type, "set") == 0) {
            if (strcmp(strtok(NULL, " "), "draw_color") == 0) {
                color.red = (char)atoi(strtok(NULL, " "));
                color.green = (char)atoi(strtok(NULL, " "));
                color.blue = (char)atoi(strtok(NULL, " "));
            } else {
                line_size = atoi(strtok(NULL, " "));
            }
        } else if (strcmp(type, "draw") == 0) {
                char *type = NULL;
                type = strtok(NULL, " ");
                if (strcmp(type, "line") == 0) {
                    prep_line(image->bitmap, line_size, color, image->info_header->height, image->info_header->width);
                } else if (strcmp(type, "rectangle") == 0) {
                    int height = image->info_header->height;
                    int width = image->info_header->width;
                    draw_rectangle(image->bitmap, line_size, color, height, width);
                } else {
                    if (strcmp(type, "triangle") == 0) {
                        int height = image->info_header->height;
                        int width = image->info_header->width;
                        draw_triangle(image->bitmap, line_size, color, height, width);
                }
            }
        } else {
            if (strcmp(type, "fill") == 0) {
                int y = atoi(strtok(NULL, " "));
                int x = atoi(strtok(NULL, " "));
                int height = image->info_header->height;
                int width = image->info_header->width;
                fill(image->bitmap, y, x, image->bitmap[x][y], color, height, width);
            }
        }
    } while (strcmp(cmd, "quit") != 0);

    free(cmd);
    free(image);
    return 0;
}
