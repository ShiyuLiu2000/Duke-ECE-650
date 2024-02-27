#ifndef __POTATO_H__
#define __POTATO_H__

#include <cstring>  // memset
#include <iostream>
#include <vector>

class Potato {
 private:
  int num_hops;    // number of hops for the game to end
  int trace[513];  // path of potato
  int count;       // number of current hops that has been made
  bool is_hot;

 public:
  Potato(int hops) : num_hops(hops), count(0), is_hot(true) {
    std::memset(trace, 0, sizeof(trace));  // initialize trace
  }
  int getHops() const { return num_hops; }
  int getHotStatus() const { return is_hot; }
  bool coolPotato() { is_hot = false; }
  void decrementHop() { num_hops--; }
  void addTrace(int playerID) {
    trace[count] = playerID;
    count++;
  }
  void printTrace() const {
    std::cout << "Trace of potato:" << std::endl;
    for (int i = 0; i < count; i++) {
      if (i != count - 1) {
        std::cout << trace[i] << ", ";
      }
      else {
        std::cout << trace[i] << std::endl;
      }
    }
  }
};

#endif  // POTATO_H