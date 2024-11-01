CC=gcc
P=simple-planner
OBJECTS= db-interface.o date-functions.o planner-functions.o # Dependencies that need to be compiled first.
CFLAGS = -fsanitize=address -g -ggdb -fno-omit-frame-pointer -Wall -O3
#CFLAGS = -g -O3
# Sometimes the warnings get overwhelming temporarily, so I use this.
LDLIBS = -lsqlite3
OUTDIR = ./debug
RELDIR = ./release
TESTS=./tests

# GNU MAKE DOES NOT LIKE SPACES!  Need to use tabs.
# To replace all spaces with tabs in Vim:
# set noexpandtab
# %retab!

debug: $(P)
	@mkdir -p $(OUTDIR)
	@rm -rf $(OUTDIR)/$(P)
	@mv $(P) $(OUTDIR)

release: OUTDIR=$(RELDIR)
release: debug

$(P): $(OBJECTS)

# Run this with something like `make test CASE=csv-handler`.
test: $(OBJECTS)
	@mkdir -p $(TESTS)
	@$(CC) $(CASE)-test.c $(CFLAGS) $(OBJECTS) $(LDLIBS) -o $(TESTS)/$(CASE)-test
