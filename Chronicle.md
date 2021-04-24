## Valgrind use info
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --run-libc-freeres=yes --log-file=./valgrind_report.log -s  可执行程序名