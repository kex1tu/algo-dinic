#include "dinic.hpp"

#include <algorithm>
#include <limits>
#include <queue>
#include <stdexcept>

Dinic::Dinic(int vertices) {
  if (vertices < 2) {
    throw std::invalid_argument("Нужно не менее двух вершин");
  }
  graph_.resize(static_cast<std::size_t>(vertices));
}

bool Dinic::valid_vertex(int vertex) const noexcept {
  return vertex >= 0 && static_cast<std::size_t>(vertex) < graph_.size();
}

void Dinic::add_edge(int from, int to, std::int64_t capacity) {
  if (!valid_vertex(from) || !valid_vertex(to) || from == to || capacity < 0) {
    throw std::invalid_argument("Некорректное ребро");
  }

  auto& outgoing = graph_[from];
  auto& incoming = graph_[to];
  // У каждой исходной дуги своя пара записей. Обратная начинает с нулевой
  // ёмкости.
  outgoing.push_back({to, incoming.size(), capacity});
  try {
    incoming.push_back({from, outgoing.size() - 1, 0});
  } catch (...) {
    // При ошибке выделения памяти сохраняем целостность парных записей.
    outgoing.pop_back();
    throw;
  }
}

std::vector<int> Dinic::build_levels(int source) {
  // BFS записывает расстояния от истока в числе дуг. Значение -1 означает
  // недостижимость.
  std::vector<int> levels(graph_.size(), -1);
  std::queue<int> queue;
  levels[source] = 0;
  queue.push(source);

  while (!queue.empty()) {
    const int vertex = queue.front();
    queue.pop();
    for (const auto& edge : graph_[vertex]) {
      // Считается просмотр записи, даже если дуга недоступна.
      ++checks_;
      if (edge.residual > 0 && levels[edge.to] == -1) {
        levels[edge.to] = levels[vertex] + 1;
        queue.push(edge.to);
      }
    }
  }
  return levels;
}

std::int64_t Dinic::push(int source, int sink, const std::vector<int>& levels,
                         std::vector<std::size_t>& current,
                         std::int64_t remaining_flow) {
  // Для каждой вершины стека храним минимум остаточных ёмкостей от истока до
  // неё.
  struct Step {
    int vertex;
    std::int64_t limit;
  };
  std::vector<Step> path{{source, std::numeric_limits<std::int64_t>::max()}};

  while (!path.empty()) {
    const auto [vertex, limit] = path.back();
    if (vertex == sink) {
      if (limit > remaining_flow) {
        throw std::overflow_error(
            "Максимальный поток превышает диапазон int64_t");
      }
      // Для каждой дуги пути current хранит индекс выбранной записи.
      // Уменьшаем её остаток и увеличиваем запас для обратного шага.
      for (std::size_t i = 0; i + 1 < path.size(); ++i) {
        const int from = path[i].vertex;
        auto& edge = graph_[from][current[from]];
        edge.residual -= limit;
        graph_[edge.to][edge.reverse].residual += limit;
      }
      return limit;
    }

    if (current[vertex] == graph_[vertex].size()) {
      path.pop_back();
      if (!path.empty()) {
        // Ветка исчерпана. Продолжаем с соседней дуги у предыдущей вершины.
        ++current[path.back().vertex];
      }
      continue;
    }

    const auto& edge = graph_[vertex][current[vertex]];
    ++checks_;
    if (edge.residual > 0 && levels[edge.to] == levels[vertex] + 1) {
      path.push_back({edge.to, std::min(limit, edge.residual)});
    } else {
      ++current[vertex];
    }
  }
  return 0;
}

std::int64_t Dinic::max_flow(int source, int sink) {
  if (!valid_vertex(source) || !valid_vertex(sink) || source == sink) {
    throw std::invalid_argument("Некорректные исток и сток");
  }

  checks_ = 0;
  phases_ = 0;
  std::int64_t flow = 0;
  while (true) {
    const auto levels = build_levels(source);
    if (levels[sink] == -1) {
      // Увеличивающих путей больше нет, поэтому поток максимален.
      return flow;
    }
    ++phases_;
    // Сохраняем позиции между проталкиваниями. Для новых уровней начинаем с
    // нуля.
    std::vector<std::size_t> current(graph_.size(), 0);
    while (const auto pushed =
               push(source, sink, levels, current,
                    std::numeric_limits<std::int64_t>::max() - flow)) {
      flow += pushed;
    }
  }
}
