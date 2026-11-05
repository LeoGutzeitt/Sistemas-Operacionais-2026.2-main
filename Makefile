# Permite compilar a Implementacao 4 a partir da raiz do repositorio.
.PHONY: all test clean

all:
	$(MAKE) -C unidade2/lgbm all

test:
	$(MAKE) -C unidade2/lgbm test

clean:
	$(MAKE) -C unidade2/lgbm clean
