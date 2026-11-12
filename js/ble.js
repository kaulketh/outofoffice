const NUS_SERVICE = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const NUS_RX = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
const NUS_TX = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

const CHUNK_SIZE = 18;
const CHUNK_DELAY = 12;

let device = null;
let server = null;
let rxCharacteristic = null;
let txCharacteristic = null;

let notificationBuffer = "";
let decoder = new TextDecoder();
const encoder = new TextEncoder();

let onLineReceived = null;
let onDisconnectedCallback = null;

export function getDeviceName() {
    return device?.name || "";
}

export async function connectBle({
                                     onLine,
                                     onConnecting,
                                     onConnected,
                                     onDisconnected
                                 }) {
    device = await navigator.bluetooth.requestDevice({
        filters: [{services: [NUS_SERVICE]}]
    });

    onLineReceived = onLine;
    onDisconnectedCallback = onDisconnected;

    device.addEventListener(
        "gattserverdisconnected",
        handleDisconnected
    );

    onConnecting();

    server = await device.gatt.connect();
    const service = await server.getPrimaryService(NUS_SERVICE);

    rxCharacteristic = await service.getCharacteristic(NUS_RX);
    txCharacteristic = await service.getCharacteristic(NUS_TX);

    notificationBuffer = "";
    decoder = new TextDecoder();

    await txCharacteristic.startNotifications();

    txCharacteristic.addEventListener(
        "characteristicvaluechanged",
        handleNotification
    );

    onConnected();
    await sendCommand("INFO");
}

export async function sendCommand(command) {
    if (!rxCharacteristic) {
        throw new Error("Keine Bluetooth-Verbindung");
    }

    const data = encoder.encode(command);

    for (let offset = 0; offset < data.length; offset += CHUNK_SIZE) {
        const chunk = data.slice(offset, offset + CHUNK_SIZE);

        if (rxCharacteristic.properties.writeWithoutResponse) {
            await rxCharacteristic.writeValueWithoutResponse(chunk);
        } else {
            await rxCharacteristic.writeValueWithResponse(chunk);
        }

        await delay(CHUNK_DELAY);
    }
}

function handleNotification(event) {
    const value = event.target.value;

    const bytes = new Uint8Array(
        value.buffer,
        value.byteOffset,
        value.byteLength
    );

    notificationBuffer += decoder.decode(bytes, {stream: true});

    while (notificationBuffer.includes("\n")) {
        const position = notificationBuffer.indexOf("\n");

        const line = notificationBuffer
            .slice(0, position)
            .replace(/\r$/, "")
            .trim();

        notificationBuffer = notificationBuffer.slice(position + 1);

        if (line.length > 0) {
            onLineReceived?.(line);
        }
    }
}

function handleDisconnected(event) {
    if (event.target !== device) {
        return;
    }

    rxCharacteristic = null;
    txCharacteristic = null;
    server = null;
    notificationBuffer = "";
    decoder = new TextDecoder();

    onDisconnectedCallback?.();
}

function delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}
