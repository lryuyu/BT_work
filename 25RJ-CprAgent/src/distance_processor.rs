use std::sync::Mutex;
use serde::{Serialize, Deserialize};
use std::time::{SystemTime, UNIX_EPOCH};
use once_cell::sync::Lazy;
use serde_json::json;
use crate::config::CONFIG;
use crate::websocket_server::send_ws_message;
use crate::first_press::aed_start;
use crate::record;

#[derive(Clone, Copy, Debug)]
pub struct DistanceData {
    pub d: i32,
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct Record {
    pub depth: i32,
    pub time: u128,
    pub frequency: f64,
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct Round {
    pub start: u128,
    pub records: Vec<Record>,
    pub avg_frequency: f64,
}

pub struct DistanceProcessor {
    pub min_dis: i32,
    pub buf: Vec<DistanceData>,
    pub max_value: i32,
    pub start_time: Option<u128>,
    pub last_time: Option<u128>,
    pub last_press_time: Option<u128>,
    pub records: Vec<Record>,
    pub rounds: Vec<Round>,
    pub first_press_triggered: bool,
}

impl DistanceProcessor {
    fn on_first_press(&self) {
        match aed_start() {
            Ok(_body) => println!("Sent AED start"),
            Err(err) => eprintln!("AED start failed: {}", err),
        }
    }

    pub fn new(min_dis: i32) -> Self {
        Self {
            min_dis,
            buf: Vec::new(),
            max_value: 0,
            start_time: None,
            last_time: None,
            last_press_time: None,
            records: Vec::new(),
            rounds: Vec::new(),
            first_press_triggered: false,
        }
    }

    pub fn reset(&mut self) {
        self.first_press_triggered = false;
        self.start_time = None;
        self.last_time = None;
        self.last_press_time = None;
        self.max_value = 0;
        self.records.clear();
        self.rounds.clear();
        self.buf.clear();
    }

    fn now_millis() -> u128 {
        SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .expect("Time went backwards")
            .as_millis()
    }

    pub fn update(&mut self, data: DistanceData) {
        self.buf.push(data);
        self.process_latest();
    }

    fn process_latest(&mut self) {
        let Some(&DistanceData { d }) = self.buf.last() else { return };

        let now = Self::now_millis();

        if d < self.min_dis { // Distance below threshold
            if self.max_value != 0 { // If there was a max value recorded
                if !self.first_press_triggered {
                    self.first_press_triggered = true;
                    self.on_first_press();
                }
                self.last_time = Some(now);

                let frequency = if let Some(prev_time) = self.last_press_time {
                    let dt = now - prev_time;
                    if dt > 0 {
                        1000.0 / (dt as f64)
                    } else {
                        0.0
                    }
                } else {
                    0.0
                };

                self.last_press_time = Some(now);

                let record = Record {
                    depth: self.max_value,
                    time: now,
                    frequency,
                };
                println!("{}", serde_json::to_string(&record).unwrap());
                record::RECORD.record("distance-a", record.clone());
                send_ws_message("distance", record.clone());
                self.records.push(record);
                self.max_value = 0;
            }

            // Check if gap > 1 second to end a round
            if let (Some(start), Some(last)) = (self.start_time, self.last_time) {
                if now - last > 1000 {
                    let records = std::mem::take(&mut self.records);
                    let avg_frequency = if !records.is_empty() {
                        let sum: f64 = records.iter().map(|r| r.frequency).sum();
                        sum / (records.len() as f64)
                    } else {
                        0.0
                    };

                    let round = Round {
                        start,
                        records,
                        avg_frequency,
                    };
                    println!("{}", serde_json::to_string(&round).unwrap());
                    record::RECORD.record("distance-b", round.clone());
                    self.rounds.push(round);
                    self.start_time = None;
                }
            }
        } else { // Distance above threshold
            if self.start_time.is_none() {
                self.start_time = Some(now);
            }
            self.max_value = self.max_value.max(d);
        }
    }
}

pub(crate) static PROCESSOR: Lazy<Mutex<DistanceProcessor>> = Lazy::new(|| {
    let cfg = &CONFIG.distance;
    Mutex::new(DistanceProcessor::new(cfg.min_dis))
});

pub fn handle(value: i32) {
    let mut processor = PROCESSOR.lock().unwrap();
    processor.update(DistanceData { d: value });
    let message = json!({
        "depth": value
    });
    send_ws_message("depth", message.clone());
}
