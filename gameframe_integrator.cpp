#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <filesystem>
#include <unistd.h>
#include <fstream>

class GameframeIntegrator {
public:
    GameframeIntegrator(const std::vector<std::string>& args) : args_(args) {}

    void configure() {
        // Environment variables
        const char* vkbasalt = std::getenv("GAMEFRAME_VKBASALT");
        const char* mangohud = std::getenv("GAMEFRAME_MANGOHUD");
        const char* api = std::getenv("GAMEFRAME_API");
        const char* fps = std::getenv("GAMEFRAME_FPS");
        const char* vsync = std::getenv("GAMEFRAME_VSYNC");
        const char* xwayland = std::getenv("GAMEFRAME_XWAYLAND");

        // Configure vkBasalt
        if (vkbasalt && std::string(vkbasalt) == "1") {
            setenv("ENABLE_VKBASALT", "1", 1);
            setenv("VKBASALT_CONFIG_FILE", "/home/user/.config/vkBasalt/vkBasalt.conf", 1);
            // Dynamic vkBasalt configuration
            std::ofstream config("/home/user/.config/vkBasalt/vkBasalt.conf");
            config << "effects = sharpen:cas\n";
            config << "casSharpness = 0.5\n";
            config.close();
        }

        // Configure MangoHud
        if (mangohud && std::string(mangohud) == "1") {
            setenv("MANGOHUD", "1", 1);
            std::string mangohud_config = "fps_limit=" + std::string(fps ? fps : "60") + ",cpu_stats,gpu_stats";
            setenv("MANGOHUD_CONFIG", mangohud_config.c_str(), 1);
        }

        // Configure Proton/Wine
        if (args_[0].find(".exe") != std::string::npos) {
            // Find latest Proton version
            std::string proton_path = "/home/user/.steam/steam/steamapps/common";
            std::string proton_bin = "proton";
            for (const auto& entry : std::filesystem::directory_iterator(proton_path)) {
                if (entry.path().filename().string().find("Proton") != std::string::npos) {
                    proton_bin = entry.path().string() + "/proton";
                    break;
                }
            }
            setenv("PROTON_ENABLE", "1", 1);
            setenv("PROTON_PATH", proton_bin.c_str(), 1);
            setenv("DXVK_HUD", "fps,frametimes", 1);
            setenv("DXVK_CONFIG_FILE", "/home/user/.config/dxvk.conf", 1);
            // Create DXVK config
            std::ofstream dxvk_config("/home/user/.config/dxvk.conf");
            dxvk_config << "dxvk.hud = fps,frametimes\n";
            dxvk_config << "dxvk.enableAsync = true\n";
            dxvk_config.close();
        }

        // Configure vsync
        if (vsync && std::string(vsync) == "1") {
            setenv("vblank_mode", "1", 1); // OpenGL vsync
            setenv("VK_PRESENT_MODE", "2", 1); // Vulkan FIFO vsync
        }

        // Configure XWayland
        if (xwayland && std::string(xwayland) == "1") {
            setenv("DISPLAY", ":99", 1);
            setenv("XDG_SESSION_TYPE", "wayland", 1);
        }
    }

    void launch() {
        std::vector<char*> c_args;
        if (std::getenv("PROTON_ENABLE")) {
            const char* proton_path = std::getenv("PROTON_PATH");
            c_args.push_back(const_cast<char*>(proton_path ? proton_path : "proton"));
            c_args.push_back(const_cast<char*>("run"));
        }
        for (const auto& arg : args_) {
            c_args.push_back(const_cast<char*>(arg.c_str()));
        }
        c_args.push_back(nullptr);

        execvp(c_args[0], c_args.data());
        std::cerr << "Failed to launch: " << args_[0] << std::endl;
        exit(1);
    }

private:
    std::vector<std::string> args_;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <command> [args...]" << std::endl;
        return 1;
    }

    std::vector<std::string> args(argv + 1, argv + argc);
    GameframeIntegrator integrator(args);
    integrator.configure();
    integrator.launch();
    return 0;
}
