#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#define __STDC_WANT_LIB_EXT1__ 1

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define WARNING_TEXT_COLOR "\033[1;33m"
#define ERROR_TEXT_COLOR "\033[1;31m"
#define INFO_TEXT_COLOR "\033[1;32m"
#define RETURN_TEXT_COLOR "\033[1;0m"
#define RET_TYPE_BUFFER_SIZE 20
#define BUFFER_SIZE 100
#define NUM_THREADS 50

typedef enum error {
  OK,
  BUFFER_OVERFLOW,
  OVERFLOW,
  UNDERFLOW,
  INVALID_CHAR,
  EMPTY_BUFFER,
} error;

typedef struct ret_type_t {
  void *buffer;
  enum error retError;
} ret_type_t;

struct ret_type_t *ret_type(void);
void destroyRetType(struct ret_type_t **destroy);

unsigned char checkValidCharacter(char checkFor);
// ignoring the return on these functions WILL cause a memory leak
struct ret_type_t *convertString(const char *toConvert, bool forceSigned)
    __attribute_warn_unused_result__;
struct ret_type_t *reverseString(const char *string, const size_t stringSize)
    __attribute_warn_unused_result__;
struct ret_type_t *
convertInteger(long long int toConvert) __attribute_warn_unused_result__;
struct ret_type_t *convertUnsignedInteger(unsigned long long int toConvert)
    __attribute_warn_unused_result__;

void *parseLine(void *input);
enum error printError(enum error toPrint);
enum error printErrorFile(enum error toPrint, FILE *printTo);
FILE *openFile(const char *pathToFile, const char *openMode);
FILE *openAppendFile(const char *pathToFile);
FILE *openWriteFile(const char *pathToFile);
FILE *openReadFile(const char *pathToFile);
ret_type_t *parseDIMACS(FILE *fileToParse,
                        FILE *writeTo) __attribute_warn_unused_result__;
enum error removeTrailingZero(char *toModify);

typedef struct clause_t {
  struct clause_t *neighbors[2];
  long long int *variables;
  size_t numVars;
  bool resolved;
  long long int *resolvedFrom;
  size_t id;
} clause_t;
typedef struct proof_t {
  struct clause_t *head;
  struct clause_t *tail;
  pthread_mutex_t proofMutex;
} proof_t;

proof_t initProof(void);
void destroyProof(proof_t *toDestroy);
enum error addClause(struct proof_t *addTo, struct clause_t *toAdd);
enum error removeClause(proof_t *removeFrom, size_t idToRemove);
clause_t *getClause(const proof_t *getFrom, size_t idToGet);
clause_t makeClause(void);

typedef struct thread_queue_t {
  int position;
  pthread_mutex_t mutex;
  pthread_cond_t condition;
  char ***buffer;
} thread_queue_t;

enum error parseLRATLine(char **array, long long int *previousID);
void *threadFunction(void *input);
char **splitLine(char *inputString, char delim);
thread_queue_t *initQueue(void);
void destroyQueue(thread_queue_t **toDestroy);
int queuePop(thread_queue_t *queue, char ***output);
int queuePush(thread_queue_t *queue, char **input);
int qSortComp(const void *lhc, const void *rhc);
bool check_RAT(clause_t *clauseToModify, size_t numVariables, size_t numHints);
size_t removeFromArray(long long int *array, size_t arraySize,
                       long long int toRemove);
bool arrayContains(long long int *array, size_t arraySize,
                   long long int checkFor);
bool UCP(clause_t tempClause, clause_t *clauseToModify);
long long int *combineArrays(long long int *appendTo, long long int *appendFrom,
                             size_t appendToSize, size_t appendFromSize);

#endif
