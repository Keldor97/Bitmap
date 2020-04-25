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

double min_d(double a, double b)
{
	return (a < b) ? a : b;
}

double max_d(double a, double b)
{
	return (a > b) ? a : b;
}

void print_red(const char* input)
{
	printf("\033[1;31m");
	printf("%s\n",input);
	printf("\033[0m");
}


/*
bitmap_component_t bin(bitmap_component_t val, int bin_count)
{
	double binned (round((double)val / bin_count) * bin_count);
	return (bitmap_component_t)max(0.0, min(binned, 255.0));
}
*/

/*
void greyscale(bitmap_pixel_rgb_t* pixels, int count)
{
	float value;
	for(int i = 0; i < count; i += 1)
	{
		bitmap_pixel_rgb_t* pixel = &pixels[i];
		value = 0.0f;
		value = 0.3f*pixel->r + 0.59f*pixel->g + 0.11f*pixel->b;

			pixel->r = value;
			pixel->g= value;
			pixel->b = value;

	}


}
*/

/*
void tontrennung(bitmap_pixel_rgb_t* pixels, int count)
{
	float value;
	for(int i = 0; i < count; i += 1)
	{
		bitmap_pixel_rgb_t* pixel = &pixels[i];

		bitmap_component_t pix = (bitmap_component_t)round((double)pixel->r + (double)pixel->g + (double)pixel->b) / 3.0f);

		pix = bin(pix, 150);

		pixel->r = pix;
		pixel->g = pix;
		pixel->b = pix;


		//value = 0.0f;
		//value = 0.3f*pixel->r + 0.59f*pixel->g + 0.11f*pixel->b;








}
}
*/


void manipulate(bitmap_pixel_rgb_t* pixels, int count)
{
	float value;
	for(int i = 0; i < count; i += 1)
	{
		bitmap_pixel_rgb_t* pixel = &pixels[i];

		//pixel->r (bitmap_component_t)min((int)pixel->r + 70 ,255);
		//pixel->g (bitmap_component_t)min((int)pixel->g + 70 ,255);
		//pixel->b(bitmap_component_t)min((int)pixel->b + 70 ,255);


		value = 0.0f;
		value = 0.3f*pixel->r + 0.59f*pixel->g + 0.11f*pixel->b;





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


int main(int argc, char** argv)
{

	// Holds the input values
	struct _arguments arguments = { 0 };

	 // Parse the argv input
	argp_parse(&argp, argc, argv, 0, 0, &arguments);



	//Read bitmap pixels:
	bitmap_error_t  error;

	bitmap_pixel_rgb_t* pixels;
	int width, height;


	error = bitmapReadPixels(arguments.input_path, (bitmap_pixel_t**)&pixels, &width, &height, BITMAP_COLOR_SPACE_RGB);

	assert(error == BITMAP_ERROR_SUCCESS);

	printf("Bitmap size: %d x %d\n",width,height);

	//Manipulate bitmap pixels:
	//manipulate(pixels, width * height);
	//greyscale(pixels, width * height);
	//tontrennung(pixels, width * height);







	int brightness = get_brightness(&arguments);


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
		.colorSpace = BITMAP_COLOR_SPACE_RGB

	};


	error = bitmapWritePixels(arguments.output,BITMAP_BOOL_TRUE, &parameters, (bitmap_pixel_t*)pixels);
	assert(error == BITMAP_ERROR_SUCCESS);

	printf("Bitmap has been writen\n");

	//Free the pixel array:
	free(pixels);

	return 0;
}
