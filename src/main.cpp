#include<windows.h>
#include<iostream>
#include<direct.h>
#include<time.h>
#include<cuda_runtime.h>
#define STB_IMAGE_IMPLEMENTATION
#include"../include/stb_image.h"
#include"../include/kernel.cuh"
#include"../include/icon.h"

using std::cout;
using std::endl;
using std::cin;

bool isHistogramCorrect(int totalPixelCount, int* histogram, int sizeOfHistogram);
void uninitialize(unsigned char* imageData, unsigned char* dev_imageData, int* dev_redHistogram, int* dev_greenHistogram, int* dev_blueHistogram, FILE* logFile);
void Tell_If_CPU_and_GPU_Histograms_Match(int* redHistogram, int* greenHistogram, int* blueHistogram, int* afterGPU_redHistogram, int* afterGPU_greenHistogram, int* afterGPU_blueHistogram);

FILE* logFile;

unsigned char* imageData;

int* dev_redHistogram = nullptr;
int* dev_greenHistogram = nullptr;
int* dev_blueHistogram = nullptr;

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

size_t sizeOfHistogram = 256 * sizeof(int);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HWND ghwnd = NULL;
DWORD dwStyle = 0;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
BOOL gbFullScreen = FALSE;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("PLP Window");

	logFile = fopen("../logs/log.txt", "w");

	if (!logFile)
	{
		cout << "failed to open log file to write application logs !" << endl;
		getchar();
		return(EXIT_FAILURE);
	}
	
	// WNDCLASSEX Initialization

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbWndExtra = 0;
	wndclass.cbClsExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	RegisterClassEx(&wndclass);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("Prathamesh Laxmikant Paropkari"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	ghwnd = hwnd;

	ShowWindow(hwnd, iCmdShow);

	UpdateWindow(hwnd);

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

	
	Tell_If_CPU_and_GPU_Histograms_Match(redHistogram, greenHistogram, blueHistogram, afterGPU_redHistogram, afterGPU_greenHistogram, afterGPU_blueHistogram);
	
	// message loop

	while (GetMessage(&msg, NULL, 0, 0))   // heart of application
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return((int)msg.wParam);

}

// call back function

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// function declaration
	void ToggleFullScreen(void);
	
	// code

	switch (iMsg)
	{
	case WM_DESTROY:
		uninitialize(imageData, dev_imageData, dev_redHistogram, dev_greenHistogram, dev_blueHistogram, logFile);
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		}
		break;
	case WM_CHAR:
		switch (LOWORD(wParam))
		{
		case 'F':
		case 'f':
			if (gbFullScreen == FALSE)
			{
				ToggleFullScreen();
				gbFullScreen = TRUE;
			}
			else
			{
				ToggleFullScreen();
				gbFullScreen = FALSE;
			}

			break;
		}

		break;

	default:
		break;

		
		
	}

	return(DefWindowProc(hwnd, iMsg, wParam,lParam));
	

	
}

void ToggleFullScreen(void)
{
	// Local variable declarations 
	MONITORINFO mi = { sizeof(MONITORINFO) };


	//code

	if (gbFullScreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}

		ShowCursor(FALSE);
	}

	else
	{
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
		
	}

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

void Tell_If_CPU_and_GPU_Histograms_Match(int* redHistogram, int* greenHistogram, int* blueHistogram, int* afterGPU_redHistogram, int* afterGPU_greenHistogram, int* afterGPU_blueHistogram)
{
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
}






