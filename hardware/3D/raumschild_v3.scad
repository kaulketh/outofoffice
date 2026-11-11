// DWD RAUMSCHILD - Vollstaendige OpenSCAD-Konstruktion
// Version: LiPo-Aufnahme V1 auf Basis des Customizer-Stands, 2026-11-10
// OpenSCAD 2021.01 kompatibel. Alle Groessen in mm.
// Von aussen keine Befestigungsbohrungen (Standard).
// Parameterwerte: Customizer-Preset aktuell_ohne_lipo (2026-11-09).


/* [Abmessungen] */
breite = 158;
tiefe  = 158;
hoehe  = 25;

/* [Wand] */
wandstaerke = 2;

/* [Ecken] */
radius = 12;

/* [Oberer Innenrand] */
rand_oben_breite = 5.5;
rand_oben_hoehe  = 0.5;

/* [Unterer Abschluss] */
// 0 = Offen
// 1 = Geschlossener Boden
// 2 = Umlaufender Innenrand
unten = 2;

/* [Boden] */
bodenstaerke = 1.5;

/* [Unterer Innenrand] */
rand_unten_breite = 0.5;
rand_unten_hoehe  = 0.5;


/* [E-Paper-Adapter] */

adapter_zapfen_aktiv = true;

// Bestätigtes Lochbild
adapter_lochabstand_x = 24.5;
adapter_lochabstand_y = 10.5;

// Zapfen
adapter_zapfen_d = 2.7;
adapter_zapfen_h = 2.5;

// Abstandshalter
adapter_sockel_d = 5;
adapter_sockel_h = 2.5;

// Position
// abstand zum Display verkuerzt wegen kuerzerem Flachkabel
adapter_x = 0;
adapter_y = 50;

/* [Montageplatte] */
platte_aktiv = true;

platte_breite = 153;
platte_hoehe = 98;
platte_dicke = 2;
platte_radius = 12;

// Abstand zum oberen Displayrand
platte_abstand_flex = 0;

/* [Montageplattenbefestigung] */

// Verdeckte Steckbefestigung: Bohrungen nur in der Montageplatte
platte_bohrungen = true;
platten_zapfen_aktiv = true;

platte_loch_d = 2.8;          // Spielbohrung fuer gedruckte Zapfen
platte_loch_x = 73;           // X-Abstand von der Plattenmitte
platte_loch_abstand_y = 8;    // Abstand von den oberen/unteren Plattenkanten

platten_zapfen_d = 2.2;
platten_zapfen_h = 3;
platten_zapfen_einbindung = 0.2; // Zapfen greift in den vorhandenen Innenrand


/* [Driver HAT - Steckhalterung] */
// Gemessen: Platine 65 x 30.2, Bohrbild 58 x 23, Bohrungen ca. 3 mm.
// Buchsenleiste und SMD-Bauteile liegen auf der Unterseite des HAT.
// Alle HAT-Befestigungen sind nur auf der INNENSEITE der Montageplatte.
hat_halterung_aktiv = true;
hat_vorschau_aktiv = true;  // Nur in "Montage"; wird nicht mitgedruckt

hat_breite = 65;
hat_hoehe = 30.2;
hat_gesamthoehe = 9.5;     // Gemessene gesamte Bauhoehe
hat_platinen_dicke = 1.6; // Nur fuer transparente Vorschau
hat_lochabstand_x = 58;
hat_lochabstand_y = 23;
hat_loch_d = 3;

// Vier Sockel an den Bohrungen, keine Schrauben oder sichtbaren Loecher.
hat_sockel_d = 5;
hat_sockel_h = 5;       // Platz fuer die 4 mm hohe Buchsenleiste + 1 mm Luft
hat_zapfen_d = 2.6;      // Als Startwert fuer ca. 3 mm Bohrungen
hat_zapfen_h = 2.1;

// X und Y beziehen sich auf die Mitte des gesamten 158-mm-Gehaeuses.
hat_x = 30;
hat_y = -8;
hat_winkel = 0;          // [0, 90, 180, 270]

// Unverbindliche Lage der Buchsenleiste fuer die Einbauvorschau.
// 51 mm entlang der langen Kante, 4.8 mm quer, 4 mm hoch.
hat_buchse_laenge = 51;
hat_buchse_breite = 4.8;
hat_buchse_hoehe = 4;
hat_buchse_rand = 2.5;   // Abstand zur nahen langen Kante, ca.

