#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

struct InputEdge {
  int from;
  int to;
  int capacity;
};

// Генерирует дуги и ёмкости графа выбранного семейства для эксперимента.
std::vector<InputEdge> generate(std::string_view family, int vertices,
                                std::uint32_t seed);
