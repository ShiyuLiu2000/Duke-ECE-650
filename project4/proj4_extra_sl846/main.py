# main.py
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from models import Base
from query import query1, query2, query3, query4, query5

DATABASE_URL = "postgresql://postgres:passw0rd@localhost/ACC_BBALL"


def main():
    engine = create_engine(DATABASE_URL)
    Base.metadata.create_all(engine)
    Session = sessionmaker(bind=engine)
    session = Session()
    query1(session, 1, 35, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    query1(session, 1, 35, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0)
    query2(session, "LightBlue")
    query2(session, "Maroon")
    query3(session, "Duke")
    query3(session, "UNC")
    query4(session, "NC", "DarkBlue")
    query5(session, 10)
    query5(session, 13)
    session.close()


if __name__ == "__main__":
    main()
