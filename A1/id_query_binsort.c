#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

struct treeNode
{
    int64_t osm_id;
    struct record * value;
    struct treeNode *left;
    struct treeNode *right;
};

int compare_index_records(const void *a, const void *b)
{
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
struct treeNode* construct_treeNode(struct treeNode* parent, struct record *r){
   if(parent == NULL) { // treeNode does not exist
        parent = (struct treeNode *)malloc(sizeof(struct treeNode));
        parent->osm_id = r->osm_id;
        parent->value = r;
    }
    else if (parent->osm_id <= r->osm_id) { // Build tree structure to the right
        parent->right = construct_treeNode(parent->right,r);
    }
    else { // Build tree structure to the left
        parent->left = construct_treeNode(parent->left,r);
    }
    return parent;
}

struct treeNode *mk_indexed(struct record *rs, int n)
{
    struct treeNode* root = NULL;
    for (int i = 0; i < n; i++)
    {
        root = construct_treeNode(root,&rs[i]);
    }
    
    return root;
}

void free_indexed(struct treeNode *data)
{
    if (data != NULL)
    {
        free_indexed(data->right);
        free_indexed(data->left);
        free(data);
    }
}

const struct record *lookup_binary(struct treeNode *data, int64_t needle)
{
    if(data == NULL)
        return NULL;
    if(data -> osm_id == needle)
        return data->value;
    if(data->osm_id > needle)
        return lookup_binary(data->left,needle);
    else
        return lookup_binary(data->right,needle);
}

int main(int argc, char **argv)
{
    return id_query_loop(argc, argv,
                         (mk_index_fn)mk_indexed,
                         (free_index_fn)free_indexed,
                         (lookup_fn)lookup_binary);
}
