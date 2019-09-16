#![feature(rustc_private)]

extern crate libremarkable;
use libremarkable::{
    appctx,
    battery,
};
use libremarkable::framebuffer::cgmath;
use libremarkable::framebuffer::common::color;
use libremarkable::input::{
    gpio,
    multitouch,
    wacom,
};
use libremarkable::ui_extensions::element::{
    UIElement,
    UIElementHandle,
    UIElementWrapper,
    UIConstraintRefresh,
};

#[cfg(feature = "enable-runtime-benchmarking")]
use libremarkable::stopwatch;

#[macro_use]
extern crate lazy_static;

// #[macro_use(c)]
// extern crate cute;

#[macro_use]
extern crate log;
extern crate env_logger;

extern crate chrono;
use chrono::{
    DateTime,
    Local,
};

extern crate gettext;
use gettext::Catalog;

extern crate rusttype;

use std::sync::atomic::{
    AtomicBool,
    Ordering,
};
use std::env;
use std::fs::File;
use std::io::Read;
use std::path::Path;
use std::string::String;
use std::thread;
use std::time::Duration;

extern crate oxidize;
use oxidize::wifi;

// extern crate hyphenation;
// use hyphenation::{Language, Load, Standard};

// extern crate cgroups_fs;
// use cgroups_fs

// extern crate textwrap;
// use textwrap::Wrapper;

lazy_static! {
    static ref WACOM_IN_RANGE: AtomicBool = AtomicBool::new(false);
}

fn on_wacom_input(_app: &mut appctx::ApplicationContext, input: wacom::WacomEvent) {
    match input {
        wacom::WacomEvent::InstrumentChange { pen, state } => {
            match pen {
                // Whether the pen is in range
                wacom::WacomPen::ToolPen => {
                    WACOM_IN_RANGE.store(state, Ordering::Relaxed);
                }
                // Whether the pen is actually making contact
                wacom::WacomPen::Touch => {
                    return;
                }
                _ => unreachable!(),
            }
        }
        _ => {}
    };
}

fn on_touch_handler(_app: &mut appctx::ApplicationContext, _input: multitouch::MultitouchEvent) {
    return;
}

fn on_button_press(app: &mut appctx::ApplicationContext, input: gpio::GPIOEvent) {
    let (btn, new_state) = match input {
        gpio::GPIOEvent::Press { button } => (button, true),
        gpio::GPIOEvent::Unpress { button } => (button, false),
        _ => return,
    };

    // Ignoring the unpressed event
    if !new_state {
        return;
    }

    // Simple but effective accidental button press filtering
    if WACOM_IN_RANGE.load(Ordering::Relaxed) {
        return;
    }

    match btn {
        gpio::PhysicalButton::MIDDLE => {
            app.clear(btn == gpio::PhysicalButton::MIDDLE);
            std::process::exit(0);
        }
        gpio::PhysicalButton::WAKEUP => {
            println!("WAKEUP button(?) pressed(?)");
        }
        _=> return,
    };
}

fn on_toggle_wifi(app: &mut appctx::ApplicationContext, _: UIElementHandle) {
    if wifi::state().unwrap() == "up" {
        wifi::disable().expect("Failed to disable wifi");
    } else{
        wifi::enable().expect("Failed to enable wifi");
        wifi::reconnect().expect("Failed to reconnect wifi");
    }
    if let UIElement::Text { ref mut text, .. } =
            app.get_element_by_name("wifi").unwrap().write().inner {
        let status;
        if wifi::state().unwrap() == "up" {
            status = "📶";
        } else{
            status = "";
        }
        *text = status.to_string();
    }
}

fn loop_update_topbar(app: &mut appctx::ApplicationContext, millis: u64) {
    let time_label = app.get_element_by_name("time").unwrap();
    let wifi_label = app.get_element_by_name("wifi").unwrap();
    let battery_label = app.get_element_by_name("battery").unwrap();
    loop {
        // Get the datetime
        let dt: DateTime<Local> = Local::now();

        if let UIElement::Text { ref mut text, .. } = time_label.write().inner {
            *text = format!("{}", dt.format("%F %I:%M %p"));
        }
        let framebuffer = app.get_framebuffer_ref();
        if let UIElement::Text { ref mut text, ref scale,.. } = battery_label.write().inner {
            let (status, _width) = get_battery_text(&framebuffer.default_font, *scale);
            *text = status;
        }
        if let UIElement::Text { ref mut text, .. } = wifi_label.write().inner {
            let status;
            if wifi::state().unwrap() == "up" {
                status = "📶";
            } else{
                status = "";
            }
            *text = status.to_string();
        }

        app.flash_element("time");
        app.flash_element("wifi");
        app.flash_element("battery");
        thread::sleep(Duration::from_millis(millis));
    }
}

