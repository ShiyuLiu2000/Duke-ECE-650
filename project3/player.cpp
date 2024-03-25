#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

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

 public:
  Player(const std::string & hostname, const char * port, int id) : player_id(id) {
    connectRingmaster(hostname, port);
    //setupNeighborConnection();
    //joinGame();
  }
  void connectRingmaster(const std::string & hostname, const char * port) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
      std::cerr << "Cannot create ringmaster socket" << std::endl;
      exit(EXIT_FAILURE);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(std::stoi(port));

    if (inet_pton(AF_INET, hostname.c_str(), &server_addr.sin_addr) <= 0) {
      std::cerr << "Invalid address" << std::endl;
      exit(EXIT_FAILURE);
    }

    if (connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
      std::cerr << "Connection Failed" << std::endl;
      exit(EXIT_FAILURE);
    }
    std::cout << "Connected to ringmaster at " << hostname << ":" << port << std::endl;
  }

  //  void setupNeighborConnection();
  /* void joinGame() {
    recv(server_fd, &player_id, sizeof(player_id), 0);
    recv(server_fd, &total_players, sizeof(total_players), 0);
  }*/
  void closeConnections() {
    close(left_player_fd);
    close(right_player_fd);
    close(server_fd);
  }
  /*void listenPotato() {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &read_fds);
    ///
    bool game_end = false;
    ///////
    if (FD_ISSET(server_fd, &read_fds)) {
      recv(server_fd, &game_end, sizeof(game_end), 0);
      if (game_end) {
        /////////
        return;
      }
    }
  }*/
  ~Player() { closeConnections(); }
};

int main(int argc, char * argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <machine_name> <port_num>" << std::endl;
    return EXIT_FAILURE;
  }
  std::string hostname = argv[1];
  const char * port = argv[2];

  Player player(hostname, port, 0);
  /////////

  return 0;
}