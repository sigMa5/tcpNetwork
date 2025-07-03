# on Pi and Nano (Linux):
sudo apt update
sudo apt install -y build-essential cmake libboost-all-dev nlohmann-json3-dev
git clone … && cd async_router
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make             # builds both server (on host) or client (on device)
