export function initPwa(installButton) {
    let deferredInstallPrompt = null;

    window.addEventListener("beforeinstallprompt", event => {
        event.preventDefault();
        deferredInstallPrompt = event;
        installButton.style.display = "block";
    });

    installButton.addEventListener("click", async () => {
        if (!deferredInstallPrompt) {
            return;
        }

        deferredInstallPrompt.prompt();
        await deferredInstallPrompt.userChoice;

        deferredInstallPrompt = null;
        installButton.style.display = "none";
    });

    window.addEventListener("appinstalled", () => {
        installButton.style.display = "none";
    });
}

export async function registerServiceWorker() {
    if (!("serviceWorker" in navigator)) {
        return;
    }

    try {
        await navigator.serviceWorker.register("./sw.js");
        console.log("Service Worker registriert");
    } catch (error) {
        console.error("Service Worker Fehler:", error);
    }
}
