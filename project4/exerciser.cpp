#include "exerciser.h"

void exercise(connection * C) {
  add_state(C, "state1");
  add_color(C, "color1");
  add_team(C, "team1", 1, 1, 0, 0);
  add_player(C, 1, 1, "play", "er1", 1, 0, 0, 0, 0, 0);
  query1(C, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}
