use std::sync::Mutex;
use serde::{Serialize, Deserialize};
use std::time::{SystemTime, UNIX_EPOCH};
use once_cell::sync::Lazy;
use crate::config::CONFIG;
use crate::websocket_server::send_ws_message;
use crate::record;

#[derive(Clone, Copy, Debug)]
pub struct FlowSample {
    pub adc_raw: u16,      // 0 ~ 4095 (12bit)
    pub timestamp: u128,   // Millisecond timestamp
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct AirflowResult {
    pub start_time: u128,
    pub end_time: u128,
    pub total_volume: u64,   // 毫升，四舍五入整数
}

pub struct AirflowProcessor {
    pub max_flow: f64,        // Max flow (L/min)
    pub prev_time: Option<u128>,
    pub total_volume: f64,    // 毫升（浮点数，内部累计）
    pub session_start: Option<u128>,
    pub threshold_flow: f64,  // Min flow gate (filter noise)
    pub active: bool,
}

impl AirflowProcessor {
    pub fn new(max_flow: f64, threshold_flow: f64) -> Self {
        Self {
            max_flow,
            prev_time: None,
            total_volume: 0.0,
            session_start: None,
            threshold_flow,
            active: false,
        }
    }

    pub fn reset(&mut self) {
        self.prev_time = None;
        self.total_volume = 0.0;
        self.session_start = None;
        self.active = false;
    }

    fn now_millis() -> u128 {
        SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .expect("Time went backwards")
            .as_millis()
    }

    fn adc_to_voltage(adc: u16) -> f64 {
        let v_out = (adc as f64 / 4095.0) * 3.3;
        let k = (6.7 + 10.0) / 10.0; // VDC restoration
        v_out * k
    }

    fn voltage_to_flow(&self, v: f64) -> f64 {
        if v < 1.0 {
            0.0
        } else {
            ((v - 1.0) / 4.0).clamp(0.0, 1.0) * self.max_flow
        }
    }

    pub fn update(&mut self, sample: FlowSample) {
        let voltage = Self::adc_to_voltage(sample.adc_raw);
        let flow = self.voltage_to_flow(voltage);

        if flow >= self.threshold_flow {
            // Start recording airflow
            if self.session_start.is_none() {
                self.session_start = Some(sample.timestamp);
                println!("airflow started");
            }

            if let Some(prev) = self.prev_time {
                let dt = (sample.timestamp - prev) as f64 / 1000.0; // 秒
                // flow: L/min -> mL/s，再积分
                self.total_volume += flow * 1000.0 / 60.0 * dt;
            }

            self.prev_time = Some(sample.timestamp);
            self.active = true;
        } else {
            // No airflow
            if self.active {
                // Airflow ended
                if let (Some(start), Some(end)) = (self.session_start, self.prev_time) {
                    let result = AirflowResult {
                        start_time: start,
                        end_time: end,
                        total_volume: self.total_volume.round() as u64, // 四舍五入，整数毫升
                    };
                    println!("{}", serde_json::to_string(&result).unwrap());
                    record::RECORD.record("airflow", result.clone());
                    send_ws_message("airflow", result.clone());
                }

                // Reset states
                self.prev_time = None;
                self.session_start = None;
                self.total_volume = 0.0;
                self.active = false;
            }
        }
    }
}

pub(crate) static PROCESSOR: Lazy<Mutex<AirflowProcessor>> = Lazy::new(|| {
    let cfg = &CONFIG.airflow;
    Mutex::new(AirflowProcessor::new(cfg.max_flow, cfg.threshold_flow))
});

pub fn handle(value: i32) {
    let sample = FlowSample {
        adc_raw: value as u16,
        timestamp: AirflowProcessor::now_millis(),
    };

    let mut processor = PROCESSOR.lock().unwrap();
    processor.update(sample);
}
