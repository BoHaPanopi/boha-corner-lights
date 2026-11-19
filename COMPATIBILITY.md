# Fahrzeug-Kompatibilität und Adapterdaten

## Statusmatrix

| Fahrzeug/Umgebung | Status | Adapter und Grenzen |
| --- | --- | --- |
| Volvo 9700 DD, Kabine `143d6`, Enrique Front Slots (`ec_fpanel`) | **Unterstützt – Praxistest vom Entwickler bestätigt** | Einziger vorhandener Adapter: `vehicle/bus/upgrade/enrique_9700/ec_fpanel.pmg`. Es werden ausschließlich die beiden privaten Lichtquellenmarker aus dem lokalen, hash-geprüften Patch ausgewertet. Das passende ETS2-Programm muss außerdem exakt dem SHA-256 in `package/game.sha256` entsprechen. Vom Entwickler am 19.11.2026 persönlich im Spiel getestet (ohne VR, Spielversion laut Entwickler 1.62.400.9): Abbiegelicht funktioniert, Lichtausrichtung und Lichtkegel passen. |
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

**Praxistest durch den Entwickler (19.11.2026):** Der Entwickler hat das Plugin
persönlich im Spiel getestet (Spielversion laut Entwicklerangabe 1.62.400.9,
Betrieb ohne VR, Volvo 9700 DD mit Enrique Front Slots). Ergebnis: Das
Abbiegelicht funktioniert, Lichtausrichtung und Lichtkegel passen. Dies ist eine
manuelle Entwicklerbestätigung, kein automatisierter Test; weitere Fahrzeuge
oder Betriebszustände werden dadurch nicht mitbestätigt.

## Kandidatenübersicht für weitere Fahrzeuge (Stand 19.11.2026)

Grundlage sind ausschließlich lokal vorhandene, ZIP-lesbare Mod-Pakete, deren
Modellpfade und Quell-Hashes direkt ausgelesen wurden. Alle SCS-Serienarchive
(`base_vehicle.scs`, DLC-Archive usw.) liegen im proprietären HashFS-Format vor
und sind mit den Werkzeugen dieses Repositories nicht lesbar – für Serien-Lkw
und Serien-Busse gibt es daher weiterhin keine Kandidatendaten. Auch mehrere
lokale Mods (Gloover-Add-Ons, MDT Volvo FH6) sind HashFS-gepackt und damit
nicht auslesbar.

Keiner der folgenden Kandidaten enthält bisher Flare-Anker: Eine String-Analyse
der Slot-Modelle fand keine Flare-Locatornamen; nur das bereits gepatchte
Enrique-Panel enthält `flare.boha.corner_l`/`flare.boha.corner_r`. Für jeden
Kandidaten müsste ein eigener, reproduzierbarer Patch zwei private Marker mit
real vermessenen Positionen ergänzen. Positionen werden nicht erfunden; sie
fehlen derzeit für alle Kandidaten. Es wurde deshalb **kein** neues Profil
registriert oder aktiviert.

