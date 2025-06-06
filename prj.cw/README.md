
# QR Scanner

Простой инструмент для детекции и выделения QR-кодов с веб-камеры и на изображениях.

## Возможности

- **Режим камеры**
– потоковое обнаружение и декодирование QR-кодов с выводом текста на экран и в консоль.
- **Режим фото**
– быстрое обнаружение «квадратной» области QR-кода без декодирования. Визуальное выделение и вывод ссылки в окне и в консоли.


## Требования

- Операционная система: Windows 10/11
- Компилятор: Visual Studio
- Библиотека: OpenCV 4


## Структура проекта

```
prj.cw/
├── include/
│   ├── code_scanner.h
│   └── image_processor.h     
├── src/
│   ├── code_scanner.cpp
│   ├── image_processor.cpp 
│   └── main.cpp      
├── test_data
├── CMakeLists.txt            
└── build.bat                 
```


## Сборка и запуск

1. Откройте Командную строку или PowerShell
2. Перейдите в корень проекта `prj.cw`
3. Выполните:

```
build.bat
```

4. После успешной сборки введите

```shell
cd .\build\Release
```

## Использование

### Режим камеры

```shell
.\qr_detector.exe
```

- Открывается веб-камера
- В каждом кадре пытается декодировать QR-код
- Если код найден, рисуется зелёная рамка и выводится текст
- Нажмите **q** для выхода


### Режим фото

```shell
.\qr_detector.exe <путь_к_изображению>
```

- Загружается указанное изображение
- Ищутся четырёхугольные области, похожие на QR-код
- Рисуется толстая зелёная рамка вокруг найденной области
- После просмотра нажмите любую клавишу для закрытия окна


## Как это работает

- **Camera Mode** (`detectAndDecode`)
Используется встроенный метод OpenCV `QRCodeDetector::detectAndDecode()`. Он одновременно находит уголки QR-кода и возвращает текст.
- **Photo Mode** (`detectPhoto`)
Используется метод `QRCodeDetector::detect()`, который возвращает только четыре угла QR-кода без декодирования.


## Решение возможных проблем

- Убедитесь, что DLL OpenCV (например, из `C:\opencv\build\x64\vc16\bin`) добавлены в системный PATH
- Если камера не открывается, проверьте права доступа или попробуйте другой индекс камеры
- Для режима фото убедитесь, что QR-код занимает заметную часть изображения и контрастирует с фоном
