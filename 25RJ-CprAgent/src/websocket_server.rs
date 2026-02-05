use futures_util::{SinkExt, StreamExt};
use serde::Serialize;
use std::sync::RwLock;
use tokio::net::TcpListener;
use tokio::sync::broadcast::{self, Sender};
use tokio_tungstenite::accept_async;
use tungstenite::protocol::Message;
use once_cell::sync::Lazy;
use serde_json::json;
use tungstenite::Utf8Bytes;

static WS_BROADCAST: Lazy<RwLock<Option<Sender<String>>>> = Lazy::new(|| RwLock::new(None));

pub async fn start_ws_server() {
    let (tx, _) = broadcast::channel::<String>(100);
    *WS_BROADCAST.write().unwrap() = Some(tx.clone());

    let listener = TcpListener::bind("0.0.0.0:9200").await.unwrap();
    println!("Listening on 0.0.0.0:9200");

    loop {
        let (stream, _) = listener.accept().await.unwrap();
        let peer = stream.peer_addr().unwrap();
        println!("WebSocket client connected: {}", peer);

        let mut rx = tx.subscribe();

        tokio::spawn(async move {
            let ws_stream = accept_async(stream).await.expect("WebSocket accept failed");
            let (mut ws_sink, _) = ws_stream.split();

            while let Ok(msg) = rx.recv().await {
                if ws_sink.send(Message::Text(Utf8Bytes::from(msg))).await.is_err() {
                    break;
                }
            }

            println!("WebSocket client disconnected: {}", peer);
        });
    }
}

pub fn send_ws_message<T: Serialize>(ty: &str, data: T) {
    if let Some(sender) = &*WS_BROADCAST.read().unwrap() {
        let message = json!({ "type": ty, "data": data }).to_string();
        let _ = sender.send(message);
    }
}
