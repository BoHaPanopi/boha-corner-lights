# BoHa Corner Lights

**Experimentelle Vorabversion 1.1.0; derzeit nur für den Volvo 9700 DD in ETS2.**

Dauerhaftes weißes Abbiegelicht über den eingeschalteten Blinkerzustand und zusätzlich
über den Lenkeinschlag. Die normalen Blinker blinken unverändert weiter.

## Funktionen

- Blinker links/rechts: die jeweilige Seite leuchtet durchgehend.
- Warnblinker: beide Seiten leuchten.
- Lenkradsteuerung unter 40 km/h: Einschalten ab 15°, Ausschalten unter 10°.
- Die Lenkradschwelle bezieht sich auf eine im Spiel eingestellte Gesamtdrehung von 900°.
- Die Blinkersteuerung bleibt unabhängig von dieser Geschwindigkeitsgrenze aktiv.
- Nach vorne überlappende Lichtkegel: 50° seitlich ausgerichtet, 110° Kegelbreite,
  25 m Reichweite. So reicht das Licht in den Bereich der normalen Scheinwerfer.
- Kein Eingriff in Motor, Rekuperation, Tankgröße oder Spielstand.
- Diagnose v2.2: unbekannte Lichtquellen werden einmalig je (Marker, Maske)-Paar
  vermerkt; Instanzplätze (128, max. 16 je Paar) werden nur bei einer
  Aktivierung innerhalb von 1,5 s nach einem eigenen Lichtschaltvorgang belegt
  (Telemetriekanäle Stand-/Abblend-/Fern-/Zusatzlicht), damit KI-Verkehr die
  Tabelle nicht füllt. Je Lichtinstanz protokolliert das Log Zustandswechsel
  (Zeitstempel, Instanzkennung, Marker, Maske, Ein/Aus; je 16 Meldungen), beim
  Beenden eine Zusammenfassung. Ohne verfügbare Lichtkanäle bleibt die
  Instanzdiagnose passiv. Alle Quellen werden unverändert an das Spiel
  durchgereicht. Vorhandene Nebelscheinwerfer lassen sich derzeit nicht
  seitengetrennt als Abbiegelicht nutzen; Details in
  [COMPATIBILITY.md](COMPATIBILITY.md).

## Unterstützter Stand

Windows x64; **genau der in `package/game.sha256` aufgeführte ETS2-Build**.
Der gepinnte Build stammt vom Entwicklungsstand 1.62.3.3 mit der experimentellen
VR-Version und ist auf dem Entwicklungssystem als aktive Installation per SHA-256,
PE-Fingerprint und Spiel-Log bestätigt. Andere 1.62-Binaries und andere
Spielversionen sind **nicht bestätigt**; jede EXE mit abweichendem Hash (z. B. ein
älterer 1.56-Stand aus einer zweiten, verwaisten Installation) wird von Installer
und DLL abgelehnt.
Der Installer prüft den SHA-256 der EXE. Das Plugin prüft zusätzlich Zeitstempel,
Image-Größe und die Signatur seiner nativen Schaltstelle. Unbekannte Builds werden abgelehnt.

Dieses Paket ist **kein allgemeines Abbiegelicht für alle Fahrzeuge**. Benötigt werden:

1. Coaches DLC mit Volvo 9700 DD, Kabine 143d6.
2. Eine eigene Installation der passenden **Volvo 9700 DD Tuning Parts by Enrique**.
3. Die am Bus montierte Zubehörkomponente **Front Slots** (`ec_fpanel`).
4. Eine unterstützte `ec_fpanel.pmg` aus dieser Mod. Der Installer prüft deren Hash.

Die Quellen enthalten jetzt eine Fahrzeugprofil-Auswahl, aber nur das vorhandene,
geprüfte Volvo-Profil ist eingetragen. Serien-Lkw, andere Busse und weitere Modfahrzeuge
werden nicht unterstützt. Telemetrie meldet Fahrzeugzustände, erzeugt aber keine
Lichtquellen oder deren Einbaupositionen. Die Kompatibilitätsmatrix und die konkret
fehlenden Daten für weitere Adapter stehen in [COMPATIBILITY.md](COMPATIBILITY.md).

Enriques komplette Mod und deren Modelle sind **nicht enthalten**. Der Installer
erzeugt die geänderte Modelldatei lokal aus deiner vorhandenen Mod mit einem kleinen
Delta-Patch und baut daraus ein separates SCS-Zusatzpaket.

