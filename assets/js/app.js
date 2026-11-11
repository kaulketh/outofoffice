"use strict";


// ============================================================
// Nordic UART Service
// ============================================================

const NUS_SERVICE =
    "6e400001-b5a3-f393-e0a9-e50e24dcca9e";

const NUS_RX =
    "6e400002-b5a3-f393-e0a9-e50e24dcca9e";

const NUS_TX =
    "6e400003-b5a3-f393-e0a9-e50e24dcca9e";


// ============================================================
// BLE
// ============================================================

let device = null;
let server = null;

let rxCharacteristic = null;
let txCharacteristic = null;


// ============================================================
// Daten
// ============================================================

let roomNumber = "";

let organization = "";
let department = "";

let batteryVoltage = null;
let batteryPercent = null;

let notificationBuffer = "";

let refreshAfterUpdate = false;

const people = new Map();

const decoder = new TextDecoder();
const encoder = new TextEncoder();


// ============================================================
// DOM
// ============================================================

const connectButton =
    document.getElementById("connectButton");

const connectionDot =
    document.getElementById("connectionDot");

const message =
    document.getElementById("message");

const batteryPanel =
    document.getElementById("batteryPanel");

const batteryValue =
    document.getElementById("batteryValue");

const batteryBar =
    document.getElementById("batteryBar");

const batteryWarning =
    document.getElementById("batteryWarning");

const lastConnected = document.getElementById("lastConnected");

const ownerCompactPanel =
    document.getElementById("ownerCompactPanel");

const ownerCompactName =
    document.getElementById("ownerCompactName");

const changeOwnerButton =
    document.getElementById("changeOwnerButton");


const ownerSetupPanel =
    document.getElementById("ownerSetupPanel");

const ownerSelect =
    document.getElementById("ownerSelect");

const ownerDoneButton =
    document.getElementById("ownerDoneButton");

const ownerCancelButton =
    document.getElementById("ownerCancelButton");


const personsPanel =
    document.getElementById("personsPanel");

const personsContainer =
    document.getElementById("persons");


const installButton =
    document.getElementById("installButton");


// ============================================================
// Meldung
// ============================================================

function setMessage(
    text,
    type = "info") {
    message.textContent = text;
    message.className = type;
}


// ============================================================
// Akku
// ============================================================

function renderBattery() {
    if (
        batteryVoltage === null ||
        batteryPercent === null
    ) {
        batteryPanel.style.display =
            "none";

        return;
    }


    batteryPanel.style.display =
        "block";


    batteryValue.textContent =
        batteryVoltage.toFixed(2) +
        " V · ca. " +
        batteryPercent +
        " %";


    const percent =
        Math.max(
            0,
            Math.min(
                100,
                batteryPercent
            )
        );


    batteryBar.style.width =
        percent + "%";


    batteryBar.classList.remove(
        "warning",
        "low"
    );


    batteryWarning.classList.remove(
        "warning",
        "critical"
    );

    batteryWarning.style.display =
        "none";


    // --------------------------------------------------------
    // Bewertung anhand der realen Spannung
    // --------------------------------------------------------

    if (batteryVoltage < 3.60) {

        batteryBar.classList.add(
            "low"
        );


        batteryWarning.textContent =
            "Akku bald laden.";

        batteryWarning.classList.add(
            "critical"
        );

        batteryWarning.style.display =
            "block";

    } else if (batteryVoltage < 3.80) {

        batteryBar.classList.add(
            "warning"
        );


        batteryWarning.textContent =
            "Akkustand niedrig – bei Gelegenheit laden.";

        batteryWarning.classList.add(
            "warning"
        );

        batteryWarning.style.display =
            "block";
    }
}


function processBattery(fields) {
    if (
        fields.length < 2 ||
        fields[1] === "NA"
    ) {
        batteryVoltage = null;
        batteryPercent = null;

        batteryPanel.style.display =
            "none";

        return;
    }


    const voltage =
        Number(fields[1]);

    const percent =
        Number(fields[2]);


    if (
        !Number.isFinite(voltage) ||
        !Number.isFinite(percent)
    ) {
        return;
    }


    batteryVoltage = voltage;
    batteryPercent = percent;

    renderBattery();
}


