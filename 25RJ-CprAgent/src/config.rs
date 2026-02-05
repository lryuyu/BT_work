use once_cell::sync::Lazy;
use serde::Deserialize;
use std::fs;

#[derive(Debug, Deserialize)]
pub struct RoomConfig {
    pub server_addr: String,
}

#[derive(Debug, Deserialize)]
pub struct LocationConfig {
    pub min_center: i32,
    pub tolerance_horizontal: i32,
    pub tolerance_vertical: i32,
}

#[derive(Debug, Deserialize)]
pub struct AirflowConfig {
    pub max_flow: f64,
    pub threshold_flow: f64,
}

#[derive(Debug, Deserialize)]
pub struct PressureConfig {
    pub min_pre: i32,
}

#[derive(Debug, Deserialize)]
pub struct DistanceConfig {
    pub min_dis: i32,
}

#[derive(Debug, Deserialize)]
pub struct AppConfig {
    pub room: RoomConfig,
    pub location: LocationConfig,
    pub airflow: AirflowConfig,
    pub pressure: PressureConfig,
    pub distance: DistanceConfig,
}

fn load_config() -> AppConfig {
    let content = fs::read_to_string("config.toml")
        .expect("Failed to read config.toml");
    toml::from_str(&content).expect("Failed to parse config.toml")
}

pub static CONFIG: Lazy<AppConfig> = Lazy::new(|| {
    load_config()
});
