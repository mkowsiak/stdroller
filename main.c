#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define I_AM_OH_SO_BIG 1000000000 // currently, I support 1GB as log file limit
                                  // at the same time, I am creating aa-zz suffixes
                                  // it means, we can store 26*26*1G of logs
                                  // before we start to overwrite and destroy things
                                  // I need suffix files because I want to make
                                  // tail -F to be happy with main log file

static int use_suffix;                // should we create sufix files or not
static unsigned long long limit;      // in case limit is not passed i will use
                                      // I_AM_OH_SO_BIG value
static char *file_name = NULL;        // file name for log

char *suffix_file_name(char *suffix, char *file_name, char suffix1st, char suffix2nd) {
  strcpy(suffix, file_name);
  int idx = strlen(file_name);
  suffix[idx++] = '.';
  suffix[idx++] = suffix1st;
  suffix[idx++] = suffix2nd;
  suffix[idx++] = '\0';
  return suffix;
}

void display_usage() {
  printf("usage:                                                                         \n");
  printf("  stdroller -f file_name|--file=file_name [--sufix|-s] [--limit|-l] [--help|-h]\n");
  printf("                                                                               \n");
  printf("options:                                                                       \n");
  printf("  -f file_name                                                                 \n");
  printf("  --file=file_name    - file name where log will be stored                     \n");
  printf("  -s                                                                           \n"); 
  printf("  --sufix             - should I create suffix files or not?                   \n");
  printf("                        If you want me to create suffix files                  \n");
  printf("                        I will create each new file after limit                \n");
  printf("                        is reached. Otherwise, I will overwrite                \n");
  printf("                        oryginal log. I mean, I will overwrite it              \n");
  printf("                        like destroying it, cleaning, you will get             \n");
  printf("                        no nothing.                                            \n");
  printf("  -l limit                                                                     \n");
  printf("  --limit=limit       - log size limit; default is 1GB. Unfortunately          \n");
  printf("                        you have to specify full size, like 1000000000 for 1GB \n");
  printf("  -h                                                                           \n");
  printf("  --help              - surprize, surprize! I will show you help message.      \n");
  exit(1);
}

void get_options(int argc, char **argv) {
  int c;
  char *next_idx = NULL;

  while (1) {
    static struct option long_options[] = {
      { "suffix", no_argument, 0, 's' },
      { "limit",  required_argument, 0, 'l' },
      { "file",  required_argument, 0, 'f'},
      { "help", no_argument, 0, 'h' },
      { 0, 0, 0, 0}
    };
    int option_index = 0;

    c = getopt_long (argc, argv, "sl:f:h?",
                     long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c) {
      case 's':
        use_suffix = 1;
        break;

      case 'l':
        limit = strtoull( optarg, &next_idx, 10 );
        if( limit == 0 ) {
          printf("Either you have passed some crazy value or 0\n");
          printf("In both cases, it makes no sense for me to proceed\n");
          exit(1);
        }
        break;
      
      case 'f':
        file_name = optarg;
        break;
      
      case 'h':
      case '?':
        display_usage();
        break;

      default:
        break;
    }
  }
}

int main(int argc, char **argv) {
  FILE *fp;
  FILE *fp_suffix = NULL;
  unsigned long long pos = 0;
  char *suffix = NULL;
  char curr_suffix_first = 'a';
  char curr_suffix_second = 'a';

  get_options(argc, argv); 

  if( limit == 0 ) {
    limit = I_AM_OH_SO_BIG;
  }

  if( file_name == NULL ) {
    printf("You have to specify file name: -h for help\n\n");
    exit(1);
  }

  // we need length of file name
  // + dot
  // + aa-zz
  // + '\0'
  suffix = malloc(strlen(file_name) + 4);
  if((fp = fopen(file_name, "w")) == NULL) {
    fprintf(stderr, "I can't open output file: %s\n", file_name);
    exit(1);
  }

  if( use_suffix ) {
    suffix = suffix_file_name(suffix, file_name, curr_suffix_first, curr_suffix_second);
    if((fp_suffix = fopen(suffix, "w")) == NULL) {
      fprintf(stderr, "I can't open suffix file: %s\n", suffix);
      fclose(fp);
      exit(1);
    }
  }

  int c;
  while((c = getc(stdin)) != EOF) {
    // instead of copying file to another location (after limit is reached)
    // I write into oryginal log and suffix log at the same time
    if(fputc(c, fp) == EOF) {
      fprintf(stderr, "I can't write to output file: %s\n", file_name);
      exit(1);
    }
    if(fp_suffix != NULL) {
      if(fputc(c, fp_suffix) == EOF) {
        fprintf(stderr, "I can't write to suffix file: %s\n", suffix);
        exit(1);
      }
    }

    pos++;
    if(pos >= limit) {

      // once limit is hit:
      // - close the log file and reopen it in "w" mode
      //   so we can start from beggining
      // - close suffix file
      // - bump up the suffix string
      // - reset byte counter
      // - re-open files
      // - start passing data again

      // close file
      fclose(fp);
      // reopen file
      if((fp = fopen(file_name, "w")) == NULL) {
        fprintf(stderr, "I can't open output file: %s\n", file_name);
        exit(1);
      }

      if(use_suffix != 0) {
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

        suffix = suffix_file_name(suffix, file_name, curr_suffix_first, curr_suffix_second);
        if((fp_suffix = fopen(suffix, "w")) == NULL) {
          fprintf(stderr, "I can't open suffix file: %s\n", suffix);
          fclose(fp);
          exit(1);
        }
      }

      pos = 0;

    }
  }
 
  fclose(fp);
  fclose(fp_suffix);
}
