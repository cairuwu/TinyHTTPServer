CC =  g++

LIB= -L/usr/local/libevent/lib -L/usr/lib64/mysql

DEBUG= -g

INCLUDE = -I/usr/local/libevent/include

CFLAGS ?= -levent -pthread -lmysqlclient

Server: main.cpp server.cpp ./log/log.cpp ./CGImysql/sql_connection_pool.cpp ./set/callback.cpp
	$(CC) $(DEBUG) -o Server $^ $(INCLUDE) $(LIB) $(CFLAGS)


clean:
	rm Server
