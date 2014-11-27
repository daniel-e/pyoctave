CXX=`python-config --includes`
LD=`python-config --libs`
all:
	mkoctfile -v runpy.cc $(CXX) $(LD)

clean:
	rm -f runpy.o runpy.oct pyexample.pyc
