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
	char ch;

	// buffer for red histogram

	int redHistogram[256] = { 0 };
	int greenHistogram[256] = { 0 };
	int blueHistogram[256] = { 0 };

	imageData = stbi_load("../resources/mars_4k.jpg", &width, &height, &channels, 3);

	if (!imageData)
	{
		cout << "Failed to load image!" << endl;
		cout << "Reason: " << stbi_failure_reason() << endl;
		cin >> ch;
		exit(EXIT_FAILURE);
	}

	cout << "Width : " << width << endl;
	cout << "Height : " << height << endl;
	cout << "Channels : " << channels << endl;

	int totalPixelsInImage = width * height;
	int totalImageArraySize = totalPixelsInImage * 3;

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

	bool isredHistogramCorrect = isHistogramCorrect(totalPixelsInImage, redHistogram, 256);

	if (!isredHistogramCorrect)
	{
		cout << "Error in calculating red histogram !" << endl;
	}

	bool isGreenHistogramCorrect = isHistogramCorrect(totalPixelsInImage, greenHistogram, 256);

	if (!isGreenHistogramCorrect)
	{
		cout << "Error in calculating green histogram !" << endl;
	}

	bool isBlueHistogramCorrect = isHistogramCorrect(totalPixelsInImage, blueHistogram, 256);

	if (!isBlueHistogramCorrect)
	{
		cout << "Error in calculating blue histogram !" << endl;
	}

	cin >> ch;
	stbi_image_free(imageData);
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