# Scenariusz 8 - rozgłaszanie i rozsyłanie grupowe

Podstawowym celem zajęć jest pokazanie, jak można wysłać datagramy UDP do wielu odbiorców jednocześnie, a także zaprezentowanie sposobu korzystania z protokołu IPv6. Omówimy także opcje gniazd.

## 1. Rozsyłanie datagramów do wielu odbiorców

Są dwie możliwości wysyłania datagramu jednocześnie do wielu odbiorców: rozgłaszanie (ang. broadcast) oraz rozsyłanie grupowe (ang. multicast). W obydwu przypadkach protokołem transportowym jest protokół UDP. Rozgłaszanie jest stosowne na przykład w protokołach DHCP i ARP.

### Rozgłaszanie (ang. broadcast)

IPv4 przewiduje możliwość rozgłaszania, czyli wysyłania kopii tego samego datagramu jednocześnie do wszystkich interfejsów w sieci lokalnej, do której należy nadawca.
Sposób realizacji takiego wysyłania zależy od użytej warstwy łącza.

Aby wysłać datagram w sposób rozgłoszeniowy należy wysłać go na adres rozgłoszeniowy. Adres rozgłoszeniowy zapisany binarnie zawiera same jedynki, czyli w notacji kropkowej jest to adres `255.255.255.255`. Aby wysłać datagram do wszystkich interfejsów w danej sieci lokalnej, można również użyć adresu ukierunkowanego rozgłaszania dla tej sieci, czyli adresu który ma same jedynki w części adresu dotyczącej hosta. W laboratorium komputerowym używane są adresy z puli `10.1.0.0/20` (maska `255.255.240.0`), czyli adresem ukierunkowanego rozgłaszania jest `10.1.15.255`.

Aby nadawca mógł wysłać komunikat rozgłoszeniowy korzystając z danego gniazda, musi najpierw ustawić odpowiednią opcję, ponieważ rozgłaszanie jest domyślnie zablokowane (szczegóły w pkt. 4).

W IPv6 **nie ma możliwości rozgłaszania**.
Rozsyłanie grupowe (ang. multicast)

Pula adresów 224.0.0.1 – 239.255.255.255 przewidziana jest do wykorzystania jako adresy grupowe dla IPv4. Wiele interfejsów może mieć przydzielony adres z tej puli. Intencja jest taka, aby można było wysłać kopię datagramu jednocześnie do wszystkich interfejsów współdzielących ten sam adres grupowy.

