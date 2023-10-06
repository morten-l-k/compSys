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

struct naive_data* mk_naive(struct record* rs, int n) {
    struct naive_data *nd = malloc(sizeof(struct naive_data));
    nd->rs = rs;
    nd->n = n;
    return nd;
}

void free_naive(struct naive_data* data) {
  free(data);
}

double distance(double lon, double lat, double test_lon, double test_lat) {
    return sqrt(pow(lon - test_lon,2) + pow(lat - test_lat, 2));
}

const struct record* lookup_naive(struct naive_data *data, double lon, double lat) {
    struct record *result = &(data->rs[0]);
    double shortest_distance = distance(lon,lat,result->lon,result->lat);

    for (int i = 1; i < data->n; i++) {
        double test_distance = distance(lon,lat,data->rs[i].lon,data->rs[i].lat);
        if (test_distance < shortest_distance) {
            result = &(data->rs[i]);
            shortest_distance = test_distance;
        }
    }
    
    return result;
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_naive,
                          (free_index_fn)free_naive,
                          (lookup_fn)lookup_naive);
}
