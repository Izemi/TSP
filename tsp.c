/**
 * tsp.c
 * 
 * Builds tours of cities using one or more heuristics.
 *
 * Command-line usage:
 *   ./TSP <datafile> [heuristic flags] <city names>
 *
 * For example:
 *   ./TSP ne_6.dat -given -nearest -insert HVN ALB MHT BDL ORH PVD
 *
 * When the input is valid, the output for each heuristic is printed to
 * standard output in the following format (using the provided location_distance
 * function):
 *
 *   <heuristic> : <total distance> <tour> 
 *
 * If no heuristic flags are provided, the program prints nothing.
 *
 * Efficiency requirements:
 *   - File reading: O(p) time.
 *   - -given   : O(n) time, O(n) space.
 *   - -nearest : O(n^2) time, O(n) space.
 *   - -insert  : O(n^3) time, O(n^2) space.
 *
 * For any invalid input, the program must not crash or hang.
 *
 * Author: Your Name
 * Version: 2025.01.31.0
 */

#define _GNU_SOURCE

#include "tsp.h"
#include "location.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

/* Function prototypes */
void route_nearest(size_t n, city tour[]);
void route_insert(size_t n, city tour[]);
void find_closest_pair(size_t n, city tour[], size_t *best_orig, size_t *best_dest);
size_t find_closest_to_tour(size_t n, city tour[], size_t tour_len);
size_t find_insertion_point(size_t n, city tour[], size_t subtour_len, size_t next);
size_t find_closest_city(city tour[], size_t c, size_t from, size_t to);
double calculate_total(size_t n, city tour[]);
void swap(city arr[], size_t i, size_t j);
void normalize_start(size_t n, city tour[]);
void normalize_direction(size_t n, city tour[]);
void print_tour(size_t n, city cities[]);

/**
 * Reads the entire location file into an array.
 * On success, allocates an array of cities (with location info) and returns 0.
 * Otherwise returns -1.
 * The caller is responsible for freeing the allocated memory.
 */
int read_location_file(const char *filename, city **locations, size_t *count) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen");
        return -1;
    }
    size_t capacity = 10;
    *locations = malloc(capacity * sizeof(city));
    if (!*locations) {
        fclose(fp);
        return -1;
    }
    char *line = NULL;
    size_t len = 0;
    size_t num = 0;
    while (getline(&line, &len, fp) != -1) {
        /* Skip blank lines */
        if (line[0] == '\n' || line[0] == '\0')
            continue;
        line[strcspn(line, "\n")] = '\0'; // remove newline
        char *name = strtok(line, ",");
        char *latStr = strtok(NULL, ",");
        char *lonStr = strtok(NULL, ",");
        if (!name || !latStr || !lonStr)
            continue;  // skip malformed lines
        if (num == capacity) {
            capacity *= 2;
            city *temp = realloc(*locations, capacity * sizeof(city));
            if (!temp) {
                free(*locations);
                fclose(fp);
                return -1;
            }
            *locations = temp;
        }
        (*locations)[num].name = strdup(name);
        (*locations)[num].loc.lat = atof(latStr);
        (*locations)[num].loc.lon = atof(lonStr);
        (*locations)[num].index = num;
        num++;
    }
    free(line);
    fclose(fp);
    *count = num;
    return 0;
}

/**
 * Main: parses command-line arguments, reads the location file,
 * builds the tour (using the order of city names on the command line),
 * and applies each requested heuristic.
 */
