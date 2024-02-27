#include <netdb.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <vector>

#include "potato.hpp"

class Ringmaster {
 private:
  int listen_fd;  // listening socket fd for player connection
  int num_players;
  int num_hops;
  std::vector<int> players;  // fd vector for player connections

  // initialize server socket by creating listening socket
  void setupServer(const char * port) {
    struct addrinfo hints, *res;
    int yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port, &hints, &res) != 0) {
      std::cerr << "Cannot getaddrinfo" << std::endl;
      exit(EXIT_FAILURE);
    }

    listen_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (listen_fd < 0) {
      std::cerr << "Cannot create socket" << std::endl;
      exit(EXIT_FAILURE);
    }

    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if (bind(listen_fd, res->ai_addr, res->ai_addrlen) < 0) {
      std::cerr << "Cannot bind socket" << std::endl;
      exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    if (listen(listen_fd, num_players) < 0) {
      std::cerr << "Cannot listen on socket" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  // wait for connections from all players and accept them
  void acceptPlayers() {
    struct sockaddr_storage player_addr;
    socklen_t addr_size = sizeof player_addr;
    int player_fd;

    for (int i = 0; i < num_players; i++) {
      player_fd = accept(listen_fd, (struct sockaddr *)&player_addr, &addr_size);
      if (player_fd < 0) {
        std::cerr << "Cannot accept player" << std::endl;
        continue;
      }
      players.push_back(player_fd);
      std::cout << "Player " << i << " is ready to play" << std::endl;
    }
  }

  // send player their id, total number of players, and their left & right player id
  void distributePlayerInfo();

  // send the potato to first random player
  void sendFirstPotato(Potato & potato) {
    int first_player = rand() % num_players;
    std::cout << "Ready to start the game, sending potato to player " << first_player
              << std::endl;
    int bytes_sent = send(players[first_player], &potato, sizeof(potato), 0);
    if (bytes_sent < 0) {
      std::cerr << "Cannot send potato to player " << first_player << std::endl;
    }
  }

  void closeConnections() {
    std::vector<int>::iterator it = players.begin();
    while (it != players.end()) {
      close(*it);
      ++it;
    }
    close(listen_fd);
  }

 public:
  Ringmaster(int n, int h, const char * port) : num_players(n), num_hops(h) {
    srand(time(NULL));
    setupServer(port);
  }
  void startGame() {
    acceptPlayers();
    Potato potato(num_hops);
    sendFirstPotato(potato);
    while (potato.getHotStatus()) {
      // game logic here concerning players to pass the potato around
      if (potato.getHops() == 0) {
        potato.coolPotato();
        break;
      }
    }
    closeConnections();
  }
};