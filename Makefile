
spider: spider.cpp
	g++ -Wall -O3 -o $@ $<

spider_test: spider.cpp
	g++ -Wall -DTEST -o $@ $< -lcppunit

test: spider
	./$< "xxxxsqxsj s0" " s1" " s2" " s3" " s4" " s5" " s6" " s7" " s8" " s9"  -

test2: spider
	./$< "xsqxs3xs1xs5 s2 s1 s2" "xs3xs4xs1xsq sk sq sj s0 s3" " s8 s3" \
	     "xs8 sk sq sj s0" " s6 s4" " sk sq sj s0 s9 s8 s7 s6 s5"       \
	     " s6 s9" " s4 s3 s2 sa sk" \
	     "xs1xs1 s7 s6 s5 s4 sj sk s7" \
	     "xs8xs7xs2 s0 s9 s8 s7 s6 s5 s4 sk sq sj s0 s9 s6 s5 s4"  - \
	     "s9s8s2sjs7s3s9s5s2s0"

ut: spider_test
	./$< --test

clean:
	rm -f *.exe
	rm -f *.stackdump
