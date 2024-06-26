#include "query_funcs.h"

#include <numeric>  // std::accumulate

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
  //std::cout << "inserted into player" << std::endl;
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
  //std::cout << "inserted into team" << std::endl;
}

void add_state(connection * C, string name) {
  work W(*C);
  std::stringstream sql;
  sql << "INSERT INTO STATE (NAME) VALUES (" << W.quote(name) << ");";
  result r = W.exec(sql.str());
  W.commit();
  //std::cout << "inserted into state" << std::endl;
}

void add_color(connection * C, string name) {
  work W(*C);
  std::stringstream sql;
  sql << "INSERT INTO COLOR (NAME) VALUES (" << W.quote(name) << ");";
  result r = W.exec(sql.str());
  W.commit();
  //std::cout << "inserted into color" << std::endl;
}

/*
 * All use_ params are used as flags for corresponding attributes
 * a 1 for a use_ param means this attribute is enabled (i.e. a WHERE clause is needed)
 * a 0 for a use_ param means this attribute is disabled
 */
// show all attributes of each player with average statistics that fall between the min and max (inclusive) for each enabled statistic
void query1(connection * C,
            int use_mpg,  // 0
            int min_mpg,
            int max_mpg,
            int use_ppg,  // 1
            int min_ppg,
            int max_ppg,
            int use_rpg,  // 2
            int min_rpg,
            int max_rpg,
            int use_apg,  // 3
            int min_apg,
            int max_apg,
            int use_spg,  // 4
            double min_spg,
            double max_spg,
            int use_bpg,  // 5
            double min_bpg,
            double max_bpg) {
  // used for read-only operations
  nontransaction N(*C);
  std::stringstream sql;
  sql << "SELECT * FROM PLAYER";
  int flags[6] = {use_mpg, use_ppg, use_rpg, use_apg, use_spg, use_bpg};
  char choices[4] = {'M', 'P', 'R', 'A'};
  int mins[4] = {min_mpg, min_ppg, min_rpg, min_apg};
  int maxs[4] = {max_mpg, max_ppg, max_rpg, max_apg};
  int sum = std::accumulate(std::begin(flags), std::end(flags), 0);
  result r;
  if (sum > 0) {
    sql << " WHERE ";
    for (int i = 0; i < 4; i++) {
      if (flags[i] == 1) {
        sql << "(" << choices[i] << "PG BETWEEN " << mins[i] << " AND " << maxs[i]
            << ") AND ";
      }
    }
    if (flags[4] == 1) {
      sql << "(SPG BETWEEN " << min_spg << " AND " << max_spg << ") AND ";
    }
    if (flags[5] == 1) {
      sql << "(BPG BETWEEN " << min_bpg << " AND " << max_bpg << ") AND ";
    }
    // erase the last AND
    std::string temp = sql.str();
    temp.erase(temp.end() - 5, temp.end());
    std::stringstream newsql;
    newsql << temp;
    newsql << ";";
    r = N.exec(newsql.str());
  }
  else {
    sql << ";";
    r = N.exec(sql.str());
  }
  // print results
  std::cout
      << "PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG"
      << std::endl;
  result::const_iterator it = r.begin();
  while (it != r.end()) {
    for (int i = 0; i < 11; i++) {
      std::cout << it[i];
      if (i < 10) {
        std::cout << " ";
      }
      else {
        std::cout << std::endl;
      }
    }
    ++it;
  }
}

// show the name of each team with the indicated uniform color
void query2(connection * C, string team_color) {
  /*
  select team.name
  from team, color
  where team.color_id = color.color_id and color.name = team)color;
  */
  nontransaction N(*C);
  std::stringstream sql;
  sql << "SELECT TEAM.NAME FROM TEAM, COLOR WHERE (TEAM.COLOR_ID = COLOR.COLOR_ID) AND "
         "(COLOR.NAME = '"
      << team_color << "');";
  result r = N.exec(sql.str());
  // print results
  std::cout << "NAME" << std::endl;
  result::const_iterator it = r.begin();
  while (it != r.end()) {
    std::cout << it[0] << std::endl;
    ++it;
  }
}

// show the first and last name of each player that plays for the indicated team, ordered from highest to lowest ppg (points per game)
void query3(connection * C, string team_name) {
  /*
  select player.first_name, player.last_name
  from player, team
  where team.team_id = player.team_id and team.name = team_name
  order by ppg desc
  */
  nontransaction N(*C);
  std::stringstream sql;
  sql << "SELECT PLAYER.FIRST_NAME, PLAYER.LAST_NAME FROM PLAYER, TEAM WHERE "
         "(TEAM.TEAM_ID = PLAYER.TEAM_ID) AND (TEAM.NAME = '"
      << team_name << "')"
      << "ORDER BY PPG DESC;";
  result r = N.exec(sql.str());
  // print results
  std::cout << "FIRST_NAME LAST_NAME" << std::endl;
  result::const_iterator it = r.begin();
  while (it != r.end()) {
    std::cout << it[0] << " " << it[1] << std::endl;
    ++it;
  }
}

// show uniform number, first name and last name of each player that plays in the indicated state and wears the indicated uniform color
void query4(connection * C, string team_state, string team_color) {
  /* 
  select player.uniform_num, player.first_name, player.last_name 
  from player, team, state, color
  where player.team_id = team.team_id and team.state_id = state.state_id 
        and team.color_id = color.color_id and state.name = team_state 
        and color.name = team_color;
  */
  nontransaction N(*C);
  std::stringstream sql;
  sql << "SELECT PLAYER.UNIFORM_NUM, PLAYER.FIRST_NAME, PLAYER.LAST_NAME FROM PLAYER, "
         "TEAM, STATE, COLOR WHERE (PLAYER.TEAM_ID = TEAM.TEAM_ID) AND (TEAM.STATE_ID = "
         "STATE.STATE_ID) AND (TEAM.COLOR_ID = COLOR.COLOR_ID) AND (STATE.NAME = '"
      << team_state << "') AND (COLOR.NAME = '" << team_color << "');";
  result r = N.exec(sql.str());
  // print results
  std::cout << "UNIFORM_NUM FIRST_NAME LAST_NAME" << std::endl;
  result::const_iterator it = r.begin();
  while (it != r.end()) {
    std::cout << it[0] << " " << it[1] << " " << it[2] << std::endl;
    ++it;
  }
}

// show first name and last name of each player, and team name and number of wins for each team that has won more than the indicated number of games
void query5(connection * C, int num_wins) {
  /*
  select player.first_name, player.last_name, team.name, team.wins
  from player, team
  where player.team_id = team.team_id and team.wins > num_wins;
  */
  nontransaction N(*C);
  std::stringstream sql;
  sql << "SELECT PLAYER.FIRST_NAME, PLAYER.LAST_NAME, TEAM.NAME, TEAM.WINS FROM PLAYER, "
         "TEAM WHERE (PLAYER.TEAM_ID = TEAM.TEAM_ID) AND (TEAM.WINS > "
      << num_wins << ");";
  result r = N.exec(sql.str());
  // print results
  std::cout << "FIRST_NAME LAST_NAME NAME WINS" << std::endl;
  result::const_iterator it = r.begin();
  while (it != r.end()) {
    std::cout << it[0] << " " << it[1] << " " << it[2] << " " << it[3] << std::endl;
    ++it;
  }
}
