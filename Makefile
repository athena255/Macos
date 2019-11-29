CXX=g++
TEST_DIR = tests/
MACH_DUMP_DIR = machdump/
MACH_LOAD_DIR = machload/
CFLAGS = -I.

mach: $(MACH_DUMP_DIR)machdump.cpp client.cpp
	$(CXX) -DVERBOSE -o client $(MACH_DUMP_DIR)machdump.cpp client.cpp $(CFLAGS)

machdumptest: $(MACH_DUMP_DIR)machdump.cpp $(MACH_DUMP_DIR)$(TEST_DIR)test_machdump.cpp
	$(CXX) -o a.out $(MACH_DUMP_DIR)machdump.cpp $(MACH_DUMP_DIR)$(TEST_DIR)test_machdump.cpp $(CFLAGS)

machloadertest: machdump.cpp machloader.cpp $(TEST_DIR)test_machloader.cpp
	$(CXX) -o a.out machdump.cpp machloader.cpp $(TEST_DIR)test_machloader.cpp $(CFLAGS)

.PHONY: clean
clean:
	rm -f a.out client
