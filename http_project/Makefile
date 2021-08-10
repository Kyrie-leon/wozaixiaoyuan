BIN=http_server
SRC=main.cc
CC=g++
FLAGS=-std=c++11 -lpthread #-DDEBUG
CURRENT_PATH=$(shell pwd)

.PHONY:all
all:CGI $(BIN)

.PHONY:CGI
	cd $(CURRENT_PATH)/cgi;\
		chmod +x doSign;\
		cd $(CURRENT_PATH)

$(BIN):$(SRC)
	$(CC) -o $@ $^ $(FLAGS)
.PHONY:clean
clean:
	rm -f $(BIN);\
		rm -rf output;

.PHONY:install
install:
	make clean;\
		make;\
	mkdir -p output/wwwroot/cgi;\
	mkdir -p output/log;\
		cp $(BIN) output;\
		cp -rf wwwroot output;\
		cp cgi/doSign output/wwwroot/cgi;\
		cp ctrl.sh output;\
		cd $(CURRENT_PATH)/output;\
		chmod +x ctrl.sh;\
		cd ..



