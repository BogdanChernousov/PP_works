import cv2
import argparse
import numpy as np
import time
import queue
import logging
import threading
import os
import sys
from datetime import datetime


def setup_logging():
    log_dir = "log"
    os.makedirs(log_dir, exist_ok=True)

    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s [%(levelname)s] %(message)s",
        handlers=[
            logging.FileHandler(
                f"{log_dir}/app_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log"
            ),
            logging.StreamHandler(),
        ],
    )
    return logging.getLogger(__name__)


logger = setup_logging()


class Sensor:
    def get(self):
        raise NotImplementedError("Subclasses must implement method get()")

    def start(self):
        # Запуск потока чтения данных
        pass

    def stop(self):
        # Остановка потока
        pass


class SensorX(Sensor):
    def __init__(self, delay: float, name: str = "SensorX"):
        self._delay = delay
        self._data = 0
        self._name = name
        self._running = False
        self._thread = None
        self._queue = queue.Queue(maxsize=1)

    def start(self):
        self._running = True
        self._thread = threading.Thread(target=self._run, name=self._name, daemon=True)
        self._thread.start()
        logger.info(f"{self._name} запущен с частотой {1/self._delay:.1f} Hz")

    def stop(self):
        self._running = False
        if self._thread:
            self._thread.join(timeout=1.0)
        logger.info(f"{self._name} остановлен")

    def _run(self):
        while self._running:
            time.sleep(self._delay)
            self._data += 1
            try:
                self._queue.put_nowait(self._data)
            except queue.Full:
                try:
                    self._queue.get_nowait()
                    self._queue.put_nowait(self._data)
                except queue.Empty:
                    pass

    def get(self) -> int:
        try:
            return self._queue.get_nowait()
        except queue.Empty:
            return self._data  # возвращаем последнее значение

    def __del__(self):
        self.stop()


class SensorCam(Sensor):
    def __init__(self, name: str, width: int, height: int):
        self._name = name
        self._width = width
        self._height = height
        self._cap = None
        self._running = False
        self._thread = None
        self._queue = queue.Queue(maxsize=1)
        self._last_frame = None
        self._init_camera()

    def _init_camera(self):
        try:
            cam_id = int(self._name) if self._name.isdigit() else self._name
        except ValueError:
            cam_id = self._name

        self._cap = cv2.VideoCapture(cam_id, cv2.CAP_V4L2)

        if not self._cap.isOpened():
            logger.error(f"Не удалось открыть камеру: {self._name}")
            sys.exit(1)

        self._cap.set(cv2.CAP_PROP_FRAME_WIDTH, self._width)
        self._cap.set(cv2.CAP_PROP_FRAME_HEIGHT, self._height)

        actual_w = self._cap.get(cv2.CAP_PROP_FRAME_WIDTH)
        actual_h = self._cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
        logger.info(f"Камера {self._name} открыта, разрешение: {actual_w}x{actual_h}")

    def start(self):
        self._running = True
        self._thread = threading.Thread(target=self._run, name="SensorCam", daemon=True)
        self._thread.start()
        logger.info("SensorCam запущен")

    def stop(self):
        self._running = False
        if self._thread:
            self._thread.join(timeout=1.0)
        if self._cap:
            self._cap.release()
        logger.info("SensorCam остановлен")

    def _run(self):
        while self._running:
            ret, frame = self._cap.read()
            if not ret:
                logger.error(
                    "Ошибка чтения кадра с камеры"
                )
                time.sleep(0.1)
                continue

            # Кладём кадр в очередь, заменяя старый
            try:
                self._queue.put_nowait(frame)
                self._last_frame = frame
            except queue.Full:
                try:
                    self._queue.get_nowait()
                    self._queue.put_nowait(frame)
                    self._last_frame = frame
                except queue.Empty:
                    pass

    def get(self):
        try:
            return self._queue.get_nowait()
        except queue.Empty:
            return self._last_frame

    def __del__(self):
        self.stop()


