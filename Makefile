
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

test3: spider
	./$< "xs4xs7xs9xs3xs6 s9" "xsjxs4xs8xs3xs7 s5" "xs5xs3xs6xskxs5 s6" \
	     "xs0xskxs6xs0xsj sk" "xsjxskxsqxs0 s7"    "xsaxs9xs0xsa s4" \
	     "xs2xs5xs0xs3 sq"    "xs7xs2xs0xsj s9"    "xs8xs6xs3xs8 s8" \
	     "xs9xs3xs9xs2 sa" "-"  \
	     "sasqs9s0sas9sjs3sjsq" \
	     "sqs6sks7s5s7s7s0s5s5" \
	     "s8sqs4sks2s1sjs2s6s8" \
	     "s4sas2s4s6s8s2sks4s8" \
	     "sjsks7s5s3sqsqs4sas2"

ut: spider_test
	./$< --test

clean:
	rm -f *.exe
	rm -f *.stackdump
