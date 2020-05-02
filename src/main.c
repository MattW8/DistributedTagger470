//#ifdef _OPENMP
#include <omp.h>
//#endif
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <mpi.h>
#include "corpus/corpus_io.h"
#include "dictionary/dictionary_generator.h"
#include "rules/rule_io.h"
#include "util/dynamic_array.h"
#include "util/config.h"
#include "util/fileio.h"
#define CONF_FILE "config"
#define DEFAULT_LEARNER_ITER 1000
#define OMP_NUM_THREADS 4
int parse_options (int, char **, bool *, size_t *, int*);

int main(int argc, char* argv[]){
    if(argc != 4) {
      printf("Usage: ./brilltagger [input_file] [input_character_count] [input_line_count]\n");
      exit(EXIT_SUCCESS);
    }

    char *filename = argv[1];

    int character_count;

    if(sscanf (argv[2], "%i", &character_count) != 1) {
      printf("Usage: ./brilltagger [input_file] [input_character_count] [input_line_count]\n");
      exit(EXIT_SUCCESS);
    }


    int line_count;

    if(sscanf (argv[3], "%i", &line_count) != 1) {
      printf("Usage: ./brilltagger [input_file] [input_character_count] [input_line_count]\n");
      exit(EXIT_SUCCESS);
    }
    //parse learning corpus
    corpus_t corpus;

    parse_corpus(filename, character_count, line_count, &corpus);

    generate_dictionary(corpus);

    return 1;
}






