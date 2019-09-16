extern crate wifiscanner;
extern crate subprocess;

use std::fs::File;
use std::io::Read;
use std::process;
use std::string::String;

fn read_attribute(attr: &str) -> Result<String, String> {
    let mut data = String::new();
    match File::open(format!("/sys/class/net/wlan0/{0}", attr)) {
        Err(e) => Err(format!("Unable to open file: {0}", e)),
        Ok(ref mut file) => match file.read_to_string(&mut data).unwrap_or(0) {
            0 => Err("Unable to read file".to_owned()),
            _ => Ok(data.trim().to_owned()),
        },
    }
}
fn wpa_cli(command: &str) -> Result<bool, String> {
    let status = process::Command::new("wpa_cli")
        .arg(command)
        .status()
        .expect("Failed to run wpa_cli");
    if !status.success() {
        return Err(format!("Failed to run wpa_cli {}", command));
    }
    return Ok(true);
}
pub fn state() -> Result<String, String> {
    Ok(read_attribute("operstate")?)
}
pub fn online() -> Result<bool, String> {
    let ip = subprocess::Exec::shell("ip r | grep default | awk '{print $3}'")
        .stdout(subprocess::Redirection::Pipe)
        .capture()
        .unwrap()
        .stdout_str();
    if ip.is_empty() {
        return Err("No default found".to_string());
    }
    let success = subprocess::Exec::shell(format!("echo -n > /dev/tcp/{}/53", ip))
        .join()
        .unwrap()
        .success();
    return Ok(success);
}
pub fn scan() -> Result<Vec<wifiscanner::Wifi>, wifiscanner::Error> {
    return wifiscanner::scan();
}
pub fn disconnect() -> Result<bool, String> {
    return wpa_cli("disconnect");
}
pub fn reconnect() -> Result<bool, String> {
    return wpa_cli("reconnect");
}
pub fn restart() -> Result<bool, String> {
    match disable() {
        Ok(_) => {},
        Err(e) => {
            println!("Failed to disable {:?}", e);
        }
    };
    return enable();
}
pub fn disable() -> Result<bool, String> {
    let status = process::Command::new("ip")
        .args(&["link", "set", "dev", "wlan0", "down"])
        .status()
        .expect("Failed to disable wifi");
    if !status.success() {
        return Ok(false);
    }
    let status = process::Command::new("systemctl")
        .args(&["stop", "wpa_supplicant"])
        .status()
        .expect("Failed to stop wpa_supplicant");
    return Ok(status.success());
}
pub fn enable() -> Result<bool, String> {
    let status = process::Command::new("ip")
        .args(&["link", "set", "dev", "wlan0", "up"])
        .status()
        .expect("Failed to enable wifi");
    if !status.success() {
        return Ok(false);
    }
    let status = process::Command::new("systemctl")
        .args(&["start", "wpa_supplicant@wlan0"])
        .status()
        .expect("Failed to start wpa_supplicant service");
    return Ok(status.success());
}
pub fn new_connection(ssid: String, password: String) -> Result<bool, String> {
    let cmd = subprocess::Exec::cmd("wpa_passphrase").args(&[ssid, password]);
    subprocess::Exec::shell(format!(
            "{} >> /etc/wpa_supplicant/wpa_supplicant-wlan0.conf",
            cmd.to_cmdline_lossy()))
        .stdout(subprocess::Redirection::Pipe)
        .join()
        .expect("Failed to add new connection");
    return wpa_cli("reconfigure");
}
