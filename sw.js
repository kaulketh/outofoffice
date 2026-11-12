const CACHE_PREFIX = "outofoffice-";
const CACHE_NAME = `${CACHE_PREFIX}V16.1`;

const APP_FILES = [
    "./",
    "./index.html",
    "./manifest.json",
    "./css/base.css",
    "./css/components.css",
    "./css/themes.css",

    "./js/main.js",

    "./assets/icon.svg",
    "./assets/kaulketh-maker-projects.png",

    "./js/pwa.js",
    "./js/battery.js",
    "./js/state.js",
    "./js/people.js",
    "./js/ble.js",
    "./js/protocol.js"
];


self.addEventListener("install", event => {
        event.waitUntil(caches.open(CACHE_NAME).then(cache =>
            cache.addAll(APP_FILES)));
        self.skipWaiting();
    }
);

self.addEventListener("activate", event => {
        event.waitUntil(caches.keys().then(names =>
                Promise.all(
                    names
                        .filter(name =>
                            name.startsWith(CACHE_PREFIX) && name !== CACHE_NAME)
                        .map(name =>
                            caches.delete(name)
                        )
                )
            )
        );
        self.clients.claim();
    }
);


self.addEventListener("fetch", event => {
        if (event.request.method !== "GET") {
            return;
        }

        const requestUrl = new URL(event.request.url);

        if (requestUrl.origin !== self.location.origin) {
            return;
        }

        /*
         * Network first:
         *
         * Bei GitHub-Pages-Updates bekommt man
         * möglichst sofort die neue Version.
         * Ohne Internet fällt die App auf den
         * lokalen Cache zurück.
         */

        event.respondWith(fetch(event.request).then(response => {
                if (response.ok) {
                    const copy = response.clone();
                    caches.open(CACHE_NAME).then(cache =>
                        cache.put(event.request, copy));
                }
                return response;
            }
        )
            .catch(() => caches.match(event.request)));
    }
);
