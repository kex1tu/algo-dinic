#include <array>
#include <cmath>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string_view>

#include "dinic.hpp"
#include "generator.hpp"

namespace {

constexpr std::array<int, 6> sizes{64, 128, 256, 512, 1024, 2048};
constexpr std::array<std::string_view, 3> families{"parallel", "random_chain",
                                                   "random_graph"};
constexpr int samples = 40;
constexpr std::uint32_t seed_base = 20260000;

// Запускает Диница на тестовых графах и сохраняет отдельные замеры и сводку в CSV.
void run_experiment(const std::filesystem::path& directory) {
  std::filesystem::create_directories(directory);
  std::ofstream measurements;
  std::ofstream summary;
  measurements.exceptions(std::ios::failbit | std::ios::badbit);
  summary.exceptions(std::ios::failbit | std::ios::badbit);
  measurements.open(directory / "measurements.csv");
  summary.open(directory / "summary.csv");
  // Один CSV хранит отдельные входы, второй сводит 40 запусков для каждого
  // размера.
  measurements << "family,vertices,edges,trial,seed,flow,phases,checks\n";
  summary << "family,vertices,edges,mean_checks,std_checks,mean_phases,"
             "checks_per_v,checks_per_v2e,doubling\n"
          << std::setprecision(std::numeric_limits<double>::max_digits10);

  for (const auto family : families) {
    double previous = 0;
    for (const int vertices : sizes) {
      std::array<std::uint64_t, samples> checks{};
      double total_checks = 0;
      int total_phases = 0;
      const int edge_count = 2 * vertices - 4;
      for (int trial = 0; trial < samples; ++trial) {
        // Seed задаётся размером и номером прогона. Для каждого входа строим
        // новую сеть.
        const auto seed =
            seed_base + static_cast<std::uint32_t>(vertices * samples + trial);
        const auto edges = generate(family, vertices, seed);
        Dinic network(vertices);
        for (const auto& edge : edges) {
          network.add_edge(edge.from, edge.to, edge.capacity);
        }

        const auto flow = network.max_flow(0, vertices - 1);
        checks[trial] = network.checks();
        total_checks += static_cast<double>(network.checks());
        total_phases += network.phases();
        measurements << family << ',' << vertices << ',' << edges.size() << ','
                     << trial << ',' << seed << ',' << flow << ','
                     << network.phases() << ',' << network.checks() << '\n';
      }

      const double mean = total_checks / samples;
      double squared_deviations = 0;
      for (const auto count : checks) {
        const double deviation = static_cast<double>(count) - mean;
        squared_deviations += deviation * deviation;
      }
      // Делитель n - 1 даёт исправленную выборочную дисперсию.
      const double stddev = std::sqrt(squared_deviations / (samples - 1));
      // V^2 * E - функция из общей верхней оценки, по которой нормируем
      // среднее.
      const double upper_bound =
          static_cast<double>(vertices) * vertices * edge_count;
      summary << family << ',' << vertices << ',' << edge_count << ',' << mean
              << ',' << stddev << ','
              << static_cast<double>(total_phases) / samples << ','
              << mean / vertices << ',' << mean / upper_bound << ',';
      if (previous > 0) {
        // Соседние размеры отличаются вдвое, поэтому это mean(V) / mean(V / 2).
        summary << mean / previous;
      }
      summary << '\n';
      previous = mean;
    }
  }
  measurements.close();
  summary.close();
}

}  // namespace

// Запускает эксперимент на трёх семействах и выводит путь к таблицам.
int main(int argc, char**) {
  if (argc != 1) {
    std::cerr << "Использование: experiment\n";
    return 1;
  }

  const std::filesystem::path directory = "results";
  try {
    run_experiment(directory);
    std::cout << "Результаты сохранены в " << directory.string() << '\n';
  } catch (const std::exception& error) {
    std::cerr << "Ошибка: " << error.what() << '\n';
    return 1;
  }
}
