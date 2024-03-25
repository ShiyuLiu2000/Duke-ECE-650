# main.py
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from models import Base
from query import query1,query2,query3,query4,query5 

DATABASE_URL = "postgresql://postgres:passw0rd@localhost/ACC_BBALL"

def main():
    engine = create_engine(DATABASE_URL)
    Base.metadata.create_all(engine)  
    Session = sessionmaker(bind=engine)
    session = Session()
    query2(session, "Orange")
    query3(session, "Duke")

    session.close()

if __name__ == "__main__":
    main()
