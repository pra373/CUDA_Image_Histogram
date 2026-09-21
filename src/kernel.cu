#include"../include/kernel.cuh"

__global__ void kernel(unsigned char* imageData, int* redHistogram, int* greenHistogram, int* blueHistogram, int width, int height)
{
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;
	int offset = x + y * width;

	if (x >= width || y >= height)
	{
		return;
	}

	int imageIndex = offset * 3;

	int R = imageData[imageIndex];
	int G = imageData[imageIndex + 1];
	int B = imageData[imageIndex + 2];

	/*redHistogram[R]++;
	greenHistogram[G]++;
	blueHistogram[B]++;*/

	atomicAdd(&redHistogram[R], 1);
	atomicAdd(&greenHistogram[G], 1);
	atomicAdd(&blueHistogram[B], 1);

}

void launchKernel(unsigned char* imageData, int* redHistogram, int* greenHistogram, int* blueHistogram, int width, int height)
{
	dim3 blockDim(32, 32);
	dim3 gridDim(((width + (blockDim.x - 1)) / blockDim.x), ((height + (blockDim.y - 1)) / blockDim.y));

	kernel << <gridDim, blockDim >> > (imageData, redHistogram, greenHistogram, blueHistogram, width, height);

}