class WindowImage:
    def __init__(self, hz: float, window_name: str = "Sensor Display"):
        self._hz = hz
        self._window_name = window_name
        self._delay = 1.0 / hz
        cv2.namedWindow(self._window_name, cv2.WINDOW_NORMAL)
        logger.info(f"Окно '{self._window_name}' создано, частота обновления: {hz} Hz")

    def show(self, img):
        if img is not None:
            cv2.imshow(self._window_name, img)
        else:
            logger.warning("Попытка показать пустое изображение")

    def wait_key(self):
        return cv2.waitKey(int(self._delay * 1000)) & 0xFF

    def __del__(self):
        try:
            cv2.destroyWindow(self._window_name)
        except:
            pass


def create_display_frame(cam_frame, sensor_data, width=800, height=600):
    if cam_frame is None:
        cam_frame = np.zeros((height, width, 3), dtype=np.uint8)
        cv2.putText(
            cam_frame,
            "No camera signal",
            (50, 300),
            cv2.FONT_HERSHEY_SIMPLEX,
            1,
            (0, 0, 255),
            2,
        )
    else:
        cam_frame = cv2.resize(cam_frame, (width, height))

    # Работа с выводом
    y_offset = 30
    cv2.putText(
        cam_frame,
        f"Sensor 100Hz: {sensor_data.get('s100', 'N/A')}",
        (10, y_offset),
        cv2.FONT_HERSHEY_SIMPLEX,
        0.6,
        (0, 255, 0),
        2,
    )
    y_offset += 30
    cv2.putText(
        cam_frame,
        f"Sensor 10Hz: {sensor_data.get('s10', 'N/A')}",
        (10, y_offset),
        cv2.FONT_HERSHEY_SIMPLEX,
        0.6,
        (0, 255, 0),
        2,
    )
    y_offset += 30
    cv2.putText(
        cam_frame,
        f"Sensor 1Hz: {sensor_data.get('s1', 'N/A')}",
        (10, y_offset),
        cv2.FONT_HERSHEY_SIMPLEX,
        0.6,
        (0, 255, 0),
        2,
    )
    y_offset += 30
    cv2.putText(
        cam_frame,
        "Press 'q' to quit",
        (10, y_offset),
        cv2.FONT_HERSHEY_SIMPLEX,
        0.5,
        (255, 255, 255),
        1,
    )

    return cam_frame


def parse_args():
    parser = argparse.ArgumentParser(description="Многопоточная система с датчиками")
    parser.add_argument("name", help="Имя камеры (0, /dev/video0)")
    parser.add_argument("width", type=int, help="Ширина кадра")
    parser.add_argument("height", type=int, help="Высота кадра")
    parser.add_argument("hz", type=float, help="Частота отображения (Hz)")
    return parser.parse_args()


def main():
    args = parse_args()

    logger.info(
        f"Запуск программы: камера={args.name}, {args.width}x{args.height}, {args.hz} Hz"
    )

    cam = SensorCam(args.name, args.width, args.height)
    sensor_100hz = SensorX(0.01, "Sensor_100Hz")
    sensor_10hz = SensorX(0.1, "Sensor_10Hz")
    sensor_1hz = SensorX(1.0, "Sensor_1Hz")

    sensors = [cam, sensor_100hz, sensor_10hz, sensor_1hz]

    for s in sensors:
        s.start()

    window = WindowImage(args.hz)

    try:
        while True:
            cam_frame = cam.get()
            sensor_data = {
                "s100": sensor_100hz.get(),
                "s10": sensor_10hz.get(),
                "s1": sensor_1hz.get(),
            }

            display = create_display_frame(cam_frame, sensor_data)
            window.show(display)

            if window.wait_key() == ord("q"):
                logger.info("Нажата клавиша 'q', завершение работы")
                break

    except KeyboardInterrupt:
        logger.info("Программа прервана пользователем")

    finally:
        logger.info("Остановка датчиков...")
        for s in sensors:
            s.stop()
        logger.info("Программа завершена")
        cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
