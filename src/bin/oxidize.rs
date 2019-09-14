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
    wacom
};
use libremarkable::ui_extensions::element::{
    UIElement,
    UIElementWrapper,
    UIConstraintRefresh,
};

#[cfg(feature = "enable-runtime-benchmarking")]
use libremarkable::stopwatch;

#[macro_use]
extern crate lazy_static;

#[macro_use]
extern crate log;
extern crate env_logger;

extern crate chrono;
use chrono::{
    DateTime,
    Local,
};

use std::sync::atomic::{
    AtomicBool,
    Ordering,
};
use std::time::Duration;
use std::thread;

lazy_static! {
    static ref WACOM_IN_RANGE: AtomicBool = AtomicBool::new(false);
}

fn on_wacom_input(app: &mut appctx::ApplicationContext, input: wacom::WacomEvent) {
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

fn on_touch_handler(app: &mut appctx::ApplicationContext, input: multitouch::MultitouchEvent) {
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

fn loop_update_topbar(app: &mut appctx::ApplicationContext, millis: u64) {
    let time_label = app.get_element_by_name("time").unwrap();
    let battery_label = app.get_element_by_name("battery").unwrap();
    loop {
        // Get the datetime
        let dt: DateTime<Local> = Local::now();

        if let UIElement::Text { ref mut text, .. } = time_label.write().inner {
            *text = format!("{}", dt.format("%F %I:%M:%S %p"));
        }

        if let UIElement::Text { ref mut text, .. } = battery_label.write().inner {
            *text = format!(
                "{0:<128}",
                format!(
                    "{0} — {1}%",
                    battery::human_readable_charging_status().unwrap(),
                    battery::percentage().unwrap()
                )
            );
        }
        app.draw_element("time");
        app.draw_element("battery");
        thread::sleep(Duration::from_millis(millis));
    }
}

fn main(){
    env_logger::init();
    let mut app: appctx::ApplicationContext =
        appctx::ApplicationContext::new(
            on_button_press, on_wacom_input, on_touch_handler);
    app.clear(true);

    app.add_element(
        "text",
        UIElementWrapper {
            position: cgmath::Point2 { x: 30, y: 50 },
            refresh: UIConstraintRefresh::Refresh,

            onclick: None,
            inner: UIElement::Text {
                foreground: color::BLACK,
                text: "Hello World! Press middle button to exit".to_owned(),
                scale: 35.0,
                border_px: 0,
            },
            ..Default::default()
        },
    );
    app.add_element(
        "battery",
        UIElementWrapper {
            position: cgmath::Point2 { x: 30, y: 215 },
            refresh: UIConstraintRefresh::Refresh,
            inner: UIElement::Text {
                foreground: color::BLACK,
                text: format!(
                    "{0:<128}",
                    format!(
                        "{0} — {1}%",
                        battery::human_readable_charging_status().unwrap(),
                        battery::percentage().unwrap()
                    )
                ),
                scale: 44.0,
                border_px: 0,
            },
            ..Default::default()
        },
    );
    let dt: DateTime<Local> = Local::now();
    app.add_element(
        "time",
        UIElementWrapper {
            position: cgmath::Point2 { x: 30, y: 150 },
            refresh: UIConstraintRefresh::Refresh,
            inner: UIElement::Text {
                foreground: color::BLACK,
                text: format!("{}", dt.format("%F %I:%M:%S %p")),
                scale: 75.0,
                border_px: 0,
            },
            ..Default::default()
        },
    );
    app.draw_elements();

    let appref = app.upgrade_ref();
    let clock_thread = thread::spawn(move || {
        loop_update_topbar(appref, 30 * 1000);
    });

    info!("Init complete. Beginning event dispatch...");
    app.dispatch_events(false, false, true);
    clock_thread.join().unwrap();
}
