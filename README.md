# Rejestrator prędkości wózka wyrzutni BSP

Urządzenie rejestruje prędkość wózka poprzez 4 czujniki hallotronowe  umieszczone na dwu rolkach napędzających wózek. Mierzony jest czas obrotu rolki, a znając jej obwód, można obliczyć prędkość.
Prędkości rolek, pokonana droga i obliczone przyspieszenie liniowe są podawane na prosty filtr uśredniający ostatni pomiar z poprzednią wartością. Zresygnowana z filtra Kalmana ze względu na 
zbyt duży strumień danych, któregi filtr nie jest w stanie obrobć na bieżąco. O ile nie ma problemu z kolejkowaniem aktualizacji, to predykcja powinna być wykonywana na bieżąco.

## Prezentacja pomiarów
Urządzenie zapisuje na kartę SD dane pomiarów w postaci tekstowej, gdzie kolejne dane rozdzielone są spacją oraz znakiem tabulacji. Tak sformatowany tekst daje się zaimportować do arkusza kalkulacyjnego 
poleceniem "Dane" -> "Z pliku tekstowego /CSV". Te same dane są też wysyłane telemetrią przez UART oraz podłączony do niego moduł bluetooth.

### Moduł bluetooth
Aby zarejestrować dane, wystarczy zbliżyć się z laptopem lub telefonem, wykryć urządzenie bluetooth nazywające się "WS-150-3I". Kod parowania urządzenia to "1234". Po sparowaniu na laptopie powinien pojawić się kolejny port szeregowy COM. 
Trzeba uruchomić dowolny terminal, podłączyć się do portu pracującego z prędkością 115200 bps. Po upływie czasu timeoutu wynoszącego 5 sekund od wykrycia ruchu wózka rozpoczyna się wysyłanie danych na terminal.
Jeżeli do rejestracji używany telefonu, należy zainstalować aplikację typu "bluetooth terminal". Po sparowaniu można uruchomić terminal i włączyć w nim logowanie do pliku. 
Dla aplikacji "Serial Bluetooth Terminal" dostępnej na Google play: https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal&hl=pl
 pliki logu można znaleźć w folderze android/Android/data/de.kai_mirich.serial_bluetooth_terminal. Foldr ten jest dostępny po podłączeniu telefonu kablem USB do komputera i wybraniu 
trybu pracy "Przesyłanie plików przez USB".

### Karta SD
Ten sam ciąg znaków jest również zapisywany na kartę SD, jeżeli jest włożona w gniazdo na płytce rejestratora. Zapisywane jest 1000 ostatnich pomiarów w kolejnych plikach z rozszerzeniem csv.
Pliki mają w nazwie kolejne numery. Jeżeli pula numerów się wyczerpie, to starsze dane będą nadpisywane. Urządzenie nie zna daty ani czasu, więc nazwa pliku jest jedynym identyfikatorem pozwalającym odróżnić kolejne pomiary. 
Oprócz plików z danymi urządzenie zakłada jeszcze plik "IndeksPomiaru.txt" gdzie zapisywany jest indeks nazwy kolejnego pliku. Gdy na karcie znajdzie się już 1000 pomiarów, to sprawdzenie ostatniego indeksu pozwala 
sprawdzić który pomiar był wykonywany ostatnio. Najlepiej jednak jest okresowo wyczyścić kartę przenosząc wszystkie pliki na komputer.

## Interpretacja danych pomiarowych
Obie metody pozyskiwania danych pomiarowych prezentują zestaw danych pozyskany od pierwszego zarejestrowanego impulsu do ostatniego, po którym przez czas timeoutu wynoszący 5 sekund nie pojawiły się nowe dane. 
Taki zestaw posiada nagłówek oraz zestaw danych w kolumnach. Kolejność danych jest następująca:
- Nr - indeks kolejnego zarejestrowanego impulsu
- timer[us] - bieący czas przechwycony z timera pracującego z częstotliwością 1 MHz  	 
- L1[m], L2[m], L3[m], L4[m] - droga wózka widziana przez każdy czujnik
- v1[m/s], v2[m/s], v3[m/s], v4[m/s] - prędkości obliczone na podstawie zarejestrowanych czasów i stałej określającej obwód rolki
- a1[m/s^2], a2[m/s^2], a3[m/s^2], a3[m/s^2] - przyspieszenia widziane przez każdy z czujników 
- L[m] - uśredniona droga wózka 
- v[m/s] - uśredniona prędkość wózka 
- a[m/s^2] - uśrednione przyspieszenie wózka 

Dane pomiarowe są rozdzielone znakiem tabulacji i spacją. Liczby są w notacji polskiej z przecinkiem dziesiętnym. W przypadku pracy z anglojęzycznym oprogramowaniem należy zamienić je na kropkę. 