// ============================================================
// BLE verbinden
// ============================================================

async function connect() {
    if (!navigator.bluetooth) {

        setMessage(
            "Web Bluetooth wird von diesem Browser nicht unterstützt.",
            "error"
        );

        return;
    }


    try {

        setMessage(
            "Raumschild auswählen …",
            "info"
        );


        device =
            await navigator.bluetooth.requestDevice({

                filters: [
                    {
                        namePrefix:
                            "RAUMSCHILD-"
                    }
                ],

                optionalServices: [
                    NUS_SERVICE
                ]
            });


        device.addEventListener(
            "gattserverdisconnected",
            handleDisconnected
        );


        setMessage(
            "Verbinde …",
            "info"
        );


        server =
            await device.gatt.connect();


        const service =
            await server.getPrimaryService(
                NUS_SERVICE
            );


        rxCharacteristic =
            await service.getCharacteristic(
                NUS_RX
            );


        txCharacteristic =
            await service.getCharacteristic(
                NUS_TX
            );


        await txCharacteristic
            .startNotifications();


        txCharacteristic.addEventListener(
            "characteristicvaluechanged",
            handleNotification
        );


        connectionDot.classList.add(
            "connected"
        );


        connectButton.textContent =
            "Verbunden";


        connectButton.disabled =
            true;


        resetRemoteData();


        setMessage(
            "Verbunden – lese Konfiguration …",
            "info"
        );


        await sendCommand(
            "INFO"
        );

    } catch (error) {

        if (error.name === "NotFoundError") {

            setMessage(
                "Keine Verbindung hergestellt.",
                "info"
            );

            return;
        }


        console.error(error);


        setMessage(
            "Verbindung fehlgeschlagen: " +
            error.message,
            "error"
        );
    }
}


// ============================================================
// Daten zurücksetzen
// ============================================================

function resetRemoteData() {
    people.clear();

    roomNumber = "";
    organization = "";
    department = "";

    batteryVoltage = null;
    batteryPercent = null;

    notificationBuffer = "";


    batteryPanel.style.display =
        "none";


    ownerCompactPanel.style.display =
        "none";


    ownerSetupPanel.style.display =
        "none";


    personsPanel.style.display =
        "none";
}


// ============================================================
// Disconnect
// ============================================================

function handleDisconnected() {
    connectionDot.classList.remove(
        "connected"
    );


    connectButton.textContent =
        "Raumschild verbinden";


    connectButton.disabled =
        false;


    setMessage(
        "Bluetooth-Verbindung getrennt.",
        "error"
    );


    rxCharacteristic = null;
    txCharacteristic = null;
    server = null;
}


// ============================================================
// BLE senden
// ============================================================

async function sendCommand(command) {
    if (!rxCharacteristic) {

        throw new Error(
            "Keine Bluetooth-Verbindung"
        );
    }


    const data =
        encoder.encode(command);


    const CHUNK_SIZE = 18;


    for (
        let offset = 0;
        offset < data.length;
        offset += CHUNK_SIZE
    ) {

        const chunk =
            data.slice(
                offset,
                offset + CHUNK_SIZE
            );


        if (
            rxCharacteristic
                .properties
                .writeWithoutResponse
        ) {

            await rxCharacteristic
                .writeValueWithoutResponse(
                    chunk
                );

        } else {

            await rxCharacteristic
                .writeValueWithResponse(
                    chunk
                );
        }


        await delay(12);
    }
}


// ============================================================
// BLE Notifications
// ============================================================

