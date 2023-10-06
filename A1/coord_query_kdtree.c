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

typedef int (*comparison_type)(int,int);

struct point {
    double lon;
    double lat;
};

struct kdtree_data {
    struct record *node; //Må jo være noden i træet
    struct point point;
    int axis;
    struct kdtree_data *left_child; //Må jo kunne være NULL, hvis der ingen child er
    struct kdtree_data *right_child;
};


struct kdtree_data* procedure(struct record* rs, struct kdtree_data* tree, struct record *r, int n) {
        sort_data(rs,n,sizeof(struct record),compare_lat); //sort on latitude
}

int compare_lon(const void *a,const void *b) {
    double arg1 = ((const struct record*)a)->lon;
    double arg2 = ((const struct record*)b)->lon;
    if (arg1 < arg2) {
        return -1;
    }
    if (arg1 > arg2) {
        return 1;
    }
    return 0;
}

int compare_lat(const void *a, const void *b) {
    double arg1 = ((const struct record*)a)->lat;
    double arg2 = ((const struct record*)b)->lat;
    if (arg1 < arg2) {
        return -1;
    }
    if (arg1 > arg2) {
        return 1;
    }
    return 0;
}

/**
 * @brief sorts array in place
 * 
 * @param rs Array to be sorted
 * @param n length of array
 * @param size size of each element in the array
 * @param func type of comparison function. i.e. comparison of lat or comparison of lon
 */
void sort_data(struct record* rs, size_t n, size_t size,comparison_type *func) {
    qsort(rs,n,size,func);
}

struct kdtree_data* mk_kd(struct record* rs, int n) {
    struct kdtree_data *kd_root = NULL;
    for (int i = 0; i < n; i++) {
        kd_root = procedure(rs, kd_root,&rs[i],0);
    }
}

void free_kd(struct kdtree_data* data) {
  free(data);
}

const struct record* lookup_kd(struct kdtree_data *data, double lon, double lat){
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_kd,
                          (free_index_fn)free_kd,
                          (lookup_fn)lookup_kd);
}
