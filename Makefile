CXX=g++
TEST_DIR = tests/
MACH_DUMP_DIR = machdump/
MACH_LOAD_DIR = machload/
MACH_EDIT_DIR = machedit/
MAC_FNMAP_DIR = machFnMap/
CFLAGS = -std=c++14 -I. -glldb

mach: $(MACH_DUMP_DIR)machdump.cpp client.cpp
	$(CXX) -DVERBOSE -o client $(MACH_DUMP_DIR)machdump.cpp client.cpp $(CFLAGS)

machdumptest: $(MACH_DUMP_DIR)machdump.cpp $(MACH_DUMP_DIR)$(TEST_DIR)test_machdump.cpp
	$(CXX) -o a.out $(MACH_DUMP_DIR)machdump.cpp $(MACH_DUMP_DIR)$(TEST_DIR)test_machdump.cpp $(CFLAGS)

machloadertest: machdump.cpp machloader.cpp $(TEST_DIR)test_machloader.cpp
	$(CXX) -o a.out machdump.cpp machloader.cpp $(TEST_DIR)test_machloader.cpp $(CFLAGS)

machedittest: $(MACH_DUMP_DIR)machdump.cpp $(MACH_EDIT_DIR)machedit.cpp $(MACH_EDIT_DIR)$(TEST_DIR)test_machedit.cpp
	$(CXX) -o a.out $(MACH_DUMP_DIR)machdump.cpp $(MACH_EDIT_DIR)machedit.cpp $(MACH_EDIT_DIR)$(TEST_DIR)test_machedit.cpp $(CFLAGS)

machfnmaptest: machFnMap/machfnmap.cpp  machFnMap/tests/test_machfnmap.cpp
	$(CXX) -o a.out machFnMap/machfnmap.cpp machFnMap/tests/test_machfnmap.cpp $(CFLAGS)

basicdylib: testVectors/basicDylib/basicdylib.cpp testVectors/basicDylib/basicdylib2.cpp
	# $(CXX) -dynamiclib testVectors/basicDylib/basicdylib.cpp -install_name -o basic.dylib 
	clang -dynamiclib testVectors/basicDylib/basicdylib.cpp -current_version 1.0 -compatibility_version 1.0 -o testVectors/basicDylib/basic.dylib
	clang -dynamiclib testVectors/basicDylib/basicdylib2.cpp -current_version 1.0 -compatibility_version 1.0 -o testVectors/basicDylib/basic2.dylib
.PHONY: clean
clean:
	rm -f a.out client newfile changedfile *.test basic.dylib
