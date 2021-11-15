//Made by Alexander Bagherzadeh
//EEL 4678 Computer Architecture
//Project 2 Branch Prediction

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

struct predictionTableData {
    int nBits;
    int nValue;
    int indexSize;
};

struct predictionData {
    int expectedPath;
    char expectedHit;
};

struct maskData {
    int mMask;
    int mBits;
    int indexMask;
};

struct hitData {
    unsigned long long hits;
    unsigned long long misses;
};

struct addressData {
    char actualHit;
    char address[64];
    int currentIndex;
    unsigned long long addressValue;
    int addressIndex;
    FILE * filePath;
};

void getIndex(struct predictionTableData *predictionValues, struct maskData *maskData);
void getCurrentIndex(struct maskData *maskData, struct addressData *addressData, struct predictionTableData *predictionValues);
void getIndexMask(struct maskData *maskData, struct predictionTableData *predictionValues);
void getIndexFromAddress(struct maskData *maskData, struct addressData *address);
void flushPredictionTable(int *predictionTable);
void initializePredictionTable(int *predictionTable, struct predictionTableData *predictionValues);
void printPredictionTable(int *predictionTable, struct predictionTableData *predictionValues);

int main(int argc, char *argv[]) {
    //Initialize variables
    struct predictionTableData predictionValues;
    struct predictionData predictionData;
    struct maskData maskData;
    struct hitData hitData;
    struct addressData addressData;
    hitData.hits = 0;
    hitData.misses = 0;
    predictionValues.nValue = 0;

    //allocate space for file path
    addressData.filePath = fopen(argv[3], "r");

    //store arguments
    maskData.mBits = atoi(argv[1]);
    predictionValues.nBits = atoi(argv[2]);

    //calculate index size
    getIndex(&predictionValues, &maskData);

    //allocate space for prediction table
    int *predictionTable = (int *)malloc((int)predictionValues.indexSize * sizeof(int));

    //initialize values in prediction table to be 2 or weakly taken
    initializePredictionTable(predictionTable, &predictionValues);

    //create index mask
    getIndexMask(&maskData, &predictionValues);

    //read the trace file
    if (addressData.filePath == NULL) { //file declaration
		puts("File could not be opened.");
	}
	else {
        char lineString[64];
        int tempCounter = 0;
        while(fgets(lineString, 64, addressData.filePath) != NULL) {
            //parse input and calculate index
            sscanf(lineString,"%s %c", &addressData.address, &addressData.actualHit);
            addressData.addressValue = strtoll(addressData.address, NULL, 16);
            getIndexFromAddress(&maskData, &addressData);
            getCurrentIndex(&maskData, &addressData, &predictionValues);
            predictionData.expectedPath = predictionTable[addressData.currentIndex];
            //switch between cases of the value in the prediction table at that index
            switch (predictionData.expectedPath) {
                case 0:
                    predictionData.expectedHit = 'n';
                    if (predictionData.expectedHit == addressData.actualHit) {
                        hitData.hits++;
                    } else {
                        hitData.misses++;
                        predictionTable[addressData.currentIndex] += 1;
                    }
                    break;
                case 1:
                    predictionData.expectedHit = 'n';
                    if (predictionData.expectedHit == addressData.actualHit) {
                        hitData.hits++;
                        predictionTable[addressData.currentIndex] -= 1;
                    } else {
                        hitData.misses++;
                        predictionTable[addressData.currentIndex] += 1;
                    }
                    break;
                case 2:
                    predictionData.expectedHit = 't';
                    if (predictionData.expectedHit == addressData.actualHit) {
                        hitData.hits++;
                        predictionTable[addressData.currentIndex] += 1;
                    } else {
                        hitData.misses++;
                        predictionTable[addressData.currentIndex] -= 1;
                    }
                    break;
                case 3:
                    predictionData.expectedHit = 't';
                    if (predictionData.expectedHit == addressData.actualHit) {
                        hitData.hits++;
                    } else {
                        hitData.misses++;
                        predictionTable[addressData.currentIndex] -= 1;
                    }
                    break;
            }
            //calculate new GHB (nValue)
            if (addressData.actualHit == 't') {
                predictionValues.nValue = (predictionValues.nValue) >> 1;
                predictionValues.nValue = (predictionValues.nValue) | ((int)pow(2, (predictionValues.nBits - 1)));
            } else {
                predictionValues.nValue = (predictionValues.nValue) >> 1;
            }
        }
        fclose(addressData.filePath);
        //calculate miss prediction rate
        unsigned long long totalHitsAndMisses = hitData.hits + hitData.misses;
        double missRatio = hitData.misses / (totalHitsAndMisses * 1.0);
        double missRate = (missRatio * 100.0);
        //output results
        printf("Miss Prediction Rate: %.2f%%", missRate);
    }

    //free memory used with mallocs
    flushPredictionTable(predictionTable);
    free(addressData.filePath);
}


void getIndexMask(struct maskData *maskData, struct predictionTableData *predictionValues) {
    maskData->indexMask = (predictionValues->indexSize - 1) << 2;
}

void getIndexFromAddress(struct maskData *maskData, struct addressData *addressData) {
    addressData->addressIndex = ((maskData->indexMask) & (addressData->addressValue)) >> 2;
}

void getCurrentIndex(struct maskData *maskData, struct addressData *addressData, struct predictionTableData *predictionValues) {
    addressData->currentIndex = (predictionValues->nValue << (maskData->mBits - predictionValues->nBits)) ^ (addressData->addressIndex);
}

void getIndex(struct predictionTableData *predictionValues, struct maskData *maskData) {
    predictionValues->indexSize = (int)pow(2, maskData->mBits);
}

void initializePredictionTable(int *predictionTable, struct predictionTableData *predictionValues) {
    for (int i = 0; i < (predictionValues->indexSize); i++) {
        predictionTable[i] = 2;
    } 
}

void printPredictionTable(int *predictionTable, struct predictionTableData *predictionValues) {
    for (int i = 0; i < (predictionValues->indexSize); i++) {
        printf("%d\n", predictionTable[i]);
    }
}

void flushPredictionTable(int *predictionTable) {
    free(predictionTable);
}