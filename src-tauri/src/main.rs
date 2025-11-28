#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use tauri::command;
use base64::{Engine as _, engine::general_purpose};
use serde::Serialize;

fn xor_transform(data: &[u8], key: &str) -> Vec<u8> {
    if key.is_empty() {
        return data.to_vec();
    }
    let key_bytes = key.as_bytes();
    let mut out = Vec::with_capacity(data.len());
    for (i, &b) in data.iter().enumerate() {
        out.push(b ^ key_bytes[i % key_bytes.len()]);
    }
    out
}

#[derive(Serialize)]
struct DecodeResult {
    content: String,
    had_bom: bool,
}

#[command]
fn decode_content(content: String, key: String) -> Result<DecodeResult, String> {
    // Clean up whitespace just in case
    let clean_content: String = content.chars().filter(|c| !c.is_whitespace()).collect();

    let bytes = general_purpose::STANDARD.decode(&clean_content)
        .map_err(|e| format!("Base64 decode failed: {}", e))?;
    
    let xored = xor_transform(&bytes, &key);
    
    // Check for BOM (EF BB BF)
    let (start, had_bom) = if xored.len() >= 3 && xored[0] == 0xEF && xored[1] == 0xBB && xored[2] == 0xBF {
        (3, true)
    } else {
        (0, false)
    };

    let s = String::from_utf8(xored[start..].to_vec())
        .map_err(|e| format!("UTF-8 decode failed: {}", e))?;
        
    Ok(DecodeResult { content: s, had_bom })
}

#[command]
fn encode_content(content: String, key: String, add_bom: bool) -> Result<String, String> {
    let mut bytes = content.into_bytes();
    if add_bom {
        let mut with_bom = vec![0xEF, 0xBB, 0xBF];
        with_bom.append(&mut bytes);
        bytes = with_bom;
    }
    
    let xored = xor_transform(&bytes, &key);
    let b64 = general_purpose::STANDARD.encode(&xored);
    Ok(b64)
}

fn main() {
    tauri::Builder::default()
        .plugin(tauri_plugin_shell::init())
        .invoke_handler(tauri::generate_handler![decode_content, encode_content])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
