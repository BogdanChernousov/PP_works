import cv2
import argparse
import time
import threading
import queue
from ultralytics import YOLO
import os


def parse_args():
    parser = argparse.ArgumentParser(description="Многопоточная обработка кадров")
    parser.add_argument("path", help="путь к видео")
    parser.add_argument(
        "workmode", choices=["single", "multi"], help="Однопоточный/многопоточный"
    )
    parser.add_argument("name", help="Имя выходного файла")
    parser.add_argument("--num_workers", type=int, default=4, help="Количество потоков")
    return parser.parse_args()


class VideoProcessor:
    def __init__(self, video_path, output_path, num_workers=4):
        self.video_path = video_path
        self.output_path = output_path
        self.num_workers = num_workers

        # Буферы
        self.input_queue = queue.Queue(maxsize=50)
        self.output_queue = queue.Queue()

        # Для восстановления порядка
        self.frame_buffer = {}
        self.next_frame_id = 0
        self.total_frames = 0

    def get_video_info(self):
        """Получить информацию о видео"""
        cap = cv2.VideoCapture(self.video_path)
        self.fps = cap.get(cv2.CAP_PROP_FPS)
        self.width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        self.height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        self.total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
        cap.release()
        print(
            f"Видео: {self.width}x{self.height}, {self.fps:.2f} fps, {self.total_frames} кадров"
        )

    def read_video(self):
        """Чтение кадров из видео в входной буфер"""
        cap = cv2.VideoCapture(self.video_path)
        frame_id = 0

        while True:
            ret, frame = cap.read()
            if not ret:
                break

            self.input_queue.put((frame_id, frame))
            frame_id += 1

            if frame_id % 30 == 0:
                print(f"Прочитано {frame_id}/{self.total_frames} кадров")

        cap.release()

        for _ in range(self.num_workers):
            self.input_queue.put((None, None))

    def process_frames_worker(self, worker_id):
        """Поток-воркер для обработки кадров (каждый поток имеет свою модель)"""
        # Создаем отдельную модель для каждого потока
        model = YOLO("yolov8s-pose.pt")

        while True:
            frame_id, frame = self.input_queue.get()

            if frame is None:
                break

            try:
                results = model(frame, verbose=False)
                processed_frame = results[0].plot()

                self.output_queue.put((frame_id, processed_frame))

            except Exception as e:
                print(f"Ошибка в воркере {worker_id}: {e}")
                self.output_queue.put((frame_id, frame))  # Отправляем исходный кадр

        print(f"Воркер {worker_id} завершил работу")

    def write_video(self):
        """Запись видео с восстановлением порядка"""
        fourcc = cv2.VideoWriter_fourcc(*"mp4v")
        out = cv2.VideoWriter(
            self.output_path, fourcc, self.fps, (self.width, self.height)
        )

        processed_count = 0

        while processed_count < self.total_frames:
            try:
                frame_id, frame = self.output_queue.get(timeout=1)

                # Буферизируем кадры для восстановления порядка
                self.frame_buffer[frame_id] = frame

                # Выдаем кадры по порядку
                while self.next_frame_id in self.frame_buffer:
                    out.write(self.frame_buffer[self.next_frame_id])
                    del self.frame_buffer[self.next_frame_id]
                    self.next_frame_id += 1
                    processed_count += 1

                    if processed_count % 30 == 0:
                        print(f"Записано {processed_count}/{self.total_frames} кадров")

            except queue.Empty:
                print("Ожидание кадров...")
                continue

        out.release()
        print("Запись видео завершена")

    def run_single_thread(self):
        """Однопоточная обработка"""
        print("Запуск однопоточной обработки...")

        cap = cv2.VideoCapture(self.video_path)
        fourcc = cv2.VideoWriter_fourcc(*"mp4v")
        out = cv2.VideoWriter(
            self.output_path, fourcc, self.fps, (self.width, self.height)
        )

        model = YOLO("yolov8s-pose.pt")

        start_time = time.time()
        frame_count = 0

        while True:
            ret, frame = cap.read()
            if not ret:
                break

            results = model(frame, verbose=False)
            processed_frame = results[0].plot()
            out.write(processed_frame)

            frame_count += 1
            if frame_count % 30 == 0:
                print(f"Обработано {frame_count}/{self.total_frames} кадров")

        elapsed = time.time() - start_time

        cap.release()
        out.release()

        print(
            f"Однопоточная обработка: {elapsed:.2f} сек, FPS: {frame_count/elapsed:.2f}"
        )
        return elapsed

    def run_multi_thread(self):
        """Многопоточная обработка"""
        print(f"Запуск многопоточной обработки с {self.num_workers} потоками...")

        start_time = time.time()

        reader_thread = threading.Thread(target=self.read_video)
        reader_thread.start()

        # Запускаем потоки-воркеры
        workers = []
        for i in range(self.num_workers):
            worker = threading.Thread(target=self.process_frames_worker, args=(i,))
            worker.start()
            workers.append(worker)

        self.write_video()

        reader_thread.join()
        for worker in workers:
            worker.join()

        elapsed = time.time() - start_time

        print(
            f"Многопоточная обработка ({self.num_workers} потоков): {elapsed:.2f} сек, FPS: {self.total_frames/elapsed:.2f}"
        )
        return elapsed


def main():
    args = parse_args()

    processor = VideoProcessor(args.path, args.name, args.num_workers)
    processor.get_video_info()

    if args.workmode == "single":
        processor.run_single_thread()
    else:
        processor.run_multi_thread()

    print(f"\nГотово! Результат сохранен в {args.name}")


if __name__ == "__main__":
    main()