function handleNotification(event) {
    const value =
        event.target.value;


    const bytes =
        new Uint8Array(
            value.buffer,
            value.byteOffset,
            value.byteLength
        );


    notificationBuffer +=
        decoder.decode(
            bytes,
            {
                stream: true
            }
        );


    while (
        notificationBuffer.includes("\n")
        ) {

        const position =
            notificationBuffer.indexOf(
                "\n"
            );


        let line =
            notificationBuffer.slice(
                0,
                position
            );


        notificationBuffer =
            notificationBuffer.slice(
                position + 1
            );


        line =
            line
                .replace(/\r$/, "")
                .trim();


        if (line.length > 0) {
            processLine(line);
        }
    }
}


// ============================================================
// Protokoll
// ============================================================

function processLine(line) {
    console.log(
        "Raumschild:",
        line
    );


    if (line === "INFO_BEGIN") {

        setMessage(
            "Lese Konfiguration …",
            "info"
        );

        return;
    }


    if (line === "INFO_END") {

        setMessage(
            "Konfiguration gelesen – lese Status …",
            "info"
        );


        sendCommand("GET")
            .catch(handleCommandError);

        return;
    }


    if (line === "STATE_BEGIN") {
        return;
    }


    if (line === "STATE_END") {
        const now =
            new Date();


        lastConnected.textContent =
            "Zuletzt verbunden: " +
            now.toLocaleTimeString(
                "de-DE",
                {
                    hour: "2-digit",
                    minute: "2-digit"
                }
            );
        rebuildOwnerSelect();
        updateOwnerDisplay();
        renderPeople();


        personsPanel.style.display =
            "block";


        if (refreshAfterUpdate) {

            setMessage(
                "Schild-Inhalt wurde aktualisiert.",
                "ok"
            );

            refreshAfterUpdate =
                false;

        } else {

            setMessage(
                "Schild-Inhalt vollständig geladen.",
                "ok"
            );
        }


        setUpdateButtons(true);

        return;
    }


    const fields =
        line.split("|");


    switch (fields[0]) {

        case "ROOM":

            roomNumber =
                fields
                    .slice(1)
                    .join("|");


            document
                .getElementById("title")
                .textContent =
                "BG Raum " +
                roomNumber;

            break;


        case "ORG":

            organization =
                fields
                    .slice(1)
                    .join("|");

            document
                .getElementById("organizationTop")
                .textContent =
                    organization;

            break;


        case "DEPT":

            department =
                fields
                    .slice(1)
                    .join("|");

            document
                .getElementById("departmentTop")
                .textContent =
                department;

            break;


        case "PERSON":

            ensurePerson(
                fields[1],
                fields
                    .slice(2)
                    .join("|")
            );

            break;


        case "BAT":

            processBattery(fields);
            break;


        case "STATE":

            processState(fields);
            break;


        case "OK":

            setMessage(
                "Änderung übernommen.",
                "ok"
            );

            break;


        case "REFRESHING":

            setMessage(
                "E-Paper wird aktualisiert …",
                "info"
            );

            setUpdateButtons(false);

            break;


        case "READY":

            refreshAfterUpdate =
                true;


            sendCommand("GET")
                .catch(handleCommandError);

            break;


        case "ERR":

            setMessage(
                "Raumschild meldet Fehler: " +
                fields
                    .slice(1)
                    .join(" "),
                "error"
            );


            setUpdateButtons(true);

            break;
    }
}


// ============================================================
// Person
// ============================================================

function ensurePerson(
    id,
    name) {
    if (!people.has(id)) {

        people.set(
            id,
            {
                id: id,
                name: name,
                status: "",
                text: ""
            }
        );

    } else if (name) {

        people.get(id).name =
            name;
    }
}


function processState(fields) {
    const id =
        fields[1];

    const name =
        fields[2] ?? "";

    const status =
        fields[3] ?? "";

    const text =
        fields
            .slice(4)
            .join("|");


    ensurePerson(
        id,
        name
    );


    const person =
        people.get(id);


    person.status =
        status;

    person.text =
        text;
}


// ============================================================
// Nutzer
// ============================================================

function ownerStorageKey() {
    return (
        "raumschild-owner-" +
        (
            roomNumber ||
            "default"
        )
    );
}


