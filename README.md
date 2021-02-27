# ParserCourse
Course work, SoftwareConstruct

группа 3530904/80102

команда : Вергун Григорий, Воронин Илья, Сорвенков Павел, Протасов Максим


Определение проблемы: Парсер usb c пакета. Удобный вывод информации в текстовом формате даных хранящихся в usb c пакете 

Выработка требований: 
-Правильное определение раздела SOP(Start of Packet)
-Вывод CRC и Header на экран в шеснадцетиричном формате
-Определение типа пакета, типа сообщения и типа данных
-Вывод правильных данных на экран с подробным описанием всех полей данных
![unknown](https://user-images.githubusercontent.com/31857466/109388140-50657580-7916-11eb-82fd-5d2c2f88676f.png)


ParserUSBC.cpp - файл с функционалом программы
ParserUSBC.h - заголовок с определением класса и структур для объектов

Пакет с данными представленн в виде битовой последовательности
В программе происходит разбор пакета по частям, декодировка из 5bit в 4bit,определение типа каждой части и вывод информации на экран в виде строки
Класс PacketUSBC имеет поле типа string для хранения результата парсинга
В программе реализованны структуры разных типов data object'ов. Так как Data object имеет фиксированный размер 32 bit, мы можем обрабатывать каждый object как int, и накладывать эти данные на структуры, для дальнейшего удобного использования полей стурктур

В функции main реализованна проверка некоторых пакетов с разными типами.

Документация по usbc пакетам взята с официального сайта usb : https://www.usb.org/document-library/usb-power-delivery
