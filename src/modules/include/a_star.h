#ifndef __A_STAR__
#define __A_STAR__

#include <stdint.h>
#include <stdbool.h>

// A* algorithm for pathfinding for the robot

#define NUM_NODES 15
#define MAX_EDGES_PER_NODE 4

typedef enum {
    EAST = 0,
    NORTH_EAST,
    NORTH,
    NORTH_WEST,
    WEST,
    SOUTH_WEST,
    SOUTH,
    SOUTH_EAST 
} Orientation;

typedef struct {
    uint8_t index;
    uint16_t g_cost; // cost from start to this node
    uint16_t h_cost; // heuristic cost from this node to the end
    uint16_t f_cost; // g + h
    uint8_t parent; // idx of parent node in path
    bool visited;
} Node;

typedef struct {
    uint8_t start;
    uint8_t end;
    uint8_t path[NUM_NODES];
    uint8_t path_len;
    uint16_t path_cost; // total weight of the path
} Path;

typedef struct {
    uint8_t num_nodes;
    uint8_t num_edges;
    uint8_t edges[NUM_NODES * MAX_EDGES_PER_NODE][2]; // each edge is a pair of node indices
    bool is_free[NUM_NODES * MAX_EDGES_PER_NODE]; // true if edge is free, false if obstacle
    uint16_t weights[NUM_NODES * MAX_EDGES_PER_NODE];
    Orientation angles[NUM_NODES * MAX_EDGES_PER_NODE][2];
    uint8_t adjacency[NUM_NODES][MAX_EDGES_PER_NODE];
    uint8_t adjacency_count[NUM_NODES];
} Graph;

void a_star_init(Graph* graph);
bool a_star_find_path(Graph* graph, Path* path, uint8_t start, uint8_t end);
Orientation get_heading(Graph* graph, uint8_t node1, uint8_t node2);
void a_star_set_edge_freeness(Graph* graph, uint8_t node1, uint8_t node2, bool is_free);

//testing purposes
void test_path(Graph* graph, Path* path, uint8_t _start, uint8_t _end);

#endif /* __A_STAR__ */