| Fahrzeug (def-Anbindung) | Quellpaket (Version, deklarierte Kompatibilität) | Kandidaten-Modellpfad | Quell-SHA-256 |
| --- | --- | --- | --- |
| DAF XF 105 (`daf.xf`) | „Slots By Kevin" 1.0 (`Daf_XG_Slots.scs`, 1.44.*) | `vehicle/truck/upgrade/frontgrill/daf.xf/b_grill_slot.pmg` (488 B) | `c34a548c69255a9dafea3672b49d27a53dbed64a17654e18c42042a12677a236` |
| DAF XF Euro 6 (`daf.xf_euro6`) | „Slots By Kevin" 1.0 (1.44.*) | `vehicle/truck/upgrade/frontgrill/daf_xf_euro6/b_grill_slot.pmg` (488 B) | `c8ce0f85e9ebd18eeb7dc70de92b5a391ab0a7a4fd87125d0f87def3e583d087` |
| Volvo FH16 2009 (`volvo.fh16`) | „Slots By Kevin" 1.0 (1.44.*) | `vehicle/truck/upgrade/frontgrill/volvo_fh16_2009/b_grill_slot.pmg` (664 B) | `7a159214fd49cac3c961cf88234d6e6e79e7d33d9753eb5eea5e439b57c270bd` |
| Volvo FH 2012 (`volvo.fh16_2012`) | „Slots By Kevin" 1.0 (1.44.*) | `vehicle/truck/upgrade/frontgrill/volvo_fh16_2012/b_grill_slot.pmg` (488 B) | `7880d5258518d66883579c5d777728d2c91b66decdda27ffdd3c96a9d19c7f21` |
| Scania R 2009 (`scania.r`) | „Slots By Kevin" 1.0 (1.44.*) | `vehicle/truck/upgrade/frontgrill/scania_rcab_2009/b_grill_slot.pmg` (488 B) | `b14205f4a650c3db41953880521047eb7d0a46a543c97df4eaaf9fc0f53eb5cc` |
| Scania R/S 2016 (`scania.r_2016`, `scania.s_2016`) | „Slots By Kevin" 1.0 (1.44.*) | `vehicle/truck/upgrade/bottomgrill/scania_2016/b_grill_slot.pmg` (488 B) | `e64e6c64b44e12e82cb177b53383c3420e66ff1de2fc264822361f139026e844` |
| Volvo FH 2020/FH5 (`volvo.fh_2020`) | „Volvo FH5 KP public MOD Rework by Sanax" 1.4.2 (`Volvo_FH5_Rework.scs`, 1.44–1.46) | `vehicle/truck/volvo_fh16_2020/accessory/frontgrill/volvo_fh_2020/slots.pmg` (664 B) | `1c518aa1636b5353d764c5eb01827fc92057fe582f9760330c2d3d26ed84446f` |

Paket-Hashes der Quellstände: `Daf_XG_Slots.scs` =
`2304ab7d1b3454476539aa8e85b33be02ee051d6441e4922611ecc4797f03b96`,
`Volvo_FH5_Rework.scs` =
`ddcc25e618f708a44222b86266e1c1c4799f9c065ff946be031e38eb859d886a`.
Das Paket „DAF XF/XG/XG+ Addons v3" (v3, ZIP-lesbar) enthält def-Anbindungen für
nahezu alle Serien-Lkw (DAF, Iveco, MAN, Mercedes-Benz, Renault, Scania, Volvo)
und ist damit ein möglicher Träger weiterer Adapter; konkrete Slot-Kandidaten
wurden dort noch nicht einzeln vermessen. Für Busse existiert außer dem
unterstützten Enrique-Volvo-9700-DD kein lokales Paket mit Front-Slot-Modellen.

Für **jeden** Kandidaten fehlen vor einer Profilaufnahme dieselben Daten:

- real vermessene linke/rechte Lichtpunkt-Positionen und Ausrichtungen im
  jeweiligen Modell (keine Flare-Anker vorhanden; nichts wird geschätzt);
- ein reproduzierbarer Patch mit erwartetem Ziel-SHA-256 nach Enrique-Muster
  (zwei private Marker über `state_change_duration`-Werte);
- eine Prüfung der def-Anbindung (Slot-Typ, Kabinen-/Chassisvarianten) auf dem
  gepinnten 1.62-Build – die Mods deklarieren nur Kompatibilität bis 1.44/1.46;
- ein Spieltest je Fahrzeug (Blinker, Warnblinker, Lenkung, beide Seiten,
  Abblendlichtüberlappung) auf dem gepinnten Build, mit und ohne VR.

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
Der Betrieb **ohne VR** ist durch den oben dokumentierten Praxistest des
Entwicklers vom 19.11.2026 bestätigt (Abbiegelicht funktioniert, Ausrichtung und
Kegel passen). Eine Sichtprüfung des Lichtbilds **im VR-Betrieb** liegt weiterhin
nicht vor. Nur das Programm mit exakt passendem Fingerprint passiert die
Installations- und Laufzeitprüfung. Ein anderer Build bleibt deaktiviert, auch
wenn Telemetrie funktioniert.

## Aussagekraft der Tests

Die automatisierten Tests prüfen Steuerlogik, Marker-/Profilwahl und Ablehnung
unbekannter Builds. Sie führen ETS2 nicht aus und sind keine Bestätigung von
Lichtkegel, Modellposition, Fahrzeugabdeckung oder Fahrverhalten. Für die übrigen
Fahrzeuge gibt es derzeit weder automatisierte noch manuelle Spieltests.
