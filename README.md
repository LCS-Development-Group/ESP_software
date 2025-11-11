# Praca Inżynierska 
**Tytuł pracy inżynierskiej:** *System monitorowania parametrów środowiskowych ze sterowaniem wilgotnością do zastosowań optoelektronicznych*
**English BSc thesis title:** *Environmental monitoring system with humidity control for optoelectronic applications*

## Autor
**Autor:** Karol Pach (275442)
**Uczelnia:** Politechnika Wrocławska 
**Wydział:** Wydział Elektroniki, Fotoniki i Mikrosystemów (W12N)
**Kierunek:** Automatyka i Robotyka (2022-2025)
**Specjalizacja:** Elektronika w Automatyce (AEU)
## Promotor
**Promotor:** Dr Maciej Kowalczyk
**Konsultant:** Mgr inż. Michał Pietrzak
Praca wykonana we współpracy z Katedrą Teorii Pola, Układów Elektronicznych i Optoelektroniki

## Readme
Niniejsze repozytorium zawiera kod programu do układu, którego stworzenie było celem pracy inżynierskiej. Zamieszczone pliki należy umieścić w folderze *main/* projektu

## Oprogramowanie
Projekt został zrealizowany przy użyciu rozszerzenia **ESP-IDF** (v5.5) do programu **Microsoft Visual Studio Code**. Użyty w projekcie mikrokontroler to **ESP32-S3-DEV-KIT-N8R8-M** firmy Waveshare. Przy programowaniu można go traktować jako zwykły ESP32-S3. System operacyjny na którym zrealizowano projekt to **Windows 11**.

## Instrukcja Pobrania Projektu
1. Zainstalować Visual Studio Code
1. Pobrać rozszerzenie ESP-IDF v5.5 (okno *Extentions*)
1. Stworzyć nowy projekt
    - Wizard lub *Shift+P* i komenda **>ESP-IDF: New Project**
    - Wybrać framework (zainstalowany v5.5)
    - Nazwać projekt (bez znaczenia)
    - Wybrać lokalizację
    - Wybrać *ESP-IDF Target:* **ESP32s3**
    - Resztę opcji zostawić domyślną
    - Kliknąć *Choose Template*
    - Zmienić *Extention* na *ESP-IDF*
    - W sekcji *get-started* powinna być opcja **Simple Project**. Jeżeli jest to wybrać ją i pominąć następny krok. Jeżeli nie ma to wybrać **hello_world**
    - Usunąć z folderu projektu pliki: *pytest_hello_world.py* oraz README.md.
1. Wyczyścić zawartość folderu `main/`
1. Skopiować do folderu `main/` zawartość tego repozytorium (clone/zip - dowolnie)


Projekt został pomyślnie pobrany i jest gotowy do dalszej pracy.
## Edycja Kodu
Wszelkie pliki `.c` i `.h` znajdujące się w `main/` można edytować. Odradzam edycję plików z np. `managed_components/`. Autor projektu nie odpowiada za niepoprawne działanie programu wynikające z jego edycji przez osoby trzecie.

## Wgrywanie programu na mikrokontroller
1. Podłączyć mikrokontroller do komputera (po stronie mikrokontrolera potrzebny jest kabel USB-C)
1. Przejść w VSCode do zakładki **ESP-IDF** (panel po lewej, logo Espressif)
1. W zakładce *Commands* dostępne są komenndy. Zaznaczając checkmarki koło komend można je wyjąć na pasek. Polecam to zrobić przynajmniej dla:
    - `Build Project`
    - `Flash Device`
    - `Monitor Device`
    - `ESP-IDF: Build, Flash and Monitor`
Nie wyjęte komendy można wywołać klikając na ich nazwy w tej zakładce (nie checkmark).
1. Kliknąć **Select Port to Use** i wybrać ten pod którym wyświetla się mikrokontroler. Uwaga, windows potrafi bez powodu w trakcie pracy zmienić port na inny. Jeżeli pojawi się błąd przy flashowaniu o tym to ustawić port na nowy właściwy.
1. To samo zrobić z **Select Monitor Port to Use**
1. Kliknąć **Build Project**. Spowoduje to utworzenie folderu `build/` oraz kompilację programu. Pierwszy Build zajmuje trochę, ale późniejsze już nie bo środowisko rekompiluje tylko to co musi (więc tylko ew. zmiany).
    - Można wyczyścić builda komendą **Full Clean**. Min. usunie to `build/`. Czasami przydatne przy dziwnych błędach.
1. Po udanej kompilacji (pokaże się okno ze stanem pamięci) kliknąć **Flash Device**. Spowoduje to wgranie kodu na mikrokontroller. Od razu po tym nastąpi jego reset i uruchomienie programu.
1. Program można śledzić za pomocą komendy **Monitor Device**. Jest to de facto serial port znanny z innych środowisk embedded. Normalnie nie pojawia się tam za dużo (ograniczyłem komunikaty), ale przydatne przy ew. debuggowaniu. Oczywiśćie działa tylko przy podłączonym kablu usb-c. Uwaga: uruchomienie monitorowania resetuje mikrokontroler.
1. kroki 6-8 można wykonać automatycznie klikając **ESP-IDF: Build, Flash and Monitor**. Wywoła to powyższe komendy jedna po drugiej.

O ile nie było żadnych błędów program powinien być wgrany i uruchomiony na mikrokontrolerze.