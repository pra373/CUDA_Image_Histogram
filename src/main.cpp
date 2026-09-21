#include<iostream>
#include<direct.h>
#include<time.h>
#include<cuda_runtime.h>
#define STB_IMAGE_IMPLEMENTATION
#include"../include/stb_image.h"
#include"../include/kernel.cuh"

using std::cout;
using std::endl;
using std::cin;

bool isHistogramCorrect(int totalPixelCount, int* histogram, int sizeOfHistogram);
void uninitialize(unsigned char* imageData, unsigned char* dev_imageData, int* dev_redHistogram, int* dev_greenHistogram, int* dev_blueHistogram, FILE* logFile);

FILE* logFile;

int main(void)
{
	unsigned char* imageData;
	int width, height, channels;

	// buffers for histograms
	int redHistogram[256] = { 0 };
	int greenHistogram[256] = { 0 };
	int blueHistogram[256] = { 0 };

	//buffers for Histograms calculated by GPU

	int afterGPU_redHistogram[256] = { 0 };
	int afterGPU_greenHistogram[256] = { 0 };
	int afterGPU_blueHistogram[256] = { 0 };

	clock_t start, end;
	cudaEvent_t cudaStart, cudaStop;

	cudaError_t error;
	unsigned char* dev_imageData;

	int* dev_redHistogram = nullptr;
	int* dev_greenHistogram = nullptr;
	int* dev_blueHistogram = nullptr;
	
	size_t sizeOfHistogram = 256 * sizeof(int);

	

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

	// **************************************** GPU Histogram Implementation starts here ************************************

	//allocote Memory for Image on GPU

	error = cudaMalloc((void**)&dev_imageData, totalImageArraySize);

	if (error != cudaSuccess)
	{
		fprintf(logFile, "error allocating GPU memory for image !\n");
		exit(EXIT_FAILURE);
	}

	//allocate memory for three histograms on GPU

	error = cudaMalloc((void**)&dev_redHistogram, sizeOfHistogram);

	if (error != cudaSuccess)
	{
		fprintf(logFile, "error allocating GPU memory for red histogram !\n");
		exit(EXIT_FAILURE);
	}

	error = cudaMalloc((void**)&dev_greenHistogram, sizeOfHistogram);

	if (error != cudaSuccess)
	{
		fprintf(logFile, "error allocating GPU memory for green histogram !\n");
		exit(EXIT_FAILURE);
	}

	error = cudaMalloc((void**)&dev_blueHistogram, sizeOfHistogram);

	if (error != cudaSuccess)
	{
		fprintf(logFile, "error allocating GPU memory for blue histogram !\n");
		exit(EXIT_FAILURE);
	}

	//copy image data from Host to Device

	error = cudaMemcpy(dev_imageData, imageData, totalImageArraySize, cudaMemcpyHostToDevice);

	if (error != cudaSuccess)
	{
		fprintf(logFile, "Failed to copy image data from CPU to GPU!\n");
		fprintf(logFile, "Reason: %s\n", cudaGetErrorString(error));
		exit(EXIT_FAILURE);
	}

	// set memory of all Histograms to 0

	error = cudaMemset(dev_redHistogram, 0, sizeOfHistogram);

	if (error != cudaSuccess)
	{
		fprintf(logFile, "Setting GPU memory for red histogram to 0 failed !\n");
		exit(EXIT_FAILURE);
	}

	error = cudaMemset(dev_blueHistogram, 0, sizeOfHistogram);

	if (error != cudaSuccess)
	{
		fprintf(logFile, "Setting GPU memory for blue histogram to 0 failed !\n");
		exit(EXIT_FAILURE);
	}

	error = cudaMemset(dev_greenHistogram, 0, sizeOfHistogram);

	if (error != cudaSuccess)
	{
		fprintf(logFile, "Setting GPU memory for green histogram to 0 failed !\n");
		exit(EXIT_FAILURE);
	}

	cudaEventCreate(&cudaStart);
	cudaEventCreate(&cudaStop);

	cudaEventRecord(cudaStart);

	launchKernel(dev_imageData, dev_redHistogram, dev_greenHistogram, dev_blueHistogram, width, height);

	cudaEventRecord(cudaStop);

	cudaEventSynchronize(cudaStop);

	float milliseconds = 0.0f;
	cudaEventElapsedTime(&milliseconds, cudaStart, cudaStop);

	float seconds = milliseconds / 1000.0f;

	fprintf(logFile, "CUDA kernel execution time: %f secs\n", seconds);

	cudaEventDestroy(cudaStart);
	cudaEventDestroy(cudaStop);

	//copy GPU histograms to CPU side

	error = cudaMemcpy(afterGPU_redHistogram, dev_redHistogram, sizeOfHistogram, cudaMemcpyDeviceToHost);

	if (error != cudaSuccess)
	{
		fprintf(logFile, "Failed to copy red histogram from GPU to CPU!\n");
		fprintf(logFile, "Reason: %s\n", cudaGetErrorString(error));
		exit(EXIT_FAILURE);
	}

	error = cudaMemcpy(afterGPU_greenHistogram, dev_greenHistogram, sizeOfHistogram, cudaMemcpyDeviceToHost);

	if (error != cudaSuccess)
	{
		fprintf(logFile, "Failed to copy green histogram from GPU to CPU!\n");
		fprintf(logFile, "Reason: %s\n", cudaGetErrorString(error));
		exit(EXIT_FAILURE);
	}

	error = cudaMemcpy(afterGPU_blueHistogram, dev_blueHistogram, sizeOfHistogram, cudaMemcpyDeviceToHost);

	if (error != cudaSuccess)
	{
		fprintf(logFile, "Failed to copy blue histogram from GPU to CPU!\n");
		fprintf(logFile, "Reason: %s\n", cudaGetErrorString(error));
		exit(EXIT_FAILURE);
	}

	// check weather GPU histogram matches CPU histogram

	bool histogramsMatch = true;

	for (int i = 0; i < 256; i++)
	{
		if (redHistogram[i] != afterGPU_redHistogram[i])
		{
			fprintf(logFile, "Red histogram mismatch at index %d\n", i);
			histogramsMatch = false;
		}

		if (greenHistogram[i] != afterGPU_greenHistogram[i])
		{
			fprintf(logFile, "Green histogram mismatch at index %d\n", i);
			histogramsMatch = false;
		}

		if (blueHistogram[i] != afterGPU_blueHistogram[i])
		{
			fprintf(logFile, "Blue histogram mismatch at index %d\n", i);
			histogramsMatch = false;
		}
	}

	if (histogramsMatch)
	{
		fprintf(logFile, "CPU and GPU histograms match successfully!\n");
	}


	uninitialize(imageData, dev_imageData, dev_redHistogram, dev_greenHistogram, dev_blueHistogram, logFile);

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

void uninitialize(unsigned char* imageData, unsigned char* dev_imageData, int* dev_redHistogram, int* dev_greenHistogram, int* dev_blueHistogram, FILE* logFile)
{

	cudaFree(dev_blueHistogram);
	cudaFree(dev_greenHistogram);
	cudaFree(dev_redHistogram);
	cudaFree(dev_imageData);
	stbi_image_free(imageData);
	fclose(logFile);
}