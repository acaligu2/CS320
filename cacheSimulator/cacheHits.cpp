#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <vector>
#include <math.h>

using namespace std;

vector<unsigned int> addresses;
vector<string> instructions;

//Declaring functions for each method
int directMapped(int cacheSize);
int setAssociative(int sets);
int fullyAssociativeLRU();
int fullyAssociativeHC();
int noWriteMiss(int sets);
int prefetch(int sets);
int missPrefetch(int sets);

int main(int argc, char* argv[]){

	//parse the file and fill into vectors

	string fileName;
	string output;
	
	if(argc != 2 && argc != 3){ cout << "./project1 <inputFile> <outputFile>" << endl; return 0; }
	else if(argc == 2){ fileName = argv[1]; }
	else{ fileName = argv[1]; output = argv[2]; } 

	ifstream file(fileName);
	if(file.is_open()){

		string line;
		while(getline(file, line)){
			
			stringstream iss(line);
			unsigned int address;
			string instruction;

			iss >> instruction >> hex >> address;
			
			addresses.push_back(address);
			instructions.push_back(instruction);

		}

	}

	file.close();

	int direct1kb = directMapped(1024);
	int direct4kb = directMapped(4096);
	int direct16kb = directMapped(16384);
	int direct32kb = directMapped(32768);

	int set2 = setAssociative(2);
	int set4 = setAssociative(4);
	int set8 = setAssociative(8);
	int set16 = setAssociative(16);

	int fullyLRU = fullyAssociativeLRU();
	int fullyHC = fullyAssociativeHC();

	int wm2 = noWriteMiss(2);
	int wm4 = noWriteMiss(4);
	int wm8 = noWriteMiss(8);
	int wm16 = noWriteMiss(16);

	int p2 = prefetch(2);
	int p4 = prefetch(4);
	int p8 = prefetch(8);
	int p16 = prefetch(16);

	int mp2 = missPrefetch(2);
	int mp4 = missPrefetch(4);
	int mp8 = missPrefetch(8);
	int mp16 = missPrefetch(16);

	//Write to file
	ofstream ofile(output);
	if(ofile.is_open()){

		//Direct Mapped
		ofile << direct1kb << "," << addresses.size() << "; ";
		ofile << direct4kb << "," << addresses.size() << "; ";
		ofile << direct16kb << "," << addresses.size() << "; ";
		ofile << direct32kb << "," << addresses.size() << ";" << endl;

		//Set Associative
		ofile << set2 << "," << addresses.size() << "; ";
		ofile << set4 << "," << addresses.size() << "; ";
		ofile << set8 << "," << addresses.size() << "; ";
		ofile << set16 << "," << addresses.size() << ";" << endl;

		//Fully Associative
		ofile << fullyLRU << "," << addresses.size() << ";" << endl;
		ofile << fullyHC << "," << addresses.size() << ";" << endl;

		//No Allocation on Write Miss
		ofile << wm2 << "," << addresses.size() << "; ";
		ofile << wm4 << "," << addresses.size() << "; ";
		ofile << wm8 << "," << addresses.size() << "; ";
		ofile << wm16 << "," << addresses.size() << ";" << endl;

		//Prefetch
		ofile << p2 << "," << addresses.size() << "; ";
		ofile << p4 << "," << addresses.size() << "; ";
		ofile << p8 << "," << addresses.size() << "; ";
		ofile << p16 << "," << addresses.size() << ";" << endl;

		//Prefetch on Miss
		ofile << mp2 << "," << addresses.size() << "; ";
		ofile << mp4 << "," << addresses.size() << "; ";
		ofile << mp8 << "," << addresses.size() << "; ";
		ofile << mp16 << "," << addresses.size() << ";" << endl;

		ofile.close();

	}

	return 0;

}

int directMapped(int cacheSize){

	//intialize hits, line size, and index of cache
	int cacheHit = 0;
	int cacheLines = cacheSize / 32;
	int cacheIndex = 0;

	//intialize Direct Cache, fill with 0s
	unsigned int lineTag = 0;
	unsigned int *directCache = new unsigned int[cacheLines];
	for(int i = 0; i < cacheLines; i++){

		directCache[i] = 0;

	}

	//Calculate hit/miss for each address in trace
	for(int i = 0; i < addresses.size(); i++){

		cacheIndex = (addresses[i] >> 5) % cacheLines;
		lineTag = addresses[i] >> ((unsigned int) (log2(cacheLines)) + 5);

		if(directCache[cacheIndex] == lineTag){

			cacheHit++;

		}else{ 

			directCache[cacheIndex] = lineTag;

		}

	}
	
	return cacheHit;

}

