cd ~/GameFrame

#in go
go build -o gameframe gameframe.go
sudo mv gameframe /usr/local/bin/

#in sh
sudo chmod +x gameframe_isolate.sh
sudo mv gameframe_isolate.sh /usr/local/bin/

#in cargo
cargo new gameframe_launcher
cd gameframe_launcher
# Replace src/main.rs with the above code
cargo build --release
sudo mv target/release/gameframe_launcher /usr/local/bin/

#in c++
g++ -o gameframe_integrator gameframe_integrator.cpp -std=c++17 -lstdc++fs
sudo mv gameframe_integrator /usr/local/bin/

#in cargo for render
cargo new gameframe_render
cd gameframe_render
# Add dependencies to Cargo.toml
echo '
[dependencies]
ash = "0.37"
winit = "0.29"
glutin = "0.30"
' >> Cargo.toml
# Replace src/main.rs with the above code
cargo build --release
sudo mv target/release/gameframe_render /usr/local/bin/

#installation for apt
sudo apt install vulkan-tools libvulkan-dev mesa-vulkan-drivers lib32-mesa-vulkan-drivers mangohud vkbasalt proton wine lm-sensors libgl1-mesa-dev libx11-dev

#  how to run ?
#  
#  gameframe wine supertuxkart.exe
#  gameframe my-app.py
