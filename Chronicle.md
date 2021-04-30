## Valgrind use info
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --run-libc-freeres=yes --log-file=./valgrind_report.log -s  可执行程序名

##LIB
1.将用户用到的库统一放到一个目录，如 /usr/loca/lib
# cp libXXX.so.X /usr/loca/lib/           

2.向库配置文件中，写入库文件所在目录
# vim /etc/ld.so.conf.d 
  /usr/local/lib  

3.更新/etc/ld.so.cache文件
# ldconfig  