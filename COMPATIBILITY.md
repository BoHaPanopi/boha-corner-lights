# Fahrzeug-Kompatibilität und Adapterdaten

## Statusmatrix

| Fahrzeug/Umgebung | Status | Adapter und Grenzen |
| --- | --- | --- |
| Volvo 9700 DD, Kabine `143d6`, Enrique Front Slots (`ec_fpanel`) | **Bedingt unterstützt** | Einziger vorhandener Adapter: `vehicle/bus/upgrade/enrique_9700/ec_fpanel.pmg`. Es werden ausschließlich die beiden privaten Lichtquellenmarker aus dem lokalen, hash-geprüften Patch ausgewertet. Das passende ETS2-Programm muss außerdem exakt dem SHA-256 in `package/game.sha256` entsprechen. |
| SCS-Serien-Lkw (u. a. DAF, Iveco, MAN, Mercedes-Benz, Renault, Scania, Volvo) | **Nicht unterstützt** | Die Serienmodelle liegen in proprietären HashFS-Archiven, die weder der Installer noch dieses Repository lesen. Es liegen keine ausgelesenen Modellpfade, keine verifizierten Lichtanker-Positionen, keine Quell-/Ziel-Hashes und keine Spieltests vor. |
| SCS-Serien-Busse außer dem genannten Volvo | **Nicht unterstützt** | Es liegen keine Fahrzeugmodelle, Lichtanker, geprüften Modelldateien oder Fahrzeugprofile vor. |
| Modfahrzeuge einschließlich anderer Volvo-9700-Modstände | **Nicht unterstützt** | Es gibt keinen lokalen Adapter/Patch dafür. Der vorhandene Installer akzeptiert nur den festgelegten Enrique-Modellpfad und Quellhash. Auch für andere Slot- und Tuning-Mods (z. B. DAF-XG-Slot-Pakete, Volvo-FH-Reworks) existieren keine verifizierten Marker, Patches oder Hashes. |
| Unbekannte ETS2-Builds | **Abgelehnt** | Installer und DLL prüfen den bekannten Build; die DLL prüft zusätzlich Zeitstempel, Image-Größe und native Gate-Bytes. Bei Abweichung wird kein Hook installiert. |

Die Profilregistry in `src/controller.h` enthält derzeit genau einen Eintrag. Sie ordnet
dem bekannten Modelladapter seine privaten Links-/Rechtsmarker und Lenkschwellen zu.
Kegelparameter und Ausrichtung entsprechen der eigenen SII-Lichtdefinition. Die
Einbaupunkte liegen im lokal erzeugten Modellpatch; numerische XYZ-Koordinaten sind in
diesem Repository nicht verfügbar und werden daher nicht behauptet oder nachgebildet.
Ein unbekannter Marker wird ignoriert, nicht als Fahrzeugprofil geraten.

## Prüfstand 19.11.2026 (korrigiert)

Eine erneute Durchsicht von Quellcode, Patch-Spezifikation und lokal verfügbaren
Mod-Paketen hat **keine** weiteren Fahrzeuge ergeben, für die Modellpfade,
Lichtpositionen und Versionsdaten zuverlässig nachweisbar wären. Die Profilregistry
behält deshalb bewusst genau den einen geprüften Volvo-Eintrag; es wurden keine
Profile aus ungeprüften Daten ergänzt.

Für den aktiv genutzten Spielstand wurde der gepinnte Build vollständig verifiziert:

- Die aktive ETS2-EXE des Entwicklungssystems (EXE-Metadaten und Pack-Set melden
  1.62.3.3) stimmt **exakt** mit dem SHA-256 in `package/game.sha256` überein.
- Der Laufzeit-Fingerprint wurde direkt gegen diese EXE gemessen: PE-Zeitstempel,
  Image-Größe und die Gate-Bytes an der hinterlegten RVA stimmen überein; das
  Byte-Muster kommt in der EXE genau einmal vor.
- `Install.ps1 -CheckOnly` bestätigte End-to-End Spielbuild, Enrique-Quelldatei
  (Hash entspricht dem gepatchten Zielstand), Patch-Kette und DLL, ohne etwas
  zu verändern.
