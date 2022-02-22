CC=gcc
P=simple-planner
CFLAGS = -fsanitize=address -g -ggdb -fno-omit-frame-pointer -Wall -O3
#CFLAGS = -g -O3
# Sometimes the warnings get overwhelming temporarily, so I use this.
LDLIBS = sqlite3
OUTDIR = ./debug
RELDIR = ./release
EXEC=$(P).run

# To replace all spaces with tabs in Vim:
# set noexpandtab
# %retab!

$(P):
	@rm -rf $(OUTDIR)
	@mkdir $(OUTDIR)
	@$(CC) $(P).c $(CFLAGS) -l $(LDLIBS) -o $(OUTDIR)/$(EXEC)

andrun: $(P)
	@$(OUTDIR)/$(EXEC) ./$(OUTDIR)

release:
	@rm -rf $(RELDIR)
	@mkdir $(RELDIR)
	@$(CC) $(P).c -l $(LDLIBS) -o $(RELDIR)/$(EXEC)

testsqlite:
	@$(CC) sqlite-testing.c $(CFLAGS) -l $(LDLIBS) -o sqlite-testing.run
