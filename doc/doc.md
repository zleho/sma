<!--- 
vim: spell:spelllang=hu
-->
---
papersize: a4
lang: hu-HU
toc: true
lof: true
lot: true
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
bibliography: doc.bib
nocite: |
    @bib01, @bib02, @bib03, @bib04, @bib05, @bib06, @bib07, @bib08, @bib09, @bib10, @bib11, @bib12, @bib13, @bib14, @bib15, @bib16
---

# Bevezetés

## Motiváció

Hangosságnak nevezzük a hang azon tulajdonságát, amely halktól hangosig skálázható.
Ez szoros összefüggésben van a hang fizikai erejével, azonban érzékelése a fülben nemcsak fizikai és fiziológiai,
hanem pszichés folyamat is.

Többen, többféleképpen értelmezik a hangnak ezt a pszichoakusztikai tulajdonságát.
Ami a különböző mérésekben azonos, hogy egytől egyig mind a logaritmikus decibel skálát használják.
Ennek az oka az emberi hallásra vezethető vissza, egy, az amplitúdójában kétszer erősebb jelet
nem hallunk kétszer hangosabban.
Az emberi hallás átlagolja a hallott hangot 600ms és 1s között. 
Ezt a mérések is tükrözik és általában valamilyen intervallumonként átlagolt értékkel dolgoznak.

A legegyszerűbb mérés pusztán a hang fizikai tulajdonságait veszi alapul. 
Ez egy adott intervallumra vett jel négyzetes átlagának a gyökét jelenti (RMS, root mean square).

További megoldás, ha a bemeneti jelet valamilyen szűrőn áteresztve értékeljük. 
A szűrőt általában kísérletezéssel állapítják meg.
Az ITU által javasolt BS-1770 az úgynevezett K-súlyozó szűrőt használja a célra.

Felismerve a tényt, hogy különböző frekvencia tartományokban a változásokra különbözően reagál a fül, 
egy lehetséges módszer a bemeneti jel kritikus tartományokra bontása, majd az egyes tartományok súlyozása valamilyen függvénnyel.
Ilyen függvény például az ISO 61672:2003 által előírt A-súlyozás.

Mindkét esetben a keletkezett jelből intervallumonként RMS-t számítunk.

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

A program telepítése bármilyen operációs rendszeren lehetséges, amelyen megtalálhatók az alábbi szoftverek, programkönyvtárak 
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

    2. Egy ideiglenes munka-könyvtár létrehozása és aktívvá tétele:
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

A program az `sma` parancs kiadásával indítható. Miután a program elindult, az először csatlakozik a lokális
PulseAudio szerverhez, majd lekérdezi a lehetséges bemeneti eszközök listáját. Ezek után a program belép a konfigurációs
állapotba, ahol kiválaszthatjuk a bemeneti eszközt (1. Ábra), valamint megadhatjuk a mérési intervallum hosszát másodpercben,
$0.1$ és $1$ másodperc között (2. Ábra), tizedmásodperces lépésközzel.

![Bemeneti eszköz kiválasztása](input.png){ width=300px }

![Mérés méretének kiválasztása](meassize.png){ width=300px }

Miután kiválasztottuk a nekünk megfelelő paramétereket, a konkrét mérés a `MEASURE` gombra való kattintással indítható el (3. Ábra). 
Ezek után aktiválódnak az egyes mérések és inaktívvá válnak az egyes konfigurációs lehetőségek, amíg a mérés véget nem ér.
A mérés befejezését a `MEASURE` gombra történő ismételt kattintás idézi elő. A mérések aktuális értékét a program számszerűen és
vizuálisan is mutatja (4. Ábra).

![Mérés indítása](measbutton.png){ width=300px }

![Mérés](meas.png){ width=300px }

A program minden pillanatban jelzi, hogy éppen milyen állapotban van (5. Ábra).

![Jelenlegi státusz](status.png){ width=300px }

## Mérések

### RMS

A program által elvégzett legegyszerűbb mérés az úgynevezett **root-mean-square** kalkuláció egy adott intervallumon, azaz

$$20\log_{10}\frac{\sqrt{\sum_{i=1}^{N} \frac{x_i^2}{N}}}{2^{-16}},$$

ahol $N=fT$, $T$ a mérési intervallum hossza másodpercben és $f=48000$ a mintavételezés frekvenciája és $x_i$ az $i$-ik mért jel. 

### ITU BS-1770

Az ITU által ajánlott algoritmus első két lépését hajtjuk végre, azaz

1. a bemeneti jelet $K$ frekvencia súlyozó szűrőn keresztül eresztve
2. RMS-t számolunk az intervallumon.

Az első lépés két szűrő együttes használata. Az első a fej akusztikáját veszi figyelembe, ahol a fej formáját tömör gömbnek veszi,
majd egy egyszerű high-pass szűrőn keresztül ereszti a jelet. A két szűrő együtthatói az ajánlásában megtalálhatóak.

### A-weighted

Az egyik leggyakrabban használt hangosság mérését elősegítő görbe az úgynevezett A-súlyozás, melyet ISO 61672:9001 és
több nemzet szabványa is így definiál:

$$A(f)=\frac{12200^4 \cdot f^4}{(f^2 + 20.6^2) \sqrt{(f^2 + 107.7^2)(f^2 + 737.9^2)} (f^2 + 12200^2)}$$

A függvény görbéjét a 6. ábra szemlélteti.

![A-súlyozás](aweighting.png){ width=50% }

Használatához a jelet általában oktávokra vagy harmad-oktávokra bontják, majd ezeken a kritikus sávba eső jelet
a sáv középértékén számolt súllyal számítják bele RMS-be az

$$\sum_{j=1}^K{w_j \cdot x_{ij}},$$

értékét, ahol $K$ a kritikus sávok száma és $x_{ij}$ pedig a $j$-k sávba szűrt jel.

A program 20 Hz és 20 kHz közötti sávjait és azok súlyait a 1. táblázatban találjuk meg.