/* [XIAO nRF52840 - flache Aufnahme V2] */
// XIAO nach Entfernen der Stiftleisten. Leitungen zum HAT werden
// direkt an der Platine angeloetet; BAT-Leitungen bleiben erhalten.
// USB-C zeigt Richtung NEGATIVES Y (untere Gehaeusekante).
xiao_halterung_aktiv = true;
xiao_vorschau_aktiv = true;
xiao_breite = 18;          // Am Original nachgemessen
xiao_laenge = 21;          // Am Original bestaetigt
xiao_platinen_dicke = 1; // Noch zu pruefen

// Lage bezogen auf die Mitte des Gehaeuses.
xiao_x = 37;
xiao_y = -65;

// Z-Wert der Bauteilseite relativ zur RUECKSEITE der Montageplatte:
// Der USB-C-Stecker auf der XIAO-Bauteilseite zeigt in Richtung Platte.
// 3.9 mm Platz zwischen Leiterplatte und Montageplatte fuer USB-C.
xiao_pcb_z_oben = -3.9;

// Vier kurze seitliche Fuehrungsabschnitte (statt durchgehender
// Leisten, um die angelöteten Anschlussleitungen besser freizuhalten).
xiao_fuehrung_spiel = 0; // Pro Seite
xiao_spiel_z = 0.2;        // Luft um die Leiterplattendicke
xiao_schiene_breite = 2.2;
xiao_schiene_tiefe = 6.2;
xiao_schiene_segment = 18;
xiao_schiene_y = 0;       // Segmentmitten +/- um Boardmitte
xiao_lippe_breite = 1;
xiao_lippe_dicke = 1;
xiao_lippe_einbindung = 0.2; // Sichere Verbindung zum Seitenblock

// Zwei verdeckte Endanschlaege nehmen Druck beim USB-Einstecken auf.
xiao_stop_aktiv = true;
xiao_stop_breite = 5;
xiao_stop_spiel = 0;

// Zwei kleine Kabelfuehrungszapfen HINTER dem Board Richtung HAT.
// Leitungen bei Bedarf mit einem kleinen weichen Band sichern.
xiao_kabelfuehrung_aktiv = false;
xiao_kabelfuehrung_y = 20;  // Von der XIAO-Mitte in Richtung +Y
xiao_kabelfuehrung_abstand = 6;
xiao_kabelfuehrung_d = 3;
xiao_kabelfuehrung_h = 4;

/* [XIAO USB-C - Vorbereitung] */
// Aussparung NUR in der unteren Kappenwand. Im Preset aktiviert:
// Das Fenster wird erst nach einer realen Stecker-Passprobe aktiviert.
xiao_usb_oeffnung = true;
xiao_usb_breite = 11;    // Vorlaeufig: Platz fuer Steckergehaeuse
xiao_usb_hoehe = 5.5;    // Vorlaeufig
xiao_usb_anschluss_z = 1.55; // Hoehe USB-Gehaeuse ueber PCB-Bauteilseite

/* [LiPo 503759 - Akkuaufnahme] */
// Akku nachgemessen: 62 x 37 x 5 mm inkl. Folierung.
// Aufnahme auf der Elektronikseite der Montageplatte (-Z).
// LiPo nicht fest einpressen; spaeter mit weichem Band/Tape sichern.
akku_halterung_aktiv = true;
akku_vorschau_aktiv = true;
akku_breite = 36;               // X-Richtung
akku_laenge = 61;               // Y-Richtung
akku_dicke = 5;
akku_x = -35;                   // global: links vom HAT
akku_y = -28;                   // global: Kabelauslass nach -Y
akku_spiel_xy = 0.8;           // pro Seite
akku_wand = 1.5;               // Fuehrungswand
akku_fuehrung_hoehe = 5;       // laesst 1 mm Raum zur Akkudicke
akku_anschlag_hoehe = 5;       // stirnseitige Anschlaege
akku_kabelauslass = 30;        // freie Breite unten fuer JST-Leitung

/* [Darstellung] */
ansicht = "Montage"; // [Montage, Blende, Kappe, Clip, Platte, Platte_Druck]

$fn = 64;

