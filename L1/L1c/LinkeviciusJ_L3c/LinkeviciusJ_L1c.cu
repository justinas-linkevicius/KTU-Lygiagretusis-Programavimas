/* Justinas Linkevicius
* IFF-3/2
* L1c
*

1. Kokia tvarka startuoja procesai?
****  tokia, kokia užrašyti

2. Kokia tvarka vykdomi procesai?
**** tokia, kokia startuoja

3. Kiek iteracijų iš eilės padaro vienas procesas?
**** vieną pilna

4. Kokia tvarka atspausdinami to paties masyvo duomenys?
****  tokia, kokia surašyti.

*/

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>

using namespace std;

#define INPUTFILE "LinkeviciusJ.txt"
#define CUDA_THREADS 10
#define MAX_DATA_PER_THREADS 10

/* Struktura saugoti vienai duomenu eilutei */
struct FileData
{
	char	stringField[255];
	int     intField;
	double  doubleField;

	FileData()
	{
		strncpy(stringField, "", 255);
		intField = 0;
		doubleField = 0.0;
	}

	FileData(string a, int b, double c)
	{
		strncpy(stringField, a.c_str(), 255);
		intField = b;
		doubleField = c;
	}
};

/* Struktura saugoti visiems duomenims */
struct ThreadData
{
	FileData array[ MAX_DATA_PER_THREADS ];
};

// lauku pavadinimai
string stringFieldName, intFieldName, doubleFieldName;

// nuskaito pradinius duomenis
void readData(ThreadData* threadDataArrays, int & threadDataSize, int & dataElementsCount)
{
	ifstream input(INPUTFILE);

	input >> stringFieldName;
	input >> intFieldName;
	input >> doubleFieldName;
	input >> dataElementsCount;

	threadDataSize = ceil((double)dataElementsCount / CUDA_THREADS);

	int line = 0;
	for (int i = 0; i < CUDA_THREADS; i++)
	{
		for (int j = 0; j < threadDataSize; j++)
		{
			string stringField;
			int intField;
			double doubleField;

			input >> stringField >> intField >> doubleField;

			// jei masyvui nebera duomenu, uzpildome tusciais elementais
			if (line < dataElementsCount)
				threadDataArrays[i].array[j] = FileData(stringField, intField, doubleField);
			else
				threadDataArrays[i].array[j] = FileData();

			line++;
		}
	}
	input.close();
}

// spausdina pradinius duomenis
void writeData(ThreadData* threadDataArrays, int & threadDataSize, int & dataElementsCount)
{
	int line = 0;
	cout << stringFieldName << "\t" << intFieldName << "\t" << doubleFieldName << "\r\n";
	for (int i = 0; i < CUDA_THREADS; i++)
	{
		cout << endl << "**** Array" << i << " ****" << endl;
		for (int j = 0; j < threadDataSize; j++)
		{
			line++;

			if (threadDataArrays[i].array[j].stringField != "")
			{
				cout.precision(2);
				cout << j << ") " << threadDataArrays[i].array[j].stringField << "\t" << threadDataArrays[i].array[j].intField << "\t" << fixed << threadDataArrays[i].array[j].doubleField << "\r\n";
			}

			if (line == dataElementsCount)
				break;
		}
	}
	cout << endl;
}

// lygiagrecioji programos dalis
// perduodamas duomenu masyvas ir kiekvieno proceso apdorojamu elementu kiekis
__global__ void printKernel(ThreadData *threadData, int* size)
{
	// get threadId
	int i = threadIdx.x;

	// spausdina proceso elementus
	for (int j = 0; j < *size; j++)
	{
		printf("process_%d: %d\t%s\t%d\t%.2f\n", i, j, threadData[i].array[j].stringField, threadData[i].array[j].intField, threadData[i].array[j].doubleField);
	}
}

// Helper function for using CUDA to add vectors in parallel.
cudaError_t printWithCuda(ThreadData* hostData, int threadsCount, int threadDataSize)
{
	cudaError_t cudaStatus;

	ThreadData* devData = 0;
	int*        devSize = 0;

	// choose gpu
	cudaStatus = cudaSetDevice(0);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
		goto Error;
	}

	// Allocate GPU buffers
	cudaStatus = cudaMalloc((void**)&devData, threadsCount * sizeof(ThreadData));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		goto Error;
	}

	cudaStatus = cudaMalloc(&devSize, sizeof(int));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		goto Error;
	}

	// Copy input vectors from host memory to GPU buffers.
	cudaStatus = cudaMemcpy(devData, hostData, threadsCount * sizeof(ThreadData), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy failed!");
		goto Error;
	}

	cudaStatus = cudaMemcpy(devSize, &threadDataSize, sizeof(int), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy failed!");
		goto Error;
	}

	// Launch a kernel on the GPU with one thread for each element.
	printf("Starting CUDA threads!\n");
	printKernel <<< 1, threadsCount >>> (devData, devSize);

	// Check for any errors launching the kernel
	cudaStatus = cudaGetLastError();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "addKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
		goto Error;
	}

	// cudaDeviceSynchronize waits for the kernel to finish, and returns
	// any errors encountered during the launch.
	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
		goto Error;
	}

	printf("End of CUDA threads!\n");

Error:
	cudaFree(devData);
	cudaFree(devSize);

	std::cin.get();

	return cudaStatus;
}

int main()
{
	ThreadData threadDataArrays[CUDA_THREADS];

	int dataElementsCount;
	int threadDataSize;

	readData(threadDataArrays, threadDataSize, dataElementsCount);

	writeData(threadDataArrays, threadDataSize, dataElementsCount);

	// Start CUDA
	cudaError_t cudaStatus = printWithCuda(threadDataArrays, CUDA_THREADS, threadDataSize);

	if (cudaStatus != cudaSuccess)
	{
		fprintf(stderr, "addWithCuda failed!");
		return 1;
	}

	// cudaDeviceReset must be called before exiting in order for profiling and
	// tracing tools such as Nsight and Visual Profiler to show complete traces.
	cudaStatus = cudaDeviceReset();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceReset failed!");
		return 1;
	}

	std::cin.get();

	return 0;
}