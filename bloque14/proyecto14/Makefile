SERVICES = minicloud-registry minicloud-vault minicloud-runner minicloud-monitor minicloud-gateway

.PHONY: all clean test run

all:
	@for s in $(SERVICES); do \
		echo "=== build $$s ==="; \
		$(MAKE) -C $$s all || exit 1; \
	done

clean:
	@for s in $(SERVICES); do \
		echo "=== clean $$s ==="; \
		$(MAKE) -C $$s clean || exit 1; \
	done

run: all
	@echo "Inicia servicios con --serve <port> ... y usa gateway --request"

test: all
	@for s in $(SERVICES); do \
		echo "=== test $$s ==="; \
		$(MAKE) -C $$s test || exit 1; \
	done
	@echo "=== test integration ==="
	@bash tests/test_stack.sh
	@echo "=== Proyecto 14 OK ==="
