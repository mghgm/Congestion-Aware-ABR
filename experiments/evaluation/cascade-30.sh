sleep 30 && \
date +"%H:%M:%S.%3N" && iperf -c 10.0.4.4 -u -l 1500 -b 3000K -t 10 -i 1 && \
sleep 30 && \
date +"%H:%M:%S.%3N" && iperf -c 10.0.4.4 -u -l 1500 -b 3600K -t 10 -i 1 && \
sleep 30 && \
date +"%H:%M:%S.%3N" && iperf -c 10.0.4.4 -u -l 1500 -b 4200K -t 10 -i 1 && \
sleep 30 && \
date +"%H:%M:%S.%3N" && iperf -c 10.0.4.4 -u -l 1500 -b 4800K -t 10 -i 1 && \
sleep 30 && \
date +"%H:%M:%S.%3N" && iperf -c 10.0.4.4 -u -l 1500 -b 5400K -t 10 -i 1 && \
sleep 30 && \
date +"%H:%M:%S.%3N" && iperf -c 10.0.4.4 -u -l 1500 -b 6000K -t 10 -i 1
