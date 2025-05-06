#include "modules/include/a_star.h"
#include <stdlib.h>
#include <string.h>


void a_star_init_graph(Graph* graph, uint8_t num_nodes) {
    graph->num_nodes = num_nodes;
    graph->num_edges = 0;

    for (uint8_t i = 0; i < NUM_NODES; i++) {
        graph->adjacency_count[i] = 0;
    }
}

void a_star_add_edge(Graph* graph, uint8_t node1, uint8_t node2, uint8_t weight) {
    if (graph->num_edges >= NUM_NODES * MAX_EDGES_PER_NODE) {
        return;
    }

    graph->edges[graph->num_edges][0] = node1;
    graph->edges[graph->num_edges][1] = node2;
    graph->weights[graph->num_edges] = weight;

    if (graph->adjacency_count[node1] < MAX_EDGES_PER_NODE) {
        graph->adjacency[node1][graph->adjacency_count[node1]] = node2;
        graph->adjacency_count[node1]++;
    }

    if (graph->adjacency_count[node2] < MAX_EDGES_PER_NODE) {
        graph->adjacency[node2][graph->adjacency_count[node2]] = node1;
        graph->adjacency_count[node2]++;
    }

    graph->num_edges++;
}

uint8_t a_star_calculate_heuristic(uint8_t node, uint8_t goal) {
    return (node > goal) ? (node - goal) : (goal - node);
}

uint8_t find_lowest_f_cost(Node* nodes, bool* open_set, uint8_t num_nodes) {
    uint8_t lowest_index = 0;
    uint8_t lowest_f_cost = 255; // max value for uint8_t

    for (uint8_t i = 0; i < num_nodes; i++) {
        if (open_set[i] && nodes[i].f_cost < lowest_f_cost) {
            lowest_f_cost = nodes[i].f_cost;
            lowest_index = i;
        }
    }

    return lowest_index;
}

void reconstruct_path(Path* path, Node* nodes, uint8_t current) {
    uint8_t count = 0;
    uint8_t path_reverse[NUM_NODES];
    
    while (current != path->start) {
        path_reverse[count++] = current;
        current = nodes[current].parent;
    }
    
    // add start node
    path_reverse[count++] = path->start;
    
    // reverse path to get from start to end
    path->path_len = count;
    for (uint8_t i = 0; i < count; i++) {
        path->path[i] = path_reverse[count - i - 1];
    }
}

// A* algorithm implementation
bool a_star_find_path(Graph* graph, Path* path, uint8_t start, uint8_t end) {
    path->start = start;
    path->end = end;
    path->path_len = 0;
    
    // create node array
    Node nodes[NUM_NODES];
    for (uint8_t i = 0; i < graph->num_nodes; i++) {
        nodes[i].index = i;
        nodes[i].g_cost = 255; // high value
        nodes[i].h_cost = a_star_calculate_heuristic(i, end);
        nodes[i].f_cost = 255;
        nodes[i].visited = false;
    }
    
    // init start node
    nodes[start].g_cost = 0;
    nodes[start].f_cost = nodes[start].h_cost;
    
    // create open and closed sets
    bool open_set[NUM_NODES] = {0};
    bool closed_set[NUM_NODES] = {0};
    
    open_set[start] = true;
    
    // main A* loop
    while (1) {
        uint8_t current = find_lowest_f_cost(nodes, open_set, graph->num_nodes);
        
        if (!open_set[current] || current == end) {
            break;
        }
        
        open_set[current] = false;
        closed_set[current] = true;
        
        for (uint8_t i = 0; i < graph->adjacency_count[current]; i++) {
            uint8_t neighbor = graph->adjacency[current][i];
            
            if (closed_set[neighbor]) {
                continue;
            }
            
            uint8_t edge_weight = 1; // default weight
            for (uint8_t j = 0; j < graph->num_edges; j++) {
                if ((graph->edges[j][0] == current && graph->edges[j][1] == neighbor) ||
                    (graph->edges[j][0] == neighbor && graph->edges[j][1] == current)) {
                    edge_weight = graph->weights[j];
                    break;
                }
            }
            
            uint8_t temporary_g_cost = nodes[current].g_cost + edge_weight;
            
            if (!open_set[neighbor]) {
                open_set[neighbor] = true;
            } else if (temporary_g_cost >= nodes[neighbor].g_cost) {
                continue;
            }
            
            // this path is the best so far, record it
            nodes[neighbor].parent = current;
            nodes[neighbor].g_cost = temporary_g_cost;
            nodes[neighbor].f_cost = nodes[neighbor].g_cost + nodes[neighbor].h_cost;
        }
    }
    
    // if end reached, reconstruct the path
    if (closed_set[end] || open_set[end]) {
        reconstruct_path(path, nodes, end);
        return true;
    }
    
    // no path found
    return false;
}

void test_path(Graph* graph, Path* path, uint8_t _start, uint8_t _end) {
    if (a_star_find_path(graph, path, _start, _end)) {
        epuck_printf("Path found from node %d to node %d:\n", path->start, path->end);
        epuck_printf("Path length: %d\n", path->path_len);
        epuck_printf("Path: ");
        for (uint8_t i = 0; i < path->path_len; i++) {
            epuck_printf("%d", path->path[i]);
            if (i < path->path_len - 1) {
                epuck_printf(" -> ");
            }
        }
        epuck_printf("\n");
    } else {
        epuck_printf("No path found from node %d to node %d\n", _start, _end);
    }
}