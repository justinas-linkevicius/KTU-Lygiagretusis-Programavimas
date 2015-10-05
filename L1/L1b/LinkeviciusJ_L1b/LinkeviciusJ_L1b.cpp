/* Justinas Linkevicius
 * IFF-3/2
 * L1b
 *

1. Kokia tvarka startuoja procesai?
**** atsitiktine

2. Kokia tvarka vykdomi procesai?
**** atsitiktine

3. Kiek iteracijų iš eilės padaro vienas procesas?
**** atsitiktinį skaičių

4. Kokia tvarka atspausdinami to paties masyvo duomenys?
****  atsitiktine.

*/

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <omp.h>

using namespace std;

#define INPUTFILE "LinkeviciusJ.txt"
#define THREADS 10

// struktura skirta saugoti pradiniams duomenims
struct FileData
{
	string  stringField;
	int          intField;
	double       doubleField;

	FileData()
	{
		stringField = "";
		intField    = 0;
		doubleField = 0.0;
	}

	FileData(string a, int b, double c) // init
	{
		stringField = a;
		intField = b;
		doubleField = c;
	}
};

// lauku pavadinimai
string stringFieldName, intFieldName, doubleFieldName;

// duomenu elementu kiekis
int dataElementsCount;

// kiekis elementu, kuriuos tures apdoroti kiekviena gija
int threadDataSize;

// pradiniu duomenu nuskaitymas
void readFile( vector<FileData*> & threadDataArray )
{
	ifstream input(INPUTFILE);

	input >> stringFieldName;
	input >> intFieldName;
	input >> doubleFieldName;
	input >> dataElementsCount;

	threadDataSize = ceil( (double) dataElementsCount / THREADS);

	int line = 0;
	for (int i = 0; i < THREADS; i++)
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

// spausdina pradinius duomenis
void writeData(vector<FileData*> threadDataArray)
{
	int line = 0;
	cout << stringFieldName << "\t" << intFieldName << "\t" << doubleFieldName << "\r\n";
	for (int i = 0; i < THREADS; i++)
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

// atlieka kiekvieno proceso dali
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

void startMultithreading(vector<FileData*> threadDataArray)
{
	omp_set_num_threads(THREADS);

	printf("Start Multithreading!\n");
	int threadId;

	// Lygiagrecioji dalis
	#pragma omp parallel private(threadId) 
	{
		threadId = omp_get_thread_num();
		
		// perduodame procesui jo duomenu masyva ir proceso id
		doThreadWork(threadId, threadDataArray[threadId]);
	}
	// Lygiagrecioji dalis / pabaiga.

	printf("End multithreading!\n");
}

int main(int argc, char* argv[])
{
	vector<FileData*> threadDataArray;

	readFile( threadDataArray );
	writeData( threadDataArray );
	startMultithreading( threadDataArray );
	
	std::cin.get();

	return 0;
}