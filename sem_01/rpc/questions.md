# Вопросы

## Охарактеризуйте задачу булочную

клиент входит в булочную и получает номер максимально выданный + 1 и с этим номером идет на обслуживание при этом он ждет пока не будут обслужанны клиенты с меньшими номерами

## Архитектура

клиент серверная

## Зачем нужен rpcgen

rpcgen создает стабы сервера, клиента, заголовочный файл и скелетоны

## Сколько точек входа

2 - кол-во процедур в *.x файле

## Показать в коде точки входа

на сервере

## Когда вызываются удаленные процедуры

В клиенте в момент вызова удаленной процедуры

## Особенность рпц/зачем он нужен

Реализовать удаленное взаимодействие подобно вызову локальных процедур

## Что такое xdr

Externel Data Representation - Внешнее представление данных

## Какие файлы создает rpcgen

- 2 секелетона
- 2 стаб файла
- мейкфайл
- заголовочный файл
- XDR файл

## Для чего он

Чтобы сделать взаимодействие машинно независимым

## Объяснить что написано в файле xdr

Там перевод из типов данных С (int, double) в типы данных XDR (xdr_int, ...)

## Показать в коде преобразование в XDR

в стаб файлах
