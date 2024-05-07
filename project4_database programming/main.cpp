#include <fstream>
#include <iostream>
#include <pqxx/pqxx>
#include <string>

#include "exerciser.h"

// using namespace std;
using namespace pqxx;

// drop existing tables in ACC_BBALL
void dropTables(connection * C) {
  std::stringstream sql;
  sql << "DROP TABLE IF EXISTS STATE CASCADE;";
  sql << "DROP TABLE IF EXISTS COLOR CASCADE;";
  sql << "DROP TABLE IF EXISTS TEAM CASCADE;";
  sql << "DROP TABLE IF EXISTS PLAYER CASCADE;";

  // "work" assures atomicity, rolls back when error happens
  work W(*C);
  result r = W.exec(sql.str());
  W.commit();

  //std::cout << "dropped tables" << std::endl;
}

// create tables with corresponding domain and constraints
void createTables(connection * C) {
  std::stringstream sql;

  // state
  sql << "CREATE TABLE STATE(";
  sql << "STATE_ID SERIAL,";
  sql << "NAME VARCHAR(15) NOT NULL,";
  sql << "CONSTRAINT STATE_IDPK PRIMARY KEY (STATE_ID)";
  sql << ");";

  // color
  sql << "CREATE TABLE COLOR(";
  sql << "COLOR_ID SERIAL,";
  sql << "NAME VARCHAR(20) NOT NULL,";
  sql << "CONSTRAINT COLOR_IDPK PRIMARY KEY (COLOR_ID)";
  sql << ");";

  // team
  sql << "CREATE TABLE TEAM(";
  sql << "TEAM_ID SERIAL,";
  sql << "NAME VARCHAR(20) NOT NULL,";
  sql << "STATE_ID INT,";
  sql << "COLOR_ID INT,";
  sql << "WINS INT CHECK (WINS >= 0) DEFAULT 0,";
  sql << "LOSSES INT CHECK (LOSSES >= 0) DEFAULT 0,";
  sql << "CONSTRAINT TEAM_IDPK PRIMARY KEY (TEAM_ID),";
  sql << "CONSTRAINT STATE_IDFK FOREIGN KEY (STATE_ID) REFERENCES STATE(STATE_ID) ON "
         "DELETE SET NULL ON UPDATE CASCADE,";
  sql << "CONSTRAINT COLOR_IDFK FOREIGN KEY (COLOR_ID) REFERENCES COLOR(COLOR_ID) ON "
         "DELETE SET NULL ON UPDATE CASCADE";
  sql << ");";

  // playerMPG, PPG, RPG, APG, SPG, BPG
  sql << "CREATE TABLE PLAYER(";
  sql << "PLAYER_ID SERIAL,";
  sql << "TEAM_ID INT,";
  sql << "UNIFORM_NUM INT,";
  sql << "FIRST_NAME VARCHAR(30) NOT NULL,";
  sql << "LAST_NAME VARCHAR(30) NOT NULL,";
  sql << "MPG INT CHECK (MPG >= 0) DEFAULT 0,";
  sql << "PPG INT CHECK (PPG >= 0) DEFAULT 0,";
  sql << "RPG INT CHECK (RPG >= 0) DEFAULT 0,";
  sql << "APG INT CHECK (APG >= 0) DEFAULT 0,";
  sql << "SPG DECIMAL(4, 1),";
  sql << "BPG DECIMAL(4, 1),";
  sql << "CONSTRAINT PLAYER_IDPK PRIMARY KEY (PLAYER_ID),";
  sql << "CONSTRAINT TEAM_IDFK FOREIGN KEY (TEAM_ID) REFERENCES TEAM(TEAM_ID) ON DELETE "
         "SET NULL ON UPDATE CASCADE";
  sql << ");";

  // "work" assures atomicity, rolls back when error happens
  work W(*C);
  result r = W.exec(sql.str());
  W.commit();

  //std::cout << "created tables" << std::endl;
}

