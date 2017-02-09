
all:
	$(MAKE) -C echos
	$(MAKE) -C echodb
	$(MAKE) -C echol

.PHONY: clean
clean:
	$(MAKE) -C echos clean
	$(MAKE) -C echodb clean
	$(MAKE) -C echol clean