int setAssociative(int sets){

	int cacheHit = 0;
	int cacheSize = 16384;

	int currentSet = 0;

	int cacheLines = (cacheSize / 32) / sets;
	
	unsigned int lineTag = 0;

	//intialize cache and lru
	unsigned int **setA = new unsigned int *[cacheLines];
	unsigned int **lruSetA = new unsigned int *[cacheLines];

	for(int i = 0; i < cacheLines; i++){

		//2D Array with N-number of sets
		setA[i] = new unsigned int[sets];
		lruSetA[i] = new unsigned int[sets];

	}

	for(int i = 0; i < cacheLines; i++){

		for(int j = 0; j < sets; j++){

			lruSetA[i][j] = j;
			setA[i][j] = -1;

		}

	}

	//Calculate hit/miss for each address in trace
	for(int i = 0; i < addresses.size(); i++){

		currentSet = (addresses[i] >> 5) % cacheLines;
		lineTag = addresses[i] >> ((unsigned int) log2(cacheLines) + 5);

		int way = 0;
		bool found = false;

		for(int j = 0; j < sets; j++){

			//found in the cache
			if(setA[currentSet][j] == lineTag){ found = true; way = j; }

		}

		//hit
		if(found){

			int lruIndex = -1;
			for(int j = 0; j < sets; j++){

				if(lruSetA[currentSet][j] == way){

					lruIndex = j;

				}

			}

			for(int j = 0; j < lruIndex; j++){

				//Update LRU to reflect new hit		
				lruSetA[currentSet][lruIndex - j] = lruSetA[currentSet][(lruIndex - 1) - j];	

			}
			
			lruSetA[currentSet][0] = way;

			cacheHit++;

		//miss
		}else{

			unsigned int temp = lruSetA[currentSet][sets - 1];
			for(int i = 0; i < sets; i++){

				lruSetA[currentSet][sets-i] = lruSetA[currentSet][(sets - 1) - i];

			}

			//determine which location to evict
			lruSetA[currentSet][0] = temp;
			setA[currentSet][lruSetA[currentSet][0]] = lineTag;

		}

	}

	return cacheHit;

}

int fullyAssociativeLRU(){

	int cacheHits = 0;
	int cacheLines = 512; //16kb divided by 32

	unsigned int lineTag = 0;
	
	unsigned int **fullyA = new unsigned int *[1];
	unsigned int **fullyALRU = new unsigned int *[1];
	for(int i = 0; i < 1; i++){

		fullyALRU[i] = new unsigned int [512];
		fullyA[i] = new unsigned int [512];		
		
	}	
	for(int i = 0; i < 1;  i++){

		for(int j = 0; j < 512; j++){
			fullyALRU[i][j] = j;
			fullyA[i][j] = -1;

		}

	}

	for(int i = 0; i < addresses.size(); i++){

		lineTag = addresses[i] >> 5;
		int index = 0;
		bool found = false;

		for(int j = 0; j < cacheLines; j++){

			if(fullyA[0][j] == lineTag){

				found = true;	
				index = j;

			}

		}

		if(found){

			int lruIndex = -1;
			for(int j = 0; j < cacheLines; j++){

				if(fullyALRU[0][j] == index){

					lruIndex = j;

				}

			}

			for(int j = 0; j < lruIndex; j++){
			
				fullyALRU[0][lruIndex - j] = fullyALRU[0][(lruIndex - 1) - j];

			}

			fullyALRU[0][0] = index;
			cacheHits++;

		}else{

			unsigned int temp = fullyALRU[0][cacheLines - 1];
			for(int j = 0; j < cacheLines; j++){

				fullyALRU[0][cacheLines - j] = fullyALRU[0][(cacheLines - 1) - j];

			}

			fullyALRU[0][0] = temp;
			fullyA[0][fullyALRU[0][0]] = lineTag;

		}

	}

	return cacheHits;

}

