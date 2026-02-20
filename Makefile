# ============================= #
NOM_ETU = NOM_PRENOM_A_CHANGER
# ============================= #

# Compiler and flags
CC     = gcc
# enable POSIX features (nanosleep...) with -D_POSIX_C_SOURCE
CFLAGS = -std=c11 -D_POSIX_C_SOURCE=200809L -Wall
LDLIBS = -lpthread

# Dirs
SRCDIR = src
OBJDIR = obj
BINDIR = bin
FILESDIR = fichiers

# Binaries
SENDER = $(BINDIR)/emetteur
RECEIVER = $(BINDIR)/recepteur

# Common / app objects
OBJ_COMMON = $(OBJDIR)/config.o $(OBJDIR)/services_reseau.o $(OBJDIR)/couche_transport.o
OBJ_APP_NC = $(OBJDIR)/appli_non_connectee.o

# TDD variants
OBJ_TDD0_S = $(OBJDIR)/proto_tdd_v0_emetteur.o
OBJ_TDD0_R = $(OBJDIR)/proto_tdd_v0_recepteur.o

OBJ_TDD1_S = $(OBJDIR)/proto_tdd_v1_emetteur.o
OBJ_TDD1_R = $(OBJDIR)/proto_tdd_v1_recepteur.o

OBJ_TDD2_S = $(OBJDIR)/proto_tdd_v2_emetteur.o
OBJ_TDD2_R = $(OBJDIR)/proto_tdd_v2_recepteur.o

OBJ_TDD3_S = $(OBJDIR)/proto_tdd_v3_emetteur.o
OBJ_TDD3_R = $(OBJDIR)/proto_tdd_v3_recepteur.o

OBJ_TDD4_S = $(OBJDIR)/proto_tdd_v4_emetteur.o
OBJ_TDD4_R = $(OBJDIR)/proto_tdd_v4_recepteur.o

# Disable automatic deletion of all intermediates
# (in the sub-make process, when calling make $(SENDER) $(RECEIVER))
.SECONDARY:

# -----------------------------------------------------------
# Top-level targets. Call e.g. `make tdd0` or `make tdd3`
# -----------------------------------------------------------
tdd%: $(OBJ_COMMON) $(OBJ_APP_NC)
	$(eval export OBJ_APP = $(OBJ_APP_NC))
	$(eval export OBJ_TDD_S = $(OBJ_TDD$*_S))
	$(eval export OBJ_TDD_R = $(OBJ_TDD$*_R))
	$(MAKE) $(SENDER) $(RECEIVER)

# Link rules
$(SENDER): $(OBJ_COMMON) $(OBJ_APP) $(OBJ_TDD_S) | $(BINDIR)
	$(CC) -o $(SENDER) $(OBJ_COMMON) $(OBJ_APP) $(OBJ_TDD_S) $(LDLIBS)

$(RECEIVER): $(OBJ_COMMON) $(OBJ_APP) $(OBJ_TDD_R) | $(BINDIR)
	$(CC) -o $(RECEIVER) $(OBJ_COMMON) $(OBJ_APP) $(OBJ_TDD_R) $(LDLIBS)

# Compile rules
# '%' matches filename
# $@  for the pattern-matched target
# $<  for the pattern-matched dependency
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) -o $@ -c $< $(CFLAGS)

$(OBJDIR) $(BINDIR):
	@mkdir -p $@

# Housekeeping
clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(BINDIR)/* $(FILESDIR)/out.*

deliver:
	mkdir $(NOM_ETU)
	cp -r src $(NOM_ETU)
	cp README.txt $(NOM_ETU)
	zip -r $(NOM_ETU).zip $(NOM_ETU)
	rm -rf $(NOM_ETU)
	@echo " "
	@echo "=================================================="
	@echo "Vous devez maintenant déposer l'archive $(NOM_ETU).zip sur Moodle."
	@echo "=================================================="