| Alsó határ (Hz) | Felső határ (Hz) | Közép frekvencia (Hz) | Súly     |
|-----------------|------------------|-----------------------|----------|
| $20.0000$       | $26.6666$        | $23.3333$             | $0.0037$ |
| $26.6666$       | $33.3333$        | $30.0000$             | $0.0074$ |
| $33.3333$       | $40.0000$        | $36.6666$             | $0.0121$ |
| $40.0000$       | $53.3333$        | $46.6666$             | $0.0209$ |
| $53.3333$       | $66.6666$        | $60.0000$             | $0.0352$ |
| $66.6666$       | $80.0000$        | $73.3333$             | $0.0515$ |
| $80.0000$       | $106.666$        | $93.3333$             | $0.0783$ |
| $106.666$       | $133.333$        | $120.000$             | $0.1160$ |
| $133.333$       | $160.000$        | $146.666$             | $0.1540$ |
| $160.000$       | $213.333$        | $186.666$             | $0.2098$ |
| $213.333$       | $266.666$        | $240.000$             | $0.2800$ |
| $266.666$       | $320.000$        | $293.333$             | $0.3448$ |
| $320.000$       | $426.666$        | $373.333$             | $0.4320$ |
| $426.666$       | $533.333$        | $480.000$             | $0.5302$ |
| $533.333$       | $640.000$        | $586.666$             | $0.6099$ |
| $640.000$       | $853.333$        | $746.666$             | $0.7008$ |
| $853.333$       | $1066.66$        | $960.000$             | $0.7826$ |
| $1066.66$       | $1280.00$        | $1173.33$             | $0.8349$ |
| $1280.00$       | $1706.66$        | $1493.33$             | $0.8808$ |
| $1706.66$       | $2133.33$        | $1920.00$             | $0.9093$ |
| $2133.33$       | $2560.00$        | $2346.66$             | $0.9188$ |
| $2560.00$       | $3413.33$        | $2986.66$             | $0.9152$ |
| $3413.33$       | $4266.66$        | $3840.00$             | $0.8931$ |
| $4266.66$       | $5120.00$        | $4693.33$             | $0.8602$ |
| $5120.00$       | $6826.66$        | $5973.33$             | $0.8004$ |
| $6826.66$       | $8533.33$        | $7680.00$             | $0.7128$ |
| $8533.33$       | $10240.0$        | $9386.66$             | $0.6261$ |
| $10240.0$       | $13653.3$        | $11946.6$             | $0.5094$ |
| $13653.3$       | $17066.6$        | $15360.0$             | $0.3863$ |
| $17066.6$       | $20480.0$        | $18733.3$             | $0.2966$ |

: Harmad oktávok és azok A-súlyai

## A programkönyvtárak felhasználása

### Fixpontos aritmetika

A `fixie` névtér `Fixed` osztálysablonja a `Fixed.h` fájlban található. A példányosításhoz kettő sablonparaméterre van szükség:

- az ábrázolásra használt egész típus,
- a fixpont helye.

```c++
template <typename Int, std::size_t Q> 
struct Fixed;
```

Példányosításkor, fordítási időben a következő elemeket ellenőrizzük:

- `std::is_integral_type<Int>`, illetve
- `Q` belefér-e `Int`-be.

Példák szabályos példányosításokra:

```c++
using fixie::Fixed;

// 1 bit előjel, 31 bit egész rész, 32 bit törtrész
using Fix16ll = Fixed<long long, 32>;

// 1 bit előjel, 0 bit egész rész, 15 bit törtrész
typedef Fixed<short, 15> Fix15s;

// 0 bit előjel, 16 bit egész rész, 16 bit törtrész
using Fix16ul = Fixed<unsigned long, 16>;
```

Tetszőleges számból kiindulva létrehozhatunk fixpontos számot, a konstruktor elvégzi a konverziót.
A konstruktorok minden esetben explicitek, hogy elkerüljük a véletlen konverziókat.
Tetszőleges másik fixpontos számból létrehozhatunk egy újat.
Lehetőség van a fixpontos reprezentáció megadására is, ha egész számot elváró konstruktor második paraméterével utalunk rá, 
hogy ne legyen konverzió.

```c++
auto x = Fix16ll(1);
auto y = Fix16ll(1.0);
auto z = Fix16ll(Fix15s(0.5));
auto w = Fix15s(1<<14, false); // 0.1
```

A beépített számtípusokra való visszatérés explicit típuskonverzió segítségével lehetséges.
Egész konverzió esetén a szám egész részét kapjuk meg.

```c++
auto xx = static_cast<int>(x);
auto yy = static_cast<double>(y);
```

A sablonosztály támogatja az összes, lebegőpontos számokra támogatott aritmetikai és logikai összehasonlító műveletet.
Továbbá lehetőség van fixpontos számok $\log_2$ számítására.

### Digitális szűrők

A `biquad` osztálysablon egy digitális, másodrendű rekurzív lineáris szűrő megvalósítása.
A sablon segítségével lehet meghatározni, hogy a példányosított osztály milyen számábrázolási módszerrel dolgozzon.

```c++
template <typename T>
class biquad;
```

Az osztály a differencia egyenletet használja:

$$y_n = b_0w_n + b_1w_{n-1} + b_2w_{n-2},$$

ahol

$$w_n = x_n - a_1w_{n-1} - a_2w_{n-2}.$$

Az $b_i$, $a_j$ konstruktor-paraméterek. Az `init()` metódussal állíthatjuk vissza a kezdeti állapotot, ahol $w_{-1} = w_{-2} = 0$.

# Fejlesztői dokumentáció

## Megvalósítási terv

### Fixpontos aritmetika

A racionális számok egy egyszerű és hatékony megvalósítása a fixpontos számábrázolás.
A fixpontos szám valamilyen skálázás után egész számként van ábrázolva a memóriában.
A skálázás mértéke függ az architechtúrától, és általában kettőnek valamely hatványa.

Legyen $Q(m,n) \doteq \{ \frac{k}{2^n} | k \in \mathbb{Z}, k \in [-2^m,2^m-1 ] \}$ az előjeles, $n+m+1$ biten ábrázolt fixpontos számok halmaza.
Ekkor egy $q \in Q(m,n)$-nak megfelelő egész szám a memóriában a $\hat{q} = [2^nq] \in [-2^{n+m},2^{n+m-1}-1]$, ahol $[.]$ az egészrész függvény.

Ha $a,b \in Q(m,n)$, akkor

- $\widehat{a+b} \doteq \hat{a} + \hat{b}$,
- $\widehat{ab} \doteq \hat{a}\hat{b}2^{-n}$,
- $\widehat{a/b} \doteq \frac{\hat{a}2^n}{\hat{b}}$,
- $\widehat{a=b} \doteq \hat{a} = \hat{b}$,
- $\widehat{a < b} \doteq \hat{a} < \hat{b}$.

A megvalósítás feladata egy olyan adattípus sablon létrehozása, amire teljesülnek a fentiek,
illetve sablonparaméterként megadható az ábrázoláshoz használható egész típus, valamint a
fixpont helye, azaz $n$ értéke.

Az osztálysablon adjon lehetőséget más beépített számtípusokra egyszerűen elvégezhető konverziókra,
valamint a többi sablonpéldányra.

A példányosítás ne legyen lehetséges degenerált esetekre, ezek vizsgálata fordítási időben történjen.
Minden konverzió legyen explicit.

Szükség van még $\log_2$ számításra fixpontos számokra.

Az osztálysablon felülete a felhasználó felé:

