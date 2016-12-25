out : main.o session.o
	g++ -static -o out main.o session.o -std=c++11 -lpthread -lboost_coroutine -lboost_context -lboost_filesystem -lboost_thread -lboost_system
	
main.o : main.cpp session.h
	g++ -c main.cpp -std=c++11 -lpthread -lboost_coroutine -lboost_context -lboost_filesystem -lboost_thread -lboost_system
	
session.o : session.cpp session.h
	g++ -c session.cpp -std=c++11 -lpthread -lboost_coroutine -lboost_context -lboost_filesystem -lboost_thread -lboost_system
	
clean:
	rm *.o
