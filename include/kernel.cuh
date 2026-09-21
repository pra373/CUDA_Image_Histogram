#pragma once

__global__ void kernel(unsigned char* imageData, int* redHistogram, int* greenHistogram, int* blueHistogram, int width, int height);
void launchKernel(unsigned char* imageData, int* redHistogram, int* greenHistogram, int* blueHistogram, int width, int height);