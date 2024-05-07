from sqlalchemy.orm import Session
from models import Player, Team, State, Color

def query1(session: Session, use_mpg, min_mpg, max_mpg, use_ppg, min_ppg, max_ppg, use_rpg, min_rpg, max_rpg, use_apg, min_apg, max_apg, use_spg, min_spg, max_spg, use_bpg, min_bpg, max_bpg):
    query = session.query(Player)
    if use_mpg:
        query = query.filter(Player.mpg.between(min_mpg, max_mpg))
    if use_ppg:
        query = query.filter(Player.ppg.between(min_ppg, max_ppg))
    if use_rpg:
        query = query.filter(Player.rpg.between(min_rpg, max_rpg))
    if use_apg:
        query = query.filter(Player.apg.between(min_apg, max_apg))
    if use_spg:
        query = query.filter(Player.spg.between(min_spg, max_spg))
    if use_bpg:
        query = query.filter(Player.bpg.between(min_bpg, max_bpg))
    print("PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG")
    for player in query:
        print(player.player_id, player.team_id, player.uniform_num, player.first_name, player.last_name, player.mpg, player.ppg, player.rpg, player.apg, player.spg, player.bpg)

def query2(session: Session, team_color):
    teams = session.query(Team).join(Color).filter(Color.name == team_color).all()
    print("NAME")
    for team in teams:
        print(team.name)

def query3(session: Session, team_name):
    players = session.query(Player).join(Team).filter(Team.name == team_name).order_by(Player.ppg.desc()).all()
    print("FIRST_NAME LAST_NAME")
    for player in players:
        print(player.first_name, player.last_name)

def query4(session: Session, team_state, team_color):
    players = session.query(Player).join(Team).join(State).join(Color).filter(State.name == team_state, Color.name == team_color).all()
    print("UNIFORM_NUM FIRST_NAME LAST_NAME")
    for player in players:
        print(player.uniform_num, player.first_name, player.last_name)

def query5(session: Session, num_wins):
    teams = session.query(Team).filter(Team.wins > num_wins).all()
    print("FIRST_NAME LAST_NAME NAME WINS")
    for team in teams:
        players = session.query(Player).filter(Player.team_id == team.team_id).all()
        for player in players:
            print(player.first_name, player.last_name, team.name, team.wins)

