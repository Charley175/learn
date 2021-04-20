DIRS = ./ThreadPool ./src/server ./src/client
#MAKEFLAGS += --no-print-directory

PHONY   := all

all: 
	@$(foreach DIR, $(DIRS), $(MAKE) -C $(DIR) ;)

PHONY +=clean

clean:
	@$(foreach DIR, $(DIRS), $(MAKE) -C $(DIR) clean;)
