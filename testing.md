# Single threaded non blocking sockets (poll() based) [Default run on single core cpu]:

## ./wrk -t4 -c100 -d30s --latency http://localhost:8080/

Running 30s test @ http://localhost:8080/
4 threads and 100 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 20.10ms 51.16ms 1.79s 99.68%
Req/Sec 181.25 197.20 2.58k 93.33%
Latency Distribution
50% 17.99ms
75% 26.18ms
90% 30.75ms
99% 41.44ms
21679 requests in 30.04s, 23.76MB read
Socket errors: connect 0, read 21686, write 0, timeout 6
Requests/sec: 721.74
Transfer/sec: 809.85KB
zed@ze-main:~/Data/Projects/Self/HttpServerCpp/temp$

## ./wrk -t4 -c100 -d30s --latency http://localhost:8080/ (Single Core)

Running 30s test @ http://localhost:8080/
4 threads and 100 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 18.66ms 28.87ms 1.66s 99.85%
Req/Sec 198.25 196.43 2.38k 92.22%
Latency Distribution
50% 17.42ms
75% 26.67ms
90% 30.95ms
99% 40.72ms
21848 requests in 30.07s, 23.94MB read
Socket errors: connect 0, read 21851, write 0, timeout 3
Requests/sec: 726.52
Transfer/sec: 815.21KB

## ./wrk -t16 -c10000 -d120s --latency http://localhost:8080/

Running 2m test @ http://localhost:8080/
16 threads and 10000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 70.72ms 88.56ms 1.99s 99.17%
Req/Sec 23.72 13.84 130.00 77.47%
Latency Distribution
50% 63.13ms
75% 72.90ms
90% 78.43ms
99% 114.81ms
24136 requests in 2.00m, 26.45MB read
Socket errors: connect 0, read 26707, write 0, timeout 91
Requests/sec: 200.97
Transfer/sec: 225.50KB

## ./wrk -t16 -c10000 -d120s --latency http://localhost:8080/ (Single Core)

Running 2m test @ http://localhost:8080/
16 threads and 10000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 67.31ms 85.23ms 1.97s 99.17%
Req/Sec 24.45 14.29 131.00 52.92%
Latency Distribution
50% 59.64ms
75% 71.78ms
90% 79.57ms
99% 116.46ms
25392 requests in 2.00m, 27.82MB read
Socket errors: connect 0, read 27656, write 0, timeout 96
Requests/sec: 211.45
Transfer/sec: 237.26KB

## ./wrk -t50 -c10000 -d120s --latency http://localhost:8080/

Running 2m test @ http://localhost:8080/
50 threads and 10000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 40.51ms 72.63ms 1.92s 99.31%
Req/Sec 30.75 76.70 7.97k 98.02%
Latency Distribution
50% 37.80ms
75% 52.08ms
90% 63.78ms
99% 80.81ms
43428 requests in 2.00m, 47.59MB read
Socket errors: connect 0, read 47557, write 0, timeout 123
Requests/sec: 361.62
Transfer/sec: 405.76KB

## ./wrk -t50 -c10000 -d120s --latency http://localhost:8080/ (Single Core)

Running 2m test @ http://localhost:8080/
50 threads and 10000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 100.95ms 99.40ms 1.95s 98.60%
Req/Sec 13.40 7.58 90.00 87.66%
Latency Distribution
50% 90.01ms
75% 98.12ms
90% 107.30ms
99% 311.27ms
16948 requests in 2.00m, 18.57MB read
Socket errors: connect 0, read 16974, write 0, timeout 92
Requests/sec: 141.12
Transfer/sec: 158.34KB

## ./wrk -t1 -c10000 -d120s --latency http://localhost:8080/

Running 2m test @ http://localhost:8080/
1 threads and 10000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 90.34ms 90.78ms 1.98s 99.00%
Req/Sec 156.41 25.66 770.00 86.20%
Latency Distribution
50% 83.18ms
75% 90.77ms
90% 96.23ms
99% 236.90ms
18667 requests in 2.00m, 20.45MB read
Socket errors: connect 0, read 19975, write 0, timeout 116
Requests/sec: 155.47
Transfer/sec: 174.44KB

## ./wrk -t1 -c10000 -d120s --latency http://localhost:8080/ (Single Core)

Running 2m test @ http://localhost:8080/
1 threads and 10000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 144.61ms 190.24ms 1.97s 96.25%
Req/Sec 121.94 18.77 570.00 82.19%
Latency Distribution
50% 108.72ms
75% 113.21ms
90% 128.88ms
99% 1.23s
14539 requests in 2.00m, 15.93MB read
Socket errors: connect 0, read 16431, write 0, timeout 320
Requests/sec: 121.11
Transfer/sec: 135.90KB

