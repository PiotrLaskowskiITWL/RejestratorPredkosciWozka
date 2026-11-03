# Rejestrator prędkości wózka wyrzutni BSP

Urządzenie rejestruje prędkość wózka poprzez 4 czujniki hallotronowe  umieszczone na rolkach napędzających wózek. Mierzony jest czas obrotu rolki a znając jej obwód można obliczyć prędkość.
Prędkości rolek i pokonana droga są podawane na filtr Kalmana, który estymuje drogę, prędkość i przyspiesznie wózka.
Dodatkowo liczony jest parametr nazywany drogą ahalanobisa, będący odległością pomiaru od jego estymaty, wyrażoną w jednostce odchylenia standardowego. Mówi to czy dany pomiar jest statystycznie poprawny, 
czy jest to odrzut spowodowany błędami pomiaru.

## Prezentacja pomiarów
Urządzenie  zapisuje dane pomairów w postaci tekstowej, gdzie kolejne dane rozdzielone są spacją oraz znakiem tabulacji. Tak sformatowany tekst daje się zaimportować do arkusza kalkulacyjnego.
Dane sa wysyłane telemetrią przez UART oraz podłaczony do niego moduł bluetooth, oraz są zapisywane na kartę SD.

### Moduł bluetooth
Aby zarejestrować dane wystarczy zbliżyc się z laptopem lub telefonem, wykryć urzadzenie bluetooth nazywajśce się "wyrzutnia". Kod parowania urządzenia to "1234". Po sparowaniu na laptopie powinien pojawić się kolejny port szeregowy COM. 
Trzeba uruchomić dowolny terminal, podłączyć się do portu pracującego z prędkością 115200bps. Poupływie timeoutu wynoszacego 5 sekund od wykrycia ruchu wozka rozpoczyna się wysyłanie danych na terminal.
Jeżeli do rejestracju używany telefonu, należy zainstalować aplikację typu "bluterm" (skrót od bluetooth terminal). Po sparowaniu można uruchomić bluterma i włączyć w nim logowanie do pliku. 
Dla aplikacji "Serial Bluetooth Terminal" pliki logu można znaleźć w folderze android/Android/data/de.kai_mirich.serial_bluetooth_terminal. Foldr ten jest dostepny po podłaczeniu telefonu kablem USB do komputera i wybraniu 
trynu pracy "Przesyłanie plików przez USB".

### Karta SD
Taki sam ciag znaków jest równiez zaspisywany na kartę SD jeżeli jest włożona w gniazdo na płytce rejestratora. W kolejnych plikach z rozszerzeniem csv i mających kolejny numer w nazwie, zapisywane jest 100 
ostatnich pomiarów. Starsze dane są nadpisywane przez kolejne. Urządzenie nie zna daty ani czasu, więc  nazwa pliku jest jedynym identyfikatorem pozwalającym odróżnić kolejne pomiary. 
Oprócz plików z danymi urządzenie zakłada jeszcze plik "IndeksPomiaru.txt" gdzie zapisywany jest indeks nazwy kolejnego pliku. Gdy na karcie znajdzie się już setka pomiarów, to sprawdzenie ostatniego indeksu pozwala 
sprawdzić ktory pomair był wykonywany ostatnio. Najleoiej jednak jest okresowo wyczyścić kartę przenosząc wszystkie pliki na komputer.
