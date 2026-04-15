import cv2
import argparse
import time
import queue
import logging
import numpy as np


class Sensor:
    def get (self):
        raise NotImplementedError("Subclasses must implement method get()")


class SensorX(Sensor):
    def __init__(self, delay: float):
        self._delay = delay
        self._data = 0

    def get(self) -> int:
        time.sleep(self._delay)
        self._data += 1
        return self._data


class SensorCam(Sensor):
        def __init__(self, name, width: int, heigth: int):
            self._name = name
            self._width = width
            self.heigth = heigth

        def get(self):

            return 0

        def __del__(self):
            print("Метод __del__ SensorCam")


class WindowImage:
    def __init__(self, hz):
        self._hz = hz

    def show(img):
        return 0

def parsing():
    parser = argparse.ArgumentParser()
    parser.add_argument("name", help = "Имя камеры в потоке")
    parser.add_argument("width", type = int, help = "Ширина")
    parser.add_argument("height", type = int, help = "Высота")
    parser.add_argument("hz", type = int, help = "Частота отображения картинки")
    args = parser.parse_args()

    print(f"Name: {args.name}")
    print(f"Width: {args.width}")
    print(f"Heigth: {args.height}")
    print(f"Hz: {args.hz}")


def setupLogging():
    return 0

def main():
    parsing()

    sensor0 = SensorX(0.01)
    sensor1 = SensorX(0.1)
    sensor2 = SensorX(1)
    print("Done!")

    return 0


if __name__ == "__main__":
    main()