function getPreferredPerson() {
    return (
        localStorage.getItem(
            ownerStorageKey()
        ) || ""
    );
}


function setPreferredPerson(id) {
    if (id) {

        localStorage.setItem(
            ownerStorageKey(),
            id
        );

    } else {

        localStorage.removeItem(
            ownerStorageKey()
        );
    }
}


function rebuildOwnerSelect() {
    const selected =
        getPreferredPerson();


    ownerSelect.innerHTML =
        '<option value="">Bitte auswählen …</option>';


    for (
        const person
        of people.values()
        ) {

        const option =
            document.createElement(
                "option"
            );


        option.value =
            person.id;

        option.textContent =
            person.name;


        ownerSelect.appendChild(
            option
        );
    }


    if (
        selected &&
        people.has(selected)
    ) {

        ownerSelect.value =
            selected;
    }
}


function updateOwnerDisplay() {
    const preferred =
        getPreferredPerson();


    if (
        preferred &&
        people.has(preferred)
    ) {

        ownerCompactName.textContent =
            people.get(preferred).name;


        ownerCompactPanel.style.display =
            "block";


        ownerSetupPanel.style.display =
            "none";

    } else {

        ownerCompactPanel.style.display =
            "none";


        ownerSetupPanel.style.display =
            "block";


        ownerCancelButton.style.display =
            "none";
    }
}


changeOwnerButton.addEventListener(
    "click",
    () => {

        const preferred =
            getPreferredPerson();


        ownerSelect.value =
            preferred;


        ownerCompactPanel.style.display =
            "none";


        ownerSetupPanel.style.display =
            "block";


        ownerCancelButton.style.display =
            preferred
                ? "inline-block"
                : "none";
    }
);


ownerDoneButton.addEventListener(
    "click",
    () => {

        const selected =
            ownerSelect.value;


        if (!selected) {

            setMessage(
                "Bitte zuerst einen Nutzer auswählen.",
                "error"
            );

            return;
        }


        setPreferredPerson(
            selected
        );


        updateOwnerDisplay();
        renderPeople();
    }
);


ownerCancelButton.addEventListener(
    "click",
    () => {

        rebuildOwnerSelect();
        updateOwnerDisplay();
    }
);


// ============================================================
// Personen darstellen
// ============================================================

function renderPeople() {
    personsContainer.innerHTML =
        "";


    const preferred =
        getPreferredPerson();


    let list =
        Array.from(
            people.values()
        );


    if (preferred) {

        list.sort(
            (a, b) => {

                if (
                    a.id === preferred
                ) {
                    return -1;
                }


                if (
                    b.id === preferred
                ) {
                    return 1;
                }


                return 0;
            }
        );
    }


    for (
        const person
        of list
        ) {

        const details =
            document.createElement(
                "details"
            );


        details.className =
            "person";


        if (
            !preferred ||
            person.id === preferred
        ) {

            details.open =
                true;
        }


        const summary =
            document.createElement(
                "summary"
            );


        summary.textContent =
            person.name;


        const body =
            document.createElement(
                "div"
            );


        body.className =
            "person-body";


        const current =
            document.createElement(
                "div"
            );


        current.className =
            "person-status";


        current.textContent =
            currentStatusText(
                person
            );


        body.innerHTML = `

        <label>
            Status
        </label>

        <input
            id="status-${person.id}"
            maxlength="48"
            autocomplete="on"
        >


        <label>
            Zusatztext
        </label>

        <textarea
            id="text-${person.id}"
            maxlength="120"
            autocomplete="on"
            placeholder="Freitext"
        ></textarea>


        <button
            class="update-button"
            data-person="${person.id}"
        >
            ${person.name} aktualisieren
        </button>
    `;


        body.prepend(
            current
        );


        details.appendChild(
            summary
        );


        details.appendChild(
            body
        );


        personsContainer.appendChild(
            details
        );


        document
            .getElementById(
                "status-" +
                person.id
            )
            .value =
            person.status;


        document
            .getElementById(
                "text-" +
                person.id
            )
            .value =
            person.text;
    }


    document
        .querySelectorAll(
            ".update-button"
        )
        .forEach(
            button => {

                button.addEventListener(
                    "click",
                    () =>
                        updatePerson(
                            button
                                .dataset
                                .person
                        )
                );
            }
        );
}


