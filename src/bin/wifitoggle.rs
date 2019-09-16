extern crate oxidize;
use oxidize::wifi;

use std::env;

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() == 3 {
        wifi::enable().expect("Failed to enable wifi");
        println!("Wifi enabled");
        wifi::new_connection(args[1].to_string(), args[2].to_string())
            .expect("Failed to add new connection");
        println!("Connection added");
        wifi::reconnect().expect("Failed to reconnect to wifi");
        println!("Reconnection triggered");
    }else if wifi::state().unwrap() == "up" {
        wifi::disable().expect("Failed to disable wifi");
        println!("Wifi disabled");
    } else{
        wifi::enable().expect("Failed to enable wifi");
        println!("Wifi enabled");
        wifi::reconnect().expect("Failed to reconnect to wifi");
        println!("Wifi reconnected");
    }
}
