// Copyright 2023 Osipyan Gurgen

#include "./parser.h"

#include "./grep.h"

void init_flags(Flags *flags) {
  flags->e = false;
  flags->i = false;
  flags->v = false;
  flags->c = false;
  flags->l = false;
  flags->n = false;
  flags->h = false;
  flags->s = false;
  flags->f = false;
  flags->o = false;
  flags->e_args = NULL;
  flags->f_args = NULL;
}

void init_grep_params(Grep_params *params) {
  params->files = NULL;
  params->patterns = NULL;
  params->count_of_files = 0;
  params->count_of_patterns = 0;
}

int parse_args(int argc, char **argv, Flags *flags, Grep_params *params) {
  int err;
  err = calc_e_f_flags(argc, argv, flags);
  if (err != EXIT_SUCCESS) {
    print_usage();
    return PARSING_ERROR;
  }
  err = parse_flags(argc, argv, flags);

  if (err != EXIT_SUCCESS) {
    print_usage();
    return PARSING_ERROR;
  }

  if (flags->e || flags->f) {
    params->count_of_files = argc - optind;
  } else {
    params->count_of_files = argc - optind - 1;
  }
  if (params->count_of_files > 0) {
    params->files = (char **)calloc(params->count_of_files, sizeof(char *));

    if (params->files == NULL) {
      return MALLOC_ERROR;
    }
  }
  err = parse_grep_params(argc, argv, flags, params);
  if (err) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int calc_e_f_flags(int argc, char **argv, Flags *flags) {
  int c;

  while (true) {
    c = getopt(argc, argv, "e:ivclnhsf:o");
    if (c == -1)
      break;
    switch (c) {
    case 'e':
      flags->e++;
      break;
    case 'f':
      flags->f++;
      break;
    case '?':
      return PARSING_ERROR;
    default: {
    };
    }
  }
  optind = 0;
  return EXIT_SUCCESS;
}

int parse_flags(int argc, char **argv, Flags *flags) {
  int c;
  int e_index = 0;
  int f_index = 0;

  if (flags->e > 0) {
    flags->e_args = calloc(flags->e, sizeof(char *));

    if (flags->e_args == NULL) {
      return MALLOC_ERROR;
    }
  }

  if (flags->f > 0) {
    flags->f_args = calloc(flags->f, sizeof(char *));

    if (flags->f_args == NULL) {
      return MALLOC_ERROR;
    }
  }

  while (true) {
    c = getopt(argc, argv, "e:ivclnhsf:o");
    if (c == -1)
      break;
    switch (c) {
    case 'e':
      flags->e_args[e_index++] = optarg;
      break;
    case 'i':
      flags->i = true;
      break;
    case 'v':
      flags->v = true;
      break;
    case 'c':
      flags->c = true;
      break;
    case 'l':
      flags->l = true;
      break;
    case 'n':
      flags->n = true;
      break;
    case 'h':
      flags->h = true;
      break;
    case 's':
      flags->s = true;
      break;
    case 'f':
      flags->f_args[f_index++] = optarg;
      break;
    case 'o':
      flags->o = true;
      break;
    case '?':
      return PARSING_ERROR;
    default: {
    };
    }
  }
  return EXIT_SUCCESS;
}

int parse_grep_params(int argc, char **argv, Flags *flags,
                      Grep_params *params) {
  int file_index = 0;
  int pattern_index = 0;
  size_t len_of_str;
  int err;
  if (flags->e > 0 || flags->f > 0) {
    params->count_of_patterns += flags->e;
    err = calc_count_of_patterns_from_files(flags, params);
    if (err) {
      clear_params(params, flags);
      exit(FILE_ERROR);
    }
    params->patterns =
        (char **)calloc(params->count_of_patterns, sizeof(char *));
    if (params->patterns == NULL) {
      return MALLOC_ERROR;
    }

    for (int i = 0; i < flags->e; i++) {
      len_of_str = strlen(flags->e_args[i]);
      params->patterns[pattern_index] = calloc(len_of_str + 1, sizeof(char));
      if (params->patterns[pattern_index] == NULL) {
        return MALLOC_ERROR;
      }
      strcat(params->patterns[pattern_index], flags->e_args[i]);
      pattern_index++;
    }
    for (int i = 0; i < flags->f; i++) {
      err = parse_patterns_from_file(flags->f_args[i], params, &pattern_index,
                                     flags->s);
      if (err == FILE_IS_BINARY) {
        if (!flags->s)
          printf("grep: %s: The file is binary or contains invalid characters",
                 flags->f_args[i]);
        clear_params(params, flags);
        exit(FILE_IS_BINARY);
      }
    }

  } else {
    params->count_of_patterns = 1;
    params->patterns =
        (char **)calloc(params->count_of_patterns, sizeof(char *));
    if (params->patterns == NULL) {
      return MALLOC_ERROR;
    }
    len_of_str = strlen(argv[optind]) + 1;
    params->patterns[pattern_index] = calloc(len_of_str, sizeof(char));

    if (params->patterns[pattern_index] == NULL) {
      return MALLOC_ERROR;
    }
    memcpy(params->patterns[pattern_index], argv[optind], len_of_str);
    optind++;
  }

  for (int i = optind; i < argc; i++) {
    params->files[file_index] = argv[i];
    file_index++;
  }
  return EXIT_SUCCESS;
}

int check_arguments(int argc) {
  if (argc == 1 || (argc - optind == -1)) {
    return TEMP_ERROR;
  }
  return EXIT_SUCCESS;
}

int parse_patterns_from_file(char *file_name, Grep_params *params,
                             int *pattern_index, bool s_flag) {
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  size_t len_of_str;

  fp = fopen(file_name, "rb");
  if (!fp) {
    if (!s_flag)
      warn("%s", file_name);
    return FILE_ERROR;
  }

  int isBinary = 0;
  unsigned char buffer[4];
  size_t bytesRead = fread(buffer, sizeof(unsigned char), 4, fp);

  for (size_t i = 0; i < bytesRead; i++) {
    if (!isprint(buffer[i]) && !isspace(buffer[i])) {
      isBinary = 1;
      break;
    }
  }

  if (isBinary) {
    fclose(fp);
    return FILE_IS_BINARY;
  }
  fclose(fp);

  fp = fopen(file_name, "r");
  if (!fp) {
    if (!s_flag)
      warn("%s", file_name);
    return EXIT_FAILURE;
  }

  while ((getline(&line, &len, fp)) != -1) {
    len_of_str = strlen(line);

    params->patterns[*pattern_index] = calloc(len_of_str, sizeof(char));
    if (params->patterns[*pattern_index] == NULL) {
      return MALLOC_ERROR;
    }
    memcpy(params->patterns[*pattern_index], line, len_of_str - 1);

    (*pattern_index)++;
  }
  if (line)
    free(line);

  fclose(fp);
  return EXIT_SUCCESS;
}

int calc_count_of_patterns_from_files(Flags *flags, Grep_params *params) {
  FILE *fp;
  char buff[128];
  for (int i = 0; i < flags->f; i++) {
    fp = fopen(flags->f_args[i], "r");
    if (!fp) {
      if (!flags->s)
        warn("%s", flags->f_args[i]);
      return EXIT_FAILURE;
    }
    while ((fgets(buff, 128, fp) != NULL)) {
      params->count_of_patterns++;
    }
    fclose(fp);
  }
  return EXIT_SUCCESS;
}

void print_usage() {
  fprintf(stderr, "Usage: grep [OPTION]... PATTERNS [FILE]...");
  fprintf(stderr, "Try 'grep --help' for more information.");
}
