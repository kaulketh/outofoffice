import {room} from "./state.js";
import {
    abortUpdates,
    completeUpdate,
    ensurePerson,
    processState,
    rebuildOwnerSelect,
    renderPeople,
    setUpdateButtons,
    showPeoplePanel,
    updateOwnerDisplay,
    processTransactionAck
} from "./people.js";
import {processBattery} from "./battery.js";
import {sendCommand} from "./ble.js";

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
        console.log("OutOfOffice:", line);

        if (line === "INFO_BEGIN") {
            return;
        }

        if (line === "INFO_END") {
            requestState();
            return;
        }

        if (line === "STATE_BEGIN") {
            return;
        }

        if (line === "STATE_END") {
            const wasUpdate = refreshAfterUpdate;
            refreshAfterUpdate = false;

            onStateComplete();
            rebuildOwnerSelect();
            updateOwnerDisplay();
            renderPeople();
            showPeoplePanel();

            if (wasUpdate) {
                completeUpdate();
            } else {
                setMessage("");
            }

            setUpdateButtons(true);
            return;
        }

        const fields = line.split("|");

        switch (fields[0]) {
            case "CAP":
                if (fields[1] === "TRANSACTION") {
                    room.transactionSupported = true;
                }
                break;

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
                if (processTransactionAck(line)) {
                    break;
                }

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
                refreshAfterUpdate = false;
                abortUpdates(room.transactionSupported);

                setMessage(
                    "OutOfOffice meldet Fehler: " +
                    fields.slice(1).join(" "),
                    "error"
                );
                break;
        }
    }

    function resetProtocol() {
        refreshAfterUpdate = false;
    }

    return {processLine, resetProtocol};
}
