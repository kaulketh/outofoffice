import {initPwa, registerServiceWorker} from "./pwa.js";
import {resetBattery} from "./battery.js";
import {resetRemoteState} from "./state.js";
import {hidePeoplePanels, initPeople, setUpdateButtons} from "./people.js";
import {connectBle, getDeviceName, sendCommand} from "./ble.js";
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


async function connect() {
    if (!navigator.bluetooth) {
        setMessage(
            "Web Bluetooth wird von diesem Browser nicht unterstützt.",
            "error"
        );
        return;
    }

    try {
        setMessage("Raumschild auswählen …", "info");

        await connectBle({
            onLine: processLine,
            onDisconnected: handleDisconnected,

            onConnecting: () => {
                setMessage("Verbinde …", "info");
            },

            onConnected: () => {
                connectionDot.classList.add("connected");
                connectButton.textContent = "Verbunden";
                connectButton.disabled = true;

                resetRemoteData();

                setMessage(
                    "Verbunden – lese Konfiguration …",
                    "info"
                );
            }
        });

    } catch (error) {
        if (error.name === "NotFoundError") {
            setMessage("Keine Verbindung hergestellt.", "info");
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
}


function handleDisconnected() {
    connectionDot.classList.remove("connected");
    connectButton.textContent = "Raumschild verbinden";
    connectButton.disabled = false;

    setMessage("Bluetooth-Verbindung getrennt.", "error");
}


function handleCommandError(error) {
    console.error(error);
    setMessage("Bluetooth-Fehler: " + error.message, "error");
    setUpdateButtons(true);
}


// ============================================================
// Init and start
// ============================================================
initPeople({
    sendCommand,
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
    await registerServiceWorker();
    setMessage("Bereit zum Verbinden.", "info");
});
