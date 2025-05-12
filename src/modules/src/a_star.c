#include "modules/include/a_star.h"
#include "modules/include/telemetry.h"
#include <stdlib.h>
#include <string.h>

void a_star_init_graph(Graph* graph, uint8_t num_nodes) {
    graph->num_nodes = num_nodes;
    graph->num_edges = 0;

    for (uint8_t i = 0; i < NUM_NODES; i++) {
        graph->adjacency_count[i] = 0;
    }
}

void a_star_add_edge(Graph* graph, uint8_t node1, uint8_t node2, uint16_t weight, uint8_t angle12, uint8_t angle21) {
    if (graph->num_edges >= NUM_NODES * MAX_EDGES_PER_NODE) {
        return;
    }

    graph->edges[graph->num_edges][0] = node1;
    graph->edges[graph->num_edges][1] = node2;

    graph->weights[graph->num_edges] = weight;

    graph->angles[graph->num_edges][0] = angle12;
    graph->angles[graph->num_edges][1] = angle21;

    graph->is_free[graph->num_edges] = true;


    if (graph->adjacency_count[node1 - 1] < MAX_EDGES_PER_NODE) {
        graph->adjacency[node1 - 1][graph->adjacency_count[node1 - 1]] = node2;
        graph->adjacency_count[node1 - 1]++;
    }

    if (graph->adjacency_count[node2 - 1] < MAX_EDGES_PER_NODE) {
        graph->adjacency[node2 - 1][graph->adjacency_count[node2 - 1]] = node1;
        graph->adjacency_count[node2 - 1]++;
    }

    graph->num_edges++;
}

uint16_t a_star_calculate_heuristic(uint8_t node, uint8_t goal) {
    // return (node > goal) ? (node - goal) : (goal - node);
    (void) node;
    (void) goal;
    return 0; // djikstra heuristic
}

uint8_t find_lowest_f_cost(Node* nodes, bool* open_set, uint8_t num_nodes) {
    uint8_t lowest_index = 0;
    uint16_t lowest_f_cost = 0xffff; // max value for uint16_t

    for (uint8_t i = 0; i < num_nodes; i++) {
        if (open_set[i] && nodes[i].f_cost < lowest_f_cost) {
            lowest_f_cost = nodes[i].f_cost;
            lowest_index = i;
        }
    }

    return lowest_index + 1; // return 1-based node index
}

void reconstruct_path(Path* path, Node* nodes, uint8_t current) {
    uint8_t count = 0;
    uint8_t path_reverse[NUM_NODES];
    uint16_t path_cost = 0;

    while (current != path->start) {
        path_reverse[count++] = current;
        path_cost += nodes[current - 1].g_cost - nodes[nodes[current - 1].parent - 1].g_cost;
        current = nodes[current - 1].parent;
    }

    // add start node
    path_reverse[count++] = path->start;

    // reverse path to get from start to end
    path->path_len = count;
    path->path_cost = path_cost;
    for (uint8_t i = 0; i < count; i++) {
        path->path[i] = path_reverse[count - i - 1];
    }
}

// A* algorithm implementation
bool a_star_find_path(Graph* graph, Path* path, uint8_t start, uint8_t end) {
    path->start = start;
    path->end = end;
    path->path_len = 0;
    path->path_cost = 0;

    // create node array
    Node nodes[NUM_NODES];
    for (uint8_t i = 0; i < graph->num_nodes; i++) {
        nodes[i].index = i + 1;
        nodes[i].g_cost = 0xffff; // high value
        nodes[i].h_cost = a_star_calculate_heuristic(i + 1, end);
        nodes[i].f_cost = 0xffff;
        nodes[i].visited = false;
    }

    // init start node
    nodes[start - 1].g_cost = 0;
    nodes[start - 1].f_cost = nodes[start - 1].h_cost;

    // create open and closed sets
    bool open_set[NUM_NODES] = {0};
    bool closed_set[NUM_NODES] = {0};

    open_set[start - 1] = true;

    // main A* loop
    while (1) {
        uint8_t current = find_lowest_f_cost(nodes, open_set, graph->num_nodes);

        if (!open_set[current - 1] || current == end) {
            break;
        }

        open_set[current - 1] = false;
        closed_set[current - 1] = true;

        for (uint8_t i = 0; i < graph->adjacency_count[current - 1]; i++) {
            uint8_t neighbor = graph->adjacency[current - 1][i];

            if (closed_set[neighbor - 1]) {
                continue;
            }

            uint16_t edge_weight = 1; // default weight
            bool edge_found = false;
            for (uint8_t j = 0; j < graph->num_edges; j++) {
                if (((graph->edges[j][0] == current && graph->edges[j][1] == neighbor) ||
                    (graph->edges[j][0] == neighbor && graph->edges[j][1] == current)) &&
                    graph->is_free[j]) {
                    edge_weight = graph->weights[j];
                    edge_found = true;
                    break;
                }
            }
            if (!edge_found) {
                continue; // skip blocked edges
            }

            uint16_t temporary_g_cost = nodes[current - 1].g_cost + edge_weight;

            if (!open_set[neighbor - 1]) {
                open_set[neighbor - 1] = true;
            } else if (temporary_g_cost >= nodes[neighbor - 1].g_cost) {
                continue;
            }

            // this path is the best so far, record it
            nodes[neighbor - 1].parent = current;
            nodes[neighbor - 1].g_cost = temporary_g_cost;
            nodes[neighbor - 1].f_cost = nodes[neighbor - 1].g_cost + nodes[neighbor - 1].h_cost;
        }
    }

    // if end reached, reconstruct the path
    if (closed_set[end - 1] || open_set[end - 1]) {
        reconstruct_path(path, nodes, end);
        return true;
    }

    // no path found
    return false;
}

