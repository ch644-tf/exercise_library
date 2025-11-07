#include "functions.h"
#include <bits/pthreadtypes.h>
#include <locale.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

struct ret_type_t *ret_type(void) {
  struct ret_type_t *ret = malloc(sizeof(ret_type_t));
  ret->buffer = malloc(RET_TYPE_BUFFER_SIZE);
  ret->retError = OK;
  return ret;
}

void destroyRetType(struct ret_type_t **destroy) {
  free((*destroy)->buffer);
  free(*destroy);
  *destroy = NULL;
}

unsigned char checkValidCharacter(char check) {
  // unsigned char isUnsusedChar = check == 157 || check == 144 || check == 143
  // || check == 141 || check == 129; //unused ascii characters
  unsigned char isNegative = check < 0;
  unsigned char isWhitespace = check == ' ' || check == '\n';
  return /* isUnsusedChar || */ (isNegative ||
                                 (check < ' ' && check != '\0')) &&
         !isWhitespace;
}

struct ret_type_t *convertString(const char *string) {
  size_t numCharacters = strlen(string);
  struct ret_type_t *result = ret_type();
  unsigned char signedBool;
  if (numCharacters > 20) {
    fprintf(
        stderr,
        "%sERROR:%s String to convert to integer is too large of an integer",
        ERROR_TEXT_COLOR, RETURN_TEXT_COLOR);
    result->retError = BUFFER_OVERFLOW;
    return result;
  } else if (numCharacters == 20) {
    if (string[0] == '-') {
      goto signed_result;
    }
    signedBool = 0;
  } else {
  signed_result:
    signedBool = 1;
  }
  unsigned long long res = 0;
  unsigned char power = 0;
  signed char neg = 1;

  for (int index = numCharacters - 1; index >= 0; index--) {
    if (string[index] >= '0' && string[index] <= '9') {
      res += powl(10, power) * (string[index] - '0');
      power++;
    } else if (string[index] == '-' && !index) {
      neg *= -1;
    } else if (checkValidCharacter(string[index])) {
      fprintf(stderr, "%sERROR:%s Attempted to check mangled character: 0x%X\n",
              ERROR_TEXT_COLOR, RETURN_TEXT_COLOR, (int)string[index]);
      result->retError = INVALID_CHAR;
      return result;
    } else {
      fprintf(stderr,
              "%sWARNING:%s Attempted to convert a non-numeric character into "
              "a numeral: %c\n",
              WARNING_TEXT_COLOR, RETURN_TEXT_COLOR, string[index]);
    }
  }
  if (signedBool) {
    *(long long int *)result->buffer = res * neg;
  } else {
    *(unsigned long long int *)result->buffer = res;
  }

  return result;
}

// do not assign the output of this function to the same pointer used as the
// string input, since the string isn't free'd here then the memory that string
// points to will be lost
struct ret_type_t *
reverseString(const char *string,
              const size_t numCharacters) { // this has a funny bug
  struct ret_type_t *result = ret_type();
  if (numCharacters > RET_TYPE_BUFFER_SIZE) {
    result->retError = BUFFER_OVERFLOW;
    return result;
  }
  char *buffer = (char *)malloc(sizeof(char) * numCharacters);
  if (buffer == 0) {
    return NULL;
  }
  for (size_t index = 0; index < numCharacters; index++) {
    buffer[index] = string[numCharacters - 1 - index];
  }
  free(result->buffer);
  result->buffer = (void *)buffer;
  return result;
}

struct ret_type_t *convertInteger(long long int integer) {
  signed char negative = integer < 0 ? -1 : 1;
  const unsigned char CHARTOINTOFFSET = '0';
  size_t count = 0;
  char *buffer = (char *)malloc(sizeof(char));
  struct ret_type_t *result = ret_type();

  if (buffer == 0) {
    result->retError = EMPTY_BUFFER;
    return result;
  }
  if (integer == 0) {
    buffer = (char *)realloc(buffer, sizeof(char) * 2);
    buffer[0] = '0';
    buffer[1] = '\0';
    result->buffer = buffer;
    return result;
  }

  integer *= negative;

  while (integer) {
    count++;

    buffer = (char *)realloc(result, sizeof(char) * count);
    if (result == 0) {
      return NULL;
    }
    buffer[count - 1] = (integer % 10) + CHARTOINTOFFSET;

    integer -= integer % 10;
    integer /= 10;
  }
  if (negative) {
    count++;
    buffer = (char *)realloc(buffer, sizeof(char) * count);
    if (result == 0) {
      return NULL;
    }
    ((char *)result->buffer)[0] = '-';
  }
  struct ret_type_t *reversed_string = reverseString(buffer, count);
  if (reversed_string->retError) {
    result->retError = reversed_string->retError;
    destroyRetType(&reversed_string);
    return result;
  }
  free(result->buffer);
  result->buffer = reversed_string->buffer;
  destroyRetType(&reversed_string);
  return result;
}