fn get_battery_text(font: &rusttype::Font, size: f32) -> (String, usize) {
    let icon;
    let battery_status = battery::human_readable_charging_status().unwrap();
    if battery_status == "Discharging"{
        icon = "🔋"
    } else {
        icon = "🔌"
    }
    let status = format!(
        "{0}{1}%",
        icon,
        battery::percentage().unwrap(),
    );
    let scale = rusttype::Scale {
        x: size as f32,
        y: size as f32,
    };
    let point = rusttype::point(0.0, font.v_metrics(scale).ascent);
    let glyphs = font.layout(&status, scale, point);
    let width = glyphs
        .map(|g| g.position().x as f32 + g.unpositioned().h_metrics().advance_width)
        .next()
        .unwrap_or(0.0)
        .ceil() as usize;
    return (format!("{0:<5}", status), width);
}

fn main(){
    env_logger::init();
    let filepath;
    match env::var_os("LC_ALL") {
        Some(val) => {
            filepath = format!("{}.mo", val.into_string().unwrap());
        }
        None => {
            filepath = "en.mo".to_string();
        }
    }
    if Path::new(&filepath).exists() {
        let file = File::open(filepath).unwrap();
        let _catalog = Catalog::parse(file).unwrap();
    } else {
        let _catalog = Catalog::empty();
    }
    let mut app: appctx::ApplicationContext =
        appctx::ApplicationContext::new(
            on_button_press, on_wacom_input, on_touch_handler);
    let framebuffer = app.get_framebuffer_ref();
    if Path::new("font.ttf").exists() {
        let mut fontfile = File::open("font.ttf").unwrap();
        let mut font_data = vec![];
        fontfile.read_to_end(&mut font_data).expect("Failed to read font file");
        let collection = rusttype::FontCollection::from_bytes(font_data);
        framebuffer.default_font = collection.into_font().unwrap();
    }
    app.clear(true);
    let (text, _width) = get_battery_text(&framebuffer.default_font, 20.0);
    app.add_element(
        "battery",
        UIElementWrapper {
            position: cgmath::Point2 { x: 10, y: 20 },
            refresh: UIConstraintRefresh::Refresh,
            inner: UIElement::Text {
                foreground: color::BLACK,
                text: text,
                scale: 20.0,
                border_px: 0,
            },
            ..Default::default()
        },
    );
    let wifistatus;
    if wifi::state().unwrap() == "up" {
        wifistatus = "📶";
    } else{
        wifistatus = "";
    }
    app.add_element(
        "wifi",
        UIElementWrapper {
            position: cgmath::Point2 { x: 100, y: 20 },
            refresh: UIConstraintRefresh::Refresh,
            inner: UIElement::Text {
                foreground: color::BLACK,
                text: wifistatus.to_owned(),
                scale: 20.0,
                border_px: 0,
            },
            ..Default::default()
        },
    );
    let dt: DateTime<Local> = Local::now();
    app.add_element(
        "time",
        UIElementWrapper {
            position: cgmath::Point2 { x: 600, y: 20 },
            refresh: UIConstraintRefresh::Refresh,
            inner: UIElement::Text {
                foreground: color::BLACK,
                text: format!("{}", dt.format("%F %I:%M %p")),
                scale: 20.0,
                border_px: 0,
            },
            ..Default::default()
        },
    );
    app.add_element(
        "wifiToggle",
        UIElementWrapper {
            position: cgmath::Point2 { x: 960, y: 580 },
            refresh: UIConstraintRefresh::Refresh,

            onclick: Some(on_toggle_wifi),
            inner: UIElement::Text {
                foreground: color::BLACK,
                text: "Toggle wifi".to_owned(),
                scale: 45.0,
                border_px: 5,
            },
            ..Default::default()
        },
    );
    // let hyphenator = Standard::from_embedded(Language::EnglishUS).unwrap();
    // let wrapper = Wrapper::with_splitter(18, hyphenator);
    app.draw_elements();

    let appref = app.upgrade_ref();
    let clock_thread = thread::spawn(move || {
        loop_update_topbar(appref, 30 * 1000);
    });

    info!("Init complete. Beginning event dispatch...");
    app.dispatch_events(false, true, true);
    clock_thread.join().unwrap();
}
