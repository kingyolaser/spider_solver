
spider: spider.cpp
	g++ -Wall -O3 -o $@ $< -lcrypto

spider_test: spider.cpp
	g++ -Wall -DTEST -o $@ $< -lcrypto -lcppunit

test: spider
	./$< "xxxxsqxsj s0" " s1" " s2" " s3" " s4" " s5" " s6" " s7" " s8" " s9"

test2: spider
	./$< "xsqxs3xs1xs5 s2 s1 s2" "xs3xs4xs1xsq sk sq sj s0 s3" " s8 s3" \
	     "xs8 sk sq sj s0" " s6 s4" " sk sq sj s0 s9 s8 s7 s6 s5"       \
	     " s6 s9" " s4 s3 s2 sa sk" \
	     "xs1xs1 s7 s6 s5 s4 sj sk s7" \
	     "xs8xs7xs2 s0 s9 s8 s7 s6 s5 s4 sk sq sj s0 s9 s6 s5 s4" "_" \
	     "s9s8s2sjs7s3s9s5s2s0"

test3: spider
	./$< "xs4xs7xs9xs3xs6 s9" "xsjxs4xs8xs3xs7 s5" "xs5xs3xs6xskxs5 s6" \
	     "xs0xskxs6xs0xsj sk" "xsjxskxsqxs0 s7"    "xsaxs9xs0xsa s4" \
	     "xs2xs5xs0xs3 sq"    "xs7xs2xs0xsj s9"    "xs8xs6xs3xs8 s8" \
	     "xs9xs3xs9xs2 sa" "_"  \
	     "sasqs9s0sas9sjs3sjsq" \
	     "sqs6sks7s5s7s7s0s5s5" \
	     "s8sqs4sks2s1sjs2s6s8" \
	     "s4sas2s4s6s8s2sks4s8" \
	     "sjsks7s5s3sqsqs4sas2"

test4: spider #スタークラブ spider 上級II 最終問題
	time ./$< "xs9xhkxh0xs8xs5 s6" "xh6xh4xsjxh4xh1 h7" "xh5xh6xs7xh6xh8 h4" \
	     "xh3xh0xskxs9xs6 s0" "xs4xsjxs9xhq h0"    "xsaxs8xs8xs4 sk"    \
	     "xh9xhaxsjxh7 hq"    "xhjxh4xh2xs5 h2"    "xs2xs3xs7xsa h8"    \
	     "xh3xsqxsaxhj hj" "_"  \
	     "s4h0hkhks6s5s7h3h5h9" \
	     "h7s7s0has0h8s9s3s3s6" \
	     "h9hqsqhkh8s2s2h2s2h6" \
	     "s3s4sqhjsasqh2sjh3h5" \
	     "s5s0has8skhqskh5h7h9"
#上記解答例
#tesuu=182
#history: 49:18:17:94:96:08:08:18:31:45:04:06:41:50:65:36:30:53:03:draw:05:90:46:56:14:61:71:71:76:97:95:draw:05:64:50:61:90:draw:13:40:10:draw:01:10:68:46:47:43:42:60:75:96:79:74:46:79:76:87:98:14:81:78:draw:39:54:56:96:93:29:12:16:14:17:18:97:78:84:21:72:47:09:04:03:73:36:13:03:90:41:24:12:41:24:32:07:23:21:70:31:32:12:07:21:23:29:42:27:21:82:20:60:03:39:21:93:21:82:21:78:39:87:30:90:78:09:32:30:93:20:87:02:34:20:78:03:87:82:18:21:84:85:43:71:78:18:34:81:30:18:40:81:32:30:60:56:68:16:04:02:42:13:61:65:62:26:60:10:15:19:25:02:29:50:53:35:50:80:54:84:84:58:84:
#Conguraturation!! examined boards=507
#real    0m0.128s

test5: spider  #スタークラブ　トーナメント　FINAL
	./$< -r1 \
	     "xc2xcqxc3xs9xs4 hj" "xh5xckxdjxsaxs4 d0" "xcaxd3xs7xd8xs7 dj" \
	     "xh5xsjxc6xd4xdq h0" "xs5xh7xckxd0 hj"    "xd2xd5xhqxh9 c5"    \
	     "xxxxh3xdkxd9 c8"    "xc7xd5xhaxd7 c9"    "xxxxsqxd6xsq h6"    \
	     "xcaxdaxdqxh4 da" "_"  \
	     "skcqh7s8hkskd8s3s2d3" \
	     "d7s3sjh8c8c0h8hqd4c2" \
	     "h2c6s6h0h6c4c5h4dkhk" \
	     "s0s2s8c7s6sacjh3h9c0" \
	     "d2s0s9c3c4d9s5c9cjd6"

ut: spider_test
	./$< --test

clean:
	rm -f *.exe
	rm -f *.stackdump
