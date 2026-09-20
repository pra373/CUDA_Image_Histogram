#include<iostream>
#include<direct.h>
#include<time.h>
#define STB_IMAGE_IMPLEMENTATION
#include"../include/stb_image.h"

using std::cout;
using std::endl;
using std::cin;

bool isHistogramCorrect(int totalPixelCount, int* histogram, int sizeOfHistogram);

int main(void)
{
	unsigned char* imageData;
	int width, height, channels;

	clock_t start, end;
	FILE* logFile;

	// buffers for histograms
	int redHistogram[256] = { 0 };
	int greenHistogram[256] = { 0 };
	int blueHistogram[256] = { 0 };

	logFile = fopen("../logs/log.txt", "w");
	
	if (!logFile)
	{
		cout << "failed to open log file to write application logs !" << endl;
		getchar();
		return(EXIT_FAILURE);
	}

	imageData = stbi_load("../resources/mars_8k.jpg", &width, &height, &channels, 3);

	if (!imageData)
	{
		fprintf(logFile, "Failed to load image!\n");
		fprintf(logFile, "Reason: %s\n", stbi_failure_reason());
		exit(EXIT_FAILURE);
	}

	fprintf(logFile, "Width of image: %d\n", width);
	fprintf(logFile, "Height of image: %d\n", height);
	fprintf(logFile, "channels in image: %d\n", channels);

	int totalPixelsInImage = width * height;
	int totalImageArraySize = totalPixelsInImage * 3;

	start = clock();

	for (int i = 0; i < totalImageArraySize; i = i + 3)
	{
		int R = imageData[i];
		int G = imageData[i + 1];
		int B = imageData[i + 2];

		//calculate Red histogram
		redHistogram[R]++;

		//calculate green histogram
		greenHistogram[G]++;

		//calculate blue histogram
		blueHistogram[B]++;

	}

	end = clock();

	double TotalCPUTime = ((double)(end - start)) / CLOCKS_PER_SEC;

	fprintf(logFile, "Total time taken by the CPU to calculate R, G, B histograms is %f secs\n", TotalCPUTime);


	bool isredHistogramCorrect = isHistogramCorrect(totalPixelsInImage, redHistogram, 256);

	if (!isredHistogramCorrect)
	{
		fprintf(logFile, "Error calculating red histogram\n");
	}

	bool isGreenHistogramCorrect = isHistogramCorrect(totalPixelsInImage, greenHistogram, 256);

	if (!isGreenHistogramCorrect)
	{
		fprintf(logFile, "Error calculating green histogram\n");
	}

	bool isBlueHistogramCorrect = isHistogramCorrect(totalPixelsInImage, blueHistogram, 256);

	if (!isBlueHistogramCorrect)
	{
		fprintf(logFile, "Error calculating blue histogram\n");
	}

	stbi_image_free(imageData);
	fclose(logFile);

	cout << "Press any key to close the application !" << endl;
	getchar();
	
	return(0);
}

bool isHistogramCorrect(int totalPixelCount, int* histogram, int sizeOfHistogram)
{
	unsigned int count = 0;

	for (int i = 0; i < sizeOfHistogram; i++)
	{
		count = count + histogram[i];
	}

	return(count == totalPixelCount);
}