## ./wrk -t12 -c100000 -d120s --latency http://localhost:8080/

Running 2m test @ http://localhost:8080/
12 threads and 100000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 125.84ms 132.16ms 1.94s 95.62%
Req/Sec 17.42 20.14 439.00 92.67%
Latency Distribution
50% 110.37ms
75% 117.11ms
90% 123.13ms
99% 811.92ms
10364 requests in 2.00m, 11.36MB read
Socket errors: connect 71904, read 25806, write 0, timeout 125
Requests/sec: 86.29
Transfer/sec: 96.82KB

## ./wrk -t12 -c100000 -d120s --latency http://localhost:8080/ (Single Core)

Running 2m test @ http://localhost:8080/
12 threads and 100000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 175.01ms 215.75ms 2.00s 94.94%
Req/Sec 15.14 24.17 707.00 96.92%
Latency Distribution
50% 125.81ms
75% 134.29ms
90% 337.15ms
99% 1.28s
9047 requests in 2.00m, 9.91MB read
Socket errors: connect 72783, read 20777, write 0, timeout 347
Requests/sec: 75.30
Transfer/sec: 84.49KB

## ./wrk -t4 -c100000 -d120s --latency http://localhost:8080/

Running 2m test @ http://localhost:8080/
4 threads and 100000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 200.93ms 214.43ms 2.00s 89.01%
Req/Sec 32.54 71.42 1.60k 98.90%
Latency Distribution
50% 136.27ms
75% 147.54ms
90% 537.41ms
99% 1.14s
6803 requests in 2.00m, 7.45MB read
Socket errors: connect 76280, read 17935, write 0, timeout 117
Requests/sec: 56.63
Transfer/sec: 63.55KB

## ./wrk -t4 -c100000 -d120s --latency http://localhost:8080/ (Single Core)

Running 2m test @ http://localhost:8080/
4 threads and 100000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 248.26ms 270.42ms 1.99s 85.60%
Req/Sec 24.61 27.85 500.00 98.50%
Latency Distribution
50% 146.37ms
75% 168.42ms
90% 632.15ms
99% 1.49s
6186 requests in 2.01m, 6.78MB read
Socket errors: connect 71768, read 14262, write 0, timeout 208
Requests/sec: 51.38
Transfer/sec: 57.65KB

# Threaded server (thread per request) [Commit hash: e600ca6] [Default run on 12 core cpu]:

## ./wrk -t4 -c100 -d30s --latency http://localhost:8080/

Running 30s test @ http://localhost:8080/
4 threads and 100 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 3.64ms 27.64ms 824.43ms 97.81%
Req/Sec 5.60k 1.49k 9.43k 69.17%
Latency Distribution
50% 204.00us
75% 330.00us
90% 587.00us
99% 139.86ms
666887 requests in 30.02s, 85.86MB read
Socket errors: connect 0, read 666881, write 0, timeout 0
Requests/sec: 22212.59
Transfer/sec: 2.86MB

## ./wrk -t4 -c100 -d30s --latency http://localhost:8080/ (Single Core)

Running 30s test @ http://localhost:8080/
4 threads and 100 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 4.24ms 31.49ms 824.74ms 98.63%
Req/Sec 1.75k 0.95k 5.32k 69.55%
Latency Distribution
50% 1.35ms
75% 1.68ms
90% 2.18ms
99% 102.03ms
199448 requests in 30.02s, 25.68MB read
Socket errors: connect 0, read 199447, write 0, timeout 0
Requests/sec: 6642.79
Transfer/sec: 0.86MB

## ./wrk -t16 -c10000 -d120s --latency http://localhost:8080/

Running 2m test @ http://localhost:8080/
16 threads and 10000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 8.35ms 79.32ms 1.98s 98.38%
Req/Sec 1.48k 1.09k 7.41k 68.43%
Latency Distribution
50% 305.00us
75% 425.00us
90% 657.00us
99% 209.29ms
2763973 requests in 2.00m, 355.85MB read
Socket errors: connect 0, read 2767781, write 0, timeout 4096
Requests/sec: 23014.52
Transfer/sec: 2.96MB

## ./wrk -t16 -c10000 -d120s --latency http://localhost:8080/ (Single Core)

Running 2m test @ http://localhost:8080/
16 threads and 10000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 34.47ms 165.47ms 1.72s 94.40%
Req/Sec 574.50 656.63 4.64k 85.07%
Latency Distribution
50% 1.28ms
75% 1.66ms
90% 3.29ms
99% 843.60ms
27395 requests in 2.00m, 3.53MB read
Socket errors: connect 0, read 31807, write 18585734, timeout 30
Requests/sec: 228.02
Transfer/sec: 30.06KB

- Connection crashed after some time the test was run

## ./wrk -t50 -c10000 -d120s --latency http://localhost:8080/

