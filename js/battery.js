const batteryPanel = document.getElementById("batteryPanel");
const batteryValue = document.getElementById("batteryValue");
const batteryBar = document.getElementById("batteryBar");
const batteryWarning = document.getElementById("batteryWarning");

let batteryVoltage = null;
let batteryPercent = null;

export function processBattery(fields) {
    if (fields.length < 2 || fields[1] === "NA") {
        resetBattery();
        return;
    }

    const voltage = Number(fields[1]);
    const percent = Number(fields[2]);

    if (!Number.isFinite(voltage) || !Number.isFinite(percent)) {
        return;
    }

    batteryVoltage = voltage;
    batteryPercent = percent;
    renderBattery();
}

export function resetBattery() {
    batteryVoltage = null;
    batteryPercent = null;
    renderBattery();
}

function renderBattery() {
    if (batteryVoltage === null || batteryPercent === null) {
        batteryPanel.style.display = "none";
        return;
    }

    batteryPanel.style.display = "block";
    batteryValue.textContent =
        `${batteryVoltage.toFixed(2)} V · ca. ${batteryPercent} %`;

    const percent = Math.max(0, Math.min(100, batteryPercent));
    batteryBar.style.width = `${percent}%`;
    batteryBar.classList.remove("warning", "low");

    batteryWarning.classList.remove("warning", "critical");
    batteryWarning.style.display = "none";

    if (batteryVoltage < 3.60) {
        batteryBar.classList.add("low");
        batteryWarning.textContent = "Akku bald laden.";
        batteryWarning.classList.add("critical");
        batteryWarning.style.display = "block";
    } else if (batteryVoltage < 3.80) {
        batteryBar.classList.add("warning");
        batteryWarning.textContent =
            "Akkustand niedrig – bei Gelegenheit laden.";
        batteryWarning.classList.add("warning");
        batteryWarning.style.display = "block";
    }
}
