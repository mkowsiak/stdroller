#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

#define I_AM_OH_SO_BIG 1000000000 // currently, I support 1GB as log file limit
                                  // at the same time, I am creating aa-zz suffixes
                                  // it means, we can store 26*26*1G of logs
                                  // before we start to overwrite and destroy things
                                  // I need suffix files because I want to make
                                  // tail -F to be happy with main log file

#define FLUSH_OVERFLOW 1024       // we want to flush each 1024 characters
                                  // in case limit size is lesser than that
                                  // we don't care as fclose will flush anyway

/* get opts flags */
static int use_suffix            = 0;      // should we create sufix files or not
static unsigned long long limit  = 0;      // in case limit is not passed i will use
                                           // I_AM_OH_SO_BIG value
static char *file_name           = NULL;   // file name for log
static int roll_over             = 0;      // should I start writing the log from the top?
static int new_line_flush        = 0;      // should I flush on new line character?

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
  printf("                                                                               \n");
  printf("usage:                                                                         \n");
  printf("  stdroller -f file_name|--file=file_name [-s|--sufix] [-l size|--limit=size]  \n");
  printf("                                          [-r|--rollover] [-n|--newline]       \n");
  printf("                                          [-h|--help]                          \n");
  printf("                                                                               \n");
  printf("options:                                                                       \n");
  printf("                                                                               \n");
  printf("  -f file_name                                                                 \n");
  printf("  --file=file_name    - File name where log will be stored.                    \n");
  printf("                                                                               \n");
  printf("  -s                                                                           \n"); 
  printf("  --sufix             - Should I create suffix files or not?                   \n");
  printf("                        If you want me to create suffix files                  \n");
  printf("                        I will create each new suffix file after limit         \n");
  printf("                        is reached. Otherwise, I will overwrite                \n");
  printf("                        oryginal log. I mean, I will overwrite it              \n");
  printf("                        like destroying it, cleaning, you will get             \n");
  printf("                        no nothing.                                            \n");
  printf("                                                                               \n");
  printf("  -l limit                                                                     \n");
  printf("  --limit=limit       - Log size limit; default is 1GB. You can use SI prefixes\n");
  printf("                        to specify the size of file, e.g.: 10K, 21k, 10G, etc. \n");
  printf("                                                                               \n");
  printf("  -r                                                                           \n");
  printf("  --rollover          - If you set roll over flag, I will not destroy content  \n");
  printf("                        However, tail -F will not work in this case. So, you   \n");
  printf("                        are the one to decide which log style you prefer.      \n");
  printf("  -n                                                                           \n");
  printf("  --newline           - Flush on new line instead of number of characters.     \n");
  printf("                                                                               \n");
  printf("  -h                                                                           \n");
  printf("  --help              - Surprize, surprize! I will show you help message.      \n");
  exit(1);
}

void get_options(int argc, char **argv) {
  int c;
  char *next_idx = NULL;

  static struct option long_options[] = {
    { "suffix", no_argument, 0, 's' },
    { "limit",  required_argument, 0, 'l' },
    { "file",  required_argument, 0, 'f'},
    { "rollover", no_argument, 0, 'r' },
    { "newline", no_argument, 0, 'n' },
    { "help", no_argument, 0, 'h' },
    { 0, 0, 0, 0}
  };
  int option_index = 0;

  while (1) {
    c = getopt_long (argc, argv, "srnl:f:h?",
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

        if( *next_idx != '\0' ) {
          if( *(next_idx + 1) != '\0') {
            printf("You can't specify more than one character as prefix.\n");
            printf("You can only use: k/K, m/M, g/G, t/T, p/P\n");
            printf("You have specified: %s\n", next_idx);
            exit(1);
          } else {
            switch( *next_idx ) {
              case 'K':
	      case 'k': 
                limit *= pow(10,3);
                break;
              case 'M':
              case 'm':
                limit *= pow(10,6);
                break;
              case 'G':
              case 'g':
                limit *= pow(10,9);
                break; 
              case 'T':
              case 't':
                limit *= pow(10,12);
                break;
              case 'P':
              case 'p':
                limit *= pow(10,15);
                break;
              default:
                printf("It looks like you have passed some strange prefix in SI\n");
                printf("You have passed '%c' - I don't get it :(\n", *next_idx);
                exit(1); 
            }
          }
        }
        break;
      
      case 'f':
        file_name = optarg;
        break;
     
      case 'r':
        roll_over = 1;
        break;

     case 'n':
        new_line_flush = 1;
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

  FILE *fp                     = NULL;  // file pointer for log file
  FILE *fp_suffix              = NULL;  // file pointer for suffix files
  unsigned long long pos       = 0;     // current count of bytes transferred
  char *suffix                 = NULL;  // name of suffix file
  char curr_suffix_first       = 'a';   // current index at first digit
  char curr_suffix_second      = 'a';   // current index at second digit
  int c                        = 0;     // character read from stdin
  int perform_flush_flag       = 0;     // should I flush data

  get_options(argc, argv); 

  if( limit == 0 ) {
    limit = I_AM_OH_SO_BIG;
  }

  if( file_name == NULL ) {
    printf("You have to specify file name: -h for help\n\n");
    exit(1);
  }

  // open log file specified by user (it will be destroyed)
  if((fp = fopen(file_name, "w")) == NULL) {
    fprintf(stderr, "I can't open output file: %s\n", file_name);
    exit(1);
  }

  // in case user wants to use suffix files, we have to
  // allocate memory for the name and open initial
  // suffix file
  if( use_suffix ) {
    // we need length of file name
    // + dot
    // + aa-zz
    // + '\0'
    if((suffix = malloc(strlen(file_name) + 4)) == NULL) {
      fprintf(stderr, "Fatal error! Failed to allocate memory\n");
      exit(1);
    }

    suffix = suffix_file_name(suffix, file_name, curr_suffix_first, curr_suffix_second);
    if((fp_suffix = fopen(suffix, "w")) == NULL) {
      fprintf(stderr, "I can't open suffix file: %s\n", suffix);
      fclose(fp);
      exit(1);
    }
  }

  while((c = getc(stdin)) != EOF) {

    if( new_line_flush == 1 && c == '\n' ) {
      perform_flush_flag = 1;
    } 

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

    // I don't think it's reasonable to check flush overflow in case
    // we already know that limit is lesser than that
    // If it's equal to FLUSH_OVERFLOW, we will flush anyway as
    // we are reopening files; I don't flush if we use new line based flush
    if( new_line_flush == 0 && limit > FLUSH_OVERFLOW && (pos % FLUSH_OVERFLOW) == 0 ) {
      perform_flush_flag = 1;
    }

    if( perform_flush_flag ) {
      perform_flush_flag = 0;
      if(fflush(fp) == EOF) {
        fprintf(stderr, "I can't flush output file: %s\n", file_name);
        exit(1);
      }
      if(fp_suffix != NULL) {
        if(fflush(fp_suffix) == EOF) {
          fprintf(stderr, "I can't flush suffix file: %s\n", suffix);
          exit(1);
	}
      } 
    }

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
      // if we want to roll over, we have to fseek
      // after reopening file in r+ mode
      // otherwise, we can discard the content
      if( roll_over == 0 ) {
        fp = fopen(file_name, "w");
      } else {
        fp = fopen(file_name, "r+");
        if( fp != NULL ) {
          fseek(fp, 0, 0);
        }
      } 
      if( fp == NULL ) {
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

      // reset current counter
      // we will start over from the begging until we reach limit
      pos = 0;

    }
  }
 
  if(fp != NULL) fclose(fp);
  if(fp_suffix != NULL) fclose(fp_suffix);
}
