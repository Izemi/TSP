# Traveling Salesperson Problem (TSP) Solver

## 📌 Overview
This project provides a **command-line tool** to solve the **Traveling Salesperson Problem (TSP)** using multiple heuristics. The goal is to find an efficient route that visits each specified city exactly once and returns to the starting point.

The program reads city locations from a **data file** and constructs tours using one or more heuristic methods. It then outputs the total distance and the computed tour for each heuristic.

---

## 🚀 Features
- **Multiple Heuristic Methods:**
  - **`-given`**: Uses the provided order of cities.
  - **`-nearest`**: Constructs a tour using the **Nearest Neighbor** heuristic.
  - **`-insert`**: Uses a **Greedy Insertion** heuristic to build the tour.
- **Efficient Parsing** of input city data.
- **Error Handling** for invalid input.
- **Computes Total Tour Distance** using great-circle distance.

---

## 🛠 Installation & Setup
### **Requirements**
- **C Compiler (gcc, clang, etc.)**
- **Make (optional, for easy compilation)**

### **Compiling the Program**
To compile the program, run:
```bash
gcc -o TSP tsp.c location.c -lm
