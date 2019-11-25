CXX=g++
TEST_DIR = tests/
CFLAGS = -I.

mach: machdump.cpp client.cpp
	$(CXX) -DVERBOSE -o client machdump.cpp client.cpp $(CFLAGS)

machdumptest: machdump.cpp $(TEST_DIR)test_machdump.cpp
	$(CXX) -o a.out machdump.cpp $(TEST_DIR)test_machdump.cpp $(CFLAGS)

machloadertest: machdump.cpp machloader.cpp $(TEST_DIR)test_machloader.cpp
	$(CXX) -o a.out machdump.cpp machloader.cpp $(TEST_DIR)test_machloader.cpp $(CFLAGS)