```c++
template <typename Int, std::size_t Q>
struct Fixed {
    using IntType = Int;

    constexpr auto power(); // Q
    constexpr auto denom(); // 2^Q

    IntType repr;
    
    Fixed();
    
    template <typename OtherInt> 
    explict Fixed(OtherInt);
    
    template <typename Floating> 
    explicit Fixed(Floating);

    template <typename OtherInt, std::size_t OtherSize>
    Fixed(Fixed<OtherInt, OtherSize>);

    template <typename OtherInt>
    explicit operator OtherInt();
    
    template <typename Floating>
    explicit operator Floating();

    Fixed& operator+=(Fixed);
    Fixed& operator-=(Fixed);
    Fixed& operator*=(Fixed);
    Fixed& operator/=(Fixed);

    template <typename OtherInt>
    Fixed& operator*=(OtherInt);
    
    template <typename OtherInt>
    Fixed& operator/=(OtherInt);
};

template <typename Int, std::size_t Q>
Fixed<Int, Q> operator-(Fixed<Int, Q>);

template <typename Int, std::size_t Q>
Fixed<Int, Q> operator+(Fixed<Int, Q>, Fixed<Int, Q>);

template <typename Int, std::size_t Q>
Fixed<Int, Q> operator-(Fixed<Int, Q>, Fixed<Int, Q>);

template <typename Int, std::size_t Q>
Fixed<Int, Q> operator*(Fixed<Int, Q>, Fixed<Int, Q>);

template <typename Int, std::size_t Q>
Fixed<Int, Q> operator/(Fixed<Int, Q>, Fixed<Int, Q>);

template <typename Int, std::size_t Q>
bool operator==(Fixed<Int, Q>, Fixed<Int, Q>);

template <typename Int, std::size_t Q>
bool operator!=(Fixed<Int, Q>, Fixed<Int, Q>);

template <typename Int, std::size_t Q>
bool operator==(Fixed<Int, Q>, Fixed<Int, Q>);

template <typename Int, std::size_t Q>
bool operator!=(Fixed<Int, Q>, Fixed<Int, Q>);

template <typename Int, std::size_t Q>
bool operator<(Fixed<Int, Q>, Fixed<Int, Q>);

template <typename Int, std::size_t Q>
bool operator<=(Fixed<Int, Q>, Fixed<Int, Q>);

template <typename Int, std::size_t Q>
bool operator>(Fixed<Int, Q>, Fixed<Int, Q>);

template <typename Int, std::size_t Q>
bool operator>=(Fixed<Int, Q>, Fixed<Int, Q>);

template <typename Int, std::size_t Q>
Fixed<Int, Q> log2(Fixed<Int, Q>);
```

### Digitális szűrők

Biquad-nak nevezzük azokat a másodrendű, lineáris, rekurzív szűrőket, melyeknek kettő zérushelyük és kettő szingularitásuk van.
Az elnevezés a bikvadratikus szóból ered, hiszen a $Z$ tartományban a szűrők átmenet-függvénye kettő darab másodrendű polinom hányadosa,
azaz:

$$H(z) = \frac{b_0+b_1z^{-1}+b_2z^{-2}}{a_0+a_1z^{-1}+a_2z^{-2}}$$

A függvény egyszerűbb alakja miután minden konstanst beosztottunk $a_0$-val:

$$H(z) = \frac{b_0+b_1z^{-1}+b_2z^{-2}}{1+a_1z^{-1}+a_2z^{-2}}$$

Az implementáció során többfelé differencia egyenletből is választhatunk. A legegyszerűbb eset a Direct Form I,
ahol az $a_0$-val való normalizálás után a képlet

$$y_n = b_0x_n + b_1x_{n-1} + b_2x_{n-2} - a_1y_{n-1} - a_2y_{n-2}.$$

A másik, úgynevezett Direct Form II esetén

$$y_n = b_0w_n + b_1w_{n-1} + b_2w_{n-2},$$

ahol

$$w_n = x_n - a_1w_{n-1} - a_2w_{n-2}.$$

Alapvető elvárás, hogy a megvalósítandó osztály ne függjön a számábrázolástól,
még akkor sem, ha az applikáció végig ugyanazt az ábrázolást használja.
A felhasználó döntése legyen az ábrázolás és a kívánt pontosság.
Az ábrázolásnál használt számtípus az osztálysablon paramétere kell, hogy legyen.
További elvárás a felület felé, hogy az osztály belső állapota tetszőleges számú lépés után visszaállítható legyen a kezdeti állapotra,
mintha az objektum éppen abban a pillanatban lett volna létrehozva.
A függvényhívás operátor segítségével imitálható, hogy az objektum tulajdonképpen egy átmenet függvényt jelképezzen.

A fentieknek megfelelően az osztálysablon felülete a következő legyen:

```c++
template <typename Number>
class biquad {
public:
    using NumberType = Number;
    biquad();
    biquad(
        Number b0, Number b1, Number b2
        Number a1, Number a2
    );

    void init();
    Number operator()(Number x);
};
```

K-súlyozás esetén kettő darab biquad szűrő egymás utáni alkalmazásával kapjuk az RMS-el mérni kívánt jelet.
Az ITU BS-1770 ajánlás megadja a biquad-ok által használt konstansok értékét.

Az A-súlyozáshoz a bemeneti jelet harmadoktávokra kell bontanunk, ehhez band-szűrőkre van szükség.
A band-pass szűrők egy lehetséges implementációja egy low-pass és egy high-pass egymás utáni alkalmazása a bemeneti jelre.
Low-pass és high-pass szűrők egyik lehetséges implementációja szintén lehetséges biquad-okkal.

Az $s$-tartományban egy másodrendű low-pass szűrő átmenet függvénye:

$$H(s) = \frac{1}{s^2 + \frac{s}{Q} + 1}.$$

Ahhoz, hogy $Z$ tartományba transzformáljuk a fenti függvényt egyrészt $s$-t ki kell fejeznünk $z$ függvényeként,
másrészt a teljes komplex teret rá kell transzformálnunk a körkörös $z$ síkra.
Ha $F_c$ a low-pass frekvencia, $F_s$ pedig a mintavételi frekvencia, akkor az így kapott transzformáció:

$$s = \frac{1}{K}\frac{z-1}{z+1},$$

ahol $K = \tan \frac{\omega T}{2}$ és $\omega T = 2 \pi \frac{F_c}{F_s}$.

Ha ezt behelyettesítjük az eredeti képletbe, azt kapjuk, hogy

$$H(z) = \frac{1}{(\frac{1}{K}\frac{z-1}{z+1})^2 + \frac{\frac{1}{K}\frac{z-1}{z+1}}{Q} + 1}.$$

További egyszerűsítés és átrendezés után kapjuk, hogy

$$H(z) = \frac{K^2 + 2K^2z_{-1} + K^2z^{-2}}{(K^2 + \frac{K}{Q} + 1) + 2(K^2-1)z^{-1} + (K^2 - \frac{K}{Q} + 1)z^{-2}}.$$

Ezek után a biquad konstansok úgy alakulnak, hogy

$$b_0 = K^2, b_1 = 2K^2 = 2b_0, b_2 = K^2 = b_0,$$

valamint

$$a_0 = K^2 + \frac{K}{Q} + 1, a_1 = 2(K^2-1), a_2 = K^2 - \frac{K}{Q} + 1.$$.

Normalizálás után kapjuk, hogy

$$b_0 = \frac{K^2}{K^2 + \frac{K}{Q} + 1}, b_1 = 2b_0, b_2 = b_0,$$

valamint

$$a_0 = 1, a_1 = \frac{2(K^2-1)}{K^2 + \frac{K}{Q} + 1}, a_2 = \frac{K^2 - \frac{K}{Q} + 1}{K^2 + \frac{K}{Q} + 1}.$$

A szűrőhöz kapcsolódó $Q$ konstans értékének megválasztása a felhasználó feladata, Butterworth szűrők esetén az érték $\frac{1}{\sqrt{2}}$.

Mivel a konstansok kiszámítása nem a mérés közben történik, így lebegőpontos számok használata engedélyezett és pontosság miatt javallott.
A mintavételi frekvencia sablonparaméter. A megvalósítandó osztálysablon felülete legyen a következő:

