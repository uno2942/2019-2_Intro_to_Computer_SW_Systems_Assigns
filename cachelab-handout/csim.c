//STUDENT ID: 20150462

#define _GNU_SOURCE
#include "cachelab.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

//Option to print HIT, MISS, EVICTION message.
static bool flag_v = false;

//Cache arguments.
static size_t s;
static size_t E;
static size_t b;
static size_t S;
static size_t B;

static size_t hit_count;
static size_t miss_count;
static size_t eviction_count;

struct cache_elem{
    bool valid;
    size_t tag;
    struct cache_elem* before;
    struct cache_elem* next;
};

static size_t get_tag_part(size_t addr){ return addr >> ( s + b ); }
static size_t get_S_part(size_t addr){ return ( ( addr << (64 - s - b)) >> (64 - s)); }
static size_t get_B_part(size_t addr){ return ( ( addr << (64 - b)) >> (64 - b)); }

static struct cache_elem* get_tail(struct cache_elem* HEAD);
static void push_back_on_list(struct cache_elem* target_line, struct cache_elem* target_elem, struct cache_elem* tail);

static struct cache_elem** init_cache();
static void free_cache();

static void find_data_in_cache(struct cache_elem** my_cache, size_t addr, ssize_t read_size);
static void store_data_in_cache(struct cache_elem** my_cache, size_t addr, ssize_t write_size);
static void write_data_on_cache(struct cache_elem* target_line, size_t addr);

void print_help();

int
main(int argc, char* argv[])
{
    struct cache_elem** my_cache;

    char* trace_name = NULL;

    //Local variables for file I/O.
    //From https://stackoverflow.com/questions/3501338/c-read-file-line-by-line
    FILE* fp;
    char* line = NULL;
    char* ptr;
    size_t len = 0;
    ssize_t read;

    char inst;
    size_t addr;
    ssize_t size;

    int c;

    bool flag_h = false;

    //parse the arguments and put it to cache arguments.
    while( (c = getopt (argc, argv, "hvs:E:b:t:")) != -1 )
    {
        switch(c)
        {
            case 'h': flag_h = true; break;
            case 'v': flag_v = true; break;
            case 's': s = atoi(optarg); break;
            case 'E': E = atoi(optarg); break;
            case 'b': b = atoi(optarg); break;
            case 't': trace_name = malloc(strlen(optarg)); 
                      strcpy(trace_name, optarg); break;
            default: printf("Not available argument"); exit(-1);
        }
    }

    //print help
    if(flag_h)
    {
        print_help();
        exit(0);
    }
    //check validity of arguments
    if(!s||!b||!E||!trace_name)
    {
        printf("%s: Missing required command line argument\n", argv[0]);
        print_help();
        return 0;
    }

    S = 1 << s;
    B = 1 << b;

    my_cache = init_cache();

    if( !(fp = fopen(trace_name, "r") ) )
        exit(-1);
    
    free(trace_name);
    
    //read lines in trace file.
    while( (read = getline(&line, &len, fp) ) != -1 ){
        
        strtok(line, "\n");

        //Line parsing.
        //From https://dojang.io/mod/page/view.php?id=376
        ptr = strtok(line, " ");
        inst = ptr[0];
        ptr = strtok(NULL, " ");
        ptr = strtok(ptr, ",");
        addr = strtoul(ptr, NULL, 16);
        ptr = strtok(NULL, ",");
        size = strtoul(ptr, NULL, 16);
        if(flag_v && inst != 'I')
            printf("%c %lx,%lx", inst, (unsigned long)addr, (long)size);
        switch(inst)
        {
            case 'L': find_data_in_cache(my_cache, addr, size); break;
            case 'S': store_data_in_cache(my_cache, addr, size); break;
            case 'M': find_data_in_cache(my_cache, addr, size); 
                      store_data_in_cache(my_cache, addr, size); break;
            default: break;
        }
        if(flag_v && inst != 'I')
            printf("\n");
    }
    printSummary(hit_count, miss_count, eviction_count);

    free_cache(my_cache);
    return 0;
}


//It generates a set of linked list with HEAD and TAIL, i.e., if there exists
//8 elements in one linked list, it shapes HEAD - 8 elements - TAIL.
//The length of linked list represents associativity and # of linked list
//represents number of sets.
static struct cache_elem**
init_cache()
{
    struct cache_elem** my_cache;
    struct cache_elem* temp;
    struct cache_elem* new_elem;
    size_t i, j;
    
    my_cache = (struct cache_elem**)calloc( S, sizeof(struct cache_elem*)) ;
    for(i = 0; i < S; i++){
        //this is HEAD of queue.
        my_cache[i] = (struct cache_elem*)calloc( 1, sizeof(struct cache_elem) );
        temp = my_cache[i];
        for(j = 0; j < E + 1; j++){
            new_elem = (struct cache_elem*)calloc( 1, sizeof(struct cache_elem) );
            temp->next = new_elem;
            new_elem->before = temp;
            temp = new_elem;
        }
    }
    return my_cache;
}

//Free the allocated memory.
static void
free_cache(struct cache_elem** my_cache)
{
    struct cache_elem* temp;
    int i;
    for(i = 0; i < S; i++){
        while(temp)
        {
            temp = my_cache[i]->next;
            free(my_cache[i]);
            my_cache[i] = temp;
        }
    }
    free(my_cache);
}

