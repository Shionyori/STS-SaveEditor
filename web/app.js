// app.js — 基于 Base64 + XOR 的编码/解码，与 C++ codec 对齐

const fileInput = document.getElementById('fileInput');
const keyInput = document.getElementById('keyInput');
const decodeBtn = document.getElementById('decodeBtn');
const encodeBtn = document.getElementById('encodeBtn');
const downloadBtn = document.getElementById('downloadBtn');
const jsonArea = document.getElementById('jsonArea');
const status = document.getElementById('status');

let lastEncodedString = null;
let lastFileName = 'download.autosave';

function setStatus(txt) { status.textContent = '状态：' + txt; }

// Base64 helpers using btoa/atob via binary string conversion
function uint8ArrayToBase64(u8) {
  let binary = '';
  const chunk = 0x8000; // avoid call stack issues on large arrays
  for (let i = 0; i < u8.length; i += chunk) {
    const slice = u8.subarray(i, Math.min(i + chunk, u8.length));
    binary += String.fromCharCode.apply(null, slice);
  }
  return btoa(binary);
}

function base64ToUint8Array(b64) {
  // remove whitespace/newlines
  b64 = b64.replace(/\s+/g, '');
  const binary = atob(b64);
  const len = binary.length;
  const u8 = new Uint8Array(len);
  for (let i = 0; i < len; i++) u8[i] = binary.charCodeAt(i);
  return u8;
}

function xorTransform(u8, keyStr) {
  if (!keyStr) return u8.slice();
  const encoder = new TextEncoder();
  const k = encoder.encode(keyStr);
  if (k.length === 0) return u8.slice();
  const out = new Uint8Array(u8.length);
  for (let i = 0; i < u8.length; i++) {
    out[i] = u8[i] ^ k[i % k.length];
  }
  return out;
}

function decodeBase64AutosaveToJsonText(b64text, key) {
  try {
    const bytes = base64ToUint8Array(b64text);
    const plain = xorTransform(bytes, key);
    const decoder = new TextDecoder('utf-8', { fatal: false });
    const s = decoder.decode(plain);
    // try to pretty-print JSON if possible
    try {
      const obj = JSON.parse(s);
      return JSON.stringify(obj, null, 2);
    } catch (e) {
      // not valid JSON, return raw string
      return s;
    }
  } catch (e) {
    throw new Error('解码失败: ' + e.message);
  }
}

function encodeJsonTextToBase64Autosave(jsonText, key) {
  // accept both raw JSON text or pre-encoded binary text
  const encoder = new TextEncoder();
  const bytes = encoder.encode(jsonText);
  const xored = xorTransform(bytes, key);
  const b64 = uint8ArrayToBase64(xored);
  return b64;
}

// File handlers
function readFileAsText(file) {
  return new Promise((resolve, reject) => {
    const fr = new FileReader();
    fr.onload = () => resolve(fr.result);
    fr.onerror = () => reject(new Error('读取文件失败'));
    fr.readAsText(file);
  });
}

decodeBtn.addEventListener('click', async () => {
  const file = fileInput.files[0];
  const key = keyInput.value || '';
  if (!file) {
    setStatus('请选择文件');
    return;
  }
  if (!key) {
    setStatus('请输入密钥');
    return;
  }
  setStatus('读取文件...');
  try {
    const text = await readFileAsText(file);
    // If file looks like binary (has null bytes), try reading as ArrayBuffer and convert to base64
    let b64text = text.trim();
    // if the content looks not base64 (contains many non-base64 chars), try to read raw bytes
    if (!/^([A-Za-z0-9+/=\r\n]+)$/.test(b64text)) {
      // read as array buffer
      const raw = await new Promise((resolve, reject) => {
        const fr = new FileReader();
        fr.onload = () => resolve(fr.result);
        fr.onerror = () => reject(new Error('读取二进制文件失败'));
        fr.readAsArrayBuffer(file);
      });
      const u8 = new Uint8Array(raw);
      b64text = uint8ArrayToBase64(u8);
    }

    const decoded = decodeBase64AutosaveToJsonText(b64text, key);
    jsonArea.value = decoded;
    lastEncodedString = null;
    lastFileName = file.name || lastFileName;
    setStatus('解码成功，可编辑 JSON。');
  } catch (e) {
    setStatus('错误：' + e.message);
    console.error(e);
  }
});

encodeBtn.addEventListener('click', () => {
  const key = keyInput.value || '';
  if (!key) {
    setStatus('请输入密钥');
    return;
  }
  const text = jsonArea.value;
  if (!text) {
    setStatus('JSON 为空');
    return;
  }
  try {
    const b64 = encodeJsonTextToBase64Autosave(text, key);
    lastEncodedString = b64;
    downloadBtn.disabled = false;
    setStatus('加密完成，点击“下载加密文件”。');
  } catch (e) {
    setStatus('错误：' + e.message);
  }
});

downloadBtn.addEventListener('click', () => {
  if (!lastEncodedString) return;
  const blob = new Blob([lastEncodedString], { type: 'text/plain;charset=utf-8' });
  const name = (lastFileName && lastFileName.length) ? (lastFileName + '.autosave') : 'download.autosave';
  const a = document.createElement('a');
  a.href = URL.createObjectURL(blob);
  a.download = name;
  document.body.appendChild(a);
  a.click();
  a.remove();
  setStatus('已生成下载：' + name);
});

// keyboard shortcuts: Ctrl+S to encode+download
window.addEventListener('keydown', (ev) => {
  if ((ev.ctrlKey || ev.metaKey) && ev.key.toLowerCase() === 's') {
    ev.preventDefault();
    encodeBtn.click();
    setTimeout(() => downloadBtn.click(), 100);
  }
});

// small helper to pretty-print status when file input changes
fileInput.addEventListener('change', () => {
  if (fileInput.files.length) setStatus('已选择：' + fileInput.files[0].name);
  else setStatus('未选择文件');
});