```c++
template <typename Number, std::size_t Fs>
class LowPass : public biquad<Number> {
public:
    LowPass(double Fc, double Q);
};
```

Hasonló levezetés során kapjuk, hogy a high-pass biquad konstansait:

$$b_0 = \frac{1}{1+\frac{K}{Q} + K^2}, b_1 = -2b_0, b_2 = b_0,$$

valamint

$$a_1 = 2\frac{K^2 -1}{1+\frac{K}{Q} + K^2}, a_2 = \frac{1- \frac{K}{Q} + K^2}{1+\frac{K}{Q} + K^2}.$$

Band-pass szűrők esetén a központi frekvenciából és a sávszélességből könnyedén meghatározható az alkalmazandó
low-pass és high-pass szűrő. A osztálysablon által megvalósítandó felület:

```c++
typename <typename Number, std::size_t Fs>
class BandPass {
public:
    BandPass(double Fc, double bandWidth, double Q);
    Number operator()(Number);
};
```

### Bemeneti jel feldolgozása

A bemeneti jel feldolgozására több problémát is számba kell venni.
Egyrészt olyan mintavételi frekvenciát és számábrázolást kell találnunk,
ami a legtöbb hangkártya által támogatott. A közös pont, amit a jelenleg piacon lévő hangkártyák támogatnak, 
az a 48kHz-es mintavétel $Q(0,15)$ számábrázolással.

Másrészt olyan feldolgozási algoritmusokat kell használni, amelyeket a legtöbb hangkártyával
foglalkozó programozási felület támogat. Szinte minden esetben építhetünk arra,
hogy a feldolgozáshoz használt programkönyvtár a hattérben bufferel számunkra,
illetve hogy valamilyen absztrakt esemény ciklusra fűzhetjük fel az általunk
megvalósított függvényeket, amik meghívódnak a nekik megfelelő események esetén.

Általában a programkönyvtárat felhasználó alkalmazás feladata, hogy a ciklusmagon egyet
iteráljon.

Szükségünk van még a lehetséges bemeneti eszközök listájára, hogy az alkalmazás felhasználója
eldönthesse melyik bemeneti eszközről jövő jelet szeretné az applikációval elemeztetni.

Fel kell készülni arra az esetre, hogy nem áll rendelkezésünkre elegendő mért jelmennyiség,
amikor a ciklus meghívja a jelfeldolgozásra biztosított függvényünket, azaz a méréseknek
képeseknek kell lenniük kezelni, hogy egy mérési intervallum jelmennyisége több részletben kerül
feldolgozásra, illetve hogy a mérési periódus vége nem feltétlen esik egybe a függvény végrehajtásának
legvégével.

Tudni kell kezelni azokat az eseteket is, amikor túl sok bemeneti jel kerül a bufferbe
és az applikáció nem képes időben feldolgozni az adott jelmennyiséget.

### Mérések

Követelmény, hogy az applikáció könnyedén kiegészíthető legyen új mérésekkel.
Ehhez a méréseknek egységes felületet kell nyújtaniuk a külvilág felé.
További elvárás, hogy ez a felület ne növelje a futási időt,
pl. ne járjon virtuális függvény hívással, ezért a mérés sablonparamétere
legyen a mérést futtató entitásnak.

A számábrázolás, illetve annak pontossága,
valamint az egy periódusban mért jel mennyisége fordítási idejű,
azaz sablonparaméter legyen.

Mivel nem biztosítható, hogy egyszerre egy mérési intervallum összes adata
rendelkezésre áll, amikor a felület megfelelő metódusa meghívásra kerül, ezért
a felületnek jeleznie kell, hogy mikor ért véget egy mérési periódus.
Ez a követelmény hatással van a mérésnél használt algoritmusokra is.

Hogy a mérési eredmény szemléltetését megkönnyítsük,
szükségünk van a mérési eredmény által felvehető maximumra is,
a minimum az összes minket érintő mérésben 0.
Természetesen ez az érték függ a felvehető minimális értéktől,
hiszen decibel alapú a skála.

A fentiek alapján az osztályoknak a következő felülettel kell rendelkezniük:

```c++
template <typename Number>
class Measurement {
public:
    static Number max();
    Measurement(std::size_t size);
    bool step(Number in, Number& out);
    void init();
};
```

Itt `Measurement` nem egy konkrét osztály, hanem egy szerződés a forráskód többi része felé.
A `step()` függvénynek `true`-val kell visszatérnie, ha egy mérési intervallum végére érkezett,
továbbá ebben az esetben meg kell hogy hívnia `init()`-et.

Minden elvégzendő mérés alapja az első mérés, azaz root-mean-square számítás.
Tulajdonképpen a többi mérés mindössze az RMS bemenetét változtatja.
Egy tetszőleges RMS-en alapuló mérésnek megfelelő osztály egy lehetséges megvalósítása:

```c++
template<typename Number>
class BasedOnRMS {
public:
    static Number max()
    { 
        return RMS<Number>::max(); 
    }
    
    BasedOnRMS(std::size_t size) : rms_(size) {}
    
    void init(); // to be defined
    bool step(Number in, Number& out)
    {
        if (step(f(in), out) {
            init();
            return true;
        }

        return false;
    }
private:
    Number f(Number x); // to be defined
    RMS<Number> rms_;
};
```

Az **root-mean-square** kalkuláció egy adott intervallumon, azaz

$$20\log_{10}\frac{\sqrt{\sum_{i=1}^{N} \frac{x_i^2}{N}}}{p_0},$$

ahol $N=fT$, $T$ a mérési intervallum hossza másodpercben,
$f$ a mintavételezés frekvenciája és $x_i$ az $i$-ik mért jel,
$p_0$ pedig a legkisebb mérhető érték abszolút értéke.

ITU mérés esetén az alkalmazás a bemeneti jelet $K$ súlyozó szűrőn keresztül eresztve RMS-t számol az intervallumon.
Az K szűrő két biquad szűrő együttese.

Az A-súlyozást ISO 61672:9001 és több nemzet szabványa is a így definiálja:

$$A(f)=\frac{12200^4 \cdot f^4}{(f^2 + 20.6^2) \sqrt{(f^2 + 107.7^2)(f^2 + 737.9^2)} (f^2 + 12200^2)}$$

A függvény görbéjét a 2.6. ábra szemlélteti.

A jelet harmad-oktávokra bontva band-pass szűrőkkel, a kritikus sávnak középértékén számolt súllyal összegezzük, azaz

$$\sum_{j=1}^K{w_j \cdot x_{ij}},$$

ahol $K$ a kritikus sávok száma és $x_{ij}$ pedig a $j$-k sávba szűrt jel.

Mivel a az emberi hallás 20Hz és 20kHz közé tehető, és egy oktáv emelkedés kétszeres szorzónak felel meg a frekvenciában,
ezért 30 darab kritikus sávunk lesz.

Az ITU által megadott biquad konstansok 48kHz-es mintavételhez vannak megadva, 
emiatt a többi mérésnél is azt használjuk az egyszerűség kedvéért.
Továbbá ezt majdnem minden forgalomban lévő hangkártya egységesen támogatja.

