#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <math.h>
#include "record.h"
#include "coord_query.h"

struct naive_data {
  struct record *rs;
  int n;
};

// create naive index (just store pointer to records + count)
struct naive_data* mk_naive(struct record* rs, int n) {
  struct naive_data *rss = malloc(sizeof(struct naive_data));
  rss->rs = rs; 
  rss->n = n;  
  return rss;
}

void free_naive(struct naive_data* data) {
  free(data);
}

// naive lookup: find record closest to (lon, lat)
const struct record* lookup_naive(struct naive_data *data, double lon, double lat) {


  // if the inputs are too big or too small clamp them to prevent overflow
  lon = fmin(fmax(lon, -180.0), 180.0);
  lat = fmin(fmax(lat,  -90.0),  90.0);

  double closest = __DBL_MAX__;
  int value = -1;

    for (int i = 0; i < data->n; i++) {
      double lon_array = data->rs[i].lon;
      double lat_array = data->rs[i].lat;

      // Euclid value
      double euclid_array = sqrt((lon - lon_array)* (lon - lon_array) + (lat - lat_array) * (lat - lat_array));
      

      if (euclid_array < closest) {
        closest = euclid_array;
        value = i;
      }
    }
    if (value == -1) {
      return NULL;
    }
    return &data->rs[value];

}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_naive,
                          (free_index_fn)free_naive,
                          (lookup_fn)lookup_naive);
}
