# Planning Poker 

Aplikacja która imituje grę w specyficzny rodzaj pokera pt "Planning Poker".

## Opis działania

Użytkownik łączy się do serwera i wybiera jedną z opcji: 
- utworzenie nowej gry
- dołączenie do istniejącej już gry

Jeżeli użytkownik utworzy nową grę staje się jej administratorem i ma
możliwość zarządzania rozgrywką i podglądu stanu rozgrywki na żywo.

Administrator ma możliwość usuwania graczy, dodawania nowych tematów głosowań, 
rozpoczynania, kończenia oraz powtórzenia głosowania, zakończenia rozgrywki.

Jeżeli użytkownik dołączy do istniejącej gry staje się graczem i proszony jest o podanie nicku
(jeżeli gracz o podanym nicku istnieje proszony jest o podanie innego).

Gracz ma możliwość dołączenia podczas trwającej rozgrywki.

Po dołączeniu do gry nowy gracz oczekuje na dodanie przez administatora
kolejnego tematu.  

Po pojawieniu się tematu dyskusji i rozpoczęciu głosowania każdy z graczy
wybiera kartę z wybranym numerem i czeka na zakończenie głosowania przez administatora.
Karty zawierają liczby pierwsze i odzwierciedlają czasochłonność danego zadania 
(im mniejsza liczba, tym mniej czasu powinno zająć zadanie).
Administator ma podgląd na wybrane przez użytkowników karty w czasie rzeczywistym.

Po zakończeniu głosowania wybrane karty zostają ujawnione wszystkim graczom.
W głosowaniu wygrywa karta z największą ilością głosów, w przypadku remisu
następuje narada i po niej powtórne głosowanie.

W przypadku opuszczenia gry przez gracza w trakcie głosowania jego głos jest unieważniony
a jego nick usuwany jest z planszy.

Jeżeli opuszczającym grę użytkownikiem jest administator, to gra zostaje automatycznie zakończona.





