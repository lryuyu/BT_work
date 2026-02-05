use std::{
    collections::HashMap,
    fs::{File, OpenOptions},
    io::Write,
    path::PathBuf,
    sync::{Arc, Mutex},
    time::{Duration, Instant},
};

use chrono::{DateTime, Local, TimeZone, Utc};
use once_cell::sync::Lazy;
use serde::Serialize;

pub static RECORD: Lazy<RecordManager> = Lazy::new(|| RecordManager::new());

#[derive(Clone)]
pub struct RecordManager {
    inner: Arc<Mutex<RecorderInner>>,
}

struct RecorderInner {
    active: bool,
    start_time: Option<Instant>,
    real_start_ts: Option<i64>,
    files: HashMap<String, File>,
    record_counts: HashMap<String, usize>,
    last_timestamps: HashMap<String, Duration>,
}

impl RecordManager {
    pub fn new() -> Self {
        Self {
            inner: Arc::new(Mutex::new(RecorderInner {
                active: false,
                start_time: None,
                real_start_ts: None,
                files: HashMap::new(),
                record_counts: HashMap::new(),
                last_timestamps: HashMap::new(),
            })),
        }
    }

    pub fn start(&self, uid: &str, modules: &[&str]) {
        let mut inner = self.inner.lock().unwrap();
        inner.active = true;
        inner.start_time = Some(Instant::now());
        inner.real_start_ts = Some(Utc::now().timestamp());
        inner.files.clear();
        inner.record_counts.clear();
        inner.last_timestamps.clear();

        let ts = inner.real_start_ts.unwrap();
        let dt: DateTime<Local> = Local.timestamp_opt(ts, 0).unwrap();
        let datetime_str = dt.format("%Y%m%d_%H%M%S").to_string();

        for &module in modules {
            let filename = format!("{}-{}-{}-{}.srt", uid, ts, datetime_str, module);
            let path = PathBuf::from(filename);
            if let Ok(file) = OpenOptions::new().create(true).write(true).truncate(true).open(&path) {
                inner.files.insert(module.to_string(), file);
                inner.record_counts.insert(module.to_string(), 0);
                inner.last_timestamps.insert(module.to_string(), Duration::ZERO);
                println!("[RecordManager] Created file for module '{}': {:?}", module, path);
            } else {
                eprintln!("[RecordManager] Failed to create file for module '{}'", module);
            }
        }

        println!("[RecordManager] Recording started.");
    }

    pub fn stop(&self) {
        let mut inner = self.inner.lock().unwrap();
        inner.active = false;
        inner.start_time = None;
        inner.real_start_ts = None;
        inner.files.clear();
        println!("[RecordManager] Recording stopped.");
    }

    pub fn record<T: Serialize>(&self, module: &str, result: T) {
        let mut inner = self.inner.lock().unwrap();

        if !inner.active {
            eprintln!("[RecordManager] Record not active.");
            return;
        }

        let start_time = match inner.start_time {
            Some(t) => t,
            None => {
                eprintln!("[RecordManager] start_time missing.");
                return;
            }
        };

        let elapsed = start_time.elapsed();
        let last_time = *inner.last_timestamps.get(module).unwrap_or(&Duration::ZERO);

        let count = {
            let entry = inner.record_counts.entry(module.to_string()).or_insert(0);
            *entry += 1;
            *entry
        };

        let text = match serde_json::to_string_pretty(&result) {
            Ok(s) => s,
            Err(e) => {
                eprintln!("[RecordManager] Serialization error: {}", e);
                return;
            }
        };

        if let Some(file) = inner.files.get_mut(module) {
            let entry = format!(
                "{}\n{} --> {}\n{}\n\n",
                count,
                format_duration_srt(last_time),
                format_duration_srt(elapsed),
                text
            );
            if let Err(e) = file.write_all(entry.as_bytes()) {
                eprintln!("[RecordManager] Write error in module '{}': {}", module, e);
            }
            if let Err(e) = file.flush() {
                eprintln!("[RecordManager] Flush error in module '{}': {}", module, e);
            }
        } else {
            eprintln!("[RecordManager] No file found for module '{}'", module);
        }

        inner.last_timestamps.insert(module.to_string(), elapsed);
    }
}

fn format_duration_srt(d: Duration) -> String {
    let total_ms = d.as_millis() as u64;
    let hours = total_ms / 3_600_000;
    let minutes = (total_ms % 3_600_000) / 60_000;
    let seconds = (total_ms % 60_000) / 1000;
    let millis = total_ms % 1000;
    format!("{:02}:{:02}:{:02},{:03}", hours, minutes, seconds, millis)
}