int fullyAssociativeHC(){

	int cacheHits = 0;
	int cacheLines = 512; //16kb divided by 32

	unsigned int lineTag = 0;
	
	unsigned int **fullyA = new unsigned int *[1];
	
	vector<int> hcBits(cacheLines - 1, 0);

	for(int i = 0; i < 1; i++){

		fullyA[i] = new unsigned int[512];		
		
	}	
	for(int i = 0; i < 1;  i++){

		for(int j = 0; j < 512; j++){
			fullyA[i][j] = -1;

		}

	}

	for(int i = 0; i < addresses.size(); i++){

		lineTag = addresses[i] >> 5;
		int index = 0;
		bool found = false;

		for(int j = 0; j < cacheLines; j++){

			if(fullyA[0][j] == lineTag){

				found = true;	
				index = j;

			}

		}

		int hcIndex = -1;

		if(found){

			hcIndex = index + cacheLines - 1;
			cacheHits++;

			while(hcIndex != 0){

				if(hcIndex % 2 == 0){
					hcIndex = (hcIndex - 2) / 2;
					hcBits[hcIndex] = 0;
				}else{

					hcIndex = (hcIndex - 1) / 2;
					hcBits[hcIndex] = 1;

				}

			}

		}else{

			hcIndex = 0;

			int evictIndex = -1;

			for(int j = 0; j < log2(cacheLines); j++){

				if(hcBits[hcIndex] == 0){

					hcBits[hcIndex] = 1;
					hcIndex = (hcIndex * 2) + 1;

				}else{

					hcBits[hcIndex] = 0;
					hcIndex = (hcIndex * 2) + 2;

				}

			}

			evictIndex = hcIndex - (cacheLines - 1);

			fullyA[0][evictIndex] = lineTag;

		}

	}

	return cacheHits;

}

int noWriteMiss(int sets){

	int cacheHits = 0;
	
	int cacheLines = ((16 * 1024) / 32) / sets;
	int currentSet = 0;

	unsigned int lineTag = 0;
	unsigned int ** setA = new unsigned int *[256];
	unsigned int **lruA = new unsigned int *[256];

	for(int i = 0; i < cacheLines; i++){

		lruA[i] = new unsigned int[sets];
		setA[i] = new unsigned int[sets];

	}
	for(int i = 0; i < cacheLines; i++){

		for(int j = 0; j < sets; j++){

			lruA[i][j] = j;
			setA[i][j] = -1;

		}

	}

	for(int i = 0; i < addresses.size(); i++){

		currentSet = (addresses[i] >> 5) % cacheLines;
		lineTag = addresses[i] >> ((unsigned int) log2(cacheLines + 5));

		int way = 0;
		bool found = false;
		for(int j = 0; j < sets; j++){

			if(setA[currentSet][j] == lineTag){ found = true; way = j; }

		}

		if(found){

			int lruIndex = -1;

			for(int j = 0; j < sets; j++){

				if(lruA[currentSet][j] == way){ lruIndex = j; }

			}

			for(int j = 0; j < lruIndex; j++){

				lruA[currentSet][lruIndex - j] = lruA[currentSet][(lruIndex - 1) - j];

			}
			lruA[currentSet][0] = way;

			cacheHits++;

		}else{

			if(instructions[i] == "L"){

				unsigned int temp = lruA[currentSet][sets - 1];
				for(int j = 0; j < sets; j++){

					lruA[currentSet][sets - j] = lruA[currentSet][(sets - 1) - j];

				}

				lruA[currentSet][0] = temp;
				setA[currentSet][lruA[currentSet][0]] = lineTag;

			}

		}



	}

	return cacheHits;

}

int prefetch(int sets){
	
	int cacheHit = 0;
	
	int cacheLines = ((16 * 1024) / 32) / sets;
	int currentSet = 0;
	int nextIndex = -1;

	unsigned int lineTag = 0;
	unsigned int nextLineTag = 0;

	unsigned int ** setA = new unsigned int *[cacheLines];
	unsigned int **lruSetA = new unsigned int *[cacheLines];

	for(int i = 0; i < cacheLines; i++){

		lruSetA[i] = new unsigned int[sets];
		setA[i] = new unsigned int[sets];

	}
	for(int i = 0; i < cacheLines; i++){

		for(int j = 0; j < sets; j++){

			lruSetA[i][j] = j;
			setA[i][j] = -1;

		}

	}

	for(int i = 0; i < addresses.size(); i++){

		unsigned int nextAddress = addresses[i] + 32;

		currentSet = (addresses[i] >> 5) % cacheLines;
		nextIndex = ((nextAddress) >> 5) % cacheLines;

		lineTag = addresses[i] >> ((unsigned int) log2(cacheLines) + 5);

		nextLineTag = nextAddress >> ((unsigned int) log2(cacheLines) + 5);

		int way = 0;
		int nextWay = 0;

		bool found = false;
		bool foundNext = false;

		for(int j = 0; j < sets; j++){

			if(setA[currentSet][j] == lineTag){ 

				found = true;
				way = j;
			
			}				

		}

		int lruIndex = -1;

		if(found){

			for(int j = 0; j < sets; j++){

				if(lruSetA[currentSet][j] == way){

					lruIndex = j;

				}

			}

			for(int j = 0; j < lruIndex; j++){
		
				lruSetA[currentSet][lruIndex - j] = lruSetA[currentSet][(lruIndex - 1) - j];	

			}

			cacheHit++;

			lruSetA[currentSet][0] = way;

		}else{


			unsigned int temp = lruSetA[currentSet][sets - 1];

			for(int j = 0; j < sets; j++){

				lruSetA[currentSet][sets - j] = lruSetA[currentSet][(sets - 1) - j];

			}

			lruSetA[currentSet][0] = temp;

			setA[currentSet][lruSetA[currentSet][0]] = lineTag;

		}
		
		for(int j = 0; j < sets; j++){

			if(setA[nextIndex][j] == nextLineTag){

				foundNext = true;
				nextWay = j;

			}

		}

		if(foundNext){

			for(int j = 0; j < sets; j++){

				if(lruSetA[nextIndex][j] == nextWay){

					lruIndex = j;

				}

			}

			for(int j = 0; j < lruIndex; j++){
		
				lruSetA[nextIndex][lruIndex - j] = lruSetA[nextIndex][(lruIndex - 1) - j];	

			}

			lruSetA[nextIndex][0] = nextWay;

		}else{

			unsigned int temp = lruSetA[nextIndex][sets - 1];
			for(int j = 0; j < sets; j++){

				lruSetA[nextIndex][sets - j] = lruSetA[nextIndex][(sets - 1) - j];

			}

			lruSetA[nextIndex][0] = temp;

			setA[nextIndex][lruSetA[nextIndex][0]] = nextLineTag;

		}

	}

	return cacheHit;

}