/* [Display] */
display_breite = 126;
display_hoehe = 100;
display_dicke = 1.18;
display_abstand_unten = 3;

/* [Frontblende] */
// Fenster verkleinert um Abdeckung des ungenutzten Bereichs zu verbessern
blenden_dicke = 2;
fenster_breite = 116;
fenster_hoehe = 85;
taschen_spiel = 0.8;
taschen_tiefe = 1.2;

/* [Flexkabel] */
kabel_breite = 16;
kabel_laenge = 20;
kabel_tiefe = 1.2;

/* [Displayhalterung] */
halter_aktiv = false;
display_bohrungen = false;

clip_breite = 11;
clip_hoehe = 9;
clip_dicke = 1.4;

clip_x = 66;
clip_y = 35;
clip_schraube_offset = 2.5;

clip_loch_d = 2.3;
vorloch_d = 1.6;
vorloch_tiefe = 2.2;

/* [Frontbefestigung] */
front_bohrungen = false;
front_schrauben_d = 2.2;
front_schrauben_x = 74.5;
front_schrauben_y = 50;


//
// MONTAGEPLATTE
// Traeger fuer Display und Elektronik
//

// ---------- ABGELEITETE POSITIONEN ----------

// Displayposition zuerst berechnen; die Platte baut darauf auf.
display_y =
    -tiefe/2
    + display_abstand_unten
    + display_hoehe/2;

// Position relativ zum Display
platte_y_oben =
    display_y + display_hoehe/2
    - platte_abstand_flex;

platte_y =
    platte_y_oben - platte_hoehe/2;

// Direkt unter dem oberen Innenrand
platte_z =
    hoehe - rand_oben_hoehe - platte_dicke;


// Vier Sockel stehen auf der Rueckseite der Montageplatte (lokal Z < 0).
// Der HAT wird mit seiner Unterseite auf die Sockel aufgesetzt, damit
// Buchsenleiste und Bauelemente frei bleiben.
// Die Sockel liegen ausschliesslich an den HAT-Befestigungsbohrungen.
module hat_zapfenhalterung() {
    // HAT-Zentrum in den lokalen Platten-Koordinaten
    translate([hat_x, hat_y - platte_y, 0])
        rotate([0, 0, hat_winkel])
            for (sx = [-1, 1], sy = [-1, 1]) {
                translate([
                    sx * hat_lochabstand_x / 2,
                    sy * hat_lochabstand_y / 2,
                    0
                ]) {
                    // Sockel beruehrt die 2-mm-Platte um 0.2 mm.
                    translate([0, 0, -hat_sockel_h])
                        cylinder(h = hat_sockel_h + 0.2,
                                 d = hat_sockel_d);

                    // Zapfen zeigen von der Platte weg, in die HAT-Bohrungen.
                    translate([0, 0, -hat_sockel_h - hat_zapfen_h])
                        cylinder(h = hat_zapfen_h + 0.1,
                                 d = hat_zapfen_d);
                }
            }
}


