#include <charconv>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string_view>
#include <system_error>

#include "generator.hpp"

// Генерирует и печатает граф в формате, который читает решатель dinic.
int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "Использование: generator parallel|random_chain|random_graph "
                 "V seed\n";
    return 1;
  }

  int vertices = 0;
  std::uint32_t seed = 0;
  const std::string_view size_argument(argv[2]);
  const std::string_view seed_argument(argv[3]);
  const auto size_end = size_argument.data() + size_argument.size();
  const auto seed_end = seed_argument.data() + seed_argument.size();
  const auto size_result =
      std::from_chars(size_argument.data(), size_end, vertices);
  const auto seed_result =
      std::from_chars(seed_argument.data(), seed_end, seed);
  // Проверяем всю строку аргумента, чтобы не принять число с хвостом вроде
  // 64abc.
  if (size_result.ec != std::errc{} || size_result.ptr != size_end ||
      seed_result.ec != std::errc{} || seed_result.ptr != seed_end) {
    std::cerr << "Ошибка: V и seed должны быть целыми числами в допустимом "
                 "диапазоне\n";
    return 1;
  }

  try {
    const auto edges = generate(argv[1], vertices, seed);
    // Формат совпадает с входом решателя, поэтому программы можно связать через
    // pipe.
    std::cout << vertices << ' ' << edges.size() << " 0 " << vertices - 1
              << '\n';
    for (const auto& edge : edges) {
      std::cout << edge.from << ' ' << edge.to << ' ' << edge.capacity << '\n';
    }
  } catch (const std::exception& error) {
    std::cerr << "Ошибка: " << error.what() << '\n';
    return 1;
  }
}
