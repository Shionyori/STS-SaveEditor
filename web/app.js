// app.js — 基于 Base64 + XOR 的编码/解码，与 C++ codec 对齐

// Wait for DOM to be ready
document.addEventListener('DOMContentLoaded', () => {
  console.log('DOM loaded');
  
  // Check for Tauri environment
  const tauriObj = window.__TAURI__;
  const invokeFn = tauriObj?.core?.invoke || tauriObj?.tauri?.invoke;
  const isTauri = !!invokeFn;
  const invoke = invokeFn;

  const fileInput = document.getElementById('fileInput');
  const keyInput = document.getElementById('keyInput');
  const decodeBtn = document.getElementById('decodeBtn');
  const encodeBtn = document.getElementById('encodeBtn');
  const downloadBtn = document.getElementById('downloadBtn');
  const jsonArea = document.getElementById('jsonArea');
  const status = document.getElementById('status');

  // Status helper with visual feedback
  function setStatus(txt, type = 'info') { 
    status.textContent = '状态：' + txt; 
    if (type === 'error') {
      status.style.backgroundColor = '#ffebee';
      status.style.color = '#c62828';
    } else if (type === 'success') {
      status.style.backgroundColor = '#e8f5e9';
      status.style.color = '#2e7d32';
    } else {
      status.style.backgroundColor = '#f1f5f9';
      status.style.color = '#333';
    }
    console.log('Status:', txt);
  }

  // CodeMirror editor instance
  let cm = null;
  if (typeof CodeMirror !== 'undefined') {
    try {
      cm = CodeMirror.fromTextArea(jsonArea, {
        mode: { name: 'javascript', json: true },
        lineNumbers: true,
        lineWrapping: true,
        autoCloseBrackets: true,
        theme: 'eclipse',
        indentUnit: 2,
        tabSize: 2,
        extraKeys: {
          "Ctrl-F": "findPersistent",
          "Cmd-F": "findPersistent"
        }
      });
      
      // Set editor to fill the container (height defined in CSS)
      cm.setSize('100%', '100%');
      
    } catch (e) {
      console.error('CodeMirror init failed:', e);
      setStatus('编辑器初始化失败: ' + e.message, 'error');
    }
  }

  let lastEncodedString = null;
  let lastFileName = 'download.autosave';
  let lastHadBOM = false;

  // ... (rest of the helper functions) ...
  
  // Base64 helpers
  function uint8ArrayToBase64(u8) {
    let binary = '';
    const chunk = 0x8000; 
    for (let i = 0; i < u8.length; i += chunk) {
      const slice = u8.subarray(i, Math.min(i + chunk, u8.length));
      binary += String.fromCharCode.apply(null, slice);
    }
    return btoa(binary);
  }

  function base64ToUint8Array(b64) {
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

  async function decodeBase64AutosaveToJsonText(b64text, key) {
    if (isTauri) {
      try {
        console.log('Invoking Rust decode_content...');
        const res = await invoke('decode_content', { content: b64text, key: key });
        console.log('Rust decode success');
        lastHadBOM = res.had_bom;
        try {
          const obj = JSON.parse(res.content);
          return JSON.stringify(obj, null, 2);
        } catch (e) {
          return res.content;
        }
      } catch (e) {
        console.error('Rust decode error:', e);
        throw new Error('Rust decode failed: ' + e);
      }
    }

    // JS Fallback
    const bytes = base64ToUint8Array(b64text);
    const plain = xorTransform(bytes, key);
    const hadBOM = (plain.length >= 3 && plain[0] === 0xEF && plain[1] === 0xBB && plain[2] === 0xBF);
    const decoder = new TextDecoder('utf-8', { fatal: false });
    const s = decoder.decode(hadBOM ? plain.subarray(3) : plain);
    try {
      const obj = JSON.parse(s);
      lastHadBOM = hadBOM;
      return JSON.stringify(obj, null, 2);
    } catch (e) {
      lastHadBOM = hadBOM;
      return s;
    }
  }

  async function encodeJsonTextToBase64Autosave(jsonText, key) {
    if (isTauri) {
      try {
        return await invoke('encode_content', { content: jsonText, key: key, addBom: lastHadBOM });
      } catch (e) {
        throw new Error('Rust encode failed: ' + e);
      }
    }
    // JS Fallback
    const encoder = new TextEncoder();
    const bytes = encoder.encode(jsonText);
    let plainBytes;
    if (lastHadBOM) {
      plainBytes = new Uint8Array(3 + bytes.length);
      plainBytes[0] = 0xEF; plainBytes[1] = 0xBB; plainBytes[2] = 0xBF;
      plainBytes.set(bytes, 3);
    } else {
      plainBytes = bytes;
    }
    const xored = xorTransform(plainBytes, key);
    return uint8ArrayToBase64(xored);
  }

  function readFileAsText(file) {
    return new Promise((resolve, reject) => {
      const fr = new FileReader();
      fr.onload = () => resolve(fr.result);
      fr.onerror = () => reject(new Error('读取文件失败'));
      fr.readAsText(file);
    });
  }

  // Event Listeners
  decodeBtn.addEventListener('click', async () => {
    console.log('Decode button clicked');
    const file = fileInput.files[0];
    const key = keyInput.value || '';
    
    if (!file) {
      setStatus('请先选择文件！', 'error');
      return;
    }
    if (!key) {
      setStatus('请输入密钥！', 'error');
      return;
    }

    setStatus('正在读取文件...');
    try {
      const text = await readFileAsText(file);
      let b64text = text.trim();
      
      if (!/^([A-Za-z0-9+/=\r\n]+)$/.test(b64text)) {
        setStatus('检测到二进制文件，正在转换...');
        const raw = await new Promise((resolve, reject) => {
          const fr = new FileReader();
          fr.onload = () => resolve(fr.result);
          fr.onerror = () => reject(new Error('读取二进制文件失败'));
          fr.readAsArrayBuffer(file);
        });
        const u8 = new Uint8Array(raw);
        b64text = uint8ArrayToBase64(u8);
      }

      setStatus('正在解码...');
      const decoded = await decodeBase64AutosaveToJsonText(b64text, key);
      
      if (cm) cm.setValue(decoded);
      else jsonArea.value = decoded;
      
      lastEncodedString = null;
      lastFileName = file.name || lastFileName;
      setStatus('解码成功！（在编辑框中 Ctrl + F 可查找关键词）', 'success');
    } catch (e) {
      setStatus('错误：' + e.message, 'error');
      console.error(e);
    }
  });

  encodeBtn.addEventListener('click', async () => {
    console.log('Encode button clicked');
    const key = keyInput.value || '';
    if (!key) {
      setStatus('请输入密钥', 'error');
      return;
    }
    const text = cm ? cm.getValue() : jsonArea.value;
    if (!text) {
      setStatus('内容为空', 'error');
      return;
    }
    try {
      setStatus('正在加密...');
      const b64 = await encodeJsonTextToBase64Autosave(text, key);
      lastEncodedString = b64;
      downloadBtn.disabled = false;
      setStatus('加密完成，请点击下载', 'success');
    } catch (e) {
      setStatus('加密错误：' + e.message, 'error');
      alert('加密错误：' + e.message);
    }
  });

  downloadBtn.addEventListener('click', () => {
    if (!lastEncodedString) return;
    const blob = new Blob([lastEncodedString], { type: 'text/plain;charset=utf-8' });
    let name;
    if (lastFileName && lastFileName.length) {
      const lower = lastFileName.toLowerCase();
      name = lower.endsWith('.autosave') ? lastFileName : (lastFileName + '.autosave');
    } else {
      name = 'download.autosave';
    }
    const a = document.createElement('a');
    a.href = URL.createObjectURL(blob);
    a.download = name;
    document.body.appendChild(a);
    a.click();
    a.remove();
    setStatus('已下载：' + name + ' （位于系统默认下载路径）', 'success');
  });

  fileInput.addEventListener('change', () => {
    if (fileInput.files.length) setStatus('已选择：' + fileInput.files[0].name);
  });

  // Global error handler
  window.onerror = function(msg, url, line, col, error) {
    const extra = !col ? '' : '\ncolumn: ' + col;
    const err = 'Error: ' + msg + '\nurl: ' + url + '\nline: ' + line + extra;
    console.error(err);
    // alert(err); // Uncomment if desperate
    return false;
  };

});

