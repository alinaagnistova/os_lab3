### Лабораторная работа №3
#### Вариант: ioctl: task_cputime, inode
#### Задание
Разработать комплекс программ на пользовательском уровне и уровне ярда, который собирает информацию на стороне ядра и передает информацию на уровень пользователя, и выводит ее в удобном для чтения человеком виде.
Программа на уровне пользователя получает на вход аргумент(ы) командной строки (не адрес!), позволяющие идентифицировать из системных таблиц необходимый путь до целевой структуры, осуществляет передачу на уровень ядра, 
получает информацию из данной структуры и распечатывает структуру в стандартный вывод. Загружаемый модуль ядра принимает запрос через указанный в задании интерфейс, определяет путь до целевой структуры по переданному запросу и 
возвращает результат на уровень пользователя.

Интерфейс передачи между программой пользователя и ядром и целевая структура задается преподавателем. Интерфейс передачи может быть один из следующих:

- syscall - интерфейс системных вызовов.
- ioctl - передача параметров через управляющий вызов к файлу/устройству.
- procfs - файловая система /proc, передача параметров через запись в файл.
- debugfs - отладочная файловая система /sys/kernel/debug, передача параметров через запись в файл. <br/>

Целевая структура может быть задана двумя способами:

- Именем структуры в заголовочных файлах Linux
- Файлом в каталоге /proc. В этом случае необходимо определить целевую структуру по пути файла в /proc и выводимым данным.
#### Как использовать
1. Собираем и загружаем модуль ядра
```markdown
cd kernel
make
sudo insmod ioctl_custom.ko 
dmesg | tail
```
>[!IMPORTANT]
> `dmesg | tail` - в логах ищем строку вида:
>
> `Module task_info loaded with wevice major number <major>` для получения `major`-номера устройства
2. Создаем символьное устройство
```markdown
sudo mknod /dev/task_info c <major> 0
sudo chmod 666 /dev/task_info
```
3. Собираем и запускаем пользовательский код
```markdown
cd user
gcc main-user.c -o main-user
./main-user <PID> 1     # получить task_cputime для <PID>
./main-user <PID> 2     # получить inode info для <PID>
```
>[!TIP]
> Проверить корректность значений cpu_time можно с помощью `cat /proc/<PID>/stat` или `cat /proc/<PID>/sched`
>
> Проверить корректность значений inode можно с помощью `stat -L /proc/<PID>/pwd`
4. После окончания работы
```markdown
sudo rmmod ioctl_custom
sudo rm /dev/task_info
make clean #для удаления файлов сборки
```
