// Copyright 2023 Osipyan Gurgen

#include "./grep.h"

#include "./parser.h"

int main(int argc, char *argv[]) {
  int err;
  Flags flags;
  Grep_params params;

  init_flags(&flags);
  init_grep_params(&params);

  if (argc == 1) {
    print_usage();
    return EXIT_SUCCESS;
  }

  err = parse_args(argc, argv, &flags, &params);
  if (err) {
    return err;
  }

  err = check_arguments(argc);
  if (err) {
    print_usage();
    return err;
  }

  err = open_files(&params, &flags);
  if (err) {
    return err;
  }

  clear_params(&params, &flags);
  return EXIT_SUCCESS;
}

int open_files(Grep_params *params, Flags *flags) {
  FILE *fp = NULL;
  regex_t regex;
  int cflags = REG_EXTENDED;
  int err;
  char *all_patterns = NULL;

  // i flag
  if (flags->i)
    cflags |= REG_ICASE;

  err = join_all_patterns(params, &all_patterns);
  if (err) {
    return err;
  }

  err = regcomp(&regex, all_patterns, cflags);
  if (err) {
    fprintf(stderr, "Could not compile regex\n");
    clear_params(params, flags);
    exit(REG_ERROR);
  }

  if (params->count_of_files == 0) {
    fp = stdin;
    grep(&regex, fp, "", params, flags);
  }

  for (int i = 0; i < params->count_of_files; i++) {
    if (is_minus(params->files[i])) {
      fp = stdin;
    } else {
      fp = fopen(params->files[i], "r");
      if (!fp) {
        if (!flags->s)
          warn("%s", params->files[i]);
        continue;
      }
    }
    if (flags->o && !flags->l && !flags->c) {
      match_grep(&regex, fp, params->files[i], params, flags);
    } else {
      grep(&regex, fp, params->files[i], params, flags);
    }
  }

  regfree(&regex);
  free(all_patterns);

  return EXIT_SUCCESS;
}

int join_all_patterns(Grep_params *params, char **all_patterns) {
  int count_of_chars = calc_count_of_chars(params);
  int count_of_straight_slashes = params->count_of_patterns - 1;
  int reti = 0;
  regex_t regex;
  *all_patterns =
      calloc(count_of_chars + count_of_straight_slashes + 1, sizeof(char));

  if (all_patterns == NULL) {
    return MALLOC_ERROR;
  }

  for (int i = 0; i < params->count_of_patterns; i++) {
    reti = regcomp(&regex, params->patterns[i], REG_EXTENDED);
    if (reti) {
      fprintf(stderr, "Could not compile regex\n");
      continue;
    }
    *all_patterns = strcat(*all_patterns, params->patterns[i]);
    if (i != params->count_of_patterns - 1) {
      *all_patterns = strcat(*all_patterns, "|");
    }
    regfree(&regex);
  }
  return EXIT_SUCCESS;
}

int calc_count_of_chars(Grep_params *params) {
  int count_of_chars = 0;
  for (int i = 0; i < params->count_of_patterns; i++) {
    count_of_chars += strlen(params->patterns[i]);
  }
  return count_of_chars;
}

bool is_minus(char *file) {
  if (strcmp(file, "-") == 0 || strcmp(file, "--") == 0) {
    return true;
  }
  return false;
}

void clear_params(Grep_params *params, Flags *flags) {
  if (params->count_of_patterns > 0) {
    for (int i = 0; i < params->count_of_patterns; i++) {
      if (params->patterns[i])
        free(params->patterns[i]);
    }
  }

  if (flags->e)
    free(flags->e_args);
  if (flags->f)
    free(flags->f_args);

  if (params->count_of_files)
    free(params->files);
  if (params->count_of_patterns)
    free(params->patterns);
}

int grep(regex_t *regex, FILE *fp, char *file_name, Grep_params *params,
         Flags *flags) {
  char *line = NULL;
  size_t len = 0;
  int number_of_line = 0;
  int count_of_lines = 0;
  int reti = 0;
  ssize_t read = 0;
  bool print_line = false;

  while ((read = getline(&line, &len, fp)) != -1) {
    reti = regexec(regex, line, 0, NULL, 0);
    print_line = false;
    number_of_line++;

    // -v flag
    if (!flags->v && !reti) {
      print_line = true;
    } else if (flags->v && reti) {
      print_line = true;
    }

    if (print_line)
      count_of_lines++;

    if (print_line && !flags->l && !flags->c) {
      if (!flags->h && params->count_of_files > 1) {
        printf("%s:", file_name);
        if (flags->n) {
          printf("%d:", number_of_line);
        }
      } else if (flags->h) {
        if (flags->n) {
          printf("%d:", number_of_line);
        }
      } else {
        if (flags->n) {
          printf("%d:", number_of_line);
        }
      }
      if (!flags->c) {
        int len_of_str = strlen(line);
        if (line[len_of_str - 1] != '\n')
          printf("%s\n", line);
        else
          printf("%s", line);
      }
    }
  }
  // -l flag
  if (flags->l && count_of_lines > 0) {
    printf("%s\n", file_name);
  } else if (flags->c && flags->l && count_of_lines > 0) {
    printf("%s:", file_name);
  } else if (flags->c && !flags->l) {
    // -c flag
    if (params->count_of_files > 1 && !flags->h) {
      printf("%s:", file_name);
    }
    printf("%d\n", count_of_lines);
  }
  fclose(fp);
  free(line);
  return EXIT_SUCCESS;
}

int match_grep(regex_t *regex, FILE *fp, char *file_name, Grep_params *params,
               Flags *flags) {
  char *line = NULL;
  size_t len = 0;
  int number_of_line = 0;
  ssize_t read = 0;

  char *s = NULL;
  regmatch_t pmatch[1];
  regoff_t length;

  if (flags->v) {
    fclose(fp);
    return EXIT_SUCCESS;
  }
  while ((read = getline(&line, &len, fp)) != -1) {
    number_of_line++;
    s = line;
    for (unsigned int i = 0;; i++) {
      if (regexec(regex, s, ARRAY_SIZE(pmatch), pmatch, 0))
        break;

      length = pmatch[0].rm_eo - pmatch[0].rm_so;

      if (!flags->h && params->count_of_files > 1) {
        printf("%s:", file_name);
        if (flags->n) {
          printf("%d:", number_of_line);
        }
      } else if (flags->h) {
        if (flags->n) {
          printf("%d:", number_of_line);
        }
      } else {
        if (flags->n) {
          printf("%d:", number_of_line);
        }
      }
      printf("%.*s\n", (int)length, s + pmatch[0].rm_so);

      s += pmatch[0].rm_eo;
    }
  }
  free(line);
  fclose(fp);
  return EXIT_SUCCESS;
}
