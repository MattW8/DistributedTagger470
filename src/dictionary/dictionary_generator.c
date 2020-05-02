#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <mpi.h>
#include "../tagger/tags.h"
#include "dictionary_generator.h"
#include "../corpus/corpus_io.h"

#include "dht.h"

void prompt_for_print(corpus_t corpus){
    char scan;
    printf("Print corpus?(Y/N):");
    scanf("%c", &scan);
    if(scan=='y' || scan=='Y')
        print_corpus(corpus);
}

float accuracy_check(corpus_t corpus){    
    float accuracy = 0.0;
    for(size_t i = 0; i < corpus.num_lines; i++){
        if(corpus.human_tags[i] == corpus.machine_tags[i])
            accuracy++;
    }
    accuracy = (accuracy/corpus.num_lines)*100;
    return accuracy;
}

void generate_dictionary(corpus_t corpus){

    
    int pid = dht_init();
    
    dht_barrier();

    double putTime;
    double tagTime;

    if(pid == 0)
        putTime = MPI_Wtime();

    if( pid == 0 )
    {
        for( int i = 0; i < corpus.num_lines; i++ )
        {
            //if( i % pid == 0)
                dht_put(corpus.words[i], corpus.human_tags[i]);
        }
    
     }
    dht_barrier();

    if(pid == 0)
        putTime = MPI_Wtime() - putTime;


    if(pid == 0)
        tagTime = MPI_Wtime();

    if( pid == 0)
    {
        for( int i = 0; i < corpus.num_lines; i++ )
        {
            corpus.machine_tags[i] = dht_get(corpus.words[i]);
        }
    }
    
    if(pid == 0)
        tagTime = MPI_Wtime() - tagTime;

    dht_destroy();

    if( pid == 0 )
    {
         printf("Final accuracy %.2f%%\n", accuracy_check(corpus));
         printf("Time to put: %f\n", putTime);
         printf("Time to tag: %f\n", tagTime);
         prompt_for_print(corpus);
    }
}



