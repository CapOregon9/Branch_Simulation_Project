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
    char * filePath;
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
    FILE *cfPtr;
    struct predictionTableData predictionValues;
    struct predictionData predictionData;
    struct maskData maskData;
    struct hitData hitData;
    struct addressData addressData;
    hitData.hits = 0;
    hitData.misses = 0;
    predictionValues.nValue = 0;

    //allocate space for file path
    addressData.filePath = (char *)malloc(sizeof(argv[3]) * sizeof(char));

    //store arguments
    maskData.mBits = atoi(argv[1]);
    //printf("%d\n", maskData.mBits);
    predictionValues.nBits = atoi(argv[2]);
    //printf("%d\n", predictionValues.nBits);
    strcpy(addressData.filePath,argv[3]);
    //printf("%s\n", addressData.filePath);
    //maskData.mBits = 8;
    //predictionValues.nBits = 5;
    //addressData.filePath = "D:/Documents/Comp_Arch/gobmk_trace.txt";

    //calculate index size
    getIndex(&predictionValues, &maskData);

    int *predictionTable = (int *)malloc((int)predictionValues.indexSize * sizeof(int));

    initializePredictionTable(predictionTable, &predictionValues);
    //printPredictionTable(predictionTable, &predictionValues);


    //create index mask
    getIndexMask(&maskData, &predictionValues);

    if ((cfPtr = fopen(addressData.filePath, "r")) == NULL) { //file declaration
		puts("File could not be opened.");
	}
	else {
        char lineString[64];
        int tempCounter = 0;
        while(fgets(lineString, 64, cfPtr) != NULL) {
            sscanf(lineString,"%s %c", &addressData.address, &addressData.actualHit);
            addressData.addressValue = strtoll(addressData.address, NULL, 16);
            //printf("%lld\n", addressData.addressValue);
            getIndexFromAddress(&maskData, &addressData);
            //printf("%d\n", addressData.addressIndex);
            getCurrentIndex(&maskData, &addressData, &predictionValues);
            //printf("%d\n", addressData.currentIndex);
            predictionData.expectedPath = predictionTable[addressData.currentIndex];
            switch (predictionData.expectedPath) {
                case 0:
                    predictionData.expectedHit = 'n';
                    if (predictionData.expectedHit == addressData.actualHit) {
                        hitData.hits++;
                        //printf("Case 1: I did a hit\n");
                    } else {
                        hitData.misses++;
                        predictionTable[addressData.currentIndex] += 1;
                        //printf("Case 1: I did a miss\n");
                    }
                    break;
                case 1:
                    predictionData.expectedHit = 'n';
                    if (predictionData.expectedHit == addressData.actualHit) {
                        hitData.hits++;
                        predictionTable[addressData.currentIndex] -= 1;
                        //printf("Case 1: I did a hit\n");
                    } else {
                        hitData.misses++;
                        predictionTable[addressData.currentIndex] += 1;
                        //printf("Case 1: I did a miss\n");
                    }
                    break;
                case 2:
                    predictionData.expectedHit = 't';
                    if (predictionData.expectedHit == addressData.actualHit) {
                        hitData.hits++;
                        predictionTable[addressData.currentIndex] += 1;
                        //printf("Case 2: I did a hit\n");
                    } else {
                        hitData.misses++;
                        predictionTable[addressData.currentIndex] -= 1;
                        //printf("Case 2: I did a miss\n");
                    }
                    break;
                case 3:
                    predictionData.expectedHit = 't';
                    if (predictionData.expectedHit == addressData.actualHit) {
                        hitData.hits++;
                        //printf("Case 3: I did a hit\n");
                    } else {
                        hitData.misses++;
                        predictionTable[addressData.currentIndex] -= 1;
                        //printf("Case 3: I did a miss\n");
                    }
                    break;
            }
            if (addressData.actualHit == 't') {
                predictionValues.nValue = (predictionValues.nValue) >> 1;
                predictionValues.nValue = (predictionValues.nValue) | ((int)pow(2, (predictionValues.nBits - 1)));
            } else {
                predictionValues.nValue = (predictionValues.nValue) >> 1;
            }
            //printf("%d\n", predictionValues.nValue);
        }
        unsigned long long totalHitsAndMisses = hitData.hits + hitData.misses;
        double missRatio = hitData.misses / (totalHitsAndMisses * 1.0);
        double missRate = (missRatio * 100.0);
        printf("Miss Prediction Rate: %.2f%%", missRate);
    }

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