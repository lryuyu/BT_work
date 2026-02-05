use std::sync::Mutex;
use serde::{Serialize, Deserialize};
use std::time::{SystemTime, UNIX_EPOCH};
use once_cell::sync::Lazy;
use crate::config::CONFIG;
use crate::websocket_server::send_ws_message;
use crate::record;

#[derive(Clone, Copy, Debug)]
pub struct LocationData {
    pub values: [i32; 5], // [Top, Left, Center, Right, Bottom]
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct LocationResult {
    pub time: u128,
    pub center_pressure: i32,
    pub symmetry: bool,
    pub deviation: Option<String>,
    pub horizontal_diff: i32,
    pub vertical_diff: i32,
}

pub struct LocationProcessor {
    pub min_center: i32,
    pub tolerance_horizontal: i32,
    pub tolerance_vertical: i32,
}

impl LocationProcessor {
    pub fn new(min_center: i32, tolerance_horizontal: i32, tolerance_vertical: i32) -> Self {
        Self {
            min_center,
            tolerance_horizontal,
            tolerance_vertical,
        }
    }

    pub fn reset(&mut self) {}

    fn now_millis() -> u128 {
        SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .expect("Time went backwards")
            .as_millis()
    }

    pub fn update(&mut self, data: LocationData) {
        let now = Self::now_millis();

        let [top, left, center, right, bottom] = data.values;

        if center < self.min_center {
            return;
        }

        let mut deviations = vec![];

        let horizontal_diff = left - right;
        let vertical_diff = top - bottom;

        if horizontal_diff.abs() > self.tolerance_horizontal {
            if horizontal_diff > 0 {
                deviations.push(Self::describe_deviation("Left", horizontal_diff));
            } else {
                deviations.push(Self::describe_deviation("Right", -horizontal_diff));
            }
        }

        if vertical_diff.abs() > self.tolerance_vertical {
            if vertical_diff > 0 {
                deviations.push(Self::describe_deviation("Up", vertical_diff));
            } else {
                deviations.push(Self::describe_deviation("Down", -vertical_diff));
            }
        }

        let symmetry = deviations.is_empty();
        let deviation = if symmetry { None } else { Some(deviations.join("+")) };

        let result = LocationResult {
            time: now,
            center_pressure: center,
            symmetry,
            deviation,
            horizontal_diff: horizontal_diff.abs(),
            vertical_diff: vertical_diff.abs(),
        };

        println!("{}", serde_json::to_string(&result).unwrap());
        record::RECORD.record("location", result.clone());
        send_ws_message("location", result.clone());
    }

    fn describe_deviation(direction: &str, diff: i32) -> String {
        if diff < 30 {
            format!("Slightly {}", direction)
        } else if diff < 70 {
            format!("Few {}", direction)
        } else {
            format!("Heavy {}", direction)
        }
    }
}

pub(crate) static PROCESSOR: Lazy<Mutex<LocationProcessor>> = Lazy::new(|| {
    let cfg = &CONFIG.location;
    Mutex::new(LocationProcessor::new(cfg.min_center, cfg.tolerance_horizontal, cfg.tolerance_vertical))
});

pub fn handle(data: [i32; 5]) {
    let mut processor = PROCESSOR.lock().unwrap();
    processor.update(LocationData { values: data });
}