// ---------- XIAO V2: STECKAUFNAHME OHNE STIFTLEISTEN ----------
// Lokale Montageplattenkoordinaten: Z=0 ist Plattenrueckseite,
// die Elektronik und die vier Fuehrungsecken zeigen in Richtung -Z.
// XIAO wird mit USB nach -Y eingeschoben; Kabel zuvor anloeten.
module xiao_einschubhalterung() {
    translate([xiao_x, xiao_y - platte_y, 0]) {
        for (sx = [-1, 1], sy = [-1, 1]) {
            // Jede Fuehrung besteht aus einem kurzen Seitenblock.
            translate([
                sx * (xiao_breite/2 + xiao_fuehrung_spiel
                      + xiao_schiene_breite/2),
                sy * xiao_schiene_y,
                -xiao_schiene_tiefe/2 + 0.1
            ])
                cube([xiao_schiene_breite,
                      xiao_schiene_segment,
                      xiao_schiene_tiefe + 0.2], center=true);

            // Obere und untere schmale Lippen, die den PCB-Rand halten.
            // Die Mitte bleibt frei fuer BAT-Pads, SMDs und Anschlussdrähte.
            for (z_lippe = [
                xiao_pcb_z_oben + xiao_spiel_z + xiao_lippe_dicke/2,
                xiao_pcb_z_oben - xiao_platinen_dicke
                    - xiao_spiel_z - xiao_lippe_dicke/2
            ])
                translate([
                    sx * (xiao_breite/2 + xiao_fuehrung_spiel
                          - xiao_lippe_breite/2
                          + xiao_lippe_einbindung),
                    sy * xiao_schiene_y,
                    z_lippe
                ])
                    cube([xiao_lippe_breite,
                          xiao_schiene_segment,
                          xiao_lippe_dicke], center=true);
        }

        // Vor dem USB-C-Port bleibt die Fuehrung offen. Die beiden
        // Endanschlaege liegen AM ANDEREN ENDE der Platine (+Y).
        if (xiao_stop_aktiv)
            for (sx = [-1, 1])
                translate([
                    sx * (xiao_breite/2 - 3),
                    xiao_laenge/2 + xiao_stop_spiel
                       + xiao_stop_breite/2,
                    -xiao_schiene_tiefe/2 + 0.1
                ])
                    cube([xiao_stop_breite,
                          xiao_stop_breite,
                          xiao_schiene_tiefe + 0.2], center=true);

        // Optionale, frei zugaengliche Anker fuer die Kabel nach oben
        // zum Driver HAT. Kein Kontakt zum e-Paper (andere Plattenseite).
        if (xiao_kabelfuehrung_aktiv)
            for (sx = [-1, 1])
                translate([
                    sx * xiao_kabelfuehrung_abstand/2,
                    xiao_kabelfuehrung_y,
                    -xiao_kabelfuehrung_h
                ])
                    cylinder(h=xiao_kabelfuehrung_h + 0.2,
                             d=xiao_kabelfuehrung_d);
    }
}

// ---------- USB-C: OPTIONALER UNTERER KAPPENAUSSCHNITT ----------
// Preset: EIN. Lage folgt der XIAO-Platine; keine Front- oder
// Seitenbohrungen. Bei der ersten physischen Passprobe nachmessen.
module xiao_usb_aussparung() {
    usb_mitte_z = platte_z + xiao_pcb_z_oben + xiao_usb_anschluss_z;
    translate([xiao_x, -tiefe/2 + wandstaerke/2, usb_mitte_z])
        cube([xiao_usb_breite, wandstaerke + 2,
              xiao_usb_hoehe], center=true);
}

// ---------- XIAO: VORSCHAU OHNE PINLEISTEN ----------
// Prozent-Objekte werden nicht mit in STL exportiert.
module xiao_platzhalter() {
    // PCB: Bauteilseite liegt in Richtung +Z, BAT-Loetpads nach -Z.
    %translate([xiao_x, xiao_y, platte_z + xiao_pcb_z_oben
                          - xiao_platinen_dicke/2])
        cube([xiao_breite, xiao_laenge,
              xiao_platinen_dicke], center=true);

    // Nur schematisch: USB-C-Buchsengehaeuse an der unteren Kante.
    %translate([xiao_x,
                xiao_y - xiao_laenge/2 + 1.5,
                platte_z + xiao_pcb_z_oben + xiao_usb_anschluss_z])
        cube([9.5, 3.0, 3.1], center=true);
}

// ---------- LiPo 503759: OFFENE AKKUAUFNAHME ----------
// Akku liegt flach auf der Rueckseite der Montageplatte, parallel
// zum Display. Zwei Laengsfuehrungen und geteilte Endanschlaege
// sichern die Position, aber NICHT gegen Herausfallen nach hinten.
// Die abschliessende Sicherung erfolgt z.B. durch ein weiches Band
// oder isolierendes Tape ueber dem Akku (nicht straff anziehen).
module akku_aufnahme() {
    translate([akku_x, akku_y - platte_y, 0]) {
        innen_b = akku_breite + 2*akku_spiel_xy;
        innen_l = akku_laenge + 2*akku_spiel_xy;
        aussen_b = innen_b + 2*akku_wand;
        // Beide Fuehrungen stehen senkrecht auf der Plattenrueckseite.
        for (sx = [-1, 1])
            translate([
                sx*(innen_b/2 + akku_wand/2),
                0,
                -akku_fuehrung_hoehe/2 + 0.1
            ])
                cube([akku_wand, innen_l + 2*akku_wand,
                      akku_fuehrung_hoehe + 0.2], center=true);

        // Oberer Endanschlag (in Richtung +Y)
        translate([0, innen_l/2 + akku_wand/2,
                   -akku_anschlag_hoehe/2 + 0.1])
            cube([aussen_b, akku_wand,
                  akku_anschlag_hoehe + 0.2], center=true);

        // Unterer Endanschlag zweigeteilt: mittig bleibt der
        // Kabelauslass in Richtung XIAO frei.
        rest = (aussen_b - akku_kabelauslass)/2;
        for (sx = [-1, 1])
            translate([
                sx*(akku_kabelauslass/2 + rest/2),
                -innen_l/2 - akku_wand/2,
                -akku_anschlag_hoehe/2 + 0.1
            ])
                cube([rest, akku_wand,
                      akku_anschlag_hoehe + 0.2], center=true);
    }
}