function currentStatusText(person) {
    let result =
        "Aktuell: " +
        (
            person.status ||
            "–"
        );


    if (person.text) {

        result +=
            " – " +
            person.text;
    }


    return result;
}


// ============================================================
// Person aktualisieren
// ============================================================

async function updatePerson(id) {
    const person =
        people.get(id);


    if (!person) {
        return;
    }


    let status =
        document
            .getElementById(
                "status-" + id
            )
            .value
            .trim();


    let text =
        document
            .getElementById(
                "text-" + id
            )
            .value
            .trim();


    if (
        status.includes("|") ||
        text.includes("|")
    ) {

        setMessage(
            'Das Zeichen "|" kann nicht verwendet werden.',
            "error"
        );

        return;
    }


    status =
        status.replace(
            /[\r\n]+/g,
            " "
        );


    text =
        text.replace(
            /[\r\n]+/g,
            " "
        );


    status =
        normalizeGerman(status);

    text =
        normalizeGerman(text);


    const command =
        `SET|${id}|${status}|${text}`;


    try {

        setUpdateButtons(false);


        setMessage(
            "Sende Änderung …",
            "info"
        );


        await sendCommand(
            command
        );

    } catch (error) {

        handleCommandError(
            error
        );
    }
}


// ============================================================
// Sonderzeichen
// ============================================================

function normalizeGerman(text) {
    return text

        .replaceAll("ä", "ae")
        .replaceAll("ö", "oe")
        .replaceAll("ü", "ue")
        .replaceAll("Ä", "Ae")
        .replaceAll("Ö", "Oe")
        .replaceAll("Ü", "Ue")
        .replaceAll("ß", "ss");
}


// ============================================================
// Buttons / Fehler
// ============================================================

function setUpdateButtons(enabled) {
    document
        .querySelectorAll(
            ".update-button"
        )
        .forEach(
            button => {

                button.disabled =
                    !enabled;
            }
        );
}


function handleCommandError(error) {
    console.error(error);


    setMessage(
        "Bluetooth-Fehler: " +
        error.message,
        "error"
    );


    setUpdateButtons(true);
}


function delay(ms) {
    return new Promise(
        resolve =>
            setTimeout(
                resolve,
                ms
            )
    );
}


// ============================================================
// PWA Installation
// ============================================================

let deferredInstallPrompt =
    null;


window.addEventListener(
    "beforeinstallprompt",
    event => {

        event.preventDefault();


        deferredInstallPrompt =
            event;


        installButton.style.display =
            "block";
    }
);


installButton.addEventListener(
    "click",
    async () => {

        if (!deferredInstallPrompt) {
            return;
        }


        deferredInstallPrompt.prompt();


        await deferredInstallPrompt
            .userChoice;


        deferredInstallPrompt =
            null;


        installButton.style.display =
            "none";
    }
);


window.addEventListener(
    "appinstalled",
    () => {

        installButton.style.display =
            "none";
    }
);


// ============================================================
// Service Worker
// ============================================================

async function registerServiceWorker() {
    if (!("serviceWorker" in navigator)) {
        return;
    }


    try {

        await navigator.serviceWorker
            .register("./sw.js");


        console.log(
            "Service Worker registriert"
        );

    } catch (error) {

        console.error(
            "Service Worker Fehler:",
            error
        );
    }
}


// ============================================================
// Start
// ============================================================

connectButton.addEventListener(
    "click",
    connect
);


window.addEventListener(
    "load",
    async () => {

        await registerServiceWorker();


        setMessage(
            "Bereit zum Verbinden.",
            "info"
        );
    }
);
