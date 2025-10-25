#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

struct index_record {
    int64_t osm_id;
    const struct record *rs;
};

struct index_data {
    struct index_record *irs;
    int n;
};

int compare_method_sort (const void *a , const void *b){
    const struct index_record *r1 = (const struct index_record*)a;
    const struct index_record *r2 = (const struct index_record*)b;

    //Make sure no overflow
    if (r1->osm_id < r2->osm_id) {
        return -1;
    } else if (r1->osm_id > r2->osm_id){
        return 1;
    } else {
        return 0;
    }
}

int compare_method_bin (const void *key, const void *element){

    int64_t k = *(const int64_t*)key;
    const struct index_record *rec = (const struct index_record*)element;
     
    //Make sure no overflow
    if (k < rec->osm_id) {
        return -1;
    } else if (k > rec->osm_id){
        return 1;
    } else {
        return 0;
    }
}


struct index_data* mk_binsort(struct record* rs, int n) {
    struct index_data *rss = malloc(sizeof(struct index_data));
    struct index_record *irs = malloc(n * sizeof(struct index_record));

    // fill index_records with osm_id and pointer to original record
    for (int i = 0; i < n; i++) {
        irs[i].osm_id = rs[i].osm_id;
        irs[i].rs = &rs[i];
    }

    qsort(irs, n, sizeof(struct index_record), compare_method_sort);

    rss->irs = irs;
    rss->n = n;

    return rss;
}


void free_binsort(struct index_data* data) {
    free(data -> irs);
    free(data);
}

const struct record* lookup_binsort(struct index_data *data, int64_t needle){

    //using Bin search
    struct index_record *found = bsearch(&needle, data->irs, data->n, sizeof(struct index_record), compare_method_bin);
    if (found) return found->rs;
    else return NULL;
    
}

int main(int argc, char** argv) {
    return id_query_loop(argc, argv,
                      (mk_index_fn)mk_binsort,
                      (free_index_fn)free_binsort,
                      (lookup_fn)lookup_binsort);
  }
  

