import {initPwa, registerServiceWorker} from "./pwa.js";
import {resetBattery} from "./battery.js";
import {resetRemoteState} from "./state.js";
import {
    abortUpdates,
    hidePeoplePanels,
    initPeople,
    setUpdateButtons
} from "./people.js";
import {
    connectBle,
    getDeviceName,
    sendCommand,
    sendTransactionCommand
} from "./ble.js";
import {createProtocol} from "./protocol.js";

const connectButton = document.getElementById("connectButton");
const connectionDot = document.getElementById("connectionDot");
const message = document.getElementById("message");
const lastConnected = document.getElementById("lastConnected");
const installButton = document.getElementById("installButton");


function setMessage(text, type = "info") {
    message.textContent = text;
    message.className = type;
}


function setConnectionState(state) {
    const labels = {
        disconnected: "Raum-Schild auswählen",
        connecting: "Verbindung wird hergestellt …",
        loading: "Inhalt wird geladen …",
        ready: "Verbunden und Inhalt vollständig geladen"
    };

    connectButton.textContent = labels[state];
    connectButton.disabled = state !== "disconnected";
    connectionDot.classList.toggle(
        "connected",
        state === "loading" || state === "ready"
    );
}


async function connect() {
    if (!navigator.bluetooth) {
        setMessage(
            "Web Bluetooth wird von diesem Browser nicht unterstützt.",
            "error"
        );
        return;
    }

    try {
        setMessage("");
        setConnectionState("connecting");

        await connectBle({
            onLine: processLine,
            onDisconnected: handleDisconnected,

            onConnecting: () => {
                setConnectionState("connecting");
            },

            onConnected: () => {
                resetRemoteData();
                setConnectionState("loading");
            }
        });

    } catch (error) {
        setConnectionState("disconnected");

        if (error.name === "NotFoundError") {
            setMessage("");
            return;
        }

        console.error(error);
        setMessage(
            "Verbindung fehlgeschlagen: " + error.message,
            "error"
        );
    }
}


function resetRemoteData() {
    resetRemoteState();
    resetBattery();
    resetProtocol();
    hidePeoplePanels();
    abortUpdates();
}


function handleDisconnected() {
    abortUpdates(true);
    setConnectionState("disconnected");
    setUpdateButtons(false);
    setMessage("Bluetooth-Verbindung getrennt.", "error");
}


function handleCommandError(error) {
    console.error(error);
    abortUpdates(true);
    setMessage(
        "Bluetooth-Fehler: " + error.message,
        "error"
    );
}


// ============================================================
// Init and start
// ============================================================
initPeople({
    sendCommand,
    sendTransactionCommand,
    setMessage,
    handleCommandError
});

const {processLine, resetProtocol} = createProtocol({
    setMessage,
    handleCommandError,

    onRoom: () => {
        const name = getDeviceName();
        document.getElementById("title").textContent = name;
        document.title = name || "O³ OutOfOffice";
    },

    onOrganization: organization => {
        document.getElementById("organizationTop").textContent = organization;
    },

    onDepartment: department => {
        document.getElementById("departmentTop").textContent = department;
    },

    onStateComplete: () => {
        setConnectionState("ready");

        const now = new Date();

        lastConnected.textContent =
            "Zuletzt verbunden: " +
            now.toLocaleTimeString("de-DE", {
                hour: "2-digit",
                minute: "2-digit"
            });
    }
});

initPwa(installButton);

connectButton.addEventListener("click", connect);

window.addEventListener("load", async () => {
    setConnectionState("disconnected");
    await registerServiceWorker();
});