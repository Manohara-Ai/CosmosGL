g++ src/blackHole.cpp src/rayEngine.cpp -o build/blackHole -Iinclude -lglad -ldl -lGL -lglfw

if lspci | grep -iq "nvidia"; then
    echo "Discrete GPU detected. Launching with PRIME offload..."
    export __NV_PRIME_RENDER_OFFLOAD=1
    export __GLX_VENDOR_LIBRARY_NAME=nvidia
else
    echo "No Discrete GPU found. Using default graphics..."
fi

./build/blackHole