### Felhasználói felület

A felhasználói felületet négy fő részre lehet bontani. Lehetőséget kell biztosítani a felhasználónak,
hogy kiválassza a használt bemeneti eszközt egy legördülő listából, valamint beállítsa a mérési intervallum hosszát
egy csúszkán, mely egy tizedmásodperctől egy egész másodpercig terjedhet.
Ezeket a konfigurációs lehetőségeket össze kell fogni. 

Ezek alatt kell lennie a mérés elindítására és a futó mérés leállítására való gombnak.
A gomb megjelenítésének tükröznie kell az állapotát,azaz annak hogy éppen fut-e mérés vagy sem.

Közvetlenül a gomb alatt kell szemléltetni az éppen futó mérések eredményét.
Az eredményeket számszerűleg és vizuálisan jeleznie kell a felhasználó felé egy úgynevezett progress bar segítségével.
A különböző mérések egymás alatt helyezkednek el.

Legalul az alkalmazás írja ki az éppen aktuális állapotát. 
A program állapotától függően legyenek a felhasználói felület más részei aktívak vagy sem.
Ha egy rész nem aktív, azt vizuálisan jelezni kell és az alkalmazásnak meg kell tiltania az interakciót a felhasználóval.

## Megvalósítás

### Fixpontos aritmetika

A példányosítás közben elvégzendő, fordítási idejű ellenőrzéseket az úgynevezett SFINAE (subsitution failure is not an error)
C++-ban használt módszerrel éri el a sablonosztály. Két ellenőrzést hajt végre a fordító.
Egyrészt a reprezentációhoz használt típus ténylegesen egész szám-e (`std::is_integral<Int>`), illetve az egész-,
valamint a törtrészt elválasztó fixpont helye megfelel-e a választott egész számnak: `Q <= 8*sizeof(Int)` .
Ezeket az ellenőrzéseket a felhasználótól elrejtett sablonparaméter formájában tesszük meg `std::enable_if_t` segítségével.

Hasonlóan járunk el a a konstruktorok, illetve a konverziós operátorok esetén. 
Egy `Float` típusról a `std::is_floating_point<Float>` segítségével állapítjuk meg, hogy ténylegesen lebegőpontos-e?

Ahhoz, hogy egy egész számot fixpontos számmá konvertáljunk, egyszerűen meg kell szoroznunk $2^Q$-val,
ami a legtöbb architechtúrán megvalósítható a bitreprezentáció balra tologatásával. 
A visszaalakítás tulajdonképpen egy egész osztás az előző konstanssal, ami jobbra tologatással ekvivalens.
Lebegőpontos számoknál is hasonlóan kapjunk a konverziókat, de ott már lebegőpontos osztást és szorzást végzünk.

A megvalósításnál a kód újrafelhasználása fontos szempont, ezért azoknál az operátoroknál, ahol ugyanazt a műveletet hajtjuk végre,
ott az egyik megvalósítása a másik meghívását jelenti, például a `+` műveletet a `+=` operátor segítségével hajtjuk végre.

A legtöbb operátornál a reprezentációhoz használt egész típus műveletei elvégzik a feladatot fixpontos számok esetén is.
Ez igaz a negálás, összeadás, kivonás, kisebb-mint, egyenlőség, egésszel való szorzás, illetve osztás operátorokra.
Fixpontos szorzás és osztás esetén szükségünk van még bittologatás műveletekre is.
A többi logikai művelet a többi meghívásával kerül megvalósításra:

- $a \neq b \doteq \neg (a = b)$,
- $a \le b \doteq a < b \vee a = b$,
- $a > b \doteq \neg (a \le b)$,
- $a \ge b \doteq \neg (a < b)$.

Mivel $\log_b a = \frac{\log_c a}{\log_c b}$, ezért logaritmust számoló algoritmus elég, ha egy kiválasztott $b$ értékre működik.
A jelfeldolgozásban használatos decibel skála miatt a $b=10$ természetes választás lenne,
azonban a jelenlegi számítógépes architechtúrák hatékonyabb lehetőségeket biztosítanak $b=2$ esetén.

Ha $y = \log_2 x$, akkor természetesen $x = 2^y$. Normalizálás, azaz kettővel való osztások vagy szorzások
(melyek természetesen bittologatások) során elérjük hogy $1 \le x < 2$, valamint megkapjuk $y$ egész részét is.
Néhány architechtúrán lehetőség van megszámolni a 0-k számát az első 1-ig bináris formában, ami tovább egyszerűsíti a normalizálást.

Ha $1 \le x < 2$, akkor $0 \le y < 1$. $y$ 2-adikus tört ábrázolására áttérve kapjuk, hogy $y = \sum{y_i2^{-i}}$, amit átrendezés után

$$y = 2^{-1}(y_1 + 2^{-1}(y_2 + 2^{-1}(y_3 + \dots))).$$

Ahonnan kapjuk, hogy

$$x = 2^{2^{-1}(y_1 + 2^{-1}(y_2 + 2^{-1}(y_3 + \dots)))}.$$

Az algoritmus lépései a következőek:

1. $x$ négyzetre emelésével kapjuk, hogy

$$x^2 = 2^{y_1}2^{2^{-1}(y_2 + 2^{-1}(y_3 + 2^{-1}(y_4 + \dots)))}.$$

Ha $y_1 = 0$, akkor

$$x^2 = 2^{2^{-1}(y_2 + 2^{-1}(y_3 + 2^{-1}(y_4 + \dots)))},$$

különben

$$x^2 = 2 \cdot 2^{2^{-1}(y_2 + 2^{-1}(y_3 + 2^{-1}(y_4 + \dots)))}.$$

2. Ha $x^2 > 2$, akkor $x_1$ mantissza bit 1 és legyen elvégezzük az első lépést $\frac{x^2}{2}$-re, különben 0 és $x^2$-re végezzük el.
Addig ismételjük a lépéseket, amíg el nem érjük a kívánt pontosságot.

### Digitális szűrők

A biquad-ok megvalósítására a Direct Form II képletet választjuk, mert az egyaránt jól működik fixpontos és lebegőpontos számok esetén.
A megvalósított képlet:

$$y_n = b_0w_n + b_1w_{n-1} + b_2w_{n-2},$$

ahol

$$w_n = x_n - a_1w{n-1} -a_2w{n-2}.$$

A konstansok beállítása a konstruktor feladat.

Ha $n=0$, akkor $w_{-1} = w_{-2} = 0$ a választott kezdeti értékek. A kezdeti értékeket az `init()` metódus állítja be, illetve vissza.

Az éppen aktuális $y_n$ kiszámítása a függvényhívás operátor feladata.

Az osztály állapotát a $w_{n-1}$, illetve $w_{n-2}$ aktuális érteke adja ki.

A low-pass szűrőket megvalósító `LowPass` osztályt a `biquad` osztálysablonból kapjuk származtatás útján.
A konstruktor feladata a biquad konstansok beállítása a bemeneti paraméterekre:

- $F_c$, az úgynevezett cut-off frekvencia, ami felett a szűrő nem enged át jelet,
- $F_s$, a mintavétel frekvenciája, valamint
- a használt $Q$ konstans.

