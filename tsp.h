/**
 * tsp.h
 * Function declarations and struct definitions for CPSC 223 PSet 2 (TSP)
 *
 * @author CPSC 223 Staff and you
 * @version 2025.01.31.0
 */

#ifndef __TSP_H__
#define __TSP_H__

#include <stdio.h>
#include <stddef.h>

#include "location.h"

typedef struct {
  const char *name; // a pointer to the city's name on the command-line
  location loc;     // the city's coordinates
  size_t index;     // the city's index on the command line
} city;


/**
 * Builds a tour by the nearest method.

 * @param n a positive integer
 * @param tour an n-element array containing the cities, with the starting
 * city at index 0
 */
void route_nearest(size_t n, city tour[]);


/**
 * Examines a range of cities in the given array and returns the index of
 * the closest one to the city at a given index.  Ties are broken
 * arbitrarily.
 *
 * @param tour a permutation of the cities
 * @param c and index into tour
 * @param from an index into tour
 * @param to an index into tour at least from + 1
 * @return the index of the closest city to tour[c] among tour[from]...tour[to]
 */
size_t find_closest_city(city tour[], size_t c, size_t from, size_t to);


/**
 * Builds a tour by the insert method.

 * @param n a positive integer
 * @param tour an n-element array containing the cities
 */
void route_insert(size_t n, city tour[]);


/**
 * Finds the indices of the closest pair of cities in the given array.
 * Ties are broken arbitrarily.
 *
 * @param n a positive integer
 * @param tour an array of n cities
 * @param best_orig a pointer to storage for the index of the first city
 * of the closest pair
 * @param best_dest a pointer to storage for the index of the second city
 * of the closest pair
 */
void find_closest_pair(size_t n, city tour[], size_t *best_orig, size_t *best_dest);


/**
 * Returns the index of the city at the end of the given array that is
 * closest to the tour containing the cities in the beginning of the array.
 *
 * @param n a positive integer
 * @param tour an array of length n containing a permutation of the cities
 * @param tour_len the size of the tour at the beginning of tour; < n
 * @return an index into tour between tour_len (inclusive) and n (exclusive)
 */
size_t find_closest_to_tour(size_t n, city tour[], size_t tour_len);


/**
 * Returns the index of the city in the tour at the beginning of the given
 * array to insert the given city after so that the length of the resulting
 * tour is minimized.
 *
 * @param n a positive integer
 * @param tour a n-element array containing the cities
 * @param subtour-len the length of the tour at the beginning of the array
 * @param next the index of the city to find the insertion point for, >= subtour_len
 */
size_t find_insertion_point(size_t n, city tour[], size_t subtour_len, size_t next);


/**
 * Reorders the given tour so that city 0 comes first.
 *
 * @param n a positive integer
 * @param tour an n-element array containing a permutation of the cities
 */
void normalize_start(size_t n, city tour[]);


/**
 * Reorders the given tour so that the index at location 1 is less than the index at location n-1
 *
 * @param n a positive integer
 * @param tour an n-element array containing a permutation of the cities
 */
void normalize_direction(size_t n, city tour[]);


/**
 * Swaps two elements in the given array.
 *
 * @param arr an array of cities
 * @param i an index into arr
 * @param j an index into arr
 */
void swap(city arr[], size_t i, size_t j);


/**
 * Calculates the total distance of the given tour.
 *
 * @param n a positive integer
 * @param tour an n-element array containing a permutation of the cities
 */
double calculate_total(size_t n, city tour[]);


/**
 * Reads lat/lon for given cities into the given array.
 *
 * @param in a file open for reading with lines in the form name,lat,lon
 * @param n a nonnegative integer
 * @param cities an array of size n
 */
int read_file(FILE *in, size_t n, city cities[]);

/**
 * Prints the names of the cities in the given array with a single space
 * between them and a newline at the end.
 *
 * @param n a positive integer
 * @param tour an n-element array containing a permutation of the cities
 */
void print_tour(size_t n, city cities[]);

#endif
