.PHONY: build-win64
build-win64:
	cmake -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE=../CMake/platforms/mingwcc64.toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DDEVILUTIONX_SYSTEM_BZIP2=OFF -DDEVILUTIONX_STATIC_LIBSODIUM=ON -DBUILD_ASSETS_MPQ=ON
	cmake --build build -j $(getconf _NPROCESSORS_ONLN)