all: ringmaster player

ringmaster: ringmaster.cpp potato.hpp
	g++ -g -o $@ $<

player: player.cpp potato.hpp
	g++ -g -o $@ $<

clean:
	rm -f *.o *~ ringmaster player