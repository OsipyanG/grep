// Copyright 2023 Osipyan Gurgen
//
#ifndef GREP_PARAMS_H
#define GREP_PARAMS_H

#include <stdbool.h>

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

enum ERRORS {
  PARSING_ERROR = 1,
  MALLOC_ERROR = 2,
  REG_ERROR = 3,
  FILE_ERROR = 4,
  TEMP_ERROR = 5,
  FILE_IS_BINARY = 6,
};

typedef struct {
  int e;
  bool i;
  bool v;
  bool c;
  bool l;
  bool n;
  bool h;
  bool s;
  int f;
  bool o;
  char **e_args;
  char **f_args;
} Flags;

typedef struct {
  int count_of_files;
  int count_of_patterns;
  char **files;
  char **patterns;
} Grep_params;

#endif