- Spiel- und Plugin-Logs vom 19.11.2026 belegen auf diesem Build: Plugin geladen
  und initialisiert, nativer Flare-Gate-Hook installiert, beide privaten
  Links-/Rechtsmarker im Spiel erkannt (je >169 000 Gate-Aufrufe), Lauf im
  VR-Modus (`-openxr -experimental_vr`).

Korrektur einer früheren Feststellung: Die zuvor gemeldete, nicht passende
EXE 1.56.1.16 gehört zu einer **zweiten, verwaisten Installation** in einer nicht
mehr in Steam registrierten Bibliothek. Sie ist nicht der genutzte Spielstand.
Für diese alte EXE gilt weiterhin: kein passender Hash, kein Fingerprint, wird
fail-closed abgelehnt. Eine vom Spiel-UI gegebenenfalls angezeigte abweichende
Versionsnummer ist in den Dateimetadaten nicht nachweisbar; maßgeblich ist
ausschließlich der SHA-256-Abgleich.

Bestanden am selben Stand: CMake-Build, `ctest` (Steuerlogik, Profilwahl,
Build-Ablehnung), Paketprüfung (`scripts/package.py` inkl. ZIP-Integritäts- und
Inhaltsprüfung) sowie eine Sicherheitsdurchsicht von Hook-Pfad, Patch-Grenzen,
ZIP-Verarbeitung und Installerskript ohne Befund.

## Noch benötigte Daten für weitere Profile

Für jedes Serien- oder Modfahrzeug muss ein Adapter separat anhand einer rechtmäßig
installierten lokalen Datei erstellt und getestet werden. Dazu fehlen hier konkret:

- das Modell und die Variante/Kabine mit den exakten, lokal ausgelesenen relativen
  Modellpfaden; bei Mods zusätzlich die eindeutige Mod- und Modellversion;
- für jede Eingabedatei ihr ermittelter SHA-256, ein reproduzierbarer Patch und der
  erwartete SHA-256 der Ausgabe; es gibt keine geprüften Hashes für andere Modelle;
- verifizierte linke/rechte Lichtpunkt-Positionen und Ausrichtungen, zulässige
  Lichtquellen-/Flare-Marker sowie Kegel-, Reichweiten- und Lenkschwellenwerte;
- der zugehörige ETS2-Programm-Build-Fingerprint und native Anschluss, falls eine
  neue native Render-Schaltstelle erforderlich wäre;
- ein Spieltest des eingebauten Adapters mit Blinker, Warnblinker, Lenkung,
  Geschwindigkeit, Abblendlichtüberlappung und Sichtprüfung beider Seiten.

Für Mods sind keine Archive, vollständigen Modelle oder Spieldateien enthalten. Der
bestehende Installer ändert nur eine passende lokale Enrique-Quelldatei nach
Hash-Prüfung, legt Sicherungen vorhandener Installationsdateien an und lässt Profile
und Spielstände unverändert. Er ist noch kein allgemeiner Adapter-Installer. Unbekannte
Modellstände werden abgewiesen; keine lokalen Quelldateien überschreiben, um einen
Patch passend zu machen.

## VR und Nicht-VR

Die vorhandene Dokumentation und der Entwicklungstest beziehen sich auf den unter
`package/game.sha256` gepinnten Build. Spiel- und Plugin-Logs vom 19.11.2026 belegen
für diesen Build einen Lauf im experimentellen VR-Modus (OpenXR) mit erfolgreicher
Plugin-Initialisierung, installiertem Hook und erkannten Markern beider Seiten.
Ein separater, dokumentierter Nicht-VR-Vergleichslauf liegt weiterhin nicht vor;
eine getrennte Nicht-VR-Bestätigung bleibt daher offen. Nur das Programm mit exakt
passendem Fingerprint passiert die Installations- und Laufzeitprüfung. Ein anderer
Build bleibt deaktiviert, auch wenn Telemetrie funktioniert.

## Aussagekraft der Tests

Die automatisierten Tests prüfen Steuerlogik, Marker-/Profilwahl und Ablehnung
unbekannter Builds. Sie führen ETS2 nicht aus und sind keine Bestätigung von
Lichtkegel, Modellposition, Fahrzeugabdeckung oder Fahrverhalten. Für die übrigen
Fahrzeuge gibt es derzeit weder automatisierte noch manuelle Spieltests.
