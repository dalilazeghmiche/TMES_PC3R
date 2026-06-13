# Makefile pour le TME1 - Producteurs/Consommateurs
# PC3R - Sorbonne Université

# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -pthread -g
LDFLAGS = -pthread

# Nom de l'exécutable
TARGET = prodcons

# Fichiers sources
SRCS = prodcons.c

# Règle par défaut
all: $(TARGET)

# Compilation
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

# Exécution avec paramètres par défaut (3 prod, 2 cons, 10 items, capacité 5)
run: $(TARGET)
	./$(TARGET)

# Exécution avec beaucoup de production
run-stress: $(TARGET)
	./$(TARGET) 5 3 50 10

# Exécution avec petit buffer (pour tester la synchronisation)
run-small: $(TARGET)
	./$(TARGET) 3 2 20 3

# Nettoyage des fichiers compilés
clean:
	rm -f $(TARGET)

# Nettoyage complet
distclean: clean
	rm -f *~

# Test de compilation sans exécution
test: $(TARGET)
	@echo "Compilation réussie !"

# Afficher l'aide
help:
	@echo "Makefile pour Producteurs/Consommateurs"
	@echo ""
	@echo "Utilisation:"
	@echo "  make           - Compile le programme"
	@echo "  make run       - Compile et exécute avec paramètres par défaut"
	@echo "  make run-stress - Exécute avec beaucoup de production"
	@echo "  make run-small  - Exécute avec petit buffer"
	@echo "  make clean     - Supprime les fichiers compilés"
	@echo "  make help      - Affiche cette aide"
	@echo ""
	@echo "Paramètres du programme:"
	@echo "  ./prodcons [nb_prod] [nb_cons] [cible] [capacite]"
	@echo "  Exemple: ./prodcons 3 2 10 5"

.PHONY: all run run-stress run-small clean distclean test help