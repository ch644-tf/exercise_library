#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define WARNING_TEXT_COLOR "\033[1;33m"
#define ERROR_TEXT_COLOR "\033[1;31m"
#define INFO_TEXT_COLOR "\033[1;32m"
#define RETURN_TEXT_COLOR "\033[1;0m"
#define RET_TYPE_BUFFER_SIZE 20
#define BUFFER_SIZE 100

typedef enum error {
  OK,
  BUFFER_OVERFLOW,
  OVERFLOW,
  UNDERFLOW,
  INVALID_CHAR,
  EMPTY_BUFFER,
} error;

typedef struct ret_type_t{
  void* buffer;
  enum error retError;
}ret_type_t;

struct ret_type_t* ret_type(void);
void destroyRetType(struct ret_type_t** );

unsigned char checkValidCharacter(char);
//ignoring the return on these functions WILL cause a memory leak
struct ret_type_t* convertString(const char* ,unsigned char ) __attribute_warn_unused_result__;
struct ret_type_t* reverseString(const char* , const size_t ) __attribute_warn_unused_result__;
struct ret_type_t* convertInteger(long long int ) __attribute_warn_unused_result__;
struct ret_type_t *convertUnsignedInteger(unsigned long long int) __attribute_warn_unused_result__;

void* parseLine(void* );
enum error printError(enum error);
enum error printErrorFile(enum error, FILE* );
FILE* openFile(const char* , const char* );
FILE* openAppendFile(const char* );
FILE* openWriteFile(const char* );
FILE *openReadFile(const char *);
ret_type_t* parseDIMACS(FILE *, FILE *, size_t) __attribute_warn_unused_result__;
enum error removeTrailingZero(char *);

typedef struct clause_t {
  struct clause_t *neighbors[2];
  long long int *variables;
  size_t numVars;
  bool resolved;
  long long int* resolvedFrom;
} clause_t;
typedef struct proof_t {
  struct clause_t *head;
  struct clause_t *tail;
  pthread_mutex_t proofMutex;
} proof_t;

proof_t initProof();
void destroyProof(proof_t*);
enum error addClause(struct proof_t *, struct clause_t *);
enum error removeClause(proof_t *, size_t);
clause_t *getClause(const proof_t *, size_t);
clause_t makeClause(void);

#endif
