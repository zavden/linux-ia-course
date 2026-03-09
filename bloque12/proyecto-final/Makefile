SERVICES = minicloud-registry minicloud-gateway minicloud-monitor minicloud-vault minicloud-runner

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
	@for s in $(SERVICES); do \
		echo "=== run $$s ==="; \
		$(MAKE) -C $$s run || exit 1; \
	done

test: all
	@for s in $(SERVICES); do \
		echo "=== test $$s ==="; \
		$(MAKE) -C $$s test || exit 1; \
	done
	@echo "=== test integration ==="
	@bash tests/test_stack.sh
	@echo "=== Proyecto final OK ==="
