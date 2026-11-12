
import { room, people } from "./state.js";

const ownerCompactPanel = document.getElementById("ownerCompactPanel");
const ownerCompactName = document.getElementById("ownerCompactName");
const changeOwnerButton = document.getElementById("changeOwnerButton");
const ownerSetupPanel = document.getElementById("ownerSetupPanel");
const ownerSelect = document.getElementById("ownerSelect");
const ownerDoneButton = document.getElementById("ownerDoneButton");
const ownerCancelButton = document.getElementById("ownerCancelButton");
const personsPanel = document.getElementById("personsPanel");
const personsContainer = document.getElementById("persons");

let services;

export function initPeople(api) {
    services = api;

    changeOwnerButton.addEventListener("click", () => {
        const preferred = getPreferredPerson();
        ownerSelect.value = preferred;
        ownerCompactPanel.style.display = "none";
        ownerSetupPanel.style.display = "block";
        ownerCancelButton.style.display = preferred ? "inline-block" : "none";
    });

    ownerDoneButton.addEventListener("click", () => {
        const selected = ownerSelect.value;

        if (!selected) {
            services.setMessage("Bitte zuerst einen Nutzer auswählen.", "error");
            return;
        }

        setPreferredPerson(selected);
        updateOwnerDisplay();
        renderPeople();
    });

    ownerCancelButton.addEventListener("click", () => {
        rebuildOwnerSelect();
        updateOwnerDisplay();
    });
}

export function ensurePerson(id, name) {
    if (!people.has(id)) {
        people.set(id, { id, name, status: "", text: "" });
    } else if (name) {
        people.get(id).name = name;
    }
}

export function processState(fields) {
    const id = fields[1];
    const name = fields[2] ?? "";
    const status = fields[3] ?? "";
    const text = fields.slice(4).join("|");

    ensurePerson(id, name);

    const person = people.get(id);
    person.status = status;
    person.text = text;
}

function ownerStorageKey() {
    return "raumschild-owner-" + (room.number || "default");
}

function getPreferredPerson() {
    return localStorage.getItem(ownerStorageKey()) || "";
}

function setPreferredPerson(id) {
    if (id) {
        localStorage.setItem(ownerStorageKey(), id);
    } else {
        localStorage.removeItem(ownerStorageKey());
    }
}

export function rebuildOwnerSelect() {
    const selected = getPreferredPerson();
    ownerSelect.innerHTML = '<option value="">Bitte auswählen …</option>';

    for (const person of people.values()) {
        const option = document.createElement("option");
        option.value = person.id;
        option.textContent = person.name;
        ownerSelect.appendChild(option);
    }

    if (selected && people.has(selected)) {
        ownerSelect.value = selected;
    }
}

export function updateOwnerDisplay() {
    const preferred = getPreferredPerson();

    if (preferred && people.has(preferred)) {
        ownerCompactName.textContent = people.get(preferred).name;
        ownerCompactPanel.style.display = "block";
        ownerSetupPanel.style.display = "none";
    } else {
        ownerCompactPanel.style.display = "none";
        ownerSetupPanel.style.display = "block";
        ownerCancelButton.style.display = "none";
    }
}

export function renderPeople() {
    personsContainer.innerHTML = "";

    const preferred = getPreferredPerson();
    const list = Array.from(people.values());

    if (preferred) {
        list.sort((a, b) => {
            if (a.id === preferred) return -1;
            if (b.id === preferred) return 1;
            return 0;
        });
    }

    for (const person of list) {
        const details = document.createElement("details");
        details.className = "person";
        details.open = !preferred || person.id === preferred;

        const summary = document.createElement("summary");
        summary.textContent = person.name;

        const body = document.createElement("div");
        body.className = "person-body";

        const current = document.createElement("div");
        current.className = "person-status";
        current.textContent = currentStatusText(person);

        const statusLabel = document.createElement("label");
        statusLabel.textContent = "Status";

        const statusInput = document.createElement("input");
        statusInput.id = `status-${person.id}`;
        statusInput.maxLength = 48;
        statusInput.autocomplete = "on";
        statusInput.value = person.status;

        const textLabel = document.createElement("label");
        textLabel.textContent = "Zusatztext";

        const textInput = document.createElement("textarea");
        textInput.id = `text-${person.id}`;
        textInput.maxLength = 120;
        textInput.autocomplete = "on";
        textInput.placeholder = "Freitext";
        textInput.value = person.text;

        const button = document.createElement("button");
        button.className = "update-button";
        button.dataset.person = person.id;
        button.textContent = `${person.name} aktualisieren`;
        button.addEventListener("click", () => updatePerson(person.id));

        body.append(
            current,
            statusLabel,
            statusInput,
            textLabel,
            textInput,
            button
        );

        details.append(summary, body);
        personsContainer.appendChild(details);
    }
}

function currentStatusText(person) {
    let result = "Aktuell: " + (person.status || "–");

    if (person.text) {
        result += " – " + person.text;
    }

    return result;
}

async function updatePerson(id) {
    if (!people.has(id)) {
        return;
    }

    let status = document.getElementById(`status-${id}`).value.trim();
    let text = document.getElementById(`text-${id}`).value.trim();

    if (status.includes("|") || text.includes("|")) {
        services.setMessage('Das Zeichen "|" kann nicht verwendet werden.', "error");
        return;
    }

    status = normalizeGerman(status.replace(/[\r\n]+/g, " "));
    text = normalizeGerman(text.replace(/[\r\n]+/g, " "));

    const command = `SET|${id}|${status}|${text}`;

    try {
        setUpdateButtons(false);
        services.setMessage("Sende Änderung …", "info");
        await services.sendCommand(command);
    } catch (error) {
        services.handleCommandError(error);
    }
}

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

export function setUpdateButtons(enabled) {
    document.querySelectorAll(".update-button").forEach(button => {
        button.disabled = !enabled;
    });
}

export function hidePeoplePanels() {
    ownerCompactPanel.style.display = "none";
    ownerSetupPanel.style.display = "none";
    personsPanel.style.display = "none";
}

export function showPeoplePanel() {
    personsPanel.style.display = "block";
}
