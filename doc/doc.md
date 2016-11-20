<!--- 
vim: spell:spelllang=hu
-->
---
papersize: a4
lang: hu-HU
toc: true
fontsize: 12pt
margin-left: 3.5cm
margin-top: 2.5cm
margin-bottom: 2.5cm
margin-right: 2.5cm
documentclass: report
classoption: twoside
header-includes:
    - \usepackage{setspace}
    - \onehalfspacing
---
# Bevezetés

## Motiváció

Hangosságnak nevezzük a hang azon tulajdonságát, amely halktól a hangosig skálázható.
Ez szoros összefüggésben van a hang fizikai erejével, azonban érzékelése a fülben nemcsak fizikai és fiziológiai,
hanem pszichés folyamat is.

Többen, többféleképpen értelmezik ezt a pszichoakusztikai tulajdonságát a hangnak.
Ami a különböző mérésekben azonos, hogy egytől egyig mind a logaritmikus decibel skálát használjak.
Ennek az oka az emberi hallásra vezethető vissza, egy az amplitúdójában kétszer erősebb jelet,
nem hallunk kétszer hangosabban.
Az emberi hallás átlagolja a hallott hangot 600ms és 1s között. 
Ezt a mérések is tükrözik és általában valamilyen intervallumonként átlagolt értékkel dolgoznak.

A legegyszerűbb mérés pusztán a hang fizikai tulajdonságait veszi alapul. 
Ez egy adott intervallumra vett jel négyzetes átlagának a gyökét jelenti (RMS, root mean square).

További megoldás ha a bemeneti jelet valamilyen szűrőn áteresztve értékeljük. 
A szűrőt általában kísérletezéssel állapítják meg.
Az ITU által javasolt BS-1770 az úgynevezett K-súlyozó szűrőt használja a célra.

Felismerve a tényt, hogy különböző frekvencia tartományokban a változásokra különbözően reagál a fül, 
egy lehetséges módszer a bementi jel kritikus tartományokra bontása, majd az egyes tartományok súlyozása valamilyen függvénnyel.
Ilyen függvény például az ISO 61672:2003 által előírt A-súlyozás.

Mindkét esetben a keletkezett jelből intervallumonként RMS számítunk.

## A megvalósított program

A program a fentebb említett három mérés (RMS, ITU BS-1770, és ISO 61672:2003) eredményét mutatja valós időben, 
miközben egyes részei külön-külön is felhasználhatóak és kombinálhatóak további jelfeldolgozásra a jövőben.

Jelenleg a legtöbb hangkártya 16 bites előjeles egészéket használ a bemeneti, illetve kimeneti jel ábrázolásaként.
A program, miután a jelet kiolvasta a hangkártyáról, ezt az egész számot egy 15+1 bit pontos fixpontos számnak tekinti,
majd végül belül végig 16 bit pontossággal számol. A decibel számításakor a $20\log_{10} \frac{p}{p_0}$ képletet használjuk, 
ahol $p_0 = 2^{-16}$, a legkisebb ábrázolható számot, legkisebb mérhető érték. $p$ maximális értéke elméletben
$1$, így a decibel maximális értéke jelen esetben $96.3296$ dB.

A forráskód jól elkülöníthető részei felelősek a következő funkciókért:

- bemeneti jel fogadása PulseAudio segítségével,
- generikus fixpontos számábrázolás, melynek pontosságát a felhasználó határozza meg,
- digitális szűrők megvalósítása biquad-okkal,
- low-pass, high-pass, és band-pass filterek egyszerű létrehozása,
- adott intervallumon mérések elvégzése,
- a progam felhasználói felületén a konfigurációs paraméterek és a mérések grafikus megjelenítése gtkmm-3.0 segítségével.

# Felhasználói dokumentáció

## A program üzembe helyezése

A program telepítése bármilyen operációs rendszeren lehetséges amelyen megtalálhatók az alábbi szoftverek, programkönyvtárak 
és azok függőségei:

- cmake, legalább 3.5.0,
- C++11 kompatibilis fordító, például g++ vagy clang megfelelő verziói,
- make,
- PulseAudio,
- gtkmm, 3.0 vagy annál újabb verzió a 3-as szériából.

A telepítés lépései a következőek:

1. A forrás beszerzése az internetről vagy a mellékelt hordozóról:
```
# git clone https://github.com/zleho/sma.git
```

2. Egy ideiglenes munka könyvtár létrehozása és aktívvá tétele:
```
# mkdir sma-build
# cd sma-build
```

3. CMake parancs futtatása a megfelelő paraméterekkel. A paraméterek leírása megtalálható a CMake dokumentációjában:
```
# cmake <sma-source>
```

4. A program fordítása:
```
# make
```

5. Opcionálisan a program telepítése végeleges helyére, ami CMake paraméter, aminek alap beállítása `/usr/local/`. 
Könyvtártól függően emelt privilégiumra lehet szükség a parancs kiadásánál:
```
# make install
```

## A program indítása és használata

![Bemeneti eszköz kiválasztása](input.png){ width=300px }

![Mérés méretének kiválasztása](meassize.png){ width=300px }

![Mérés indítása](meas.png){ width=300px }

![Mérés](meas.png){ width=300px }

![Jelenlegi státusz](status.png){ width=300px }


## A programkönyvtárak felhasználása

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

# Fejlesztői dokumentáció

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

## Megvalósítási terv

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

### Bemeneti jel feldolgozása

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

### Számábrázolás

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

### Digitális szűrők

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

### Az eredmény megjelenítése

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

## Megvalósítás

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

### Fixpontos aritmetika

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

### RMS számítás

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

#### Logaritmus számítás

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

#### Decibel számítás

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

### Digitális szűrők

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

#### ITU-BS1770 szűrői

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

#### Band-pass szűrők

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

#### $A$-súlyozott mérés

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

## Tesztelés

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

### Fixpontos aritmetika

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

### Logaritmus számítás

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

### Bementi jel generálása

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

### Band-pass szűrők

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

### Az alkalmazás

Lorem ipsum dolor sit amet, duo modus quidam consequat an. Alii vocibus intellegat ut duo. Eos ex melius aeterno vivendo, posse doming reformidans id vel. In tale mundi sea. Ex mea assum tincidunt efficiantur. Option pertinax ex sea, ferri malis phaedrum nam no.

