#ifndef __POTATO_HPP__
#define __POTATO_HPP__

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
  Potato() : num_hops(0), count(0), is_hot(true) {
    std::memset(trace, 0, sizeof(trace));  // initialize trace
  }
  Potato(int hops) : num_hops(hops), count(0), is_hot(true) {
    std::memset(trace, 0, sizeof(trace));  // initialize trace
  }
  // rule of three
  Potato(const Potato & rhs) :
      num_hops(rhs.num_hops), count(rhs.count), is_hot(rhs.is_hot) {
    std::memcpy(trace, rhs.trace, sizeof(trace));
  }
  Potato & operator=(const Potato & rhs) {
    if (this != &rhs) {
      num_hops = rhs.num_hops;
      count = rhs.count;
      is_hot = rhs.is_hot;
      std::memcpy(trace, rhs.trace, sizeof(trace));
    }
    return *this;
  }
  ~Potato(){};
  int getHops() const { return num_hops; }
  int getHotStatus() const { return is_hot; }
  void coolPotato() { is_hot = false; }
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

#endif  // POTATO_HPP