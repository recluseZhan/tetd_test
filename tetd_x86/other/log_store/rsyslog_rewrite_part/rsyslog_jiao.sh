cd rsyslog
make clean
make
sudo make install
sudo cp /usr/local/lib/rsyslog/omstdout.so /usr/lib/x86_64-linux-gnu/rsyslog/

sudo systemctl restart rsyslog
sudo systemctl status rsyslog