Orientation get_heading(Graph* graph, uint8_t node1, uint8_t node2) {
    bool inverse = false;
    uint8_t edge_nb = 0;
    for (uint16_t i = 0; i < (graph->num_edges); ++i ){
        if ((graph->edges[i][0] == node1) && (graph->edges[i][1] == node2)){
            edge_nb = i; 
            break;
        } else if ((graph->edges[i][1] == node1) && (graph->edges[i][0] == node2)){
            edge_nb = i;
            inverse = true;
            break;
        }
    }
    return graph->angles[edge_nb][inverse];
}

void a_star_set_edge_freeness(Graph* graph, uint8_t node1, uint8_t node2, bool is_free) {
    for (uint8_t i = 0; i < graph->num_edges; i++) {
        if ((graph->edges[i][0] == node1 && graph->edges[i][1] == node2) ||
            (graph->edges[i][0] == node2 && graph->edges[i][1] == node1)) {
            graph->is_free[i] = is_free;
            return;
        }
    }
}

void test_path(Graph* graph, Path* path, uint8_t _start, uint8_t _end) {
    if (a_star_find_path(graph, path, _start, _end)) {
        epuck_printf("Path found from node %d to node %d:\n", path->start, path->end);
        epuck_printf("Path length: %d\n", path->path_len);
        epuck_printf("Total path weight: %d\n", path->path_cost);
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

void a_star_init(Graph* graph){

    a_star_init_graph(graph, 15);

    a_star_add_edge(graph, 1,  5,  120, SOUTH, NORTH_WEST);
    a_star_add_edge(graph, 1,  3,  90, EAST, NORTH);
    a_star_add_edge(graph, 1,  2,  7, SOUTH, NORTH);
    a_star_add_edge(graph, 2,  6,  60, WEST, NORTH_EAST);
    a_star_add_edge(graph, 2,  4,  35, SOUTH, WEST);
    a_star_add_edge(graph, 4,  10, 95, EAST, EAST);
    a_star_add_edge(graph, 4,  9,  120, SOUTH, NORTH);
    a_star_add_edge(graph, 5,  6,  15, NORTH_EAST, NORTH_WEST);
    a_star_add_edge(graph, 5,  7,  8, SOUTH_WEST, NORTH);
    a_star_add_edge(graph, 7,  8,  30, SOUTH, SOUTH_WEST);
    a_star_add_edge(graph, 6,  8,  17, SOUTH_EAST, NORTH_EAST);
    a_star_add_edge(graph, 7,  13, 107, EAST, EAST);
    a_star_add_edge(graph, 8,  9,  76, SOUTH_EAST, WEST);
    a_star_add_edge(graph, 11, 13, 63, WEST, NORTH);
    a_star_add_edge(graph, 13, 14, 66, SOUTH, WEST);
    a_star_add_edge(graph, 14, 15, 36, EAST, WEST);
    a_star_add_edge(graph, 9,  10, 20, EAST, WEST);
    a_star_add_edge(graph, 10, 12, 47, SOUTH, WEST);
    a_star_add_edge(graph, 3,  12, 90, SOUTH, NORTH);
    a_star_add_edge(graph, 12, 15, 25, SOUTH, EAST);
    a_star_add_edge(graph, 3,  15, 235, EAST, SOUTH);
    a_star_add_edge(graph, 11, 14, 15, SOUTH, NORTH);
    a_star_add_edge(graph, 9,  11, 9, SOUTH, NORTH);
}