int main(int argc, char **argv)
{
    /* Minimum: program name, location file, one heuristic flag, and at least 2 city names */
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <datafile> [heuristic flags] <city names>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* argv[1] is the location data file */
    char *location_filename = argv[1];

    /* Heuristic flags are all arguments (after argv[1]) that begin with '-' */
    int arg = 2;
    while (arg < argc && argv[arg][0] == '-') {
        arg++;
    }
    int city_start = arg;
    int num_cmd_cities = argc - city_start;
    if (num_cmd_cities < 2) {
        fprintf(stderr, "Error: At least two city names must be provided.\n");
        return EXIT_FAILURE;
    }
    int num_heuristics = city_start - 2;
    /* If no heuristic flags are provided, produce no output */
    if (num_heuristics == 0)
        return 0;

    /* Read the full location file */
    city *all_locations = NULL;
    size_t num_locations = 0;
    if (read_location_file(location_filename, &all_locations, &num_locations) != 0) {
        fprintf(stderr, "Error reading location file '%s'\n", location_filename);
        return EXIT_FAILURE;
    }

    /* Build the tour array by looking up each command-line city name in the location file.
       (Each city on the command line is assumed to appear in the file.) */
    city *cmd_cities = malloc(num_cmd_cities * sizeof(city));
    if (!cmd_cities) {
        perror("malloc");
        for (size_t i = 0; i < num_locations; i++)
            free((void *)all_locations[i].name);
        free(all_locations);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < num_cmd_cities; i++) {
        char *name = argv[city_start + i];
        bool found = false;
        for (size_t j = 0; j < num_locations; j++) {
            if (strcmp(all_locations[j].name, name) == 0) {
                cmd_cities[i].name = strdup(all_locations[j].name);
                cmd_cities[i].loc = all_locations[j].loc;
                cmd_cities[i].index = i;
                found = true;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "Error: city '%s' not found in location file\n", name);
            for (int k = 0; k < i; k++)
                free((void *)cmd_cities[k].name);
            free(cmd_cities);
            for (size_t j = 0; j < num_locations; j++)
                free((void *)all_locations[j].name);
            free(all_locations);
            return EXIT_FAILURE;
        }
    }
    for (size_t i = 0; i < num_locations; i++)
        free((void *)all_locations[i].name);
    free(all_locations);

    /* Process each heuristic flag (in the order provided) */
    for (int h = 2; h < city_start; h++) {
        /* Make a working copy of the tour */
        city *tour = malloc(num_cmd_cities * sizeof(city));
        if (!tour) {
            perror("malloc");
            for (int i = 0; i < num_cmd_cities; i++)
                free((void *)cmd_cities[i].name);
            free(cmd_cities);
            return EXIT_FAILURE;
        }
        memcpy(tour, cmd_cities, num_cmd_cities * sizeof(city));

        if (strcmp(argv[h], "-given") == 0) {
            /* -given: no changes */
        } else if (strcmp(argv[h], "-nearest") == 0) {
            route_nearest(num_cmd_cities, tour);
        } else if (strcmp(argv[h], "-insert") == 0) {
            route_insert(num_cmd_cities, tour);
        } else {
            fprintf(stderr, "Error: Invalid heuristic '%s'\n", argv[h]);
            free(tour);
            continue;
        }
        normalize_start(num_cmd_cities, tour);
        normalize_direction(num_cmd_cities, tour);
        double total = calculate_total(num_cmd_cities, tour);
        printf("%-10s: %12.2f ", argv[h], total);
        print_tour(num_cmd_cities, tour);
        free(tour);
    }

    for (int i = 0; i < num_cmd_cities; i++) {
        free((void *)cmd_cities[i].name);
    }
    free(cmd_cities);

    return EXIT_SUCCESS;
}

/*---------------------- Heuristic and Utility Functions ----------------------*/

void route_nearest(size_t n, city tour[])
{
    for (size_t i = 1; i < n; i++) {
        size_t closest = i;
        double minDist = location_distance(&tour[i - 1].loc, &tour[i].loc);
        for (size_t j = i + 1; j < n; j++) {
            double d = location_distance(&tour[i - 1].loc, &tour[j].loc);
            if (d < minDist) {
                minDist = d;
                closest = j;
            }
        }
        swap(tour, i, closest);
    }
}