//If ADDR is in cache, increase HIT count and put the read data to end of the list.
//to mark it recently used. Otherwise, increase MISS count and wirte the data
//from main memory.
//READ_SIZE represents how many it reads. It may not be used in this assignment.
//I just wanted to include the case that READ_SIZE > block size.

static void
find_data_in_cache(struct cache_elem** my_cache, 
                    size_t addr, 
                    ssize_t read_size)
{
    size_t tag_part = get_tag_part(addr);
    size_t S_part = get_S_part(addr);
    ssize_t offset = get_B_part(addr);

    struct cache_elem* target_line = my_cache[S_part];
    struct cache_elem* temp = target_line->next;
    size_t i;
    
    for( i = 0; i < E; i++){
        if( temp!=NULL && temp->valid && 
            temp->tag == tag_part)
        {
            if(flag_v)
                printf(" hit");
            hit_count++;
            push_back_on_list(target_line, temp, get_tail(target_line));
            read_size -= ((ssize_t)B - offset);
            break;
        }
        temp = temp -> next;
    }
    //Could not find in cache
    if( i == E)
    {
        if(flag_v)
            printf(" miss");
        miss_count++;
        write_data_on_cache(target_line, addr);
        read_size -= ((ssize_t)B - offset);
    }
    if( read_size > 0 )
    {
        size_t addr2 = ( tag_part << (s + b) ) + ( ( ( S_part + 1) << b ) );
        
        //Overflow check
        if( ( ( (int) addr ) < 0 && ( (int) addr2 ) > 0) ||
            ( ( (int) addr ) > 0 && ( (int) addr2 ) < 0) )
            {
                printf("Invalid Memory Access");
                exit(-1);
            }
        find_data_in_cache(my_cache, addr2, read_size);
    }
}

//Try to store data into cache. If there is a room, increase HIT count and
//put the read data to end of the list to mark it recently used. Otherwise,
//increase MISS count and wirte the data from main memory.
//WRITE_SIZE represents how many it writes. It may not be used in this assignment.
//I just wanted to include the case that WRITE_SIZE > block size.

static void
store_data_in_cache(struct cache_elem** my_cache,
                     size_t addr,
                     ssize_t write_size)
{
    size_t tag_part = get_tag_part(addr);
    size_t S_part = get_S_part(addr);
    ssize_t offset = get_B_part(addr);

    struct cache_elem* target_line = my_cache[S_part];
    struct cache_elem* temp = target_line->next;
    size_t i;
    
    for( i = 0; i < E; i++){
        if( (temp != NULL && temp->valid && temp->tag == tag_part)
          )
        {
            if(flag_v)
                printf(" hit");
            hit_count++;
            push_back_on_list(target_line, temp, get_tail(target_line));
            write_size -= ((ssize_t)B - offset);
            break;
        }
        temp = temp->next;
    }
    //Could not find in cache
    if( i == E)
    {
        if(flag_v)
            printf(" miss");
        miss_count++;
        write_data_on_cache(target_line, addr);
        write_size -= ((ssize_t)B - offset);
    }
    if( write_size > 0 )
    {
        printf("\n\n\n\n A\n\n\n\n\n");
        size_t addr2 = ( tag_part << (s + b) ) + ( ( ( S_part + 1) << b ) );
        
        //Overflow check
        if( ( ( (int) addr ) < 0 && ( (int) addr2 ) > 0) ||
            ( ( (int) addr ) > 0 && ( (int) addr2 ) < 0) )
            {
                printf("Invalid Memory Access");
                exit(-1);
            }
        store_data_in_cache(my_cache, addr2, write_size);
    }
}

//Write the data to front of the linked list by LRU policy.
//The least recently used cache data is in front of the linked list.
static void
write_data_on_cache(struct cache_elem* target_line, size_t addr){
    size_t tag_part = get_tag_part(addr);

    struct cache_elem* tail = get_tail(target_line);
    struct cache_elem* target_elem = target_line->next;
    bool eviction_flag = false;
    
    //If the taret space is not valid, it is not a eviction.
    if(target_elem->valid)
        eviction_flag = true;
    
    target_elem->tag = tag_part;
    target_elem->valid = true;

    push_back_on_list(target_line, target_elem, tail);

    if(eviction_flag)
        {
            if(flag_v)
                printf(" eviction");
            eviction_count++;
        }
}

//Get tail of linked list.
static struct cache_elem* get_tail(struct cache_elem* HEAD){
    struct cache_elem* temp = HEAD;
    while(temp->next)
        temp = temp->next;
    return temp;
}

//Push back the data into linked list.
static void
push_back_on_list(struct cache_elem* target_line,
                     struct cache_elem* target_elem,
                     struct cache_elem* tail)
{
    target_elem->before->next = target_elem->next;
    target_elem->next->before = target_elem->before;

    tail->before->next = target_elem;
    target_elem->before = tail->before;
    target_elem->next = tail;
    tail->before = target_elem;
}

void print_help(){
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("  -h         Print this help message\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\n");
    printf("Examples:\n");
    printf("  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}