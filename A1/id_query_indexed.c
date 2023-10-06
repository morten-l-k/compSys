#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"
struct index_record
{
    int64_t osm_id;
    const struct record *record;
};
struct indexed_data
{
    struct index_record *irs;
    int n;
};

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
    for (int i = 0; i < n; i++)
    {
        data->irs[i].osm_id = rs[i].osm_id;
        data->irs[i].record = &rs[i];
    }

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

const struct record *lookup_indexed(struct indexed_data *data, int64_t needle)
{
    struct index_record *records = data[0].irs;
    for (int i = 0; i < data[0].n; i++)
    {
        if (records[i].osm_id == needle)
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
