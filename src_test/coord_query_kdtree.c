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

// keep track of lat and lon
struct coord{
    double x;
    double y;
    const struct record *rec;
};

//keeps track of the value, dimension and children notes
struct node{
    struct coord *p;
    int dimension;
    struct node *leftnode;
    struct node *rightnode;
};

//keeps track of the records, tree and coordinates
struct coord_data {
    struct record *rs;
    int n;
    struct node *root;
    struct coord *coords;
};



//comparison function for lon for quicksort
int compare_x(const void *a, const void *b) {
    const struct coord *r1 = (const struct coord*)a;
    const struct coord *r2 = (const struct coord*)b;

    
    if (r1->x < r2->x) {
        return -1;
    }
    else if (r1->x > r2->x) {
        return 1;
    }
    else {
        return 0;
    }
    
}

//comparison function for lat for quicksort
int compare_y(const void *a, const void *b) {
    const struct coord *r1 = (const struct coord*)a;
    const struct coord *r2 = (const struct coord*)b;
     if (r1->y < r2->y) {
        return -1;
    }
    else if (r1->y > r2->y) {
        return 1;
    }
    else {
        return 0;
    }
}

//Function to create the tree
struct node* create_tree (struct coord* coords, int n, int dim){
    if (n==0) {
        return NULL;
    } else if (n==1) { //if only one coordinate is left
        struct node* node = malloc(sizeof(struct node));
        node->dimension = dim;
        node->p = &coords[0];
        node->leftnode = NULL;
        node->rightnode = NULL;
        return node;
    }

    //sorts the coordinates in relation to the dimension
    int current_dim = dim;
    if (dim == 0) {
        qsort(coords, n, sizeof(struct coord), compare_x);
    } else {
        qsort(coords, n, sizeof(struct coord), compare_y);
    }

    int mid = n/2;
  
    struct coord* left_points = coords;
    struct coord* right_points = coords+mid+1;
    struct node* node = malloc(sizeof(struct node));
    
    //Recursion
    node->dimension = current_dim;
    node->p = &coords[mid];
    node->leftnode = create_tree(left_points, mid, (dim+1)%2);
    node->rightnode = create_tree(right_points, n-(mid+1), (dim+1)%2);
  
    return node;
}

//function to make space for the tree and space for the tree
struct coord_data* mk_kdtree(struct record* rs, int n) {
    struct coord_data* data = malloc(sizeof(struct coord_data));
    data->rs = rs;
    data->n = n;

    //make space for the coordinates
    struct coord* coords = malloc(n * sizeof(struct coord));
    for (int i = 0; i < n; i++) {
        coords[i].x = rs[i].lon;
        coords[i].y = rs[i].lat;
        coords[i].rec = &rs[i];
    }

    data-> root = create_tree(coords, n, 0);
    data->coords = coords;

    return data;
}

// Recursion function for freeing the nodes
void free_nodes(struct node* node) {
    if (!node) return;
    free_nodes(node->leftnode);
    free_nodes(node->rightnode);
    free(node);
}

//Free the KdTree and coordinates
void free_kdtree(struct coord_data *data) {
    if (!data) return;
    free_nodes(data->root);
    free(data->coords);
    free(data);
}

//Helper function for euclidian formular
double distance (double x1, double x2, double y1, double y2){
    double x = x1-x2;
    double y = y1-y2;
    return sqrt(x*x+y*y);
}

//Helper function for the recursion in lookup   
const struct record* lookup_recursive(double *closest, 
                                      struct coord_data *data, 
                                      double query_lon, 
                                      double query_lat, 
                                      struct node *node,
                                      const struct record **best){
    if (node == NULL) return NULL;
    
    //calculates the euclidian distance between the node and the query
    double first_diff = distance(query_lon, node->p->x, query_lat, node->p->y);
    
    //updates closest ditsance and the ID of the record
    if (first_diff < *closest) {
        *closest = first_diff;
        *best = node->p->rec;
    }

    struct node *first = NULL;
    struct node *second = NULL;

    //finding out which way to go in the tree,
    //by looking at the dimension, furthermore the lon and lat
    if (node->dimension == 0) {
        if ( query_lon < node->p->x) {
            first = node->leftnode;
            second = node->rightnode;
        } else {
            first = node->rightnode;
            second = node->leftnode;
        }
    } else {
        if (query_lat < node->p->y) {
            first = node->leftnode;
            second = node->rightnode;
        } else {
            first = node->rightnode;
            second = node->leftnode;
        }
    }

    //Goes down the first route
    lookup_recursive(closest, data, query_lon, query_lat, first, best);

    //checking if its lucrative to go down the other route
    double diff;
    if (node->dimension == 0) {
        diff = query_lon - node->p->x;
    } else {
        diff = query_lat - node->p->y;
    }
    
    //If it is, we go down the route
    if (diff * diff < (*closest) * (*closest)) {
        lookup_recursive(closest, data, query_lon, query_lat, second, best);
    }

    
    return *best;
}

//function to look up the query in the kdtree
const struct record* lookup_kdtree(struct coord_data *data, double lon, double lat) {  

    // if the inputs are too big or too small clamp them to prevent overflow
    lon = fmin(fmax(lon, -180.0), 180.0);
    lat = fmin(fmax(lat,  -90.0),  90.0);


    const struct record *best = NULL;
    double closest = INFINITY;

    lookup_recursive(&closest, data, lon, lat, data->root, &best);

    return best;

}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_kdtree,
                          (free_index_fn)free_kdtree,
                          (lookup_fn)lookup_kdtree);
}