Running 2m test @ http://localhost:8080/
50 threads and 10000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 8.61ms 81.35ms 1.99s 98.36%
Req/Sec 589.28 655.81 8.94k 85.76%
Latency Distribution
50% 327.00us
75% 447.00us
90% 682.00us
99% 209.48ms
2720965 requests in 2.00m, 350.31MB read
Socket errors: connect 0, read 2727236, write 0, timeout 3653
Requests/sec: 22655.91
Transfer/sec: 2.92MB

## ./wrk -t50 -c10000 -d120s --latency http://localhost:8080/ (Single Core)

Running 2m test @ http://localhost:8080/
50 threads and 10000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 20.07ms 124.09ms 1.99s 96.25%
Req/Sec 217.18 249.77 3.69k 86.04%
Latency Distribution
50% 1.27ms
75% 1.48ms
90% 2.25ms
99% 444.14ms
926014 requests in 2.00m, 119.22MB read
Socket errors: connect 0, read 927569, write 0, timeout 4544
Requests/sec: 7710.25
Transfer/sec: 0.99MB

- Connection never crashed

## ./wrk -t1 -c10000 -d120s --latency http://localhost:8080/

Running 2m test @ http://localhost:8080/
1 threads and 10000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 36.30ms 150.92ms 2.00s 95.16%
Req/Sec 8.44k 1.27k 16.45k 80.08%
Latency Distribution
50% 7.18ms
75% 12.70ms
90% 26.36ms
99% 845.24ms
1002727 requests in 2.00m, 129.10MB read
Socket errors: connect 0, read 1002692, write 0, timeout 10949
Requests/sec: 8354.35
Transfer/sec: 1.08MB

## ./wrk -t1 -c10000 -d120s --latency http://localhost:8080/ (Single Core)

Running 2m test @ http://localhost:8080/
1 threads and 10000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 24.86ms 138.54ms 1.98s 95.47%
Req/Sec 8.08k 738.48 14.68k 86.22%
Latency Distribution
50% 1.07ms
75% 1.38ms
90% 2.80ms
99% 823.45ms
958637 requests in 2.00m, 123.42MB read
Socket errors: connect 0, read 961538, write 0, timeout 7563
Requests/sec: 7987.14
Transfer/sec: 1.03MB

## ./wrk -t12 -c100000 -d120s --latency http://localhost:8080/

Running 2m test @ http://localhost:8080/
12 threads and 100000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 0.00us 0.00us 0.00us -nan%
Req/Sec 0.00 0.00 0.00 -nan%
Latency Distribution
50% 0.00us
75% 0.00us
90% 0.00us
99% 0.00us
0 requests in 2.01m, 0.00B read
Socket errors: connect 0, read 1983, write 20310907, timeout 0
Requests/sec: 0.00
Transfer/sec: 0.00B

- Server Crashed

## ./wrk -t12 -c100000 -d120s --latency http://localhost:8080/ (Single Core)

Running 2m test @ http://localhost:8080/
12 threads and 100000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 0.00us 0.00us 0.00us -nan%
Req/Sec 0.00 0.00 0.00 -nan%
Latency Distribution
50% 0.00us
75% 0.00us
90% 0.00us
99% 0.00us
0 requests in 2.00m, 0.00B read
Socket errors: connect 72793, read 320, write 0, timeout 0
Requests/sec: 0.00
Transfer/sec: 0.00B

- Crashed - terminate called after throwing an instance of 'std::system_error'

## ./wrk -t4 -c100000 -d120s --latency http://localhost:8080/

Running 2m test @ http://localhost:8080/
4 threads and 100000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 0.00us 0.00us 0.00us -nan%
Req/Sec 0.00 0.00 0.00 -nan%
Latency Distribution
50% 0.00us
75% 0.00us
90% 0.00us
99% 0.00us
0 requests in 2.00m, 0.00B read
Socket errors: connect 71768, read 9772, write 0, timeout 0
Requests/sec: 0.00
Transfer/sec: 0.00B

- Server said "Resource unavailable" terminal called, std::system_error

## ./wrk -t4 -c100000 -d120s --latency http://localhost:8080/ (Single Core)

Running 2m test @ http://localhost:8080/
4 threads and 100000 connections
Thread Stats Avg Stdev Max +/- Stdev
Latency 0.00us 0.00us 0.00us -nan%
Req/Sec 0.00 0.00 0.00 -nan%
Latency Distribution
50% 0.00us
75% 0.00us
90% 0.00us
99% 0.00us
0 requests in 2.00m, 0.00B read
Socket errors: connect 71768, read 11473, write 0, timeout 0
Requests/sec: 0.00
Transfer/sec: 0.00B

- terminate called after throwing an instance of 'std::system_error'