struct ret_type_t *convertUnsignedInteger(unsigned long long int integer) {
  const unsigned char CHARTOINTOFFSET = '0';
  struct ret_type_t *result = ret_type();
  if (result == 0) {
    return NULL;
  }
  if (result->buffer == 0) {
    result->retError = EMPTY_BUFFER;
    return result;
  }

  if (integer == 0) {
    ((char *)result->buffer)[0] = '0';
    ((char *)result->buffer)[1] = '\0';
    return result;
  }

  for (; integer; integer /= 10) {
    *(char *)result->buffer = (integer % 10) + CHARTOINTOFFSET;
  }
  struct ret_type_t *reversed_string =
      reverseString((char *)result->buffer, strlen((char *)result->buffer));
  if (reversed_string->retError) {
    result->retError = reversed_string->retError;
  } else {
    free(result->buffer);
    result->buffer = reversed_string->buffer;
  }
  destroyRetType(&reversed_string);
  return result;
}

//input should be {pthread_mutex_t*, pthread_mutex_t*,char*,FILE*,const unsigned int*}
void *parseLine(void *input) {
  // copy input to local buffer to prevent race conditions between other threads
  // (yes this was a problem)
  pthread_mutex_lock(((pthread_mutex_t **)input)[0]);
  char buffer[BUFFER_SIZE];
  memcpy(buffer, ((void**) input)[2], BUFFER_SIZE);
  pthread_mutex_unlock(((pthread_mutex_t **)input)[0]);
  // create other variables
  void **input_array = (void **)input;
  pthread_mutex_t *output_lock = (pthread_mutex_t*) input_array[1];
  FILE *output = (FILE *)input_array[3];
  const unsigned int numVars = *(const unsigned int*)input_array[4];
  enum error *result = malloc(sizeof(enum error));
  *result = OK;
  char **tokens = malloc(sizeof(char *));
  // split input string into tokens and store in tokens
  tokens[0] = strtok(buffer, " ");
  size_t numTokens = 1;
  while (tokens[numTokens - 1] && strcmp(tokens[numTokens - 1], "0")) {
    numTokens++;
    tokens = realloc(tokens, numTokens * sizeof(char *));
    tokens[numTokens - 1] = strtok(NULL, " ");
  }
  // check if comment
  if (tokens[0][0] == 'c' || tokens[0][0] == 'C') {
    return result;
  }

  // convert string tokens into variables
  long long int *variables = malloc(sizeof(long long int) * numTokens);
  if (variables == 0) {
    *result = EMPTY_BUFFER;
    return result;
  }
  struct ret_type_t *function_return;
  for (size_t index = 0; index < numTokens - 1; index++) {
    function_return = convertString(tokens[index]);
    variables[index] = *((unsigned long long int *)function_return->buffer);
    if (variables[index] > numVars) {
      *result = INVALID_CHAR;
    }
    destroyRetType(&function_return);
  }
  free(tokens);
  tokens = NULL;
  // all the variables in the line are in variables[]
  // all variables in the same clause are OR'd
  // all clauses are AND'd
  pthread_mutex_lock(output_lock);
  fprintf(output, "(");
  for (size_t index = 0; index < numTokens - 2; index++) {
    if (variables[index] >= 0) {
      if (fprintf(output, "%lli %lc ", variables[index], L'∨') < 0) {
        perror("fprintf");
        *result = INVALID_CHAR;
      }
    } else {
      if (fprintf(output, "%lc%lli %lc ", L'¬', variables[index] * -1, L'∨') <
          0) {
        perror("fprintf");
        *result = INVALID_CHAR;
      }
    }
  }
  fprintf(output, "%lli) %lc\n", variables[numTokens - 2], L'∧');
  pthread_mutex_unlock(output_lock);

  free(variables);
  return result;
}

enum error printError(enum error input) {
  return printErrorFile(input, stderr);
}

enum error printErrorFile(enum error inputError, FILE *outputFile) {
  fprintf(outputFile, "%sERROR%s: ", ERROR_TEXT_COLOR, RETURN_TEXT_COLOR);
  switch (inputError) {
  case BUFFER_OVERFLOW:
    fprintf(outputFile,"Attempted out of bounds read/write of buffer");
    break;
  case OVERFLOW:
    fprintf(outputFile,"Caught an integer overflow");
    break;
  case UNDERFLOW:
    fprintf(outputFile,"Caught an integer underflow");
    break;
  case INVALID_CHAR:
    fprintf(outputFile,"Invalid input");
    break;
  case EMPTY_BUFFER:
    fprintf(outputFile, "Buffer was malformed or null");
    break;
  default:
    fprintf(outputFile, "Unknown error or no error: %d",inputError);
    break;
  }
  fprintf(outputFile, "\n");
  return inputError;
}

