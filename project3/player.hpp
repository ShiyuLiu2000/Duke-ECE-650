#ifndef __PLAYER_HPP__
#define __PLAYER_HPP__

#include <netinet/in.h>  // sockaddr_in

#include <string>

#include "potato.hpp"

class Player {
 private:
  int server_fd;  // ringmaster socket fd
  int player_id;
  int left_player_fd;
  int right_player_fd;
  sockaddr_in server_addr;
  sockaddr_in left_player_addr;
  sockaddr_in right_player_addr;
  int total_players;

  void connectRingmaster(const std::string & hostname, int port);
  void listenPotato();
  void setupNeighbor();

 public:
  Player(const std::string & hostname,
         int port,
         int id);  // ringmaster server info and player id
  ~Player();
  void joinGame();
  void playGame();  // if hop == 0, signal ringmaster to stop the game
  void closeConnections();
};

#endif  // PLAYER_HPP