A low-pass biquad paramétereket az alábbi képletek alapján számoljuk.
Legyen $K = \tan \frac{\omega T}{2}$ és $\omega T = 2 \pi \frac{F_c}{F_s}$, ekkor

$$b_0 = \frac{K^2}{K^2 + \frac{K}{Q} + 1}, b_1 = 2b_0, b_2 = b_0,$$

valamint

$$a_1 = \frac{2(K^2-1)}{K^2 + \frac{K}{Q} + 1}, a_2 = \frac{K^2 - \frac{K}{Q} + 1}{K^2 + \frac{K}{Q} + 1}.$$

A high-pass szűrők hasonló elven alapulnak. A használt konstansok:

$$b_0 = \frac{1}{1+\frac{K}{Q} + K^2}, b_1 = -2b_0, b_2 = b_0,$$

valamint

$$a_1 = 2\frac{K^2 -1}{1+\frac{K}{Q} + K^2}, a_2 = \frac{1- \frac{K}{Q} + K^2}{1+\frac{K}{Q} + K^2}.$$

A band-pass szűrőket egy low-pass és egy high-pass szűrő egymás utáni alkalmazásával kapjuk.
A konstruktor bemeneti paraméterei a sáv középfrekvenciája, a sávszélesség, valamint a választott Q konstans.
Ha a sáv középfrekvenciája $F_c$, a sávszélesség pedig $d$, akkor

- a low-pass szűrő cut-off frekvenciája $F_c + \frac{d}{2}$, valamint
- a high-pass szűrő cut-off frekvenciája $F_c - \frac{d}{2}$.

A `BandPass` osztály `init()` függvényének feladata a két szűrőnek megfelelő objektum `init()` metódusának meghívása.

### Mérések

Minden, az alkalmazás által elvégzendő mérés alapja a RMS kalkuláció, azaz
$$20\log_{10}\frac{\sqrt{\sum_{i=1}^{N} \frac{x_i^2}{N}}}{2^{-Q}}.$$

A képlet felbontható

$$20\log_{10}\sqrt{\sum_{i=1}^{N} \frac{x_i^2}{N}} - 20\log_{10}2^{-Q},$$

ahonnan kapjuk, hogy

$$20\log_{10}\sqrt{\sum_{i=1}^{N} \frac{x_i^2}{N}} + 20Q\log_{10}2.$$

Legyen $C_Q = 20Q\log_{10}2$. Az összeg első fele tovább alakítható:

$$10\log_{10}\sum_{i=1}^{N} \frac{x_i^2}{N},$$

Tovább bontva kapjuk, hogy

$$10\log_{10}\sum_{i=1}^{N}{x_i^2} + 10\log_{10}N.$$

Legyen $C_N = 10\log_{10}N$. Logaritmus alapját kettőre változtatva:

$$\frac{10}{\log_2 10}\log_{2}\sum_{i=1}^{N}{x_i^2}.$$

Legyen $C = \frac{10}{\log_2 10}$. Ekkor az egész számítás a következőre egyszerűsödik:

$$C_Q + C_N + C\log_{2}\sum_{i=1}^{N}{x_i^2}.$$

Az `RMSdB` osztálysablon a fenti képletet valósítja meg. A `step()` függvény minden lépésben kiszámolja a bemenet négyzetét,
majd hozzáadja azt a számontartott négyzetösszeg részlethez, valamint növel egy számlálót,
aminek segítségével nyílván tartja, hogy hol jár a periódusban. Ha a számláló a mérési intervallum végét jelzi,
akkor a végső képlet alapján kiszámolja az RMS-t. A konstansok kiszámítása az objektum létrehozásánál történik meg,
nem vesz el futási időt a mérés elvégzése közben. Ha `step()` ad vissza új mérési értéket, akkor visszaállítja az objektum kezdeti állapotát az
`init()` függvény meghívásával.

A K súlyozás esetünkben azt jelenti, hogy a bemeneti jelet áteresztjük két biquad-on egymás után.
Az két biquad paraméterei a táblázatokban találhatóak és 48kHz-es mintavételi frekvenciára vonatkoznak.

+-------+---------------------+-------+---------------------+
|       |                     | $b_0$ |  $1.53512485958697$ |
+-------+---------------------+-------+---------------------+
| $a_1$ | $-1.69065929318241$ | $b_1$ | $-2.69169618940638$ |
+-------+---------------------+-------+---------------------+
| $a_2$ |  $0.73248077421585$ | $b_2$ |  $1.19839281085285$ |
+-------+---------------------+-------+---------------------+

: Az első fázis együtthatói

+-------+---------------------+-------+------+
|       |                     | $b_0$ |  $1$ |
+-------+---------------------+-------+------+
| $a_1$ | $-1.99004745483398$ | $b_1$ | $-2$ |
+-------+---------------------+-------+------+
| $a_2$ |  $0.99007225036625$ | $b_2$ |  $1$ |
+-------+---------------------+-------+------+

: Az második fázis együtthatói

Az `ITUBS1770` osztály `init()` függvényének feladata a két fázisnak megfelelő osztályok ugyanazon nevű függvényének meghívása.
A `step()` függvény először alkalmazza a két fázist a bemeneti értékre, majd meghívja 
az `RMSdB` egy példányának ugyanazon nevű függvényét. Ha ez a hívás jelzi a mérési periódus végét,
akkor meghívódik az `init()` függvény. Az osztály konstruktora hozza létre a két fázisnak megfelelő biquad-okat.

Az `AWeighted` osztály esetén a konstruktor feladata a `BandPass` objektumok létrehozása,
valamint a nekik megfelelő

$$A(f)=\frac{12200^4 \cdot f^4}{(f^2 + 20.6^2) \sqrt{(f^2 + 107.7^2)(f^2 + 737.9^2)} (f^2 + 12200^2)}$$

értékek kiszámítása. Mindkettő esetén szükség van a harmadoktávok középfrekvenciájának kiszámítására,
melyet két egymásba ágyazott ciklus segítségével könnyedén megtehetünk. A külső ciklus végig iterál az emberi fül
számára felfogható oktávokon, 20Hz-től kezdve duplázva a frekvenciát egészen 20480Hz-ig.
A belső ciklusban feladata a harmadoktávokon való végigiterálás. Ehhez az oktáv sávszélességét három egyenlő részre osztjuk.
A legbelső ciklusmagban kiszámítjuk a harmadoktáv középfrekvenciáját, valamint a középfrekvenciának megfelelő súlyt.
Továbbá a konstruktor felkészíti a tárolt `RMSdb` példányt a munkára azzal, hogy átadja neki a mérési intervallum hosszát.

A `step()` függvény egy lépésében, először felbontja a jelet harmadoktávokra, azaz keresztül ereszti az összes szűrőn,
majd az adott kritikus sávnak megfelelő súllyal összegzi azokat, majd erre végrehajtja `RMSdB` egy példányának ugyanazon nevű függvényét.
Ha ez a függvény a mérési periódus végét jelzi, akkor minden band-pass szűrőn meghívódik az `init()` függvény.

### Felhasználói felület

