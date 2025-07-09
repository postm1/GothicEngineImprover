// Supported with union (c) 2020 Union team

// User API for zTBBox3D
// Add your methods here

// Возвращает индекс оси (0=X, 1=Y, 2=Z) с наибольшим размером AABB
int zTBBox3D::GetLongestAxis() const
{
    // Вычисляем размеры по каждой оси
    const float dx = maxs.n[0] - mins.n[0];
    const float dy = maxs.n[1] - mins.n[1];
    const float dz = maxs.n[2] - mins.n[2];

    // Определяем ось с максимальным размером
    if (dx > dy && dx > dz) {
        return 0; // Ось X самая длинная
    }
    else if (dy > dz) {
        return 1; // Ось Y самая длинная
    }
    else {
        return 2; // Ось Z самая длинная
    }
}

void zTBBox3D::SplitByLongAxis(zTBBox3D& leftBBox, zTBBox3D& rightBBox) const {
    // Находим самую длинную ось
    int axis = GetLongestAxis();

    // Получаем центр Bounding Box
    zVEC3 center = GetCenter();

    leftBBox.Init();
    rightBBox.Init();


    // В зависимости от оси делим BBOX по центру
    if (axis == 0) {  // Разделение по оси X
        // Левый BBox: добавляем минимальные значения, максимум по оси X до центра
        leftBBox.AddPoint(mins);
        leftBBox.AddPoint(zVEC3(center.n[0], maxs.n[1], maxs.n[2])); // Ограничиваем правую часть по оси X

        // Правый BBox: минимальные значения для правого, минимум по оси X от центра
        rightBBox.AddPoint(zVEC3(center.n[0], mins.n[1], mins.n[2])); // Минимум по оси X для правого BBox
        rightBBox.AddPoint(maxs);  // Добавляем максимальные значения
    }
    else if (axis == 1) {  // Разделение по оси Y
        // Левый BBox: добавляем минимальные значения, максимум по оси Y до центра
        leftBBox.AddPoint(mins);
        leftBBox.AddPoint(zVEC3(maxs.n[0], center.n[1], maxs.n[2])); // Ограничиваем правую часть по оси Y

        // Правый BBox: минимальные значения для правого, минимум по оси Y от центра
        rightBBox.AddPoint(zVEC3(mins.n[0], center.n[1], mins.n[2])); // Минимум по оси Y для правого BBox
        rightBBox.AddPoint(maxs);  // Добавляем максимальные значения
    }
    else {  // Разделение по оси Z
        // Левый BBox: добавляем минимальные значения, максимум по оси Z до центра
        leftBBox.AddPoint(mins);
        leftBBox.AddPoint(zVEC3(maxs.n[0], maxs.n[1], center.n[2])); // Ограничиваем правую часть по оси Z

        // Правый BBox: минимальные значения для правого, минимум по оси Z от центра
        rightBBox.AddPoint(zVEC3(mins.n[0], mins.n[1], center.n[2])); // Минимум по оси Z для правого BBox
        rightBBox.AddPoint(maxs);  // Добавляем максимальные значения
    }


}


float zTBBox3D::GetMiddleOfAxis(int axis) const {
    // Получаем минимальное и максимальное значение по указанной оси
    float min = this->mins[axis]; // минимальное значение по оси
    float max = this->maxs[axis]; // максимальное значение по оси

    // Среднее значение по оси
    return (min + max) / 2.0f;
}

void zTBBox3D::MergeBox(const zTBBox3D& box) {
    // Для каждой оси (X, Y, Z) обновляем минимальные и максимальные значения
    for (int axis = 0; axis < 3; ++axis) {
        if (box.mins[axis] < this->mins[axis]) {
            this->mins[axis] = box.mins[axis];  // Обновляем минимальное значение
        }
        if (box.maxs[axis] > this->maxs[axis]) {
            this->maxs[axis] = box.maxs[axis];  // Обновляем максимальное значение
        }
    }
}

float Area() const {
    // Проверка на вырожденный случай (например, нулевой объём)
    if (mins[0] == maxs[0] || mins[1] == maxs[1] || mins[2] == maxs[2]) {
        return 0.0f;
    }

    // Вычисляем длины рёбер по осям X, Y, Z
    float dx = maxs[0] - mins[0]; // Длина по X
    float dy = maxs[1] - mins[1]; // Длина по Y
    float dz = maxs[2] - mins[2]; // Длина по Z

    // Площадь поверхности AABB = 2*(dx*dy + dy*dz + dz*dx)
    return 2.0f * (dx * dy + dy * dz + dz * dx);
}

inline bool IsIntersectingAVX(const zTBBox3D& bbox3D) const;
inline bool IsIntersectingRayAVX(const zVEC3& rayOrigin, const zVEC3& rayDirection, float& scaleMin, float& scaleMax) const;