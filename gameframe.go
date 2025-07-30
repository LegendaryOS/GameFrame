package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

type HardwareInfo struct {
	GPUVendor string
	VulkanSupport bool
	OpenGLVersion string
}

func detectHardware() HardwareInfo {
	// Check GPU vendor via lspci
	cmd := exec.Command("lspci", "-v")
	output, _ := cmd.Output()
	vendor := "unknown"
	if strings.Contains(string(output), "NVIDIA") {
		vendor = "nvidia"
	} else if strings.Contains(string(output), "AMD") {
		vendor = "amd"
	} else if strings.Contains(string(output), "Intel") {
		vendor = "intel"
	}

	// Check Vulkan support
	vulkanSupport := false
	if _, err := exec.Command("vulkaninfo", "--summary").Output(); err == nil {
		vulkanSupport = true
	}

	// Check OpenGL version
	openglVersion := "unknown"
	if output, err := exec.Command("glxinfo").Output(); err == nil {
		for _, line := range strings.Split(string(output), "\n") {
			if strings.Contains(line, "OpenGL version") {
				openglVersion = strings.TrimSpace(strings.Split(line, ":")[1])
				break
			}
		}
	}

	return HardwareInfo{vendor, vulkanSupport, openglVersion}
}

func loadGameProfile(game string) map[string]string {
	// Load game-specific configuration (placeholder for profile system)
	profiles := map[string]map[string]string{
		"supertuxkart.exe": {
			"fps_limit": "60",
			"vsync": "1",
			"vkbasalt": "1",
			"mangohud": "1",
		},
	}
	return profiles[filepath.Base(game)]
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: gameframe <command> [args...]")
		fmt.Println("Example: gameframe wine supertuxkart.exe")
		os.Exit(1)
	}

	// Detect hardware
	hardware := detectHardware()
	fmt.Printf("Detected GPU: %s, Vulkan: %v, OpenGL: %s\n", hardware.GPUVendor, hardware.VulkanSupport, hardware.OpenGLVersion)

	// Extract command and arguments
	cmd := os.Args[1:]
	targetCmd := strings.Join(cmd, " ")

	// Load game profile
	profile := loadGameProfile(cmd[0])

	// Prepare environment isolation script
	isolationScript := "/usr/local/bin/gameframe_isolate.sh"
	if _, err := os.Stat(isolationScript); os.IsNotExist(err) {
		fmt.Println("Error: Isolation script not found at", isolationScript)
		os.Exit(1)
	}

	// Set environment variables based on profile and hardware
	env := append(os.Environ(),
		fmt.Sprintf("GAMEFRAME_GPU=%s", hardware.GPUVendor),
		fmt.Sprintf("GAMEFRAME_VULKAN=%v", hardware.VulkanSupport),
		fmt.Sprintf("GAMEFRAME_OPENGL=%s", hardware.OpenGLVersion),
		fmt.Sprintf("GAMEFRAME_FPS_LIMIT=%s", profile["fps_limit"]),
		fmt.Sprintf("GAMEFRAME_VSYNC=%s", profile["vsync"]),
		fmt.Sprintf("GAMEFRAME_VKBASALT=%s", profile["vkbasalt"]),
		fmt.Sprintf("GAMEFRAME_MANGOHUD=%s", profile["mangohud"]),
	)

	// Run isolation script
	isolateCmd := exec.Command("bash", isolationScript, targetCmd)
	isolateCmd.Stdout = os.Stdout
	isolateCmd.Stderr = os.Stderr
	isolateCmd.Env = env

	if err := isolateCmd.Run(); err != nil {
		fmt.Printf("Error running isolation script: %v\n", err)
		os.Exit(1)
	}
}
