use std::sync::Mutex;
use serde::{Serialize, Deserialize};
use std::time::{SystemTime, UNIX_EPOCH};
use once_cell::sync::Lazy;
use crate::config::CONFIG;
use crate::websocket_server::send_ws_message;
use crate::record;

// Struct representing pressure data
#[derive(Clone, Copy, Debug)]
pub struct PressureData {
    pub p: i32,
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct PressureEvent {
    pub press_time: u128,
    pub release_time: u128,
    pub duration_ms: u128,
}

pub struct PressureProcessor {
    pub min_pre: i32,
    pub pressure_active: bool,
    pub pressure_start_time: Option<u128>,
}

impl PressureProcessor {
    // Constructor
    pub fn new(min_pre: i32) -> Self {
        Self {
            min_pre,
            pressure_active: false,
            pressure_start_time: None,
        }
    }

    pub fn reset(&mut self) {
        self.pressure_active = false;
        self.pressure_start_time = None;
    }

    // Get current time in milliseconds since UNIX epoch
    fn now_millis() -> u128 {
        SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .expect("Time went backwards")
            .as_millis()
    }

    // Update processor state with new pressure data
    pub fn update(&mut self, data: PressureData) {
        let now = Self::now_millis();

        // Match current state and new pressure condition
        match (self.pressure_active, data.p > self.min_pre) {
            // Pressure started but not yet active
            (false, true) => {
                match self.pressure_start_time {
                    None => {
                        self.pressure_start_time = Some(now);
                        println!("start pressure count");
                    }
                    Some(start) => {
                        // Activate if pressure lasts >= 3 seconds
                        if now - start >= 3000 {
                            self.pressure_active = true;
                            println!("pressure activated");
                        }
                    }
                }
            }

            // Pressure below threshold before activation
            (false, false) => {
                if self.pressure_start_time.is_some() {
                    println!("pressure aborted before 3s");
                    self.pressure_start_time = None;
                }
            }

            // Pressure released after activation
            (true, false) => {
                if let Some(start) = self.pressure_start_time {
                    let event = PressureEvent {
                        press_time: start,
                        release_time: now,
                        duration_ms: now - start,
                    };
                    println!("{}", serde_json::to_string(&event).unwrap());
                    record::RECORD.record("pressure", event.clone());
                    send_ws_message("pressure", event.clone());
                }

                self.pressure_active = false;
                self.pressure_start_time = None;
            }

            // Pressure ongoing and active — no change
            (true, true) => {}
        }
    }
}

pub(crate) static PROCESSOR: Lazy<Mutex<PressureProcessor>> = Lazy::new(|| {
    let cfg = &CONFIG.pressure;
    Mutex::new(PressureProcessor::new(cfg.min_pre))
});

pub fn handle(value: i32) {
    let mut processor = PROCESSOR.lock().unwrap();
    processor.update(PressureData { p: value });
}