void route_insert(size_t n, city tour[])
{
    size_t best_orig = 0, best_dest = 0;
    find_closest_pair(n, tour, &best_orig, &best_dest);
    if (tour[best_orig].index > tour[best_dest].index) {
        size_t temp = best_orig;
        best_orig = best_dest;
        best_dest = temp;
    }
    if (best_orig != 0)
        swap(tour, 0, best_orig);
    if (best_dest != 1)
        swap(tour, 1, best_dest);

    size_t subtour_len = 2;
    while (subtour_len < n) {
        size_t next = find_closest_to_tour(n, tour, subtour_len);
        size_t pos = find_insertion_point(n, tour, subtour_len, next);
        city temp = tour[next];
        for (size_t i = next; i > pos + 1; i--) {
            tour[i] = tour[i - 1];
        }
        tour[pos + 1] = temp;
        subtour_len++;
    }
}

void find_closest_pair(size_t n, city tour[], size_t *best_orig, size_t *best_dest)
{
    double minDist = INFINITY;
    for (size_t i = 0; i < n; i++) {
        for (size_t j = i + 1; j < n; j++) {
            double d = location_distance(&tour[i].loc, &tour[j].loc);
            if (d < minDist) {
                minDist = d;
                *best_orig = i;
                *best_dest = j;
            }
        }
    }
}

size_t find_closest_to_tour(size_t n, city tour[], size_t tour_len)
{
    double minDist = INFINITY;
    size_t best = tour_len;
    for (size_t i = tour_len; i < n; i++) {
        double dmin = INFINITY;
        for (size_t j = 0; j < tour_len; j++) {
            double d = location_distance(&tour[i].loc, &tour[j].loc);
            if (d < dmin)
                dmin = d;
        }
        if (dmin < minDist) {
            minDist = dmin;
            best = i;
        }
    }
    return best;
}

size_t find_insertion_point(size_t n, city tour[], size_t subtour_len, size_t next)
{
    double minIncrease = INFINITY;
    size_t best = 0;
    for (size_t i = 0; i < subtour_len; i++) {
        size_t j = (i + 1) % subtour_len;
        double d_old = location_distance(&tour[i].loc, &tour[j].loc);
        double d_new = location_distance(&tour[i].loc, &tour[next].loc) +
                       location_distance(&tour[next].loc, &tour[j].loc);
        double increase = d_new - d_old;
        if (increase < minIncrease) {
            minIncrease = increase;
            best = i;
        }
    }
    return best;
}

size_t find_closest_city(city tour[], size_t c, size_t from, size_t to)
{
    double minDist = INFINITY;
    size_t best = from;
    for (size_t i = from; i < to; i++) {
        double d = location_distance(&tour[c].loc, &tour[i].loc);
        if (d < minDist) {
            minDist = d;
            best = i;
        }
    }
    return best;
}

double calculate_total(size_t n, city tour[])
{
    double tot = 0.0;
    for (size_t i = 0; i < n - 1; i++) {
        tot += location_distance(&tour[i].loc, &tour[i + 1].loc);
    }
    tot += location_distance(&tour[n - 1].loc, &tour[0].loc);
    return tot;
}

void swap(city arr[], size_t i, size_t j)
{
    if (i != j) {
        city temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

void normalize_start(size_t n, city tour[])
{
    size_t start = 0;
    for (size_t i = 0; i < n; i++) {
        if (tour[i].index == 0) {
            start = i;
            break;
        }
    }
    if (start == 0)
        return;
    city *temp = malloc(n * sizeof(city));
    if (!temp) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    size_t pos = 0;
    for (size_t i = start; i < n; i++)
        temp[pos++] = tour[i];
    for (size_t i = 0; i < start; i++)
        temp[pos++] = tour[i];
    memcpy(tour, temp, n * sizeof(city));
    free(temp);
}

void normalize_direction(size_t n, city tour[])
{
    if (n < 3)
        return;
    if (tour[1].index > tour[n - 1].index) {
        size_t start = 1, end = n - 1;
        while (start < end) {
            swap(tour, start, end);
            start++;
            end--;
        }
    }
}

void print_tour(size_t n, city cities[])
{
    if (n == 0)
        return;
    fprintf(stdout, "%s", cities[0].name);
    for (size_t i = 1; i < n; i++) {
        fprintf(stdout, " %s", cities[i].name);
    }
    fprintf(stdout, " %s\n", cities[0].name);
}
