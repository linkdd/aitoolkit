DESTDIR := /usr/local

.PHONY: test
test:
	@make -C tests all

.PHONY: docs
docs:
	@make -C docs all

.PHONY: install
install:
	@cp -R include/ $(DESTDIR)
