# net

g++ server-pool.cpp -o server
g++ client.cpp -o client
sudo apt-get install apache2-utils  安装 ab 测试程序

编译测试程序
g++ server-pool-ab.cpp -o server-ab
./server-ab

测试
```
ubuntu2004@DESKTOP-OEAU8BF:~$ ab -n 1 -t 10 -c 1  http://127.0.0.1:12345/index.html
This is ApacheBench, Version 2.3 <$Revision: 1843412 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient)
apr_socket_recv: Connection refused (111)
ubuntu2004@DESKTOP-OEAU8BF:~$ ab -n 1 -t 10 -c 1  http://127.0.0.1:12345/index.html
This is ApacheBench, Version 2.3 <$Revision: 1843412 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient)
Completed 5000 requests
Completed 10000 requests
Completed 15000 requests
Completed 20000 requests
Completed 25000 requests
Completed 30000 requests
Completed 35000 requests
Completed 40000 requests
Completed 45000 requests
Completed 50000 requests
Finished 50000 requests


Server Software:
Server Hostname:        127.0.0.1
Server Port:            12345

Document Path:          /index.html
Document Length:        102 bytes

Concurrency Level:      1
Time taken for tests:   7.235 seconds
Complete requests:      50000
Failed requests:        0
Total transferred:      10000000 bytes
HTML transferred:       5100000 bytes
Requests per second:    6911.21 [#/sec] (mean)
Time per request:       0.145 [ms] (mean)
Time per request:       0.145 [ms] (mean, across all concurrent requests)
Transfer rate:          1349.85 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.0      0       0
Processing:     0    0   0.0      0       1
Waiting:        0    0   0.0      0       0
Total:          0    0   0.0      0       1

Percentage of the requests served within a certain time (ms)
  50%      0
  66%      0
  75%      0
  80%      0
  90%      0
  95%      0
  98%      0
  99%      0
 100%      1 (longest request)
```