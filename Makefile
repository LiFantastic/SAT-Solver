default:
	cd ./primitives; rm *.a; make copy;
	cd ./sat_solver; make clean; make copy; ./sat -c ../extra_data/f0010-01-s.cnf > result
	cd ./c2D_code; make copy;# ./bin/darwin/c2D -c ../test_use/data/f0010-01-s.cnf -C -E > result