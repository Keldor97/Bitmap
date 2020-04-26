#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "bitmap.h"
#include "arg_parse.h"

int min(int a, int b)
{
	return (a < b) ? a : b;
}

int max(int a, int b)
{
	return (a > b) ? a : b;
}



void print_red(const char* input)
{
	printf("\033[1;31m");
	printf("%s\n",input);
	printf("\033[0m");
}



void brightness_change(bitmap_pixel_hsv_t* pixels, int count, int brightness)
{

	for(int i = 0; i < count; i += 1)
	{
		bitmap_pixel_hsv_t* pixel = &pixels[i];

		pixel->v = min(255, max(0 , pixel->v + brightness));

	}

}


// Get value from brightness option and return a valid value
int get_brightness(struct _arguments *arguments)
{
	if(arguments->brightness_adjust != 0x0)
	{
		// Compare string length with position of non numeric value
		if(strlen(arguments->brightness_adjust) != strspn(arguments->brightness_adjust, "0123456789-+"))
		{
			print_red("No valid digit!");
			exit(-1);
		}



		int brightness_value = atoi(arguments->brightness_adjust);
		printf("Value: %d\n",brightness_value);

		// Checks the brightness range
		if(brightness_value < -100 || brightness_value > 100 )
		{
			print_red("Brightness must be -100 to 100!");
			exit(-1);
		}

		return brightness_value;
	}
}

void error_check(int error)
{
	switch (error)
	{
		case BITMAP_ERROR_SUCCESS:
			break;

		case BITMAP_ERROR_INVALID_PATH:
			print_red("Invalid Path!");
			exit(-1);

		case BITMAP_ERROR_INVALID_FILE_FORMAT:
			print_red("File is not a bitmap!");
			exit(-1);

		case BITMAP_ERROR_IO:
			exit(-1);

		case BITMAP_ERROR_MEMORY:
			exit(-1);

		case BITMAP_ERROR_FILE_EXISTS:
			exit(-1);
	}
}


int main(int argc, char** argv)
{

	// Holds the input values
	struct _arguments arguments = { 0 };

	 // Parse the argv input
	argp_parse(&argp, argc, argv, 0, 0, &arguments);


	//Read bitmap pixels:
	bitmap_error_t  error;
	bitmap_pixel_hsv_t* pixels;
	int width, height, brightness;



	error = bitmapReadPixels(arguments.input_path, (bitmap_pixel_t**)&pixels, &width, &height, BITMAP_COLOR_SPACE_HSV);
	error_check(error);
	//assert(error == BITMAP_ERROR_SUCCESS);


	brightness = get_brightness(&arguments);

	brightness_change(pixels, width * height, brightness);


	// Checks for output sets by user
	if(arguments.output == 0x0)
	{
		arguments.output = strtok(arguments.input_path, ".");
		arguments.output = strcat(arguments.output, "_changed.bmp"); //TODO: dark , light depends -b '+' or -b '-'
	}


	//Write bitmap pixels:
	bitmap_parameters_t parameters =
	{

		.bottomUp = BITMAP_BOOL_TRUE,
		.widthPx = width,
		.heightPx = height,
		.colorDepth = BITMAP_COLOR_DEPTH_24,
		.compression = BITMAP_COMPRESSION_NONE,
		.dibHeaderFormat = BITMAP_DIB_HEADER_INFO,
		.colorSpace = BITMAP_COLOR_SPACE_HSV

	};


	error = bitmapWritePixels(arguments.output,BITMAP_BOOL_TRUE, &parameters, (bitmap_pixel_t*)pixels);
	error_check(error);
	//assert(error == BITMAP_ERROR_SUCCESS);



	//Free the pixel array:
	free(pixels);

	return 0;
}