A `Config` osztály feladata a konfigurációs lehetőségek logikai és vizuális összefogása.
A logikai összevonás tulajdonképpen tartalmazás, a vizuális pedig származtatás a gtkmm `Gtk::Frame` osztályából.
Mivel `Gtk::Frame`-nek mindössze egy eleme lehet, ezért szükség van még egy közbenső `Gtk::VBox` példányra, melynek feladata,
hogy egymás alá helyezze a beállítható elemeket: 

- egy legördülő listát, azaz `Gtk::ComboBoxText`-t tartalmazó `Gtk::Frame`,
az elérhető bemeneti eszközök listájával, aminek feltöltése az `Application` osztály feladata, valamint
- egy csúszkát (`Gtk::Scale`) tartalmazó `Gtk::Frame`, melyen a mérési intervallum hosszát állíthatja be a felhasználó.
A csúszkán az egymás melletti értékek között 0.1s a különbség, minimum értéke 0.1, maximum pedig 1s.

A `Measurement` osztálysablon a sablonparaméterként kapott mérés elvégzéséért, valamint az aktuális eredmény megjelenítéséért felelős.
Az osztály származtatás útján öröklődik `Gtk::Frame` osztályból, melynek címkéje az aktuális mérést tükrözi és konstruktorparaméter.
A mérési eredmény grafikus megjelenítéséért egy tartalmazott `Gtk::ProgressBar` példány felelős.
`Gtk::ProgressBar` általában valamilyen arányszámot szemléltet 0 és 1 között, de mindkét érték változtatható.
A mi esetünkben a maximális érték a mérés maximális értéke, melyet a `max()` osztálystatikus függvény segítségével kapunk, 
ami jelentősen függ a használt fixpontos ábrázolástól. Ha $Q=16$, akkor `max()` által visszaadott érték $20Q\lg 2$.
A mérést magát a `measure()` függvény végzi a mérés objektum `step()` függvénye segítségével. Ha egy mérési periódus végére értünk,
akkor `measure()` új értéket ad a `Gtk::ProgressBar` objektumnak.

A különböző méréseket a `Measurements` osztálysablon fogja egybe egy `Gtk::VBox` segítségével,
ami sablon paramétereiként megkapja az összes elvégzendő mérésnek megfelelő osztályt,
majd mindegyikből példányosít egy `Measurement` osztályt. Ezen osztályok egy-egy példányát egy `std::tuple<Measurement...>` objektum
tartalmazza. Az osztály `measure()` metódusának feladata az összes tárolt mérésnek megfelelő objektum ugyanazon nevű függvényének meghívása.

A `Gtk::Window`-ból származtatott `Application` osztálynak több feladata van.
Egyrészt tartalmaz egy `PulseAudio` esemény ciklust, és futtatja azt, amikor nincs más dolga (idle-time),
másrészt feldolgozza az úgynevezett callback függvények eredményét. Jelenleg az alábbi függvényhívások eredménye érdekes számunkra:

- a program indításánál csatlakozás a lokális `PulseAudio` szerverhez,
- a bemeneti eszközök listájának elkérése, melyeket konfigurációs állapotba állíthatunk,
- a mérés megkezdésekor csatlakozás a bemeneti eszközhöz,
- a mérés közben a a bemeneti jel feldolgozása, valamint
- buffer túlcsordulás kezelése.

Az osztály fogja össze a képernyőn megjelenő különböző részeket, azaz a `Config` osztály egy példányát,
egy `Gtk::ToggleButton`-t, mellyel a felhasználó a mérést indíthatja, illetve leállíthatja, a mérésekkel példányosítva a `Measurements` sablont,
valamint egy `Gtk::StatusBar`-t, ahol az applikáció jelenlegi állapotát tartjuk számon, illetve mérés közben jelzi a túlcsordulások számát.

Az applikáció állapotától függően a felhasználói felület egyes részei inaktívak lehetnek:

- semmi sem aktív kezdeti állapotban, valamint amikor a program lekéri a bemeneti eszközök listáját,
- konfigurációs állapotban aktív a konfigurációs felület és a mérést indító gomb,
- a bemeneti eszközhöz csatlakozás közben semmi sem aktív,
- mérések elvégzése közben csak a mérést leállító gomb, valamint a méréseket jelző felület aktív.

Indulás után közvetlenül a program kezdeti állapotban van, 
majd miután csatlakozott `PulseAudio`-hoz, lekéri a bemeneti eszközök listáját és átmegy az annak megfelelő állapotba.
Ha megérkezett a bemeneti eszközök listája a program átmegy konfigurációs állapotba, amíg a felhasználó meg nem nyomja a mérést indító gombot.
Ekkor a program csatlakozik a bemeneti eszközhöz és ha ez sikeres, átkerül a mérés állapotba,
ahol addig marad, amíg a felhasználó be nem fejezi a mérést a gombra kattintással.
Ezek után a program visszamegy a bemeneti eszközök listáját elkérő állapotba.
A `setStatus()` metódus felel az állapotátmenetek miatti felhasználói felület átalakításáért.

## Tesztelés

A tesztelés során fontos szempont, hogy a tesztesetek futtatása és az eredmények elvégzése automatikus legyen.
Erre azért van szükség, hogy a későbbi módosítások során ne okozzunk regressziót,
azaz ne rontsuk el az alkalmazásnak, illetve a programkönyvtárnak a már működő részeit. 

Fontos követelmény, hogy az automatikus tesztek kellően gyorsan fussanak, hiszen 
a módosításokról kapott gyors visszajelzés megkönnyíti a fejlesztés menetét. Ha az automatikus tesztek lassan futnak,
akkor kevésbe frekventáltan futtatják őket fejlesztés közben a fejlesztők.

Törekedni kell arra, hogy mérhető legyen a letesztelt funkciók mennyisége.
Ehhez egy jó kiindulási alap annak a mérése, hogy a tesztelés során milyen kódsorokon futunk keresztül.
Ezt százalékos formában szokás közölni.

### Statikus kódanalízis

A legkönnyebben ellenőrizhető hibák közé tartoznak azok, melyeket még fordítási időben ellenőrizhetünk.
Ezért fordításnál a fordítóban bekapcsoljuk az összes lehetséges figyelmeztetést, és minden figyelmeztetést fordítási hibának tekintünk.
Ha úgy véljük, hogy az adott helyen az adott figyelmeztetés nem jelent hibát, akkor és csak azon a helyen elnyomjuk a figyelmeztetés jelentését.

Lehetőség szerint forráskódot további statikus analízisnek vetjük alá és ha kell, akkor az ehhez használt fordítón kívül egyéb szoftvereket is alkalmazunk.

### Futtatási idejű kódanalízis

A tesztesetek futtatása közben olyan futtatási idejű analízisnek vetjük alá az alkalmazást,
melyeket a futtatási környezet rendszerszerű használat közben nem végez el. Ezek növelik a futási időt,
amit a valós idejű követelmények miatt nem engedhetünk meg magunknak.

Ez a lépés többek között a biztonságkritikus hibák felderítésében segít. A teljesség igénye nélkül, a fázis a következőket ellenőrzi:

- felszabadítjuk-e az összes lefoglalt memóriát,
- buffer túl-, illetve alulcsordulás,
- a függvény hívási verem helyes használata.

### Fixpontos aritmetika

