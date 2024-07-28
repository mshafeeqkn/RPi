# Define the subdirectories
SUBDIRS := app linux-6.6.31 STM32-Firmware 

# Default target to build all subdirectories
.PHONY: all
all: $(SUBDIRS)

# Rule to build each subdirectory
.PHONY: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

# Clean all subdirectories
.PHONY: clean
clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

# Phony target to ensure make doesn't get confused with directories/files of the same name
.PHONY: clean-all
clean-all: clean

