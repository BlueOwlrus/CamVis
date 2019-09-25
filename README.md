# CamVis
Student project. Implements a mesh clipping algorithm (boolean operation of 2 poligonal meshes).

Это студенческий проект, который реализует алгоритм вычитания одного меша из другого (булевая операция над полигональной сеткой). В качестве практической цели реализации стоит задача симуляция выполнения G-кода на фрезерном ЧПУ станке. Подобные программы обычно уже встроены в CAM/CAD пакеты, они помогают визуально проверить программу обработки детали до того, как отправлять ее на станок.

**Проект реализован не до конца и работает не всегда стабильно. Однако бОльшая часть задач была выполнена - алгоритм позволяет вычитать из одного меша другой, если отсекатель является выпуклым многогранником (ограничение алгоритма в угоду производительности).
Этот проект писался для себя и давно, поэтому я не особо соблюдал правильное форматирование кода (можно заметить смешение camelCase и under_score). Так же надо учитывать, что весь файл main.cpp - является временным и нужен сугубо для тестирования алгоритма. Поэтому там можно найти жестко прописанные пути к файлам и вся отладочная информация отправляется в стандартный вывод. Рендер тоже был написан минималистично, т.к. нужен только для проверки работоспособности алгоритма**

# Алгоритм
Описание алгоритма можно найти в прикрепленном файле Text.pdf

# Примеры
Отсекатель: низкополигональный цилиндр, имитирующий фрезу. Позиционировался в ручном режиме (имитирует сверление фрезой).
![](https://raw.githubusercontent.com/BlueOwlrus/CamVis/master/Pics/Screenshot_4.jpg)
Тот же пример, но уже в режиме скелетной полигональной сетки (отрисовываются только ребра)
![](https://raw.githubusercontent.com/BlueOwlrus/CamVis/master/Pics/Screenshot_5.jpg)



Анимация Симуляция на основе G-кода
![](https://raw.githubusercontent.com/BlueOwlrus/CamVis/master/Pics/CamVisAnim2.gif)


Тот же базовый блок, но в качестве отсекателя - низкополигональная сфера.
![](https://raw.githubusercontent.com/BlueOwlrus/CamVis/master/Pics/Screenshot_6.jpg)
![](https://raw.githubusercontent.com/BlueOwlrus/CamVis/master/Pics/Screenshot_7.jpg)
