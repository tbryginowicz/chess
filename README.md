<h1>Gra w szachy z interfejsem graficznym</h1>

Całość napisana w C, interfejs graficzny zaimplementowany z wykorzystaniem biblioteki SDL.

Żeby poprawnie uruchomić grę w folderze z plikiem binarnym muszą się także znajdować oba pliki .ttf

Uruchomienie programu bez argumentów włącza nową rozgrywkę, lub jako argument można podać zapisaną wcześniej grę (np. ./chess board_state)


<h2>Instalacja</h2>
Do uruchomienia gry potrzebna jest biblioteka SDL i SDL_ttf:
 <br><br>

```
sudo apt-get install libSDL2-2.0

sudo apt-get install libsdl2-ttf-2.0-0
```

Pobrać skompilowany plik i oba fonty z releases i umieścic je w tym samym folderze
```
chmod +x chess
```
```
./chess
```

<h2>Przykładowe funkcje</h2>

<h3>Szach mat</h3>
<img src="https://i.imgur.com/O9kYG4h.gif">

<h3>Bicie w przelocie</h3>
<img src="https://i.imgur.com/fMwpSuQ.gif">

<h3>Roszada</h3>
<img src="https://i.imgur.com/GMsP5BB.gif">

<h3>Pat</h3>
<img src="https://i.imgur.com/mmzLOjs.gif">

<h3>Zapis i ładowanie gry</h3>
<img src="https://i.imgur.com/tjNgMi7.gif">
