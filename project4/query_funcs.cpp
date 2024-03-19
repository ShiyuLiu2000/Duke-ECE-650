#include "query_funcs.h"

void add_player(connection * C,
                int team_id,
                int jersey_num,
                string first_name,
                string last_name,
                int mpg,
                int ppg,
                int rpg,
                int apg,
                double spg,
                double bpg) {
  // "work" assures atomicity, rolls back when error happens
  work W(*C);
  std::stringstream sql;
  sql << "INSERT INTO PLAYER (TEAM_ID, UNIFORM_NUM, FIRST_NAME, LAST_NAME, MPG, PPG, "
         "RPG, APG, SPG, BPG) VALUES ("
      << team_id << ", " << jersey_num << ", " << W.quote(first_name) << ", "
      << W.quote(last_name) << ", " << mpg << ", " << ppg << ", " << rpg << ", " << apg
      << ", " << spg << ", " << bpg << ");";
  result r = W.exec(sql.str());
  W.commit();
  std::cout << "inserted into player" << std::endl;
}

void add_team(connection * C,
              string name,
              int state_id,
              int color_id,
              int wins,
              int losses) {
  work W(*C);
  std::stringstream sql;
  sql << "INSERT INTO TEAM (NAME, STATE_ID, COLOR_ID, WINS, LOSSES) VALUES ("
      << W.quote(name) << ", " << state_id << ", " << color_id << ", " << wins << ", "
      << losses << ");";
  result r = W.exec(sql.str());
  W.commit();
  std::cout << "inserted into team" << std::endl;
}

void add_state(connection * C, string name) {
  work W(*C);
  std::stringstream sql;
  sql << "INSERT INTO STATE (NAME) VALUES (" << W.quote(name) << ");";
  result r = W.exec(sql.str());
  W.commit();
  std::cout << "inserted into state" << std::endl;
}

void add_color(connection * C, string name) {
  work W(*C);
  std::stringstream sql;
  sql << "INSERT INTO COLOR (NAME) VALUES (" << W.quote(name) << ");";
  result r = W.exec(sql.str());
  W.commit();
  std::cout << "inserted into color" << std::endl;
}

/*
 * All use_ params are used as flags for corresponding attributes
 * a 1 for a use_ param means this attribute is enabled (i.e. a WHERE clause is needed)
 * a 0 for a use_ param means this attribute is disabled
 */
// show all attributes of each player with average statistics that fall between the min and max (inclusive) for each enabled statistic
void query1(connection * C,
            int use_mpg,
            int min_mpg,
            int max_mpg,
            int use_ppg,
            int min_ppg,
            int max_ppg,
            int use_rpg,
            int min_rpg,
            int max_rpg,
            int use_apg,
            int min_apg,
            int max_apg,
            int use_spg,
            double min_spg,
            double max_spg,
            int use_bpg,
            double min_bpg,
            double max_bpg) {
}

// show the name of each team with the indicated uniform color
void query2(connection * C, string team_color) {
}

// show the first and last name of each player that plays for the indicated team, ordered from highest to lowest ppg (points per game)
void query3(connection * C, string team_name) {
}

// show uniform number, first name and last name of each player that plays in the indicated state and wears the indicated uniform color
void query4(connection * C, string team_state, string team_color) {
}

// show first name and last name of each player, and team name and number of wins for each team that has won more than the indicated number of games
void query5(connection * C, int num_wins) {
}
