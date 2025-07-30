use std::env;
use std::process::Command;
use std::fs;
use std::io;

fn detect_hardware() -> (String, bool, String) {
    // Check GPU vendor
    let vendor = env::var("GAMEFRAME_GPU").unwrap_or("unknown".to_string());
    
    // Check Vulkan support
    let vulkan = Command::new("vulkaninfo")
        .arg("--summary")
        .output()
        .is_ok();
    
    // Check OpenGL version
    let opengl = Command::new("glxinfo")
        .output()
        .map(|o| String::from_utf8_lossy(&o.stdout))
        .unwrap_or_default();
    let opengl_version = opengl
        .lines()
        .find(|line| line.contains("OpenGL version"))
        .map(|line| line.split(":").last().unwrap_or("unknown").trim().to_string())
        .unwrap_or("unknown".to_string());

    (vendor, vulkan, opengl_version)
}

fn adjust_fps(temp: u32) -> u32 {
    // Predictive FPS adjustment based on temperature
    match temp {
        t if t > 85 => 30,
        t if t > 70 => 60,
        t if t > 50 => 120,
        _ => 144,
    }
}

fn main() -> io::Result<()> {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        eprintln!("Usage: {} <command> [args...]", args[0]);
        std::process::exit(1);
    }

    // Detect hardware
    let (vendor, vulkan, opengl_version) = detect_hardware();
    println!("GPU: {}, Vulkan: {}, OpenGL: {}", vendor, vulkan, opengl_version);

    // Get temperature
    let temp = env::var("GAMEFRAME_TEMP")
        .unwrap_or("70".to_string())
        .parse::<u32>()
        .unwrap_or(70);
    let fps_limit = adjust_fps(temp);

    // Set graphics API
    let graphics_api = if vulkan { "vulkan" } else { "opengl" };
    
    // Check for XWayland
    let xwayland = Command::new("xwayland")
        .arg("-version")
        .output()
        .is_ok();
    if xwayland {
        env::set_var("GAMEFRAME_XWAYLAND", "1");
    }

    // Apply environment settings
    env::set_var("GAMEFRAME_API", graphics_api);
    env::set_var("GAMEFRAME_FPS", fps_limit.to_string());
    env::set_var("GAMEFRAME_VSYNC", if fps_limit <= 60 { "1" } else { "0" });

    // Launch the integrator
    let status = Command::new("/usr/local/bin/gameframe_integrator")
        .args(&args[1..])
        .status()?;

    if !status.success() {
        eprintln!("Game launch failed with status: {}", status);
        std::process::exit(1);
    }

    Ok(())
}
