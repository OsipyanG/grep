// Copyright 2023 OsipyanG
#ifndef PARSER_H
#define PARSER_H

#define _GNU_SOURCE
#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <getopt.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./params.h"

void init_flags(Flags *flags);
void init_grep_params(Grep_params *params);

int parse_args(int argc, char **argv, Flags *flags, Grep_params *params);
int parse_flags(int argc, char **argv, Flags *flags);
int parse_grep_params(int argc, char **argv, Flags *flags, Grep_params *params);
int calc_count_of_patterns_from_files(Flags *flags, Grep_params *params);
int calc_e_f_flags(int argc, char **argv, Flags *flags);

int parse_patterns_from_file(char *file_name, Grep_params *params,
                             int *pattern_index, bool s_flag);
int check_arguments(int argc);

void clear_params(Grep_params *params, Flags *flags);
void print_usage();

#endif
