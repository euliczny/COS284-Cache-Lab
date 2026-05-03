//Names and IDs: Ethan Uliczny (@00700982), Evan Smith (@__)

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include "cachelab.h"
#include <string.h>

typedef struct {
    int valid; //checks if line is occupied
    unsigned long tag; //which bock?
    int lru; //least recently used
} Line;

typedef struct {
    Line *lines; //a pointer to an array of lines
} Set;

typedef struct {
    Set *sets; // a pointer to an array of sets
} Cache;


void simulate(Cache *cache, int E, int verbose, unsigned long set_index, 
              unsigned long tag, int *hits, int *misses, int *evictions, char *vModeString) {
    
    Set *set = &cache->sets[set_index];

    //increments lru before checking for hits, misses or evictions
    for (int i = 0; i < E; i++) {
        //makes sure the line is valid first
        if(set->lines[i].valid) {
            set->lines[i].lru++;
        }
    }

    //check for a hit
    for (int i = 0; i < E; i++) {
        if (set->lines[i].valid && set->lines[i].tag == tag) {
            set->lines[i].lru = 0; //resets the recently used so that it is less likely to get removed compared to others
            (*hits)++;
            strcat(vModeString, " hit");
            return;
        }
    }
    
    (*misses)++;
    //find empty spot for adress
    for (int i = 0; i < E; i++) {
        if (!set->lines[i].valid) {
            set->lines[i].valid = 1;
            set->lines[i].tag = tag;
            set->lines[i].lru = 0;
            strcat(vModeString, " miss");
            return;
        }
    }

    //if no hits and no empty spots in cache
    (*evictions)++;
    int lru_index = 0;
    for (int i = 1; i < E; i++) {
        if (set->lines[i].lru > set->lines[lru_index].lru) {
            lru_index = i;
        }
    }
    set->lines[lru_index].tag = tag;
    set->lines[lru_index].lru = 0;
    strcat(vModeString, " miss eviction");
}


int main(int argc, char *argv[]) {
    int s = 0, E = 0, b = 0;
    int verbose = 0;
    char *tracefile = NULL;
    char vModeString[32];

    //parses the input statement
    int opt;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
            case 's': s = atoi(optarg);     break;
            case 'E': E = atoi(optarg);     break;
            case 'b': b = atoi(optarg);     break;
            case 't': tracefile = optarg;   break;
            case 'v': verbose = 1;          break;
        }
    }



    int S = 1 << s;  // number of sets

    Cache cache;
    cache.sets = malloc(S * sizeof(Set));

    //setting up the cache in memory
    for (int i = 0; i < S; i++) {
        cache.sets[i].lines = malloc(E * sizeof(Line));
        for (int j = 0; j < E; j++) {
            cache.sets[i].lines[j].valid = 0;
            cache.sets[i].lines[j].tag = 0;
            cache.sets[i].lines[j].lru = 0;
        }
    }


     //opening the tracefile 
    FILE *fp = fopen(tracefile, "r");
    if (fp == NULL){
        printf("Error: Could not open the file.\n");
        return 1;
    }   

    //set up for reading the tracefile. This helps us get those parts of the memmory access
    char operation;
    unsigned long address;
    int size;
    int hits = 0, misses = 0, evictions = 0;

    //" %c" reads the type of memory access and skips if 'I'
    //" %lx" reads the hexidecimal adress
    //",%d" reads the size and comma in the line
    while (fscanf(fp, " %c %lx,%d", &operation, &address, &size) == 3) {
        if (operation == 'I') continue;

        unsigned long set_index = (address >> b) & ((1 << s) - 1); //shifst the offset bits off and seperates the s bits from the tag bits
        unsigned long tag = address >> (b + s); //shifts the s and b bits off the address to just have the tag alone

        vModeString[0] = '\0';

        simulate(&cache, E, verbose, set_index, tag, &hits, &misses, &evictions, vModeString);

        //M has to be done twice because its a load AND store
        if (operation == 'M'){
            simulate(&cache, E, verbose, set_index, tag, &hits, &misses, &evictions, vModeString);
        }

        printf("%c %lx,%d%s\n", operation, address, size, vModeString);
    }


    printSummary(hits, misses, evictions);
    fclose(fp);
    return 0;
}