Adresy grupowe dla IPv6 przydziela się z puli ff00::/8, czyli są to adresy, które mają jedynki na początkowych 8 bitach. Pula ta jest podzielona na części dla różnych obszarów rozgłaszania ([szczegóły](https://datatracker.ietf.org/doc/html/rfc4291#section-2.7)).

Aby nadawca mógł skorzystać z rozsyłania grupowego musi najpierw przyłączyć się do grupy rozsyłania grupowego związanej z konkretnym adresem. W tym celu również wykorzystuje się opcje gniazd (szczegóły w pkt. 4).

## 2. Przykładowe programy dla IPv4

Program odbierający datagramy UDP `multi-recv` przyjmuje dwa parametry:

- `multicast_dotted_address` – adres grupowy, na którym ma nasłuchiwać,
- `local_port` – port, na którym ma nasłuchiwać.

Adres grupowy musi być adresem IPv4 podanym w notacji kropkowej. Program nie akceptuje nazw domenowych. Program nasłuchuje na podanym adresie grupowym, na adresie rozgłoszeniowym oraz na adresie jednostkowym. Przykładowe wywołanie:

```asm
./multi-recv 239.10.11.12 10001
```

Program wysyłający datagramy UDP `multi-send` przyjmuje również dwa parametry:

- `remote_address` – adres, na który wysyłane są datagramy,
- `remote_port` – port, na który wysyłane są datagramy.

Przykładowe wywołania:

```asm
./multi-send 255.255.255.255 10001
./multi-send 10.1.15.255 10001
./multi-send 239.10.11.12 10001
```

Programu tego można również użyć, aby wysłać komunikat na adres jednostkowy maszyny, na której działa program nasłuchujący np.:

```asm
./multi-send 10.1.1.21 10001
```

## 3. Przykładowe programy dla IPv6

Program odbierający datagramy UDP `multi-recv6` przyjmuje dwa parametry:

- `multicast_colon_address` – adres grupowy, na którym ma nasłuchiwać,
- `local_port` – port, na którym ma nasłuchiwać.

Adres grupowy musi być adresem IPv6 podanym w notacji z dwukropkami. Program nie akceptuje nazw domenowych. Oprócz podanego adresu program nasłuchuje również na wszystkich adresach przydzielonych maszynie. Przykładowe wywołanie:

```asm
./multi-recv6 ff18::4242 10001
```

Program wysyłający `multi-send6` przyjmuje również dwa parametry:

- `remote_address` – adres, na który wysyłane są datagramy,
- `remote_port` – port, na który wysyłane są datagramy.

Przykładowe wywołanie:

```asm
./multi-send6 ff18::4242 10001
```

Programu tego można też użyć, aby wysłać komunikat na adres jednostkowy np.:

```asm
./multi-send6 2001:6a0:5001:1:21d:9ff:fe04:e7b4 10001
```

## 4. Opcje gniazd

Opcje gniazd umożliwiają zmianę domyślnego trybu wykorzystania gniazd. Do odczytywania oraz ustawiania opcji służą funkcje:

```c
int getsockopt (int socket, int level, int option_name, void *option_value, socklen_t *option_len);
int setsockopt (int socket, int level, int option_name, const void *option_value, socklen_t option_len);
```

gdzie level określa poziom (grupę) opcji:

- `SOL_SOCKET` – opcje gniazd
- `IPPROTO_TCP` – opcje protokołu TCP
- `IPPROTO_IP` – opcje protokołu IP
- `IPPROTO_IPv6` – opcje IPv6

### Opcje dotyczące rozgłaszania (wyłącznie IPv4)

- Uaktywnienie rozgłaszania
```c
optval = 1;
setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void*)&optval, sizeof (optval))
```

### Opcje dotyczące rozsyłania grupowego

- Dołączenie do grupy rozsyłania grupowego IPv4
```c
struct ip_mreq ip_mreq;
ip_mreq.imr_interface.s_addr = ...
ip_mreq.imr_multiaddr = ...

setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&ip_mreq, sizeof(ip_mreq)) 
```
- Opuszczenie grupy rozsyłania grupowego IPv4
```c
setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&ip_mreq, sizeof (ip_mreq))
```
- Dołączenie do grupy rozsyłania grupowego IPv6

```c
struct ipv6_mreq ipv6_mreq;
ipv6_mreq.ipv6mr_interface = ...
ipv6_mreq.ipv6mr_multiaddr = ...


setsockopt(sock, SOL_IPV6, IPV6_ADD_MEMBERSHIP, (void*)&ipv6_mreq, sizeof (ipv6_mreq))
```
- Opuszczenie grupy rozsyłania grupowego IPv6
```c
setsockopt(sock, IPPROTO_IP, IPV6_DROP_MEMBERSHIP, (void*)&ipv6_mreq, sizeof (ipv6_mreq))
```
- Ustanowienie wartości TTL/liczby przeskoków dla wyjściowych pakietów rozsyłania grupowego

```c
optval = ...
setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&optval, sizeof (optval))
setsockopt(sock, SOL_IPV6, IPV6_MULTICAST_HOPS, (void*)&optval, sizeof (optval))
```

- Włączenie / wyłączenie dostarczania pakietów rozsyłanych grupowo do siebie

```c
optval = 1 / 0
setsockopt(sock, SOL_IP, IP_MULTICAST_LOOP, (void*)&optval, sizeof (optval))
setsockopt(sock, SOL_IPV6, IPV6_MULTICAST_LOOP, (void*)&optval, sizeof (optval))
```
### Przykłady innych opcji

- Wyłączenie/włączenie algorytmu Nagle’a: `level=IPPROTO_TCP`, `option_name=TCP_NODELAY`

- Wyłączenie/włączenie opcji podtrzymywania aktywności: `level=SOL_SOCKET`, `option_name=SO_KEEPALIVE`

- Ustawienie parametrów dla opcji podtrzymywania aktywności: `level=IPPROTO_TCP`, `option_name=TCP_KEEPCNT/TCP_KEEPIDLE/TCP_KEEPINTV`

Więcej informacji w sekcji 7 podręcznika dla `socket`, `tcp`, `ip` i `ipv6` (np. `man 7 socket`).

## 5. Funkcje konwersji adresów użyte w przykładach

`int inet_pton (int af, const char *src, void *dst)` – konwersja adresów z postaci tekstowej, kropkowej lub z dwukropkami, czyli z domyślnej reprezentacji

- `af` – rodzina adresów: `AF_INET` lub `AF_INET6`
- `src` – wskaźnik do napisu zawierającego adres
- `dst` – wskaźnik do struktury adresowej

`inet_aton` – konwersja adresów IPv4 z postaci tekstowej do binarnej (szczególny przypadek funkcji `inet_pton`), fukcja nie ustawia `errno`.

## 6. Ćwiczenia

- Przeczytaj kod przykładowych programów. Zwróć uwagę na rodzinę adresów w funkcji `socket` w przypadku wykorzystania IPv6 oraz na użyte struktury adresowe i sposób ich wypełniania.

- Zmodyfikuj kod programu `multi-recv`, tak aby wypisywał adres i numer portu, z którego otrzymał dane.

- Sprawdź czy program `multi-recv6` działa z programem `multi-send`? A czy program `multi-recv` działa z programem `multi-send6`? Poeksperymentuj.

## 7. Ćwiczenie punktowane (1.5 pkt)

Napisz programy `time-server` i `time-client`.

Zarówno klient jak i serwer przyjmują dwa parametry - adres grupowy oraz numer portu, na którym nasłuchuje serwer.

Klient rozsyła na podany adres grupowy komunikat "GET TIME".

Jeśli klient nie otrzyma odpowiedzi od żadnego serwera w czasie 3 sekund, to ponawia żądanie. Po wysłaniu żądania klient wypisuje na standardowe wyjście:

```console
Sending request [n]
```

gdzie *n* jest kolejnym numerem żądania (1, 2 lub 3).

Jeśli po 3 próbach klient nie otrzyma żadnej odpowiedzi, wypisuje komunikat:
```console
Timeout: unable to receive response. 
```

i kończy działanie.

Jeśli klient dostanie odpowiedź, to wypisuje adres IP serwera, od którego ją otrzymał oraz przekazany czas. Przykładowy wynik:

```console
Response from: 192.168.2.50
Received time: Sat Apr 17 17:11:18 2021
```

Serwer po otrzymaniu komunikatu `GET TIME`, odsyła klientowi (na jego adres jednostkowy) komunikat z bieżącym czasem. Po otrzymaniu komunikatu wypisuje adres IP klienta, który wysłał żądanie:

```console
Request from: 192.168.2.50
```

Jeśli serwer otrzyma inny komunikat niż `GET TIME`, wypisuje go i nie przesyła odpowiedzi klientowi:

```console
Received unknown command: xxxx
```

Serwer nie kończy w tym przypadku działania.

Programy można napisać dla wersji 4 lub 6 protokołu IP. W przypadku wybrania wersji 6, prosimy o nazwanie ich `time-server6` i `time-client6`.

Wskazówka: klient może użyć funkcji `poll` z jednym gniazdem w obserwowanym zbiorze, aby oczekiwać na odpowiedź tylko przez ustalony czas.


Rozwiązania można prezentować w trakcie zajęć nr 8 lub 9. 
