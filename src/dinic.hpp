#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

// Решатель задачи о максимальном потоке.
class Dinic {
 public:
  // Создаёт пустые списки смежности сети.
  explicit Dinic(int vertices);

  // Добавляет дугу и парную обратную запись, чтобы алгоритм мог
  // отменить часть ранее отправленного потока.
  void add_edge(int from, int to, std::int64_t capacity);

  // Добавляет поток по фазам, пока сток достижим в остаточной сети.
  std::int64_t max_flow(int source, int sink);

  // Возвращает число проверок дуг в BFS и DFS за последнее решение.
  std::uint64_t checks() const noexcept { return checks_; }

  // Возвращает число фаз, в которых сток был достижим и запускался DFS.
  int phases() const noexcept { return phases_; }

 private:
  struct Edge {
    int to;
    std::size_t reverse;
    std::int64_t residual;
  };

  // Проверяет номер вершины перед обращением к списку смежности.
  bool valid_vertex(int vertex) const noexcept;

  // Поиском в ширину строит уровни от истока, чтобы DFS мог идти
  // по кратчайшим путям в остаточной сети.
  std::vector<int> build_levels(int source);

  // Поиском в глубину находит один путь по уровням текущей фазы
  // и отправляет по нему столько потока, сколько позволяет самая узкая дуга.
  std::int64_t push(int source, int sink, const std::vector<int>& levels,
                    std::vector<std::size_t>& current,
                    std::int64_t remaining_flow);

  std::vector<std::vector<Edge>> graph_;
  std::uint64_t checks_ = 0;
  int phases_ = 0;
};