// Nur Vorschau: wird nicht als STL mit exportiert.
module akku_platzhalter() {
    %translate([akku_x, akku_y, platte_z - akku_dicke/2 - 0.15])
        cube([akku_breite, akku_laenge, akku_dicke], center=true);
}

module montageplatte() {
    union() {
        difference() {
            linear_extrude(height = platte_dicke)
                rundes_rechteck(platte_breite,
                                 platte_hoehe,
                                 platte_radius);

            // Nur in der INNEREN Montageplatte, nie in der Front!
            if (platte_bohrungen)
                for (sx = [-1, 1], sy = [-1, 1]) {
                    translate([
                        sx * platte_loch_x,
                        sy * (platte_hoehe/2 - platte_loch_abstand_y),
                        -0.01
                    ])
                        cylinder(h = platte_dicke + 0.02,
                                 d = platte_loch_d);
                }
        }

        if (hat_halterung_aktiv)
            hat_zapfenhalterung();

        // XIAO V2: kurze Fuehrungen auf der HAT-Seite der Platte.
        if (xiao_halterung_aktiv)
            xiao_einschubhalterung();

        // LiPo-Aufnahme auf derselben Plattenseite wie HAT und XIAO.
        if (akku_halterung_aktiv)
            akku_aufnahme();
    }
}


//
// E-PAPER-ADAPTERHALTERUNG
// Vier Zapfen auf Abstandssockeln
//

module adapter_halterung() {

    for (sx = [-1, 1], sy = [-1, 1]) {

        translate([
            adapter_x + sx * adapter_lochabstand_x/2,
            adapter_y + sy * adapter_lochabstand_y/2,
            0
        ]) {

            // Abstandssockel
            // ragt von der Blendenrueckseite
            // nach hinten ins Gehaeuse

            translate([0, 0, -adapter_sockel_h])
                cylinder(
                    h = adapter_sockel_h + 0.2,
                    d = adapter_sockel_d
                );

            // Zentrierzapfen
            translate([
                0, 0,
                -adapter_sockel_h - adapter_zapfen_h
            ])
                cylinder(
                    h = adapter_zapfen_h + 0.1,
                    d = adapter_zapfen_d
                );
        }
    }
}


// Abgerundetes Rechteck (2D)
module rundes_rechteck(x, y, r) {
    offset(r = r)
        square(
            [x - 2*r, y - 2*r],
            center = true
        );
}


// Gehäuse
module gehaeuse() {

    difference() {

        // Außenkörper
        linear_extrude(height = hoehe)
            rundes_rechteck(
                breite, tiefe, radius
            );

        // Innerer Hohlraum
        // Zwischen unterem und oberem Rand
        translate([0, 0, -0.01])
            linear_extrude(height = hoehe + 0.02)
                rundes_rechteck(
                    breite - 2*wandstaerke,
                    tiefe - 2*wandstaerke,
                    max(0.01, radius - wandstaerke)
                );

    }

    // Oberer Innenrand
    translate([0, 0, hoehe - rand_oben_hoehe])
        innenrand(
            rand_oben_breite,
            rand_oben_hoehe
        );

    // Unterer Abschluss
    if (unten == 1) {

        // Geschlossener Boden
        linear_extrude(height = bodenstaerke)
            rundes_rechteck(
                breite, tiefe, radius
            );

    } else if (unten == 2) {

        // Unterer Innenrand
        innenrand(
            rand_unten_breite,
            rand_unten_hoehe
        );
    }
}


// Umlaufender Innenrand
module innenrand(b, h) {

