This is a guide created by me to help change Duke VM kernel to Ubuntu 20.04 104 generic version, used for this rootkit project.



- reserve VM on Duke VM webpage
- `sudo apt-get upgrade linux-image-unsigned-5.4.0-104-generic`
- `sudo nano /etc/default/grub`, then change first line to `GRUB_DEFAULT="1>2"`
- `sudo update-grub`
- `sudo reboot`
- `uname -r` should be good to show version 104 now
- `sudo apt-get install linux-headers-$(uname -r)` otherwise `make` will go wrong
- `sudo apt-get install -y gcc g++ make valgrind emacs build-essential`