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

enum error{
  OK,
  BUFFER_OVERFLOW,
  OVERFLOW,
  UNDERFLOW,
  INVALID_CHAR,
  EMPTY_BUFFER,
};

typedef struct ret_type_t{
  void* buffer;
  enum error retError;
}ret_type_t;

struct ret_type_t* ret_type(void);
void destroyRetType(struct ret_type_t** );

unsigned char checkValidCharacter(char );
struct ret_type_t* convertString(const char* );
struct ret_type_t* reverseString(const char* , const size_t );
struct ret_type_t* convertInteger(long long int );
struct ret_type_t *convertUnsignedInteger(unsigned long long int);

void* parseLine(void* );
enum error printError(enum error);
enum error printErrorFile(enum error, FILE* );
FILE* openFile(const char* , const char* );
FILE* openAppendFile(const char* );
FILE* openWriteFile(const char* );
FILE* openReadFile(const char* );
enum error parseDIMCAS(FILE *, FILE *, size_t);
enum error removeTrailingZero(char *);

#endif
