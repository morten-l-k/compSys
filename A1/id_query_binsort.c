#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <math.h>

#include "record.h"
#include "id_query.h"

struct bin_data {
    struct record *rs;
    int n;
};

int compare_index_records(const void *a, const void *b) {
    const struct record *record_a = (const struct record *)a;
    const struct record *record_b = (const struct record *)b;

    if (record_a->osm_id < record_b->osm_id) {
        return -1;
    }
    else if (record_a->osm_id > record_b->osm_id) {
        return 1;
    }
    else {
        return 0;
    }
}


struct bin_data *mk_indexed(struct record *rs, int n) {
    struct bin_data *data = malloc(sizeof(struct bin_data));
    qsort(rs,n,sizeof(struct record),compare_index_records);
    data->rs = rs;
    data->n = n;
    return data;
}

void free_indexed(struct bin_data *data) {
    free(data);   
}

struct record* binary_search(struct record *rs, int n, int64_t needle) {
    // int left = 0;
    // int right = n - 1;
    // struct record *return_value = &rs[0];
    // // printf("LEFT IS %d, right is %d\n",left,right);

    // while (left <= right) {
    //     int median = floor((left + right) / 2);
    //     printf("MEDIAN IS %d\n",median);
    //     if (rs[median].osm_id < needle) {
    //         printf("MEDIAN VALUE IS SMALLER THAN NEEDLE: %lld\n ",rs[median].osm_id);
    //         left = median + 1;
    //     } else if (rs[median].osm_id > needle) {
    //         printf("MEDIAN VALUE IS LARGER THAN NEEDLE: %lld\n ",rs[median].osm_id);
    //         right = median - 1;
    //     } else {
    //         return return_value = &rs[median];
    //     }
    // }
    // // printf("RETURN_VALUE IS %lld\n",return_value->osm_id);
    // // printf("RETURN_NAME IS %s\n",return_value->name);
    // return return_value;
}

const struct record *lookup_binary(struct bin_data *data, int64_t needle) {
    // printf("IN LOOKUP\n");
    struct record *hit = binary_search(data->rs,data->n,needle);
    return hit;
}


int main(int argc, char **argv) {
    return id_query_loop(argc, argv,
                         (mk_index_fn)mk_indexed,
                         (free_index_fn)free_indexed,
                         (lookup_fn)lookup_binary);
}
