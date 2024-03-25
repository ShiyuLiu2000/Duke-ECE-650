from sqlalchemy import create_engine, Column, Integer, String, Float, ForeignKey
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import relationship

Base = declarative_base()

class State(Base):
    __tablename__ = 'state'
    state_id = Column(Integer, primary_key=True)
    name = Column(String(15), nullable=False)

class Color(Base):
    __tablename__ = 'color'
    color_id = Column(Integer, primary_key=True)
    name = Column(String(20), nullable=False)

class Team(Base):
    __tablename__ = 'team'
    team_id = Column(Integer, primary_key=True)
    name = Column(String(20), nullable=False)
    state_id = Column(Integer, ForeignKey('state.state_id'), nullable=True)
    color_id = Column(Integer, ForeignKey('color.color_id'), nullable=True)
    wins = Column(Integer, default=0)
    losses = Column(Integer, default=0)

class Player(Base):
    __tablename__ = 'player'
    player_id = Column(Integer, primary_key=True)
    team_id = Column(Integer, ForeignKey('team.team_id'), nullable=True)
    uniform_num = Column(Integer)
    first_name = Column(String(30), nullable=False)
    last_name = Column(String(30), nullable=False)
    mpg = Column(Integer, default=0)
    ppg = Column(Integer, default=0)
    rpg = Column(Integer, default=0)
    apg = Column(Integer, default=0)
    spg = Column(Float)
    bpg = Column(Float)

    team = relationship("Team", back_populates="players")

Team.players = relationship("Player", order_by=Player.player_id, back_populates="team")
