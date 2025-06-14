// Supported with union (c) 2020 Union team

// User API for zTBBox3D
// Add your methods here

// ¬озвращает индекс оси (0=X, 1=Y, 2=Z) с наибольшим размером AABB
int zTBBox3D::GetLongestAxis() const
{
    // ¬ычисл€ем размеры по каждой оси
    const float dx = maxs.n[0] - mins.n[0];
    const float dy = maxs.n[1] - mins.n[1];
    const float dz = maxs.n[2] - mins.n[2];

    // ќпредел€ем ось с максимальным размером
    if (dx > dy && dx > dz) {
        return 0; // ќсь X сама€ длинна€
    }
    else if (dy > dz) {
        return 1; // ќсь Y сама€ длинна€
    }
    else {
        return 2; // ќсь Z сама€ длинна€
    }
}

void zTBBox3D::SplitByLongAxis(zTBBox3D& leftBBox, zTBBox3D& rightBBox) const {
    // Ќаходим самую длинную ось
    int axis = GetLongestAxis();

    // ѕолучаем центр Bounding Box
    zVEC3 center = GetCenter();

    leftBBox.Init();
    rightBBox.Init();


    // ¬ зависимости от оси делим BBOX по центру
    if (axis == 0) {  // –азделение по оси X
        // Ћевый BBox: добавл€ем минимальные значени€, максимум по оси X до центра
        leftBBox.AddPoint(mins);
        leftBBox.AddPoint(zVEC3(center.n[0], maxs.n[1], maxs.n[2])); // ќграничиваем правую часть по оси X

        // ѕравый BBox: минимальные значени€ дл€ правого, минимум по оси X от центра
        rightBBox.AddPoint(zVEC3(center.n[0], mins.n[1], mins.n[2])); // ћинимум по оси X дл€ правого BBox
        rightBBox.AddPoint(maxs);  // ƒобавл€ем максимальные значени€
    }
    else if (axis == 1) {  // –азделение по оси Y
        // Ћевый BBox: добавл€ем минимальные значени€, максимум по оси Y до центра
        leftBBox.AddPoint(mins);
        leftBBox.AddPoint(zVEC3(maxs.n[0], center.n[1], maxs.n[2])); // ќграничиваем правую часть по оси Y

        // ѕравый BBox: минимальные значени€ дл€ правого, минимум по оси Y от центра
        rightBBox.AddPoint(zVEC3(mins.n[0], center.n[1], mins.n[2])); // ћинимум по оси Y дл€ правого BBox
        rightBBox.AddPoint(maxs);  // ƒобавл€ем максимальные значени€
    }
    else {  // –азделение по оси Z
        // Ћевый BBox: добавл€ем минимальные значени€, максимум по оси Z до центра
        leftBBox.AddPoint(mins);
        leftBBox.AddPoint(zVEC3(maxs.n[0], maxs.n[1], center.n[2])); // ќграничиваем правую часть по оси Z

        // ѕравый BBox: минимальные значени€ дл€ правого, минимум по оси Z от центра
        rightBBox.AddPoint(zVEC3(mins.n[0], mins.n[1], center.n[2])); // ћинимум по оси Z дл€ правого BBox
        rightBBox.AddPoint(maxs);  // ƒобавл€ем максимальные значени€
    }


}


float zTBBox3D::GetMiddleOfAxis(int axis) const {
    // ѕолучаем минимальное и максимальное значение по указанной оси
    float min = this->mins[axis]; // минимальное значение по оси
    float max = this->maxs[axis]; // максимальное значение по оси

    // —реднее значение по оси
    return (min + max) / 2.0f;
}

void zTBBox3D::MergeBox(const zTBBox3D& box) {
    // ƒл€ каждой оси (X, Y, Z) обновл€ем минимальные и максимальные значени€
    for (int axis = 0; axis < 3; ++axis) {
        if (box.mins[axis] < this->mins[axis]) {
            this->mins[axis] = box.mins[axis];  // ќбновл€ем минимальное значение
        }
        if (box.maxs[axis] > this->maxs[axis]) {
            this->maxs[axis] = box.maxs[axis];  // ќбновл€ем максимальное значение
        }
    }
}