extern crate wifiscanner;
extern crate wpactrl;

use std::fs::File;
use std::io::BufReader;
use std::io::BufRead;
use std::io::Read;
use std::io::Seek;
use std::io::SeekFrom;
use std::io::Write;
use std::process;
use std::path::Path;

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
pub fn state() -> Result<String, String> {
    Ok(read_attribute("operstate")?)
}
pub fn scan() -> Result<Vec<wifiscanner::Wifi>, wifiscanner::Error> {
    return wifiscanner::scan();
}
pub fn disconnect() {
    let mut wpa = wpactrl::WpaCtrl::new().open().unwrap();
    wpa.request("disconnect");
}
pub fn reconnect() {
    let mut wpa = wpactrl::WpaCtrl::new().open().unwrap();
    wpa.request("reconnect");
}
pub fn restart() {
    disable();
    enable();
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
        .args(&["start", "wpa_supplicant"])
        .status()
        .expect("Failed to start wpa_supplicant");
    return Ok(status.success());
}
