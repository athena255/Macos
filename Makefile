CXX=g++
TEST_DIR = tests/
MACH_DUMP_DIR = machdump/
MACH_LOAD_DIR = machload/
MACH_EDIT_DIR = machedit/
CFLAGS = -std=c++14 -I.

mach: $(MACH_DUMP_DIR)machdump.cpp client.cpp
	$(CXX) -DVERBOSE -o client $(MACH_DUMP_DIR)machdump.cpp client.cpp $(CFLAGS)

machdumptest: $(MACH_DUMP_DIR)machdump.cpp $(MACH_DUMP_DIR)$(TEST_DIR)test_machdump.cpp
	$(CXX) -o a.out $(MACH_DUMP_DIR)machdump.cpp $(MACH_DUMP_DIR)$(TEST_DIR)test_machdump.cpp $(CFLAGS)

machloadertest: machdump.cpp machloader.cpp $(TEST_DIR)test_machloader.cpp
	$(CXX) -o a.out machdump.cpp machloader.cpp $(TEST_DIR)test_machloader.cpp $(CFLAGS)

machedittest: $(MACH_DUMP_DIR)machdump.cpp $(MACH_EDIT_DIR)machedit.cpp $(MACH_EDIT_DIR)$(TEST_DIR)test_machedit.cpp
	$(CXX) -o a.out $(MACH_DUMP_DIR)machdump.cpp $(MACH_EDIT_DIR)machedit.cpp $(MACH_EDIT_DIR)$(TEST_DIR)test_machedit.cpp $(CFLAGS)

.PHONY: clean
clean:
	rm -f a.out client newfile changedfile
