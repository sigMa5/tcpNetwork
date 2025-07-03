# on Windows 11 (Host):
# - install Boost.Asio & nlohmann_json via vcpkg
# - open Developer PowerShell:
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
