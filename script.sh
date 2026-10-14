# 1. Download the matching .deb package for your architecture
ARCH=$(dpkg --print-architecture)
wget https://github.com/WiringPi/WiringPi/releases/latest/download/wiringpi_${ARCH}.deb

# 2. Install the package
sudo apt install -y ./wiringpi_${ARCH}.deb

# 3. Verify installation
gpio -v