#include "contract.h"
#include <stdlib.h>

struct ContractInfo* initIoServerContract(size_t capacity){
    struct ContractInfo* contractinfo = (struct ContractInfo*)malloc(sizeof(struct ContractInfo));
    contractinfo->capacity=capacity;
    struct Playerinfo **playerinfo = (struct Playerinfo **)malloc(capacity*sizeof(struct Playerinfo *));
    char *filled = (char*)malloc(capacity*sizeof(char));
    char *snapshot = (char*)malloc(capacity*sizeof(char));
    contractinfo->filled=filled;
    contractinfo->snapshots=snapshot;
    return contractinfo;
}
