// server.js
const express = require('express');
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const WebSocket = require('ws');
const path = require('path');
const cors = require('cors');

const app = express();
app.use(cors());
const PORT = 3000;

// Thay COM3 bằng COM ảo của bạn (hoặc COM2 tùy thiết lập Proteus/Bluetooth)
const SERIAL_PORT = 'COM2';
const SERIAL_BAUD = 9600;

// Serve thư mục public
app.use(express.static(path.join(__dirname, 'public')));

const server = app.listen(PORT, () => {
  console.log(`HTTP server running at http://localhost:${PORT}`);
});

// WebSocket server
const wss = new WebSocket.Server({ server });

wss.on('connection', ws => {
  console.log('Web client connected via WebSocket');
  ws.send(JSON.stringify({ info: 'connected' }));
});

// Mở SerialPort
const port = new SerialPort({
  path: SERIAL_PORT,
  baudRate: SERIAL_BAUD,
  autoOpen: false
});

const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));

port.open(err => {
  if (err) {
    console.error('Failed to open serial port:', err.message);
    return;
  }
  console.log('Serial port opened:', SERIAL_PORT);
});

parser.on('data', line => {
  line = line.trim();
  console.log('Serial raw:', line);

  // cố gắng parse JSON. Nếu không phải JSON, gửi raw
  let payload;
  try {
    payload = JSON.parse(line);
  } catch (e) {
    payload = { raw: line };
  }

  // broadcast tới tất cả WebSocket clients
  const msg = JSON.stringify({ from: 'serial', data: payload });
  wss.clients.forEach(client => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(msg);
    }
  });
});

port.on('error', err => {
  console.error('Serial port error:', err.message);
});
