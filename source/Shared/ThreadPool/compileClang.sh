clear
clear
echo "====================================================="
clang -mcpu=cortex-m3 --std=c++11 -o out.out ./JsPromise.cpp ./ThreadPool.cpp -pthread
