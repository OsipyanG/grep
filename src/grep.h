// Copyright 2023 Osipyan Gurgen
#ifndef GREP_H
#define GREP_H

#define _GNU_SOURCE

#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "./params.h"

int open_files(Grep_params *params, Flags *flags);

int grep(regex_t *regex, FILE *fp, char *file_name, Grep_params *params,
         Flags *flags);

int match_grep(regex_t *regex, FILE *fp, char *file_name, Grep_params *params,
               Flags *flags);

bool is_minus(char *file);

void clear_params(Grep_params *params, Flags *flags);

int join_all_patterns(Grep_params *params, char **all_patterns);

int calc_count_of_chars(Grep_params *params);

#endif // GREP_H