A fixpontos számokon elvégzett aritmetikai műveletek tesztelése a lebegőpontos számok segítségével lehetséges.
Először a lebegőpontos számokat átalakítjuk fixpontos számokká, majd elvégezzük a műveleteket mindkét ábrázolás segítségével.
Az eredményt visszaalakítjuk lebegőpontos számmá, majd összehasonlítjuk a lebegőpontos számokon elvégzett, ugyanazon művelet eredményével.
Természetesen figyelembe kell vennünk, hogy a két reprezentáció nem ugyanolyan pontossággal dolgozik.

A fentiekhez azonban először meg kell bizonyosodnunk arról, hogy a lebegőpontos és fixpontos ábrázolás közötti konverziók jól működnek.
Ennek automatikus analíziséhez a következő módszert alkalmazzuk:

1. Veszünk véletlenszerűen egy lebegőpontos számot a fixpontos ábrázolás tartományából.
2. Fixpontos számmá alakítjuk.
3. Visszaalakítjuk lebegőpontos számma.
4. Összehasonlítjuk a két lebegőpontos számot. Meg kell győződnünk róla, hogy nem térnek el többel, mint amit a fixpontos ábrázolás hibaként behoz.
5. A visszaalakított számból újra fixpontos számot képzünk.
6. A két fixpontos számnak meg kell egyeznie.

A kettes alapú logaritmus számításnál a szabványos C programkönyvtár `log` függvény által számolt eredményhez hasonlítjuk a számolt eredményt.
További függvények megvalósításánál hasonlóan járunk el a jövőben.

### Digitális szűrők

A biquad implementáció teszteléséhez szükségünk van egy referencia megvalósításhoz. A nyílt forrású MATLAB klón,
az Octave szoftver csomag rendelkezik a szükséges tulajdonságokkal, hiszen beépített funkció a programkönyvtár által megvalósított Direct Form II.

A tesztelés menete a következő:

1. Véletlenszerű bemeneti jelsorozatot generálunk. Ezt a bemeneti jelet elmentjük, mint referencia input.
2. A jelsorozat keresztül engedjük az Octave megfelelő implementációján.
3. A kimeneti jelet elmentjük, mint a referencia futás outputja.
4. Az elmentett bemeneti jelet keresztül engedjük a saját megvalósításon.
5. A referencia és a számolt kimenetet összehasonlítjuk.

A fentieket végrehajtjuk lebegőpontos és fixpontos számábrázolás mellett is.

A low-pass, illetve a high-pass szűrők esetén kétféle generált jelre van szükségünk:

- egy olyan, amit a szűrő átereszt, azaz a cut-off frekvencia alatt, illetve felett van, valamint
- egy olyan, amit nem.

A band-pass szűrőkhöz három tartományból generálunk jelet:

- a sávszélesség alatti jel,
- a tartomány feletti jel, valamint
- a két frekvencia határ közé eső jel.

Minden esetben a generált jelet egyszerűen képezhetjük az alábbi függvénnyel. Legyen $f$ a generált jel frekvenciája, $N$ az egy periódusba eső jelek száma.
Ekkor a $k$-ik generált jel:

$$\sin \frac{2\pi kf}{N}.$$

Az eredmény ellenőrzésénél azt vizsgáljuk, hogy a kimeneti jel energiája nulla-e? Ha igen, akkor megszűrte a generált jelet, egyébként nem.
A jel energiáját a $\sum_{i=0}^N \frac{x_i^2}{N}$ képlettel kapjuk meg.

### Mérések

A **root-mean-square** számítás tesztelésénél is olyan bemeneti értékeket kell keresnünk, melyekre meg tudjuk mondani,
hogy milyen kimeneti értéket várunk a számítás végén. Elég egy darab mérési periódusra elvégezni a mérést, és a várt értékhez hasonítani a kapott eredményt.

A választott bemenetek és az elvárt eredmények:

- csupa $0$, ilyenkor az elvárt eredmény is $0$,
- csupa $1$, ilyenkor az elvárt eredmény a maximálisan felvehető érték,
- $(-1)^n$, ilyenkor a maximálisan felvehető értéket varjuk el,
- $\sin \frac{2\pi k}{N}$, ilyenkor az elvárt érték az $\pi$ osztva a legkisebb mértékkel.

A többi mérés automatikus tesztelése nem reális követelmény, mert az analízishez nem áll elegendő információ rendelkezésre,
illetve a generátor lefejlesztése ugyanazon hibalehetőségeket hozza magával, mint a tesztelendő implementáció.

### Felhasználói felület és bemeneti jel feldolgozása

A program többi része manuális tesztelésnek van alávetve, hiszen a felhasználói felület,
illetve a hangkártyáról érkező jel feldolgozása nehezen automatizálható. A PulseAudio lehetőséget nyújt úgynevezett monitor eszközök használatára,
ahol egy kimeneti eszközre küldött jelet kapunk vissza bemeneti jelként. Így bemeneti eszköz nélkül is tesztelhetjük az alkalmazást.

A felhasználói felület tesztelésénél a felület egyes elemeinél leellenőrizzük, hogy megfelelnek-e az eléjük támasztott követelményeknek, azaz

- konfigurációs állapotban megnyomható-e a mérést elindító gomb,
- konfigurációs állapotban inaktív-e az alkalmazás mérés eredményét jelző része, illetve kezdeti értéket mutat-e,
- mérés közben megnyomható-e mérést leállító gomb,
- mérés közben inaktív-e a konfigurációs felület,
- a program minden állapotában a megfelelő állapot üzenet tükröződik-e,
- minden állapotban hiba nélkül kiléphetünk-e a programból,
- program jelzi-e a felhasználó felé a fellépő hibaeseteket.

Miután ezekről meggyőződtünk, a felület és a bemeneti jelhez kapcsoló forrás módosítása esetén ezeket a teszteseteket újra ellenőrizni kell,
hogy nem okozunk regressziót a működésben.

## Továbbfejlesztési lehetőségek

Az alkalmazás lehetővé teszi, hogy könnyedén adjunk hozzá új méréseket a rendszerhez. Ehhez több irányban indulhatunk el:

- a K súlyozáshoz hasonlóan egy digitális szűrőn keresztül eresztett bemeneti jelre RMS-t számolunk,
- az A súlyozáshoz hasonlóan a kritikus sávokra bontott jelet súlyozott összegére számolunk RMS-t,
- esetleg a fenti kettőt kombináljuk,
- fentebb fel nem sorolt új mérési formákat vezetünk be.

További lehetőség, hogy az alkalmazásban beállítható legyen a mintavételi frekvencia,
ekkor azonban a ITU BS1700 által ajánlott K frekvencia súlyozás az eredeti formájában nem használható,
hiszen a használt biquad konstansok 48kHz-re vonatkoznak.

A programkönyvtár esetében több matematikai függvény fixpontos megvalósítására van lehetőség.
Jelenleg csak a kettes alapú logaritmus függvényt használjuk, ezért csak azt valósítottuk meg.

A band-pass, low-pass, illetve high-pass digitális szűrőkön felül, további biquad alapú szűrők hozzáadásával lehet erősíteni a programkönyvtárat,
illetve nem biquad alapú szűrőket is készíthetünk.

# Irodalomjegyzék
