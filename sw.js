const CACHE_NAME =
    "outofoffice-v6";

const APP_FILES = [
    "./",
    "./index.html",
    "./manifest.json",
    "./assets/icon.svg",
    "./assets/kaulketh-maker-projects.png"
];


self.addEventListener(
    "install",
    event => {

        event.waitUntil(
            caches
                .open(CACHE_NAME)
                .then(
                    cache =>
                        cache.addAll(
                            APP_FILES
                        )
                )
        );

        self.skipWaiting();
    }
);


self.addEventListener(
    "activate",
    event => {

        event.waitUntil(
            caches
                .keys()
                .then(
                    names =>
                        Promise.all(
                            names
                                .filter(
                                    name =>
                                        name !==
                                        CACHE_NAME
                                )
                                .map(
                                    name =>
                                        caches.delete(
                                            name
                                        )
                                )
                        )
                )
        );

        self.clients.claim();
    }
);


self.addEventListener(
    "fetch",
    event => {

        if (
            event.request.method !==
            "GET"
        ) {
            return;
        }


        const requestUrl =
            new URL(
                event.request.url
            );


        if (
            requestUrl.origin !==
            self.location.origin
        ) {
            return;
        }


        /*
         * Network first:
         *
         * Bei GitHub-Pages-Updates bekommen wir
         * möglichst sofort die neue Version.
         *
         * Ohne Internet fällt die App auf den
         * lokalen Cache zurück.
         */

        event.respondWith(
            fetch(
                event.request
            )
                .then(
                    response => {

                        const copy =
                            response.clone();


                        caches
                            .open(
                                CACHE_NAME
                            )
                            .then(
                                cache =>
                                    cache.put(
                                        event.request,
                                        copy
                                    )
                            );


                        return response;
                    }
                )
                .catch(
                    () =>
                        caches.match(
                            event.request
                        )
                )
        );
    }
);
