#include "exerciser.h"

void exercise(connection * C) {
  add_state(C, "state1");
  add_color(C, "color1");
  add_team(C, "team1", 1, 1, 0, 0);
}
