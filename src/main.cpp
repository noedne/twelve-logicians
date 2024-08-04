#include <iostream>
#include <limits>
#include <unordered_map>
#include <vector>

using Position = uint8_t;
using Positions = std::vector<Position>;

constexpr uint64_t factorial(unsigned n) {
  return n <= 1 ? 1 : (n * factorial(n - 1));
}

struct Hash {
  Position max_position;
  uint64_t operator()(const Positions &positions) const noexcept {
    uint64_t hash = 0;
    for (size_t i = 0; i < positions.size(); i++) {
      hash += positions[i] * factorial(max_position - static_cast<unsigned>(i));
    }
    return hash;
  }
};

using Round = uint8_t;
// value is the first round the key was eliminated
using Map = std::unordered_map<Positions, Round, Hash>;
// represents no round
const Round MAX_ROUND = std::numeric_limits<Round>::max();

void init_map_recursive(unsigned num_logicians, Positions &position_acc,
                        std::vector<bool> &position_used, Map &map) {
  if (position_acc.size() == num_logicians) {
    map[position_acc] = MAX_ROUND;
    return;
  }
  for (Position position = 0; position < position_used.size(); position++) {
    if (position_used[position]) {
      continue;
    }
    position_acc.push_back(position);
    position_used[position] = true;
    init_map_recursive(num_logicians, position_acc, position_used, map);
    position_acc.pop_back();
    position_used[position] = false;
  }
}

Map init_map(unsigned num_logicians, unsigned num_positions) {
  Positions position_acc;
  std::vector<bool> position_used(num_positions, false);
  Map map =
      Map{factorial(num_positions) / factorial(num_positions - num_logicians),
          Hash{static_cast<Position>(num_positions - 1)}};
  init_map_recursive(num_logicians, position_acc, position_used, map);
  return map;
}

bool says_yes(size_t logician, unsigned num_positions,
              const Positions &positions, Round round, const Map &map) {
  auto my_position = positions[logician];
  std::vector<bool> position_used(num_positions, false);
  Position max_below = 0;
  Position min_above = static_cast<Position>(num_positions - 1);
  for (auto position : positions) {
    position_used[position] = true;
    if (position == my_position) {
      continue;
    }
    if (max_below <= position && position < my_position) {
      max_below = position + 1;
    } else if (my_position < position && position <= min_above) {
      min_above = position - 1;
    }
  }
  auto positions_copy = positions;
  for (Position position = 0; position < num_positions; position++) {
    if (position_used[position] ||
        (max_below <= position && position <= min_above)) {
      continue;
    }
    positions_copy[logician] = position;
    if (map.at(positions_copy) >= round) {
      return false;
    }
  }
  return true;
}

// returns whether map changed
bool perform_round(unsigned num_logicians, unsigned num_positions,
                   const Positions &positions, Round round, Map &map,
                   std::vector<Round> &first_yes_rounds) {
  for (unsigned logician = 0; logician < num_logicians; logician++) {
    if (first_yes_rounds[logician] == MAX_ROUND &&
        says_yes(logician, num_positions, positions, round, map)) {
      first_yes_rounds[logician] = round;
    }
  }
  bool changed = false;
  for (auto &[key, value] : map) {
    if (value < MAX_ROUND) {
      continue;
    }
    for (unsigned logician = 0; logician < num_logicians; logician++) {
      if (first_yes_rounds[logician] >= round &&
          says_yes(logician, num_positions, key, round, map) !=
              (first_yes_rounds[logician] == round)) {
        value = round;
        changed = true;
        break;
      }
    }
  }
  return changed;
}

// returns first round each logician says yes
std::vector<Round> simulate(unsigned num_logicians, unsigned num_positions,
                            const Positions &positions) {
  auto map = init_map(num_logicians, num_positions);
  std::vector<Round> first_yes_rounds(num_logicians, MAX_ROUND);
  for (Round round = 0; round < MAX_ROUND; round++) {
    if (!perform_round(num_logicians, num_positions, positions, round, map,
                       first_yes_rounds)) {
      break;
    }
  }
  return first_yes_rounds;
}

void simulate_io() {
  std::cout << "Number of logicians: ";
  unsigned num_logicians;
  std::cin >> num_logicians;
  std::cout << "Number of positions: ";
  unsigned num_positions;
  std::cin >> num_positions;
  std::cout << "Position of each logician: ";
  Positions positions;
  for (unsigned i = 0; i < num_logicians; i++) {
    unsigned position;
    std::cin >> position;
    positions.push_back(static_cast<Position>(position - 1));
  }
  auto first_yes_rounds = simulate(num_logicians, num_positions, positions);
  std::cout << "First yes said in round: ";
  for (auto round : first_yes_rounds) {
    if (round == MAX_ROUND) {
      std::cout << "N ";
    } else {
      std::cout << (static_cast<unsigned>(round) + 1) << ' ';
    }
  }
  std::cout << '\n';
}

int main() { simulate_io(); }