// parse player.txt and load into database
void parsePlayer(connection * C, std::istream & stream) {
  std::string line;
  while (std::getline(stream, line).good()) {
    size_t pos = line.find(" ");
    line = line.substr(pos + 1);

    pos = line.find(" ");
    int team_id = std::stoi(line.substr(0, pos));
    line = line.substr(pos + 1);

    pos = line.find(" ");
    int jersey_num = std::stoi(line.substr(0, pos));
    line = line.substr(pos + 1);

    pos = line.find(" ");
    std::string first_name = line.substr(0, pos);
    line = line.substr(pos + 1);

    pos = line.find(" ");
    std::string last_name = line.substr(0, pos);
    line = line.substr(pos + 1);

    pos = line.find(" ");
    int mpg = std::stoi(line.substr(0, pos));
    line = line.substr(pos + 1);

    pos = line.find(" ");
    int ppg = std::stoi(line.substr(0, pos));
    line = line.substr(pos + 1);

    pos = line.find(" ");
    int rpg = std::stoi(line.substr(0, pos));
    line = line.substr(pos + 1);

    pos = line.find(" ");
    int apg = std::stoi(line.substr(0, pos));
    line = line.substr(pos + 1);

    pos = line.find(" ");
    double spg = std::stod(line.substr(0, pos));
    line = line.substr(pos + 1);

    double bpg = std::stod(line);

    add_player(
        C, team_id, jersey_num, first_name, last_name, mpg, ppg, rpg, apg, spg, bpg);
  }
  //std::cout << "parsed player.txt" << std::endl;
}

// parse team.txt and load into database
void parseTeam(connection * C, std::istream & stream) {
  std::string line;
  while (std::getline(stream, line).good()) {
    size_t pos = line.find(" ");
    line = line.substr(pos + 1);

    pos = line.find(" ");
    std::string name = line.substr(0, pos);
    line = line.substr(pos + 1);

    pos = line.find(" ");
    int state_id = std::stoi(line.substr(0, pos));
    line = line.substr(pos + 1);

    pos = line.find(" ");
    int color_id = std::stoi(line.substr(0, pos));
    line = line.substr(pos + 1);

    pos = line.find(" ");
    int wins = std::stoi(line.substr(0, pos));
    line = line.substr(pos + 1);

    int losses = std::stoi(line);

    add_team(C, name, state_id, color_id, wins, losses);
  }
  //std::cout << "parsed team.txt" << std::endl;
}

// parse state.txt and load into database
void parseState(connection * C, std::istream & stream) {
  std::string line;
  while (std::getline(stream, line).good()) {
    size_t pos = line.find(" ");
    line = line.substr(pos + 1);
    add_state(C, line);
  }
  //std::cout << "parsed state.txt" << std::endl;
}

// parse color.txt and load into database
void parseColor(connection * C, std::istream & stream) {
  std::string line;
  while (std::getline(stream, line).good()) {
    size_t pos = line.find(" ");
    line = line.substr(pos + 1);
    add_color(C, line);
  }
  //std::cout << "parsed color.txt" << std::endl;
}

int main(int argc, char * argv[]) {
  //Allocate & initialize a Postgres connection object
  connection * C;

  try {
    //Establish a connection to the database
    //Parameters: database name, user name, user password
    C = new connection("dbname=ACC_BBALL user=postgres password=passw0rd");
    if (C->is_open()) {
      //cout << "Opened database successfully: " << C->dbname() << endl;
    }
    else {
      cout << "Can't open database" << endl;
      return 1;
    }
  }
  catch (const std::exception & e) {
    cerr << e.what() << std::endl;
    return 1;
  }

  //TODO: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL database
  //      load each table with rows from the provided source txt files

  dropTables(C);
  createTables(C);

  // parse and load state.txt
  std::ifstream stateStream("state.txt");
  if (!stateStream.good()) {
    std::cerr << "Cannot open state.txt" << std::endl;
    exit(EXIT_FAILURE);
  }
  parseState(C, stateStream);
  stateStream.close();

  // parse and load color.txt
  std::ifstream colorStream("color.txt");
  if (!colorStream.good()) {
    std::cerr << "Cannot open color.txt" << std::endl;
    exit(EXIT_FAILURE);
  }
  parseColor(C, colorStream);
  colorStream.close();

  // parse and load team.txt
  std::ifstream teamStream("team.txt");
  if (!teamStream.good()) {
    std::cerr << "Cannot open team.txt" << std::endl;
    exit(EXIT_FAILURE);
  }
  parseTeam(C, teamStream);
  teamStream.close();

  // parse and load player.txt
  std::ifstream playerStream("player.txt");
  if (!playerStream.good()) {
    std::cerr << "Cannot open player.txt" << std::endl;
    exit(EXIT_FAILURE);
  }
  parsePlayer(C, playerStream);
  playerStream.close();

  exercise(C);

  //Close database connection
  C->disconnect();

  return 0;
}
