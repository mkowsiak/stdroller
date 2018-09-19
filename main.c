#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define I_AM_OH_SO_BIG 1000000000 // currently, I support 1GB as log file limit
                                  // at the same time, I am creating aa-zz suffixes
                                  // it means, we can store 26*26*1G of logs
                                  // before we start to overwrite and destroy things
                                  // I need suffix files because I want to make
                                  // tail -F to be happy with main log file

char *suffix_file_name(char *suffix, char *file_name, char suffix1st, char suffix2nd) {
  strcpy(suffix, file_name);
  int idx = strlen(file_name);
  suffix[idx++] = '.';
  suffix[idx++] = suffix1st;
  suffix[idx++] = suffix2nd;
  suffix[idx++] = '\0';
  return suffix;
}

int main(int argc, char **argv) {
  FILE *fp;
  FILE *fp_suffix;
  unsigned long long pos = 0;
  char *suffix = NULL;
  char curr_suffix_first = 'a';
  char curr_suffix_second = 'a';
  
  if(argc != 2) {
    fprintf(stderr, "You have to specify output file\n");
    exit(1);
  } else {
    // we need length of file name
    // + dot
    // + aa-zz
    // + '\0'
    suffix = malloc(strlen(argv[1]) + 4);
    if((fp = fopen(argv[1], "w")) == NULL) {
      fprintf(stderr, "I can't open output file: %s\n", argv[1]);
      exit(1);
    }
    suffix = suffix_file_name(suffix, argv[1], curr_suffix_first, curr_suffix_second);
    if((fp_suffix = fopen(suffix, "w")) == NULL) {
      fprintf(stderr, "I can't open suffix file: %s\n", suffix);
      fclose(fp);
      exit(1);
    }

    int c;
    while((c = getc(stdin)) != EOF) {
      // instead of copying file to another location (after limit is reached)
      // I write into oryginal log and suffix log at the same time
      if(fputc(c, fp) == EOF) {
        fprintf(stderr, "I can't write to output file: %s\n", argv[1]);
        exit(1);
      }
      if(fputc(c, fp_suffix) == EOF) {
        fprintf(stderr, "I can't write to suffix file: %s\n", suffix);
        exit(1);
      }

      pos++;
      if(pos > I_AM_OH_SO_BIG) {

        // once limit is hit:
        // - close the log file and reopen it in "w" mode
        //   so we can start from beggining
        // - close suffix file
        // - bump up the suffix string
        // - reset byte counter
        // - re-open files
        // - start passing data again

        // close files
        fclose(fp);
        fclose(fp_suffix);

        // bump up sufixes
        // once 'z' is reached, we start to overwrite
        // files
        if(curr_suffix_second == 'z') {
          curr_suffix_second = 'a';
          if(curr_suffix_first == 'z') {
            curr_suffix_first = 'a';
          } else {
            curr_suffix_first ++;
          }
        } else {
          curr_suffix_second ++;
        }

        // reopen files
        if((fp = fopen(argv[1], "w")) == NULL) {
          fprintf(stderr, "I can't open output file: %s\n", argv[1]);
          exit(1);
        }
        suffix = suffix_file_name(suffix, argv[1], curr_suffix_first, curr_suffix_second);
        if((fp_suffix = fopen(suffix, "w")) == NULL) {
          fprintf(stderr, "I can't open suffix file: %s\n", suffix);
          fclose(fp);
          exit(1);
        }

        pos = 0;
      }
    }
 
    fclose(fp);
    fclose(fp_suffix);
  }
}
