#include"../include/kernel.cuh"

__global__ void kernel(unsigned char* imageData, int* redHistogram, int* greenHistogram, int* blueHistogram, int width, int height)
{
	__shared__ unsigned int SredHistogram[256];
	__shared__ unsigned int SgreenHistogram[256];
	__shared__ unsigned int SblueHistogram[256];

	int histogramIndex = threadIdx.x + blockDim.x * threadIdx.y;

	if (histogramIndex < 256)
	{
		SredHistogram[histogramIndex] = 0;
		SblueHistogram[histogramIndex] = 0;
		SgreenHistogram[histogramIndex] = 0;
	}

	__syncthreads();

	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;
	int offset = x + y * width;

	if (x < width && y < height)
	{
		int imageIndex = offset * 3;

		int R = imageData[imageIndex];
		int G = imageData[imageIndex + 1];
		int B = imageData[imageIndex + 2];

		atomicAdd(&SredHistogram[R], 1);
		atomicAdd(&SgreenHistogram[G], 1);
		atomicAdd(&SblueHistogram[B], 1);

	}
	__syncthreads();

	if (histogramIndex < 256)
	{
		atomicAdd(&redHistogram[histogramIndex], SredHistogram[histogramIndex]);
		atomicAdd(&greenHistogram[histogramIndex], SgreenHistogram[histogramIndex]);
		atomicAdd(&blueHistogram[histogramIndex], SblueHistogram[histogramIndex]);
	}

}

void launchKernel(unsigned char* imageData, int* redHistogram, int* greenHistogram, int* blueHistogram, int width, int height)
{
	dim3 blockDim(32, 32);
	dim3 gridDim(((width + (blockDim.x - 1)) / blockDim.x), ((height + (blockDim.y - 1)) / blockDim.y));

	kernel << <gridDim, blockDim >> > (imageData, redHistogram, greenHistogram, blueHistogram, width, height);

}