    difference() {

        linear_extrude(height = h)
            rundes_rechteck(
                breite - 2*wandstaerke,
                tiefe - 2*wandstaerke,
                max(0.01, radius - wandstaerke)
            );

        translate([0, 0, -0.01])
            linear_extrude(height = h + 0.02)
                rundes_rechteck(
                    breite - 2*(wandstaerke + b),
                    tiefe - 2*(wandstaerke + b),
                    max(0.01, radius - wandstaerke - b)
                );
    }
}



// ---------- FRONTBOHRUNGEN ----------

module front_befestigungsbohrungen(z0, h) {

    for (sx = [-1, 1], sy = [-1, 1]) {
        translate([
            sx * front_schrauben_x,
            sy * front_schrauben_y,
            z0
        ])
            cylinder(h = h, d = front_schrauben_d);
    }
}


// ---------- DISPLAY-VORBOHRUNGEN ----------

module display_vorbohrungen() {

    for (sx = [-1, 1], sy = [-1, 1]) {
        translate([
            sx * (clip_x + clip_schraube_offset),
            display_y + sy * clip_y,
            -0.01
        ])
            cylinder(
                h = vorloch_tiefe + 0.01,
                d = vorloch_d
            );
    }
}


// ---------- FRONTBLENDE ----------

module frontblende() {

    difference() {

        linear_extrude(height = blenden_dicke)
            rundes_rechteck(
                breite, tiefe, radius
            );

        // Displaytasche
        translate([0, display_y, -0.01])
            linear_extrude(
                height = taschen_tiefe + 0.01
            )
                rundes_rechteck(
                    display_breite + taschen_spiel,
                    display_hoehe + taschen_spiel,
                    1
                );

        // Sichtfenster
        translate([0, display_y, -0.01])
            linear_extrude(
                height = blenden_dicke + 0.02
            )
                rundes_rechteck(
                    fenster_breite,
                    fenster_hoehe,
                    1
                );

        // Flexkabelkanal
        translate([
            0,
            display_y + display_hoehe/2
                + kabel_laenge/2 - 1,
            -0.01
        ])
            linear_extrude(
                height = kabel_tiefe + 0.01
            )
                square(
                    [kabel_breite, kabel_laenge],
                    center = true
                );

        // Optionale Display-Vorbohrungen
        if (display_bohrungen)
            display_vorbohrungen();

        // Optionale Frontbohrungen
        if (front_bohrungen)
            front_befestigungsbohrungen(
                -0.01,
                blenden_dicke + 0.02
            );
    }
        // Adapteraufnahme optional
    if (adapter_zapfen_aktiv)
        adapter_halterung();
}


// ---------- EINZELNER HALTECLIP ----------

module display_clip(rechts = true) {

    difference() {

        linear_extrude(height = clip_dicke)
            square(
                [clip_breite, clip_hoehe],
                center = true
            );

        translate([
            rechts ? clip_schraube_offset
                   : -clip_schraube_offset,
            0,
            -0.01
        ])
            cylinder(
                h = clip_dicke + 0.02,
                d = clip_loch_d
            );
    }
}


// ---------- VIER HALTECLIPS ----------

module display_halteklemmen() {

    for (sx = [-1, 1], sy = [-1, 1]) {
        translate([
            sx * clip_x,
            display_y + sy * clip_y,
            -clip_dicke
        ])
            display_clip(sx == 1);
    }
}


// ---------- KAPPE MIT OPTIONALEN BOHRUNGEN ----------

module gehaeuse_mit_bohrungen() {

    difference() {

        gehaeuse();

        if (front_bohrungen)
            front_befestigungsbohrungen(
                hoehe - rand_oben_hoehe - 0.05,
                rand_oben_hoehe + 0.1
            );

        // Vorbereiteter Ladezugang an der Unterkante, im Preset EIN.
        if (xiao_usb_oeffnung)
            xiao_usb_aussparung();
    }
}


// ---------- ZAPFEN FUER DIE MONTAGEPLATTE ----------

// Zapfen sitzen unter dem oberen Innenrand der Kappe und ragen
// durch die vier Bohrungen der herausnehmbaren Montageplatte.
// Sie positionieren die Platte, verhindern aber allein nicht
// das Abheben (verdeckte Sicherung bei Bedarf spaeter).
module montageplatten_zapfen() {

