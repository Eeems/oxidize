extern crate wifiscanner;
use wifiscanner;

extern crate wpactrl;
use wpactrl::WpaCtrl;

use std::fs::File;
use std::io::BufReader;
use std::io::BufRead;
use std::io::Read;
use std::io::Seek;
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
pub fn init() {
    if Path::new("/lib/systemd/system/wpa_supplicant.service").exists() {
        let mut file = File::open("/lib/systemd/system/wpa_supplicant.service").unwrap();
        let reader = BufReader::new(&file);
        let mut newtext = "";
        let mut changed = false;
        for (num, line) in reader.lines().enumerate() {
            let text: String = line.unwrap().chars().collect();
            if text == "ExecStart=/usr/sbin/wpa_supplicant -u\n" {
                changed = true;
                newtext = "ExecStart=/usr/sbin/wpa_supplicant -u -O /var/run/wpa_supplicant\n";
            } else{
                newtext += text;
            }
        }
        if changed {
            file.seek(SeekFrom::Start(0))?;
            file.write_all(newtext.as_bytes());
        }
    }
}
pub fn state() -> Result<String, String> {
    Ok(read_attribute("operstate")?)
}
pub fn scan() -> Result<Vec<Wifi>, Error> {
    return wifiscanner::scan();
}
pub fn disconnect() -> Result<String> {
    let wpa = WpaCtrl::new().open().unwrap();
    return wpa.request("disconnect").unwrap();
}
pub fn reconnect() -> Result<String> {
    let wpa = WpaCtrl::new().open().unwrap();
    return wpa.request("reconnect").unwrap();
}
pub fn disable() -> Result<bool> {
    let status = process::Command::new("ip")
        .arg(&["link", "set", "dev", "wlan0", "down"])
        .status()
        .expect("Failed to disable wifi");
    return Ok(status.success());
}
pub fn enable() -> Result<bool> {
    let status = process::Command::new("ip")
        .arg(&["link", "set", "dev", "wlan0", "up"])
        .status()
        .expect("Failed to enable wifi");
    return Ok(status.success());
}
