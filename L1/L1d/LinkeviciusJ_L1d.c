/* Justinas Linkevicius
* IFF-3/2
* L1d - Open MPI
* mpiCC -o linkevicius LinkeviciusJ_L1d.c && mpirun -np 10 linkevicius

1. Kokia tvarka startuoja procesai?
**** atsitiktine

2. Kokia tvarka vykdomi procesai?
**** tokia, kokia startuoja

3. Kiek iteracijų iš eilės padaro vienas procesas?
**** visas

4. Kokia tvarka atspausdinami to paties masyvo duomenys?
****  tokia, kokia surašyti.

5. Kurioje programoje trumpiausias vieno proceso kodas? 

6. Kokiu  kompiuteriu  vykdėte  savo  programas?  Nurodykite  branduolių  skaičių  ir  dažnius,  OA  apimtį,  OS, NVIDIA plokštės tipą.

*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>

using namespace std;

#define INPUTFILE "LinkeviciusJ.txt"

string stringFieldName, intFieldName, doubleFieldName;
int threadsCount;
int dataElementsCount;
int threadDataSize;

struct FileData
{
	string  stringField;
	int          intField;
	double       doubleField;

	FileData()
	{
		stringField = "";
		intField = 0;
		doubleField = 0.0;
	}

	FileData(string a, int b, double c)
	{
		stringField = a;
		intField = b;
		doubleField = c;
	}
};

void readFile(vector<FileData*> & threadDataArray)
{
	ifstream input(INPUTFILE);

	input >> stringFieldName;
	input >> intFieldName;
	input >> doubleFieldName;
	input >> dataElementsCount;

	threadDataSize = ceil((double)dataElementsCount / threadsCount);

	int line = 0;
	for (int i = 0; i < threadsCount; i++)
	{
		FileData *threadData = new FileData[threadDataSize];

		for (int j = 0; j < threadDataSize; j++)
		{
			string stringField;
			int intField;
			double doubleField;

			input >> stringField >> intField >> doubleField;

			// jei masyvui nebera duomenu, uzpildome tusciais elementais
			if (line < dataElementsCount)
				threadData[j] = FileData(stringField, intField, doubleField);
			else
				threadData[j] = FileData();
			
			line++;
		}

		threadDataArray.push_back(threadData);
	}
	input.close();
}

void writeData(vector<FileData*> threadDataArray)
{
	int line = 0;
	cout << stringFieldName << "\t" << intFieldName << "\t" << doubleFieldName << "\r\n";
	for (int i = 0; i < threadsCount; i++)
	{
		cout << endl << "**** Masyvas" << i << " ****" << endl;
		for (int j = 0; j < threadDataSize; j++)
		{
			line++;

			if (threadDataArray[i][j].stringField != "")
			{
				cout.precision(2);
				cout << j << ") " << threadDataArray[i][j].stringField << "\t" << threadDataArray[i][j].intField << "\t" << fixed << threadDataArray[i][j].doubleField << "\r\n";
			}

			if (line == dataElementsCount)
				break;
		}
	}
	cout << endl;
}

void doThreadWork(int threadId, FileData* threadData)
{
	for (int i = 0; i < threadDataSize; i++)
	{
		if (threadData[i].stringField != "")
		{
			printf("process_%d: %d\t%s\t%d\t%.2f\n", threadId, i, threadData[i].stringField.c_str(), threadData[i].intField, threadData[i].doubleField);
		}
	}
}

int main(int argc, char** argv)
{
	int  procNr;
	int  procCount, procNameLength;
	char procName[MPI_MAX_PROCESSOR_NAME];

	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &procNr);
	MPI_Comm_size(MPI_COMM_WORLD, &procCount);
	MPI_Get_processor_name(procName, &procNameLength);

	threadsCount = procCount;

	if (procNr == 0)
	{
		printf("Programa pradejo darba\n");
		
		vector<FileData*> threadDataArray;
		readFile(threadDataArray);

		writeData(threadDataArray);

		// send data size to all processes
		for (int i = 1; i < threadsCount; i++)
		{
			MPI_Send(&threadDataSize, sizeof(int), MPI_INT, i, 0, MPI_COMM_WORLD);
		}

		// send messages to other proccesses
		for (int i = 1; i < threadsCount; i++)
		{
			for (int j = 0; j < threadDataSize; j++)
			{
				char *messageBuffer = new char[255];
				if (threadDataArray[i][j].stringField != "")
				{
					snprintf(messageBuffer, 255, "%d\t%s\t%d\t%.2f", j, threadDataArray[i][j].stringField.c_str(), threadDataArray[i][j].intField, threadDataArray[i][j].doubleField);
				} else
				{
					snprintf(messageBuffer, 255, "");
				}

				MPI_Send(messageBuffer, 255, MPI_CHAR, i, j + 1, MPI_COMM_WORLD);
				delete[] messageBuffer;
			}
		}

		// atlieka darbo dali kuri priskirta pirmam procesui
		doThreadWork(procNr, threadDataArray[procNr]);
		
		// pirma karta laukiame visu procesu pries duomenu spausdinima, kita karta po duomenu spausdinimo
		MPI_Barrier(MPI_COMM_WORLD);
		sleep(1);
		MPI_Barrier(MPI_COMM_WORLD);
		
		printf("Programa baige darba\n");
	}
	else
	{
		MPI_Barrier(MPI_COMM_WORLD);

		int dataSize = 0;
		MPI_Recv(&dataSize, sizeof(int), MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

		for (int j = 0; j < dataSize; j++)
		{
			char *messageBuffer = new char[255];
			MPI_Recv(messageBuffer, 255, MPI_CHAR, 0, j + 1, MPI_COMM_WORLD, &status);

			if (strcmp(messageBuffer, "") != 0)
				printf("process_%d: %s\n", procNr, messageBuffer);
			
			delete[] messageBuffer;
		}
		
		MPI_Barrier(MPI_COMM_WORLD);
	}
	MPI_Finalize();

	return 0;
}