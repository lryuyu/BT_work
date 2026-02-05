use crate::config::CONFIG;
use reqwest::Error;
use reqwest::blocking::get;

pub fn aed_start() -> Result<String, Error> {
    let cfg = &CONFIG.room;
    let url = &cfg.server_addr;
    let response = get(url)?.text()?;
    Ok(response)
}