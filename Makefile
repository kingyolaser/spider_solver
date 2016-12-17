
spider: spider.cpp
	g++ -Wall -O3 -o $@ $<

spider_test: spider.cpp
	g++ -Wall -DTEST -o $@ $< -lcppunit

test: spider
	./$< "xskxsqxsj s0" " s1" " s2" " s3" " s4" " s5" " s6" " s7" " s8" " s9"  -

ut: spider_test
	./$< --test

clean:
	rm -f *.exe
	rm -f *.stackdump
