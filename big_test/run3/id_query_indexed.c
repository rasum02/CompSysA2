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


struct index_data* mk_indexed(struct record* rs, int n) {
    struct index_data *rss = malloc(sizeof(struct index_data));
    // making space for n index_records
    struct index_record *irs = malloc(n * sizeof(struct index_record));

    for (int i = 0; i < n; i++) {
        irs[i].osm_id = rs[i].osm_id;
        irs[i].rs = &rs[i];
    }

    rss->irs = irs;
    rss->n = n;

    return rss;
}
void free_indexed(struct index_data* data) {
    free(data -> irs);
    free(data);
}

const struct record* lookup_indexed(struct index_data *data, int64_t needle){
    //Looking up the indexed
for (int i = 0; i < data->n; i++) {
    if(data -> irs[i].osm_id == needle) {
        return data->irs[i].rs;
    }

}
return NULL;
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_indexed,
                    (free_index_fn)free_indexed,
                    (lookup_fn)lookup_indexed);
}



