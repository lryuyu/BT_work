mod distance_processor;
mod pressure_processor;
mod location_processor;
mod airflow_processor;
mod record;
mod config;
mod websocket_server;
mod first_press;

use std::{
    io::{BufRead, BufReader},
    net::{TcpListener, TcpStream},
    sync::mpsc::{self, Sender},
    thread,
};
use tiny_http::{Server, Response};
use url::Url;

fn handle_client(
    stream: TcpStream,
    tx_location: Sender<[i32; 5]>,
    tx_pressure: Sender<i32>,
    tx_airflow: Sender<i32>,
    tx_distance: Sender<i32>,
) {
    let reader = BufReader::new(stream);

    for line in reader.lines().flatten() {
        if line.trim().is_empty() {
            continue;
        }
        record::RECORD.record("record-raw", line.clone());
        // println!("{}",line);

        // Decode data
        let parts: Vec<i32> = line
            .split('-')
            .filter_map(|s| s.trim().parse().ok())
            .collect();

        if parts.len() != 8 {
            eprintln!("Invalid data: {}", line);
            continue;
        }

        // Distribute
        let loc_data: [i32; 5] = parts[0..5].try_into().unwrap();
        let _ = tx_location.send(loc_data);
        let _ = tx_pressure.send(parts[5]);
        let _ = tx_airflow.send(parts[6]);
        let _ = tx_distance.send(parts[7]);
    }

    println!("Connection closed");
}

#[tokio::main]
async fn main() -> std::io::Result<()> {
    // Processor channels
    let (tx_location, rx_location) = mpsc::channel::<[i32; 5]>();
    let (tx_pressure, rx_pressure) = mpsc::channel::<i32>();
    let (tx_airflow, rx_airflow) = mpsc::channel::<i32>();
    let (tx_distance, rx_distance) = mpsc::channel::<i32>();

    // Start processor thread
    thread::spawn(move || {
        for data in rx_location {
            location_processor::handle(data);
        }
    });

    thread::spawn(move || {
        for data in rx_pressure {
            pressure_processor::handle(data);
        }
    });

    thread::spawn(move || {
        for data in rx_airflow {
            airflow_processor::handle(data);
        }
    });

    thread::spawn(move || {
        for data in rx_distance {
            distance_processor::handle(data);
        }
    });

    {
        thread::spawn(move || {
            let server = Server::http("0.0.0.0:9000").unwrap();
            println!("Listening on 0.0.0.0:9000");
            for request in server.incoming_requests() {
                let url = request.url();
                let url_str = format!("http://localhost{}", url);
                let parsed = Url::parse(&url_str).unwrap();
                let path = url.split('?').next().unwrap_or(url);
                match path {
                    "/start" => {
                        let uid = parsed
                            .query_pairs()
                            .find(|(k, _)| k.as_ref() == "uid")
                            .map(|(_, v)| v.to_string())
                            .filter(|v| !v.trim().is_empty())
                            .unwrap_or_else(|| "TRAIN".to_string());

                        record::RECORD.start(&uid, &["record-raw", "pressure", "airflow", "distance-a", "distance-b", "location"]);

                        if let Ok(mut dp) = distance_processor::PROCESSOR.lock() {
                            dp.reset();
                        }

                        if let Ok(mut ap) = airflow_processor::PROCESSOR.lock() {
                            ap.reset();
                        }

                        if let Ok(mut lp) = location_processor::PROCESSOR.lock() {
                            lp.reset();
                        }

                        if let Ok(mut pp) = pressure_processor::PROCESSOR.lock() {
                            pp.reset();
                        }

                        let _ = request.respond(Response::from_string("started"));
                    }
                    "/stop" => {
                        record::RECORD.stop();
                        let _ = request.respond(Response::from_string("stopped"));
                    }
                    _ => {
                        let _ = request.respond(Response::from_string("not found").with_status_code(404));
                    }
                }
            }
        });
    }

    tokio::spawn(async {
        websocket_server::start_ws_server().await;
    });

    // Start TCP service
    let listener = TcpListener::bind("0.0.0.0:8888")?;
    println!("Listening on 0.0.0.0:8888");

    for stream in listener.incoming() {
        match stream {
            Ok(stream) => {
                println!("New connection: {}", stream.peer_addr()?);
                let tx_loc = tx_location.clone();
                let tx_pre = tx_pressure.clone();
                let tx_air = tx_airflow.clone();
                let tx_dis = tx_distance.clone();

                thread::spawn(move || {
                    handle_client(stream, tx_loc, tx_pre, tx_air, tx_dis);
                });
            }
            Err(e) => {
                eprintln!("Connection failed: {}", e);
            }
        }
    }

    Ok(())
}