#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"
// enkel
struct index_record
{
    int64_t osm_id;
    const struct record *record;
};
// liste
struct indexed_data
{
    struct index_record *irs;
    int n;
};
int compare_index_records(const void *a, const void *b)
{
    const struct index_record *record_a = (const struct index_record *)a;
    const struct index_record *record_b = (const struct index_record *)b;

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

struct indexed_data *mk_indexed(struct record *rs, int n)
{
    struct indexed_data *data = (struct indexed_data *)malloc(sizeof(struct indexed_data));
    if (data == NULL)
    {
        fprintf(stderr, "Error: Failed to allocate memory for indexed_data\n");
        exit(EXIT_FAILURE);
    }
    data->irs = (struct index_record *)malloc(n * sizeof(struct index_record));
    if (data->irs == NULL)
    {
        fprintf(stderr, "Error: Failed to allocate memory for index_record\n");
        exit(EXIT_FAILURE);
    }
    // opretter en indekseret datastruktur, der indeholder kopier af de oprindelige data, som er sorteret efter osm_id
    for (int i = 0; i < n; i++)
    {
        data->irs[i].osm_id = rs[i].osm_id;
        data->irs[i].record = &rs[i];
    }

    // Sorter indeksdatastrukturen
    // qsort(data->irs, n, sizeof(struct index_record),compare_index_records);
    data->n = n;

    return data;
}

void free_indexed(struct indexed_data *data)
{
    if (data != NULL)
    {
        free(data->irs);
        free(data);
    }
}
// binary search tree
//  struct TreeNode
//  {
//      struct TreeNode* left;
//      struct TreeNode* right;
//  };

const struct record *lookup_indexed(struct indexed_data *data, int64_t needle)
{
    struct index_record *records = data[0].irs;
    for (int i = 0; i < data[0].n; i++)
    {
        if (records[i].osm_id == needle) // check for corrupt memory..
        {
            return records[i].record;
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    return id_query_loop(argc, argv,
                         (mk_index_fn)mk_indexed,
                         (free_index_fn)free_indexed,
                         (lookup_fn)lookup_indexed);
}
