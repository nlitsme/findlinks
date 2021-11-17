CMAKEARGS+=$(if $(D),-DCMAKE_BUILD_TYPE=Debug,-DCMAKE_BUILD_TYPE=Release)
CMAKEARGS+=$(if $(COV),-DOPT_COV=1)
CMAKEARGS+=$(if $(PROF),-DOPT_PROF=1)
CMAKEARGS+=$(if $(LIBCXX),-DOPT_LIBCXX=1)

all:
	cmake -B build . $(CMAKEARGS)
	$(MAKE) -C build $(if $(V),VERBOSE=1)

llvm:
	CC=clang CXX=clang++ cmake -B build . $(CMAKEARGS)
	$(MAKE) -C build $(if $(V),VERBOSE=1)

clean:
	$(RM) -r build CMakeFiles CMakeCache.txt CMakeOutput.log

