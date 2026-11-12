
import { room } from "./state.js";
import {
    ensurePerson,
    processState,
    rebuildOwnerSelect,
    updateOwnerDisplay,
    renderPeople,
    setUpdateButtons,
    showPeoplePanel
} from "./people.js";
import { processBattery } from "./battery.js";
import { sendCommand } from "./ble.js";

export function createProtocol({
    setMessage,
    handleCommandError,
    onRoom,
    onOrganization,
    onDepartment,
    onStateComplete
}) {
    let refreshAfterUpdate = false;

    function requestState() {
        sendCommand("GET").catch(handleCommandError);
    }

    function processLine(line) {
        console.log("Raumschild:", line);

        if (line === "INFO_BEGIN") {
            setMessage("Lese Konfiguration …", "info");
            return;
        }

        if (line === "INFO_END") {
            setMessage("Konfiguration gelesen – lese Status …", "info");
            requestState();
            return;
        }

        if (line === "STATE_BEGIN") {
            return;
        }

        if (line === "STATE_END") {
            onStateComplete();
            rebuildOwnerSelect();
            updateOwnerDisplay();
            renderPeople();
            showPeoplePanel();

            const message = refreshAfterUpdate
                ? "Schild-Inhalt wurde aktualisiert."
                : "Schild-Inhalt vollständig geladen.";

            setMessage(message, "ok");
            refreshAfterUpdate = false;
            setUpdateButtons(true);
            return;
        }

        const fields = line.split("|");

        switch (fields[0]) {
            case "ROOM":
                room.number = fields.slice(1).join("|");
                onRoom(room.number);
                break;

            case "ORG":
                room.organization = fields.slice(1).join("|");
                onOrganization(room.organization);
                break;

            case "DEPT":
                room.department = fields.slice(1).join("|");
                onDepartment(room.department);
                break;

            case "PERSON":
                ensurePerson(fields[1], fields.slice(2).join("|"));
                break;

            case "BAT":
                processBattery(fields);
                break;

            case "STATE":
                processState(fields);
                break;

            case "OK":
                setMessage("Änderung übernommen.", "ok");
                break;

            case "REFRESHING":
                setMessage("E-Paper wird aktualisiert …", "info");
                setUpdateButtons(false);
                break;

            case "READY":
                refreshAfterUpdate = true;
                requestState();
                break;

            case "ERR":
                setMessage(
                    "Raumschild meldet Fehler: " + fields.slice(1).join(" "),
                    "error"
                );
                setUpdateButtons(true);
                break;
        }
    }

    function resetProtocol() {
        refreshAfterUpdate = false;
    }

    return { processLine, resetProtocol };
}
