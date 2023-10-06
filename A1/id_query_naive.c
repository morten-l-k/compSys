#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"
#include "id_query_naive.h"

// struct naive_data
// {
//   struct record *rs;
//   int n;
// };

struct naive_data *mk_naive(struct record *rs, int n)
{


  struct naive_data *data = (struct naive_data *)malloc(sizeof(struct naive_data));
  if (data == NULL)
  {
    fprintf(stderr, "Error: Failed to allocate memory for naive_data.\n");
    exit(EXIT_FAILURE);
  }

  data->rs = rs;
  data->n = n;
  return data;
}

void free_naive(struct naive_data *data)
{
  free(data);
}

const struct record *lookup_naive(struct naive_data *data, int64_t needle)
{
  struct record *records = data[0].rs;
  for (int64_t i = 0; i < data[0].n; i++)
  {
    if (records[i].osm_id == needle) // check for corrupt memory..
    {
      return &records[i];
    }
  }
  return NULL;
}

int main(int argc, char **argv)
{
  return id_query_loop(argc, argv,
                       (mk_index_fn)mk_naive,
                       (free_index_fn)free_naive,
                       (lookup_fn)lookup_naive);
}
