#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>

#include "dinic.hpp"

// Считывает сеть, запускает алгоритм Диница и печатает максимальный поток.
int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  try {
    int vertices, source, sink;
    std::int64_t edges;
    if (!(std::cin >> vertices >> edges >> source >> sink) || edges < 0) {
      throw std::invalid_argument("Ожидается строка V E s t");
    }

    Dinic network(vertices);
    for (std::int64_t i = 0; i < edges; ++i) {
      int from;
      int to;
      std::int64_t capacity;
      if (!(std::cin >> from >> to >> capacity)) {
        throw std::invalid_argument("Ожидается строка u v c");
      }
      network.add_edge(from, to, capacity);
    }
    std::cout << network.max_flow(source, sink) << '\n';
  } catch (const std::exception& error) {
    std::cerr << "Ошибка: " << error.what() << '\n';
    return 1;
  }
}