FILE *openFile(const char *path, const char *args) {
  FILE *output = fopen(path, args);
  if (output == NULL) {
    printError(EMPTY_BUFFER);
  }
  return output;
}

FILE *openAppendFile(const char *path) {
  return openFile(path, "a");
}

FILE *openWriteFile(const char *path) {
  return openFile(path, "w");
}

FILE *openReadFile(const char *path) {
  return openFile(path, "r");
}

enum error parseDIMCAS(FILE *inputFile, FILE *outputFile, size_t numThreads) {
  char *oldLocale = setlocale(LC_ALL, NULL);
  setlocale(LC_ALL, "C.UTF-8");
  pthread_mutex_t buffer_lock, output_lock;
  pthread_mutex_init(&buffer_lock, NULL);
  pthread_mutex_init(&output_lock, NULL);
  char buffer[BUFFER_SIZE];
  char *lineOne[] = {malloc(sizeof(char) * 3), malloc(sizeof(char) * 5),
                     malloc(sizeof(char) * (BUFFER_SIZE - 8) / 2),
                     malloc(sizeof(char) * (BUFFER_SIZE - 8) / 2)};
  pthread_t threadIDArray[numThreads];
  unsigned char count = 0;
  enum error *return_error;
  enum error result = OK;
  void *thread_function_input[5];
  char scan_string[50];
  struct ret_type_t
      *function_return; // function_return = convertString(lineOne[2])
  unsigned int numClauses;
  fgets(buffer, BUFFER_SIZE, inputFile);
  while (buffer[0] == 'c' || buffer[0] == 'C') {
    fgets(buffer, BUFFER_SIZE, inputFile);
  }
  sprintf(scan_string, " %%3[^ ] %%5[^ ] %%%d[^ ] %%%d[^ \\n] ",
          (BUFFER_SIZE - 8) / 2, (BUFFER_SIZE - 8) / 2);
  if (!sscanf(buffer, scan_string, lineOne[0], lineOne[1], lineOne[2],
              lineOne[3])) {
    result = printError(INVALID_CHAR);
    goto freeLineOne;
  }
  function_return = convertString(lineOne[2]);
  if (function_return->retError) {
    result = printError(function_return->retError);
    goto freeRetType;
  }
  const unsigned int NUM_VARIABLES = *(unsigned int *)function_return->buffer;
  destroyRetType(&function_return);
  function_return = convertString(lineOne[3]);
  if (function_return->retError) {
    result = printError(function_return->retError);
    goto freeRetType;
  }
  const unsigned int NUM_CLAUSES = *(unsigned int *)function_return->buffer;
  numClauses = NUM_CLAUSES;
  destroyRetType(&function_return);
  while (numClauses) {
    for (; count < numThreads && count < numClauses; count++) {
      pthread_mutex_lock(&buffer_lock);
      fgets(buffer, BUFFER_SIZE, inputFile);
      pthread_mutex_unlock(&buffer_lock);
      if (removeTrailingZero(buffer)) {
	result = printError(BUFFER_OVERFLOW);
      }
      thread_function_input[0] = (void *)&buffer_lock;
      thread_function_input[1] = (void *)&output_lock;
      thread_function_input[2] = (void *)buffer;
      thread_function_input[3] = (void *)outputFile;
      thread_function_input[4] = (void *)&NUM_VARIABLES;
      pthread_create(&threadIDArray[count], NULL, parseLine,(void*) thread_function_input);
    }
    for (; count; count--) {
      pthread_join(threadIDArray[count - 1], (void **)&return_error);
      numClauses--;
      if (*return_error) {
        result = printError(*return_error);
	goto freeReturnError;
      }
      free(return_error);
      return_error = NULL;
    }
  }

freeReturnError:
  if(return_error != NULL){
    free(return_error);
    return_error = NULL;
  }
freeRetType:
  if(function_return != NULL){
    destroyRetType(&function_return);
  }
 freeLineOne:
   pthread_mutex_destroy(&buffer_lock);
   pthread_mutex_destroy(&output_lock);
   free(lineOne[0]);
   lineOne[0] = NULL;
   free(lineOne[1]);
   lineOne[1] = NULL;
   free(lineOne[2]);
   lineOne[2] = NULL;
   free(lineOne[3]);
   lineOne[3] = NULL;
   setlocale(LC_ALL, oldLocale);
   return result;
}

enum error removeTrailingZero(char *input) {
  char *zero;
  char *whitespace[] = {" 0 ",   " 0\t", " 0\n",  "\t0 ", "\t0\t",
                        "\t0\n", "\n0 ", "\n0\t", "\n0\n"};
  for (size_t index = 0; index < 9; index++) {
    zero = strstr(input, whitespace[index]);
    if (zero) {
      zero[0] = '\0';
      return OK;
    }
  }
  return INVALID_CHAR;
}
