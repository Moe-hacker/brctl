all:
	$(CC) -std=gnu99 -c -o libk2v.o src/k2v.c
	ar -r libk2v.a libk2v.o
	rm libk2v.o
	$(CC) -fPIE -static -Wl,--gc-sections -fstack-protector-all -fstack-clash-protection -mshstk  -D_FORTIFY_SOURCE=3 -Wno-unused-result -O2 -std=gnu99 -Wno-gnu-zero-variadic-macro-arguments -L. -lk2v -o brctl src/brctl.c -z noexecstack -z now
	strip brctl
	rm libk2v.a
	cp brctl module/
	cd module&&zip -r ../brctl.zip .
format:
	clang-format -i src/*.c
check:
	clang-tidy --checks=*,-clang-analyzer-security.insecureAPI.strcpy,-altera-unroll-loops,-cert-err33-c,-concurrency-mt-unsafe,-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling,-readability-function-cognitive-complexity,-cppcoreguidelines-avoid-magic-numbers,-readability-magic-numbers,-bugprone-easily-swappable-parameters,-cert-err34-c,-misc-include-cleaner,-readability-identifier-length,-bugprone-signal-handler,-cert-msc54-cpp,-cert-sig30-c,-altera-id-dependent-backward-branch,-cppcoreguidelines-avoid-non-const-global-variables *.c --
