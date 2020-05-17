#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <pmmintrin.h>
#include <immintrin.h>

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

// Checks if specific bit in interger is set
int is_bit_set(unsigned long num, int bit)
{
    return (num >> bit) & 0x01;
}


// Calculates brightness values with SSE
void brightness_change_sse(bitmap_pixel_hsv_t* pixels, int count, float level)
{
	// Vc: Value current
	// Vn: Value new
	// Vmax: Maximum of Value
	// d: Adjust of value

	// Formular: Vn = Vc + (Vc * ((d - |d|) / 2)) + ((Vmax - Vc) * ((d + |d|) / 2))

	float level_negative = (level - fabs(level)) / 2;
	float level_positive = (level + fabs(level)) / 2;
	float pixel_max = 255.0f;

	float* ptr_ln = &level_negative;
	float* ptr_lp = &level_positive;
	float* ptr_pm = &pixel_max;

	float* save_pixel = (float*)malloc(sizeof(float) * 4);
	float* load_pixel = (float*)malloc(sizeof(float) * 4);
	
	__m128 level_negative_sse = _mm_load_ps1(ptr_ln);
	__m128 level_positive_sse = _mm_load_ps1(ptr_lp);
	__m128 pixel_max_sse = _mm_load_ps1(ptr_pm);

	

	for(int i = 0; i < count/4; i++)
	{
		for(int k = i*4; k < i*4+4; k++)
		{
			bitmap_pixel_hsv_t *pixel = &pixels[k];
			load_pixel[k-4*i] = pixel->v;
		}

		__m128 pixel_sse = _mm_load_ps(load_pixel);
		__m128 result_1_sse = _mm_mul_ps(pixel_sse, level_negative_sse);
		__m128 result_2_sse = _mm_sub_ps(pixel_max_sse, pixel_sse);
		 	   result_2_sse = _mm_mul_ps(result_2_sse, level_positive_sse);
			   result_2_sse = _mm_add_ps(result_2_sse, result_1_sse);
			   result_2_sse = _mm_add_ps(pixel_sse, result_2_sse);
		_mm_store_ps(save_pixel	, result_2_sse);
		
		

		for (int j = i*4; j < i*4+4; j++)
		{
			bitmap_pixel_hsv_t *pixeld = &pixels[j];
			pixeld->v = (bitmap_component_t)save_pixel[j-4*i];
			
		}
	}

	free(load_pixel);
	free(save_pixel);


}

// Calculates brightness values with AVX2
void brightness_change_avx(bitmap_pixel_hsv_t* pixels, int count, float level)
{
	// Vc: Value current
	// Vn: Value new
	// Vmax: Maximum of Value
	// d: Adjust of value

	// Formular: Vn = Vc + (Vc * ((d - |d|) / 2)) + ((Vmax - Vc) * ((d + |d|) / 2))

	

	float level_negative = (level - fabs(level)) / 2;
	float level_positive = (level + fabs(level)) / 2;
	float pixel_max = 255.0f;

	float* ptr_ln = &level_negative;
	float* ptr_lp = &level_positive;
	float* ptr_pm = &pixel_max;

	float* save_pixel;
	posix_memalign((void**)&save_pixel, 32, sizeof(float)*8);

	float* load_pixel;
	posix_memalign((void**)&load_pixel, 32, sizeof(float)*8);
	
	
	__m256 level_negative_avx = _mm256_broadcast_ss(ptr_ln);
	__m256 level_positive_avx = _mm256_broadcast_ss(ptr_lp);
	__m256 pixel_max_avx= _mm256_broadcast_ss(ptr_pm);

	

	for(int i = 0; i < count/8; i++)
	{
		
		for(int k = i*8; k < i*8+8  ; k++)
		{
			bitmap_pixel_hsv_t *pixel = &pixels[k];
			load_pixel[k-8*i] = pixel->v;
			
			
		}
		 
		__m256 pixel_avx = _mm256_load_ps(load_pixel);
		__m256 result = _mm256_sub_ps(pixel_max_avx, pixel_avx);
			   result = _mm256_mul_ps(result, level_positive_avx);
			   result = _mm256_fmadd_ps(pixel_avx,level_negative_avx, result);
			   result = _mm256_add_ps(pixel_avx, result);
 
		_mm256_store_ps(save_pixel, result);
					   

		for (int j = i*8; j < i*8+8; j++)
		{
			bitmap_pixel_hsv_t *pixeld = &pixels[j];
			pixeld->v = (bitmap_component_t)save_pixel[j-8*i];				
		}
	}

	free(load_pixel);
	free(save_pixel);


}

// Checks for AVX2 with assembler and cpuid
int check_avx2()
{
	unsigned long rax, rbx, rcx, rdx;

	__asm__ __volatile__(
		".intel_syntax noprefix	\n"
		"mov rax, 7				\n"
		"mov rcx, 0				\n"
		"cpuid					\n"
		".att_syntax prefix		\n"
		: "=a" (rax), "=b" (rbx), "=c" (rcx), "=d" (rdx)
	);

	if(is_bit_set(rbx,5) == 1)
	{
		return 1;
	}
	return 0;
}



// Get value from brightness option and return a valid value
float get_brightness(struct _arguments *arguments)
{
	float brightness_value;

	if(arguments->brightness_adjust != 0x0)
	{
		// Compare string length with position of non numeric value
		if(strlen(arguments->brightness_adjust) != strspn(arguments->brightness_adjust, "0123456789-+."))
		{
			print_red("No valid digit!");
			exit(-1);
		}


		brightness_value = (float)atof(arguments->brightness_adjust);
		printf("Value: %.2f\n",brightness_value);

		// Checks the brightness range
		if(brightness_value < -1.0f || brightness_value > 1.0f )
		{
			print_red("Brightness must be -1.0 to 1.0!");
			exit(-1);
		}
		
	}
	return brightness_value;
}

// Error Handling
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
			print_red("IO Error!");
			exit(-1);

		case BITMAP_ERROR_MEMORY:
			print_red("Memory Error!");
			exit(-1);

		case BITMAP_ERROR_FILE_EXISTS:
			print_red("File already exists!");
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
	int width, height;
	float brightness;


	error = bitmapReadPixels(arguments.input_path, (bitmap_pixel_t**)&pixels, &width, &height, BITMAP_COLOR_SPACE_HSV);
	error_check(error);
	

	brightness = get_brightness(&arguments);

	// Checks if AVX2 is supported
	if(check_avx2() == 1){
		printf("Calculate with AVX2\n");
		brightness_change_avx(pixels, width * height, brightness);
	}
	else{
		printf("AVX2 not supported, use instead SSE\n");
		brightness_change_sse(pixels, width * height, brightness);
	}

	

	// Checks for output set by user
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
	



	//Free the pixel array:
	free(pixels);

	return 0;
}
