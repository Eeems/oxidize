#![feature(rustc_private)]

extern crate libremarkable;
use libremarkable::appctx;
use libremarkable::framebuffer::cgmath;
use libremarkable::framebuffer::common::color;
use libremarkable::input::{
    gpio,
    multitouch,
    wacom,
};
use libremarkable::ui_extensions::element::UIConstraintRefresh;

#[cfg(feature = "enable-runtime-benchmarking")]
use libremarkable::stopwatch;

#[macro_use]
extern crate lazy_static;

// #[macro_use(c)]
// extern crate cute;

#[macro_use]
extern crate log;
extern crate env_logger;

extern crate gettext;
use gettext::Catalog;

extern crate rusttype;

use std::sync::atomic::{
    AtomicBool,
    Ordering,
};
use std::env;
use std::fs::{self, File};
use std::io::Read;
use std::path::Path;

extern crate oxidize;

extern crate hyphenation;
use hyphenation::{Language, Load, Standard};

extern crate textwrap;
use textwrap::Wrapper;

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

fn draw_folder(app: &mut appctx::ApplicationContext, folder_path: &str){
    let dir = Path::new(folder_path);
    if !dir.exists() || !dir.is_dir() {
        return;
    }
    let mut data = vec![String::from("."), String::from("..")];
    if let Ok(entries) = fs::read_dir(dir) {
        for entry in entries {
            if let Ok(entry) = entry {
                let filename = entry.file_name()
                    .to_string_lossy()
                    .into_owned();
                data.push(filename);
            }
        }
    }
    let hyphenator = Standard::from_embedded(Language::EnglishUS).unwrap();
    let wrapper = Wrapper::with_splitter(700, hyphenator);
    let mut y = 20.0;
    for path in data {
        for line in wrapper.wrap_iter(path.as_str()) {
            app.display_text(
                cgmath::Point2{ x: 10.0, y: y },
                color::BLACK,
                20.0,
                1,
                0,
                line.into_owned(),
                UIConstraintRefresh::Refresh
            );
            y += 50.0;
        }
    }
    app.draw_elements();
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
    draw_folder(app.upgrade_ref(), "/");
    info!("Init complete. Beginning event dispatch...");
    app.dispatch_events(true, true, true);
}