    for (sx = [-1, 1], sy = [-1, 1]) {

        translate([
            sx * platte_loch_x,
            platte_y + sy * (platte_hoehe/2 - platte_loch_abstand_y),
            hoehe - rand_oben_hoehe - platten_zapfen_h
                 + platten_zapfen_einbindung
        ])
            cylinder(
                h = platten_zapfen_h,
                d = platten_zapfen_d
            );
    }
}


// ---------- GEHAEUSE MIT OPTIONALEN STECKZAPFEN ----------

module gehaeuse_mit_plattenzapfen() {

    union() {
        gehaeuse_mit_bohrungen();

        if (platten_zapfen_aktiv)
            montageplatten_zapfen();
    }
}


// ---------- DRIVER HAT: TRANSPARENTE EINBAUVORSCHAU ----------
// Nur zur Positionierung, kein Teil des STL-Exports.
// Der 4-mm-Buchsenblock liegt zwischen Platine und Montageplatte.
module hat_platzhalter() {
    translate([hat_x, hat_y, platte_z - hat_sockel_h])
        rotate([0, 0, hat_winkel]) {
            // HAT-Leiterplatte (schematisch)
            %translate([0, 0, -hat_platinen_dicke / 2])
                cube([hat_breite, hat_hoehe, hat_platinen_dicke],
                     center = true);

            // Buchsenleiste auf Unterseite, also in Richtung Montageplatte
            %translate([
                0,
                -hat_hoehe/2 + hat_buchse_rand + hat_buchse_breite/2,
                hat_buchse_hoehe / 2
            ])
                cube([hat_buchse_laenge,
                      hat_buchse_breite,
                      hat_buchse_hoehe], center = true);

            // Nur die notwendige Bauhoehe nach hinten schematisch anzeigen
            %translate([0, 0, -(hat_gesamthoehe - hat_buchse_hoehe)/2])
                cube([hat_breite,
                      hat_hoehe,
                      hat_gesamthoehe - hat_buchse_hoehe], center = true);
        }
}


// ---------- DARSTELLUNG ----------

if (ansicht == "Montage") {

    gehaeuse_mit_plattenzapfen();

    // Displayvorschau
    %translate([
        0,
        display_y,
        hoehe + display_dicke/2
    ])
        cube([
            display_breite,
            display_hoehe,
            display_dicke
        ], center = true);

    // Frontblende
    color([0.15, 0.25, 0.40])
        translate([0, 0, hoehe])
            frontblende();

    // Halteclips
    if (halter_aktiv)
        color([0.9, 0.55, 0.15])
            translate([0, 0, hoehe])
                display_halteklemmen();
    
    // HAT als transparente Einbauvorschau (nicht druckbar)
    if (platte_aktiv && hat_halterung_aktiv && hat_vorschau_aktiv)
        hat_platzhalter();

    // XIAO ohne Stiftleisten mit schematischem USB-C (Vorschau).
    if (platte_aktiv && xiao_halterung_aktiv && xiao_vorschau_aktiv)
        xiao_platzhalter();

    // LiPo: transparente Einbauvorschau, nicht druckbar.
    if (platte_aktiv && akku_halterung_aktiv && akku_vorschau_aktiv)
        akku_platzhalter();

    // Montageplatte
    if (platte_aktiv) {

    color([0.65, 0.70, 0.75])
        translate([
            0,
            platte_y,
            platte_z
        ])
            montageplatte();
    }

} else if (ansicht == "Blende") {

    frontblende();

} else if (ansicht == "Kappe") {

    gehaeuse_mit_plattenzapfen();

} else if (ansicht == "Clip") {

    display_clip();
}
else if (ansicht == "Platte") {

    // Einzelansicht in derselben Orientierung wie bei "Montage".
    // Keine Spiegelung: Positionen von HAT, XIAO und Akku bleiben gleich.
    montageplatte();

} else if (ansicht == "Platte_Druck") {

    // Druckfertig drehen, NICHT spiegeln: so bleibt das Teil passgenau.
    // Die glatte Displayseite liegt auf Z=0, die Elektronikhalterungen
    // zeigen nach oben. Eine 180-Grad-Drehung um X ist eine echte
    // Drehung (ohne Aenderung der Haendigkeit der Geometrie).
    translate([0, 0, platte_dicke])
        rotate([180, 0, 0])
            montageplatte();
}