## Download und Installation

Lade die ZIP-Datei unter **Releases** herunter und entpacke sie vollständig.
`Source code (zip)` ist der Quellcode, nicht das fertige Plugin-Paket.

1. ETS2 vollständig schließen.
2. PowerShell im entpackten Ordner öffnen.
3. Den Installer mit deinen eigenen Pfaden ausführen:

```powershell
.\Install.ps1 -GamePath "D:\SteamLibrary\steamapps\common\Euro Truck Simulator 2" -EnriquePackage "D:\Pfad\zur\Enrique-Mod.scs"
```

`EnriquePackage` kann auch die passende ZIP-Datei des ursprünglichen Mod-Pakets sein.
Die Datei muss als ZIP lesbar sein. Proprietäre HashFS-Archive werden nicht unterstützt.
Optional kann mit `-ModDirectory "D:\ETS2-Profil\mod"` ein anderer Modordner angegeben werden.
Zum ausschließlichen Prüfen ohne Änderungen: `-CheckOnly` ergänzen.

4. Im ETS2-Mod-Manager **BoHa - Corner Lights** aktivieren und oberhalb von Enrique anordnen.
5. Am Volvo die Enrique-Komponente **Front Slots** montieren.
6. Elektrik einschalten und im Stand nachts links/rechts blinken und lenken.

Die DLL wird nach `bin\win_x64\plugins\BoHaCornerLights.dll` kopiert. Das erzeugte
Zusatzpaket heißt `BoHa - Corner Lights.scs` und liegt im ETS2-Modordner.
Vorhandene Dateien mit diesen Namen werden vor dem Ersetzen gesichert.
Der Installer bearbeitet keine Profile oder Spielstände und lädt nichts herunter.

## Deinstallation

ETS2 schließen, das Zusatzpaket im Mod-Manager deaktivieren und die beiden oben
genannten Dateien entfernen. Falls vorher schon eine andere Version installiert war,
können die vom Installer angelegten `.bak`-Dateien wiederhergestellt werden.
Die ursprüngliche Enrique-Mod wird nicht verändert.

## Teststand und Grenzen

- Lokale Steuerungstests für Blinker, Warnblinker, Lenkung, Hysterese, Geschwindigkeit,
  Rückwärtsfahrt, fehlende Daten, unbekannte Lichtquellen und unbekannte Builds bestanden.
- Modellkonvertierung und Plugin-Initialisierung im Spiel bestätigt.
- **Vom Entwickler bestätigter Praxistest (19.11.2026, ohne VR):** Am Volvo 9700 DD
  mit Enrique Front Slots funktioniert das Abbiegelicht; Lichtausrichtung und
  Lichtkegel passen. Der VR-Betrieb ist per Logs belegt (Plugin initialisiert,
  Marker erkannt), eine Sichtprüfung in VR steht noch aus.
- Es wird eine interne Lichtfunktion mit MinHook abgefangen. Das ist keine reine
  Telemetrie-Ausgabe und benötigt bei Spielupdates möglicherweise eine Anpassung.
- Bei ungewöhnlichem Verhalten Plugin entfernen und den Fehler mit Build-Hash sowie
  `BoHaCornerLights.log` melden. Logs vor Veröffentlichung auf persönliche Angaben prüfen.
- Die automatisierten Tests sind keine Fahrtests und bestätigen keine weiteren Fahrzeuge.

## Selbst bauen

Visual Studio mit C++-Werkzeugen, CMake >= 3.20 und Windows SDK installieren:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
python scripts/package.py --dll build/Release/BoHaCornerLights.dll
```

Die portable ZIP-Datei entsteht in `dist/`. Im Repository liegen ausschließlich
Quellcode, Lizenzen, eigene Lichtdefinitionen und Patch-Anweisungen. Es enthält
keine Spiel-EXE, vollständige Fremdmod oder persönlichen Spielstände.

## Lizenz

Eigener Code: MIT, siehe `LICENSE`. MinHook und SCS SDK behalten ihre jeweiligen
Lizenzen; siehe `THIRD_PARTY_NOTICES.md` und `third_party/`.
Enrique und SCS Software sind weder Herausgeber noch Unterstützer dieses Projekts.
