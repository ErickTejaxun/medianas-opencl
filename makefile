
olc: MedianasMultiplos_ocl.cpp
	g++ MedianasMultiplos_ocl.cpp -o multiplosOCL -lOpenCL

secuencial: MedianasMultiplos_sec.cpp
	g++ MedianasMultiplos_sec.cpp -o multiplosSecuencial

clean:
	rm multiplosSecuencial multiplosOCL