int missPrefetch(int sets){
	
	int cacheHit = 0;
	
	int cacheLines = ((16 * 1024) / 32) / sets;
	int currentSet = 0;
	int nextIndex = -1;

	unsigned int lineTag = 0;
	unsigned int nextLineTag = 0;

	unsigned int ** setA = new unsigned int *[cacheLines];
	unsigned int **lruSetA = new unsigned int *[cacheLines];

	for(int i = 0; i < cacheLines; i++){

		lruSetA[i] = new unsigned int[sets];
		setA[i] = new unsigned int[sets];

	}
	for(int i = 0; i < cacheLines; i++){

		for(int j = 0; j < sets; j++){

			lruSetA[i][j] = j;
			setA[i][j] = -1;

		}

	}

	for(int i = 0; i < addresses.size(); i++){

		unsigned int nextAddress = addresses[i] + 32;

		currentSet = (addresses[i] >> 5) % cacheLines;
		nextIndex = ((nextAddress) >> 5) % cacheLines;

		lineTag = addresses[i] >> ((unsigned int) log2(cacheLines) + 5);

		nextLineTag = nextAddress >> ((unsigned int) log2(cacheLines) + 5);

		int way = 0;
		int nextWay = 0;

		bool found = false;
		bool foundNext = false;

		for(int j = 0; j < sets; j++){

			if(setA[currentSet][j] == lineTag){ 

				found = true;
				way = j;
			
			}				

		}

		int lruIndex = -1;

		//hit
		if(found){

			//Update LRU
			for(int j = 0; j < sets; j++){

				if(lruSetA[currentSet][j] == way){

					lruIndex = j;

				}

			}

			for(int j = 0; j < lruIndex; j++){
		
				lruSetA[currentSet][lruIndex - j] = lruSetA[currentSet][(lruIndex - 1) - j];	

			}

			cacheHit++;

			lruSetA[currentSet][0] = way;

		//miss	
		}else{


			unsigned int temp = lruSetA[currentSet][sets - 1];

			for(int j = 0; j < sets; j++){

				lruSetA[currentSet][sets - j] = lruSetA[currentSet][(sets - 1) - j];

			}

			lruSetA[currentSet][0] = temp;

			setA[currentSet][lruSetA[currentSet][0]] = lineTag;

			//Handle prefetch
			for(int j = 0; j < sets; j++){

				if(setA[nextIndex][j] == nextLineTag){

					foundNext = true;
					nextWay = j;

				}

			}

			if(foundNext){

				for(int j = 0; j < sets; j++){

					if(lruSetA[nextIndex][j] == nextWay){

						lruIndex = j;

					}

				}

				for(int j = 0; j < lruIndex; j++){
		
					lruSetA[nextIndex][lruIndex - j] = lruSetA[nextIndex][(lruIndex - 1) - j];	

				}

				lruSetA[nextIndex][0] = nextWay;

			}else{

				unsigned int temp = lruSetA[nextIndex][sets - 1];
				for(int j = 0; j < sets; j++){

					lruSetA[nextIndex][sets - j] = lruSetA[nextIndex][(sets - 1) - j];

				}	

				lruSetA[nextIndex][0] = temp;

				setA[nextIndex][lruSetA[nextIndex][0]] = nextLineTag;

			}

		}
		
		
	}

	return cacheHit;

}
