#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"


int compare_index_records(const void *a, const void *b) {
    const struct record *record_a = (const struct record *)a;
    const struct record *record_b = (const struct record *)b;

    if (record_a->osm_id < record_b->osm_id)
    {
        return -1;
    }
    else if (record_a->osm_id > record_b->osm_id)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


struct treeNode *mk_indexed(struct record *rs, int n) {
}

void free_indexed(struct treeNode *data) {
   
}

const struct record *lookup_binary(struct treeNode *data, int64_t needle) {

}

int main(int argc, char **argv)
{
    return id_query_loop(argc, argv,
                         (mk_index_fn)mk_indexed,
                         (free_index_fn)free_indexed,
                         (lookup_fn)lookup_binary);
}
