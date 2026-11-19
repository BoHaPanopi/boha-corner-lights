# Fahrzeug-Kompatibilität und Adapterdaten

## Statusmatrix

| Fahrzeug/Umgebung | Status | Adapter und Grenzen |
| --- | --- | --- |
| Volvo 9700 DD, Kabine `143d6`, Enrique Front Slots (`ec_fpanel`) | **Bedingt unterstützt** | Einziger vorhandener Adapter: `vehicle/bus/upgrade/enrique_9700/ec_fpanel.pmg`. Es werden ausschließlich die beiden privaten Lichtquellenmarker aus dem lokalen, hash-geprüften Patch ausgewertet. Das passende ETS2-Programm muss außerdem exakt dem SHA-256 in `package/game.sha256` entsprechen. |
| SCS-Serien-Lkw | **Nicht unterstützt** | Es liegen keine Fahrzeugmodelle, Lichtanker, geprüften Modelldateien oder Fahrzeugprofile vor. |
| SCS-Serien-Busse außer dem genannten Volvo | **Nicht unterstützt** | Es liegen keine Fahrzeugmodelle, Lichtanker, geprüften Modelldateien oder Fahrzeugprofile vor. |
| Modfahrzeuge einschließlich anderer Volvo-9700-Modstände | **Nicht unterstützt** | Es gibt keinen lokalen Adapter/Patch dafür. Der vorhandene Installer akzeptiert nur den festgelegten Enrique-Modellpfad und Quellhash. |
| Unbekannte ETS2-Builds | **Abgelehnt** | Installer und DLL prüfen den bekannten Build; die DLL prüft zusätzlich Zeitstempel, Image-Größe und native Gate-Bytes. Bei Abweichung wird kein Hook installiert. |

Die Profilregistry in `src/controller.h` enthält derzeit genau einen Eintrag. Sie ordnet
dem bekannten Modelladapter seine privaten Links-/Rechtsmarker und Lenkschwellen zu.
Kegelparameter und Ausrichtung entsprechen der eigenen SII-Lichtdefinition. Die
Einbaupunkte liegen im lokal erzeugten Modellpatch; numerische XYZ-Koordinaten sind in
diesem Repository nicht verfügbar und werden daher nicht behauptet oder nachgebildet.
Ein unbekannter Marker wird ignoriert, nicht als Fahrzeugprofil geraten.

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
`package/game.sha256` gepinnten Build; im Repository sind keine separaten VR- und
Nicht-VR-Fingerprints oder vergleichenden Fahrtests hinterlegt. Deshalb ist weder eine
allgemeine VR-Kompatibilität noch eine getrennte Nicht-VR-Kompatibilität bestätigt.
Nur das Programm mit exakt passendem Fingerprint passiert die Installations- und
Laufzeitprüfung. Ein anderer Build bleibt deaktiviert, auch wenn Telemetrie funktioniert.

## Aussagekraft der Tests

Die automatisierten Tests prüfen Steuerlogik, Marker-/Profilwahl und Ablehnung
unbekannter Builds. Sie führen ETS2 nicht aus und sind keine Bestätigung von
Lichtkegel, Modellposition, Fahrzeugabdeckung oder Fahrverhalten. Für die übrigen
Fahrzeuge gibt es derzeit weder automatisierte noch manuelle Spieltests.
