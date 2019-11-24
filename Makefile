CXX=g++
TEST_DIR = tests/
CFLAGS = -I.

machdemo: machdump.cpp $(TEST_DIR)test_machdump.cpp
	$(CXX) -DVERBOSE -o a.out machdump.cpp $(TEST_DIR)test_machdump.cpp $(CFLAGS)

machdumptest: machdump.cpp $(TEST_DIR)test_machdump.cpp
	$(CXX) -o a.out machdump.cpp $(TEST_DIR)test_machdump.cpp $(CFLAGS)

machloadertest: machdump.cpp machloader.cpp $(TEST_DIR)test_machloader.cpp
	$(CXX) -o a.out machdump.cpp machloader.cpp $(TEST_DIR)test_machloader.cpp $(CFLAGS)
