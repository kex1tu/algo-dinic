#include "generator.hpp"

#include <cstddef>
#include <random>
#include <stdexcept>
#include <unordered_set>

namespace {

bool has_path_to_sink(const std::vector<InputEdge>& edges, int sink) {
  const auto vertex_count = static_cast<std::size_t>(sink) + 1;
  std::vector<std::vector<int>> outgoing(vertex_count);
  for (const auto& edge : edges) {
    outgoing[edge.from].push_back(edge.to);
  }

  std::vector<bool> visited(vertex_count, false);
  std::vector<int> queue{0};
  visited[0] = true;
  for (std::size_t index = 0; index < queue.size(); ++index) {
    for (const int next : outgoing[queue[index]]) {
      if (next == sink) {
        return true;
      }
      if (!visited[next]) {
        visited[next] = true;
        queue.push_back(next);
      }
    }
  }
  return false;
}

}  // namespace

std::vector<InputEdge> generate(std::string_view family, int vertices,
                                std::uint32_t seed) {
  if (vertices < 4) {
    throw std::invalid_argument(
        "Для эксперимента нужно не менее четырёх вершин");
  }
  if (family != "parallel" && family != "random_chain" &&
      family != "random_graph") {
    throw std::invalid_argument(
        "Ожидается parallel, random_chain или random_graph");
  }

  const auto edge_count = 2 * static_cast<std::size_t>(vertices - 2);
  const int sink = vertices - 1;
  std::mt19937 rng(seed);
  std::vector<InputEdge> edges;
  edges.reserve(edge_count);

  if (family == "parallel") {
    // Через каждую промежуточную вершину проходит отдельный путь длины два.
    for (int vertex = 1; vertex < sink; ++vertex) {
      edges.push_back({0, vertex, 0});
      edges.push_back({vertex, sink, 0});
    }
  } else {
    const bool with_chain = family == "random_chain";
    std::unordered_set<std::uint64_t> used;
    used.reserve(edge_count);
    // Кодируем упорядоченную пару вершин, чтобы отсеивать дубликаты.
    // деление на V даёт from, остаток от деления даёт to
    const auto edge_key = [vertices](int from, int to) {
      return static_cast<std::uint64_t>(from) *
                 static_cast<std::uint64_t>(vertices) +
             static_cast<std::uint64_t>(to);
    };

    // Исключаем дуги в исток и из стока для обоих случайных семейств.
    std::uniform_int_distribution<int> source_vertex(0, sink - 1);
    std::uniform_int_distribution<int> target_vertex(1, sink);
    // Для random_graph отбираем случайные топологии с достижимым стоком.
    do {
      edges.clear();
      used.clear();
      if (with_chain) {
        for (int vertex = 0; vertex < sink; ++vertex) {
          edges.push_back({vertex, vertex + 1, 0});
          used.insert(edge_key(vertex, vertex + 1));
        }
      }
      while (edges.size() < edge_count) {
        const int from = source_vertex(rng);
        const int to = target_vertex(rng);
        if (from != to && used.insert(edge_key(from, to)).second) {
          edges.push_back({from, to, 0});
        }
      }
    } while (!with_chain && !has_path_to_sink(edges, sink));
  }

  // Назначаем ёмкости после того, как выбраны все пары вершин.
  std::uniform_int_distribution<int> capacity(1, 100);
  for (auto& edge : edges) {
    edge.capacity = capacity(rng);
  }
  return edges;
}
