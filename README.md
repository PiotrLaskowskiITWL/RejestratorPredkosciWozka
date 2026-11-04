# Rejestrator prędkości wózka wyrzutni BSP

Urządzenie rejestruje prędkość wózka poprzez 4 czujniki hallotronowe  umieszczone na rolkach napędzających wózek. Mierzony jest czas obrotu rolki, a znając jej obwód, można obliczyć prędkość.
Prędkości rolek i pokonana droga są podawane na filtr Kalmana, który estymuje drogę, prędkość i przyspieszenie wózka.
Dodatkowo liczony jest parametr nazywany drogą Mahalanobisa, będący odległością pomiaru od jego estymaty, wyrażoną w jednostce odchylenia standardowego. Mówi to, czy dany pomiar jest statystycznie poprawny, 
czy jest to odrzut spowodowany błędami pomiaru.

## Prezentacja pomiarów
Urządzenie  zapisuje dane pomiarów w postaci tekstowej, gdzie kolejne dane rozdzielone są spacją oraz znakiem tabulacji. Tak sformatowany tekst daje się zaimportować do arkusza kalkulacyjnego.
Dane są wysyłane telemetrią przez UART oraz podłączony do niego moduł bluetooth, oraz są zapisywane na kartę SD.

### Moduł bluetooth
Aby zarejestrować dane, wystarczy zbliżyć się z laptopem lub telefonem, wykryć urządzenie bluetooth nazywające się "wyrzutnia". Kod parowania urządzenia to "1234". Po sparowaniu na laptopie powinien pojawić się kolejny port szeregowy COM. 
Trzeba uruchomić dowolny terminal, podłączyć się do portu pracującego z prędkością 115200 bps. Po upływie czasu timeoutu wynoszącego 5 sekund od wykrycia ruchu wózka rozpoczyna się wysyłanie danych na terminal.
Jeżeli do rejestracji używany telefonu, należy zainstalować aplikację typu "bluetooth terminal". Po sparowaniu można uruchomić terminal i włączyć w nim logowanie do pliku. 
Dla aplikacji "Serial Bluetooth Terminal" pliki logu można znaleźć w folderze android/Android/data/de.kai_mirich.serial_bluetooth_terminal. Foldr ten jest dostępny po podłączeniu telefonu kablem USB do komputera i wybraniu 
trybu pracy "Przesyłanie plików przez USB".

### Karta SD
Taki sam ciąg znaków jest również zapisywany na kartę SD, jeżeli jest włożona w gniazdo na płytce rejestratora. Zapisywane jest 100 ostatnich pomiarów w kolejnych plikach z rozszerzeniem csv.
Pliki mają w nazwie kolejne numery. Jeżeli pula numerów się wyczerpie, to starsze dane będą nadpisywane. Urządzenie nie zna daty ani czasu, więc nazwa pliku jest jedynym identyfikatorem pozwalającym odróżnić kolejne pomiary. 
Oprócz plików z danymi urządzenie zakłada jeszcze plik "IndeksPomiaru.txt" gdzie zapisywany jest indeks nazwy kolejnego pliku. Gdy na karcie znajdzie się już setka pomiarów, to sprawdzenie ostatniego indeksu pozwala 
sprawdzić który pomiar był wykonywany ostatnio. Najlepiej jednak jest okresowo wyczyścić kartę przenosząc wszystkie pliki na komputer.

## Interpretacja danych pomiarowych
Obie metody pozyskiwania danych pomiarowych prezentują zestaw danych pozyskany od pierwszego zarejestrowanego impulsu do ostatniego, po którym przez czas timeoutu wynoszący 5 sekund nie pojawiły się nowe dane. 
Taki zestaw posiada nagłówek oraz zestaw danych w kolumnach. Kolejność danych jest następująca:
Nr - numer kolejnego zarejestrowanego impulsu
t1[us], t2[us], t3[us], t4[us] - czas między zarejestrowanymi impulsami dla 4 czujników 	 
v1[m/s], v2[m/s], v3[m/s], v4[m/s] - prędkości obliczone na podstawie zarejestrowanych czasów i stałej określającej obwód rolki
L[m] - droga wózka estymowana filtrem Kalmana na podstawie pomiarów
v[m/s] - prędkość wózka estymowana filtrem Kalmana
a[m/s^2] - przyspieszenie wózka estymowane filtrem Kalmana
PoslizgL1[m], PoslizgL2[m], PoslizgL3[m], PoslizgL4[m] - błędy drogi rolek wózka estymowane filtrem Kalmana
MahalanobisV1[sigma], MahalanobisV2[sigma], MahalanobisV3[sigma], MahalanobisV4[sigma] - droga Mahalanobisa, określająca odległość próbki pomiaru od jej estymaty w jednostkach odchylenia standardowego sigma.

Dane pomiarowe są rozdzielone znakiem tabulacji i spacją. Liczby są w notacji polskiej z przecinkiem dziesiętnym. W przypadku pracy z anglojęzycznym oprogramowaniem należy zamienić je na kropkę. 
