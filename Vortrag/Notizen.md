# Vortragsnotizen Team 142

# Problemstellung

## Moore Kurven (F.4)

- Eliakim Hastings Moore, US-amerikanischer Mathematiker, beschäftigte sich mit abstrakter Algebra und Grundlagen der Geometrie, entwickelte die bekannte Hilbert Kurve weiter zu seiner Moore Kurve
- raumfüllende Kurve, kurze Erklärung: bei n→ $\inf$ raumfüllend, beliebige Genauigkeit/Granularität durch Steigerung des Grads
- Eingehen auf exponentielles Wachstum der Seitenlänge  und der Punktanzahl → Speicherlimitierungen bzw nur niedrige Grade für uns wirklich relevant

## Anwendungen (F.5)

- Folgende Anwendungen & Eigenschaften für raumfüllende/FASS Kurven generell (Und nicht nur Moore):
- mappings nützlich zur Verarbeitung mehrdimensionaler Datensätze
- Dithering kurz erklären
- Warum? Spatiale, also räumliche Nähe wir auch nach dem mappen in 1 Dimension größtenteils erhalten → z.B. Bilddaten, die nah beieinander liegen sind auch im Speicher nah beieinander
- Beispiel: Google's s2 library, durch 64 bit integer ist <1cm^2 auf der erde indexierbar, Anordnung und Indexierung ist sinnvoll und sehr viel besser als zb Benutzung von Längen und Breitengrad zur Indexierung, Einteilung in Zellen verschiedener Levels (Benutzt wird allerdings die Hilbert Kurve)
- Unsere Aufgabe: alle Punkte von gegebenem Grad berechnen, evtl. kurzes Zahlenbeispiel ( Größenordnung)

# Lösungsansatz

## Berechnung eines einzelnen Punkts:

- (10) Moore ist sozusagen eine geloopte Version der Hilbert Kurve, also zuerst Hilbert vom entsprechenden Index, dann noch verschieben
- Erklären der einzelnen Schritte:
1. (11-13) Verschiebungen von Hilbert(n-1) zu Moore(n)
2. (14 - 24) Wie bekomme ich einen Punkt von Hilbert mit gegebenem Index?
    - → Binärdarstellung
    - → Verschiebung/Rotieren/Spiegeln in Quadranten bis man beim gewünschten Grad angekommen ist

## Berechnung der gesamten Kurve:

### Naiver Ansatz:

- Nachteile:.
    - Jetzt Algorithmus für jeden Punkt der Kurve
    - jeder Punkt Quasi exklusiv berechnet 4^n
    - Also abhängig vom grad der Kurve
- Vorteile:
    - Möglichkeit einzelne Punkte der Kurve zu berechnen
- Als würde man die Kurve entlang laufen Punkt für Punkt
- die Länge der Binärzahl 2^n die einen Punkt Repräsentiert
- denn es braucht immer 2 Stellen je Grad der Kurve
- aufwand pro Punkt log2(2^n) * c = n*c
- (es wird über die Zahl im Binärsystem iteriert je zwei Ziffern)
- linearer Anstieg

### Reuse-Algorithmus

- Bottom-up
- Wiederverwendung von Ergebnissen
- erster Grad hard-coded in Moore- und Hilbert-Kurve gleich
- immer 4 mal n - 1 Hilbert und am Ende Moore
- Konkretes Beispiel der Aufbau vom 2 Grad zum 3 Grad der Hilbert Kurve
- Zu beobachten exponentieller Wachstum
- Nachteile:
    - nicht möglich exklusiv ein Punkt zu berechnen
    - Mehr Speicherzugriffe durch die Abhängigkeit von vorheriger Kurve
- Vorteil: Rechenaufwand pro Punkt nahezu konstant
- (Wiederverwendung von Daten)

### Berechnung in konstanter Zeit

- Rekursive Definition der Rechenzeit (top-down)
    - c für Operationen: Bewegen, Spiegeln, Rotierenk
    - mal Anzahl der Punkte in dem Grad und das für alle s
- Geschlossene Form die den Rechenaufwand wiedergibt
- die Rechenzeit wächst exponentiell ebenso die Anzahl der Punkte
- im Durchschnitt konvergiert gegen eine Konstante mit

    $$ lim_{n\to\infty} $$

# Performanz

- (46) Auffälligkeiten in Laufzeitübersicht
    - Performanzeinbruch bein Grad 16
    - Vektorisierte Implementierungen besonders effizient bei Grad 6
- (46) Auffälig schlechtere Performanz von `c_iter` im Gegensatz zu den *batch*-Implementierungen
    - Dynamischer Approach kann bei `c_iter` nicht genutzt werden → viele Rechenoperationen pro Punkt (dafür kein Speicherzugriff)
- (48) Ausführung: Wie kann mit mithilfe von Vektorregistern parallelisiert werden?
    - Aufgabenstellung: `uint64_t`

        → xmm-Register: 2 Punkte gleichzeitig

        → ymm-Register: 4 Punkte gleichzeitig

    - Abänderung: `uint32_t`
        - kann Koordinaten bis Grad 32 korrekt darstellen
        - Grad 33 würde 560 Trillionen Bytes benötigen (bei korrekter Verwendung von 64 bit ints sogar das Doppelte)

        → xmm-Register: 4 Punkte gleichzeitig

        → ymm-Register: 8 Punkte gleichzeitig

- (55) Laufzeitvergleich `asm_128` mit `asm_256`
    - Spike bei Grad 6 (später auf Cache eingehen)
    - bei Höheren Graden wenig Performanzgewinn
- (56) Laufzeitvergleich von `c_batch` mit den Assembler-Implementierungen
    - Auch Spike bei Grad 6
    - Starker Einbruch bei Grad 16 (am Ende darauf eingehen)
    - Generell: deutlich effizienter als C Programm durch Vektorisieren
- (57) Spike bei Grad 6 genauer untersuchen
    - Annahme: Programm bis Grad 6 sehr cache freundlich
        - Auf Speicheranforderung der Moore Kurve und Größe d. L1 Caches auf Maschine eingehen
        - wenige L1-Misses gemessen (perf, Instruments)
- (60) Performance Einbruch bei Grad 16
    - Kurve benötigt mehr Speicher als RAM zur verfügung

        → Pagefaults

# Zusammenfassung (62)

- Punkt für Punkt
    - Nur Berechnungen, wenige Speicherzugriffe (nur 1x schreiben pro Punkt, nicht lesen)
    - Trotzdem deutlich langsamer, da Zwischenergebnisse nicht wiederverwendet werden können
- Dynamischer Ansatz
    - trotz vieler Speicherzugriffer deutlich schneller
        - Wiederverwendung von Zwischenergebnissen
        - durch Cache sind Speicherzugriffe deutlich billiger
- Optimierungsmöglichkeiten
    - dynamische Anpassung der Int-Breite

        → doppelt so viele Punkte auf einmal verarbeitbar

        → kleinere Speicheranforderung bis Grad 16

    - Benutzung von AVX-512 (oder sogar auf Grafikkarte)

        → in vielen Fällen AVX2 nicht viel schneller als 128 bit Register (vmtl. Von-Neumann-Bottleneck)

    - Andere Datenstrukturen
        - `x[i]` näher an `y[i]`

            → Bei Transformationen von einzelnem Punkt wird nicht im Speicher gesprungen