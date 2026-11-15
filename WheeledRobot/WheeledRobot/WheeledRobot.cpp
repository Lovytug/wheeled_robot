#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <chrono>
#include <string>

using namespace std;

atomic<bool> running(true);


//потокобезопасная очередь
class MessageQueue
{
private:
	queue<string> messages;
	mutex mtx;
	condition_variable cv;
	bool closed = false;

public:

	void send(const string& message)
	{
		{
			lock_guard<mutex> lock(mtx);

			if (closed)
				return;

			messages.push(message);
		}

		cv.notify_one();
	}

	bool receive(string& message)
	{
		unique_lock<mutex> lock(mtx);

		cv.wait(lock, [&]() {
			return !messages.empty() || closed;
			});

		if (messages.empty())
			return false;

		message = messages.front();
		messages.pop();

		return true;
	}

	void close()
	{
		{
			lock_guard<mutex> lock(mtx);
			closed = true;
		}

		cv.notify_all();
	}
};



class Communication
{
public:
	MessageQueue raspberryToArduino;
	MessageQueue arduinoToRaspberry;
};



MessageQueue raspberryOutgoing;
MessageQueue raspberryIncoming;

MessageQueue arduinoOutgoing;
MessageQueue arduinoIncoming;

MessageQueue raspberryResponses;



void RaspberrySender(Communication& channel)
{
	string message;

	while (raspberryOutgoing.receive(message))
	{
		channel.raspberryToArduino.send(message);

		cout << "[PI SEND] " << message << endl;
	}
}




void RaspberryReceiver(Communication& channel)
{
	string message;

	while (channel.arduinoToRaspberry.receive(message))
	{
		raspberryIncoming.send(message);
	}
}



void RaspberryProcessor()
{
	string message;

	while (raspberryIncoming.receive(message))
	{
		cout << "[PI PROCESS] Получено: "
			<< message << endl;

		// Здесь обработка данных Arduino
		raspberryResponses.send(message);
	}
}



void NotifyOperator(const string& message)
{
	cout << "[OPERATOR] " << message << endl;
}


void ExecuteMovement(const Point& point)
{
	int currentX, currentY = 0;
	getPolarFromDecard(point.x, point.y, currentX, currentY);
	raspberryOutgoing.send("Execute movement: tetha, d");
	cout << "[PI] Отправил команду на перемещение в точку ("
		<< point.x << ", " << point.y << ")" << endl;
}

void getPolarFromDecard(int x, int y, int currentX, int currentY) {
	cout << "[PI] перевел из x y в tetha, d";

}

void EnableRGBDCamera()
{
	cout << "[PI] Включение RGB-D камеры" << endl;
}

void ProcessCameraData()
{
	cout << "[PI] Обработка данных с камеры" << endl;
}


bool IsObstacleDetected()
{
	cout << "[PI] Проверка наличия препятствия" << endl;


	return false;
}


void AddObstacleToGlobalMap()
{
	cout << "[PI] Нанесение препятствия на глобальную карту" << endl;
}

vector<Point> CalculateScanningRoute(const vector<Point>& path)
{
	cout << "[PI] Вычисление маршрута сканирования" << endl;


	return path;
}
Point getDistance()
{
	cout << "[PI] Определение расстояния d и theta" << endl;
	//определили какую то d и theta
	return Point(100, 100);
}

void LabelSelection()
{
	cout << "[PI] Выделение метки" << endl;
}

bool IsObjectFound()
{
	cout << "[PI] Проверка результата обнаружения" << endl;

	// Заглушка: объект пока не найден
	return false;
}





string WaitForArduinoResponse()
{
	string response;

	cout << "[PI] Ожидание ответа Arduino..." << endl;

	if (!raspberryResponses.receive(response))
		return "CONNECTION_LOST";

	return response;
}



void RaspberryMain()
{
	cout << "\n========== ЗАПУСК RASPBERRY PI ==========\n";

	//Считывание вариантов маршрутов

	cout << "\n[PI] Считывание вариантов маршрутов\n";

	vector<vector<Point>> paths = ReadPaths();

	//Перебор маршрутов

	for (size_t route = 0; route < paths.size(); route++)
	{
		cout << "\n====================================\n";
		cout << "[PI] Маршрут " << route + 1
			<< " из " << paths.size() << endl;
		cout << "====================================\n";

		//Перебор точек маршрута

		for (size_t routePart = 0;
			routePart < paths[route].size();
			routePart++)
		{
			cout << "\n[PI] Участок маршрута "
				<< routePart + 1 << endl;

			// Выполнить перемещение
			ExecuteMovement(paths[route][routePart]);

			string response = WaitForArduinoResponse();

			if (response == "MOVE_COMPLETED")
			{
				cout << "[PI] Перемещение выполнено" << endl;
				continue;
			}
			else if (response == "WHEEL_STUCK")
			{
				NotifyOperator("Колесо отвалилось");
				//TODO сделать какой то выход из всего цикла

			}
			else if (response == "OBSTACLE_DETECTED")
			{
				NotifyOperator("Обнаружено препятствие");
				EnableRGBDCamera();
				// Обработать данные с камеры
				ProcessCameraData();
				// Проверить наличие препятствия
				if (IsObstacleDetected())
				{
					AddObstacleToGlobalMap();
				}
				//TODO перейти к новому формированию маршутов
				//сейчас пока просто выход из функции
				return;
			}
			else
			{
				NotifyOperator("Ошибка связи или неизвестный ответ Arduino");
				//TODO выход из цикла

			}

		}


		//Вычисление маршрута сканирования
		cout << "\n[PI] Подготовка к сканированию\n";

		EnableRGBDCamera();

		vector<Point> scanningRoute =
			CalculateScanningRoute(paths[route]);

		//Перебор участков сканирования

		for (size_t part = 0;
			part < scanningRoute.size();
			part++)
		{
			cout << "\n[PI] Участок сканирования "
				<< part + 1 << endl;

			// Выполнить перемещение
			ExecuteMovement(scanningRoute[part]);

			string response = WaitForArduinoResponse();

			if (response == "MOVE_COMPLETED")
			{
				cout << "[PI] Перемещение выполнено, сканируй" << endl;
				LabelSelection();
				if (IsObjectFound())
				{
					// Определить расстояние и высоту
					Point point = getDistance();
					// Сообщить оператору
					NotifyOperator("Объект обнаружен");
					ExecuteMovement(point);

					string response = WaitForArduinoResponse();

					if (response == "MOVE_COMPLETED")
					{
						cout << "[PI] Перемещение выполнено, метка найдена" << endl;
						cout << "\n[PI] Маршрут " << route + 1
							<< " пройден\n";
						//TODO сделать переход к следующему маршруту
					}
					else if (response == "WHEEL_STUCK")
					{
						NotifyOperator("Колесо отвалилось");
						//TODO сделать какой то выход из всего цикла

					}
					else if (response == "OBSTACLE_DETECTED")
					{
						NotifyOperator("Обнаружено препятствие");
						EnableRGBDCamera();
						// Обработать данные с камеры
						ProcessCameraData();
						// Проверить наличие препятствия
						if (IsObstacleDetected())
						{
							AddObstacleToGlobalMap();
						}
						//TODO перейти к новому формированию маршута сканирования
						//сейчас пока просто выход из функции
						return;
					}
					else
					{
						NotifyOperator("Ошибка связи или неизвестный ответ Arduino");
						//TODO выход из цикла

					}
				}
				else
				{
					NotifyOperator("Объект не обнаружен");
				}
				continue;
			}
			else if (response == "WHEEL_STUCK")
			{
				NotifyOperator("Колесо отвалилось");
				//TODO сделать какой то выход из всего цикла

			}
			else if (response == "OBSTACLE_DETECTED")
			{
				NotifyOperator("Обнаружено препятствие");
				EnableRGBDCamera();
				// Обработать данные с камеры
				ProcessCameraData();
				// Проверить наличие препятствия
				if (IsObstacleDetected())
				{
					AddObstacleToGlobalMap();
				}
				//TODO перейти к новому вычислению маршрута сканирования
				//сейчас пока просто выход из функции
				return;
			}
			else
			{
				NotifyOperator("Ошибка связи или неизвестный ответ Arduino");
				//TODO выход из цикла

			}
		}

		cout << "\n[PI] Маршрут " << route + 1
			<< " пройден\n";
		cout << "метка не найдена";
	}

	NotifyOperator("Все маршруты обработаны");

	cout << "\n========== ЗАВЕРШЕНИЕ RASPBERRY PI ==========\n";
}


//void RaspberryMain()
//{
//    cout << "[PI MAIN] Инициализация" << endl;
//
//    this_thread::sleep_for(chrono::seconds(2));
//
//    vector<vector<Point>> paths = ReadPaths();
//    cout << "[PI MAIN] Отправка команды Arduino" << endl;
//
//    raspberryOutgoing.send("START_SENSORS");
//
//    this_thread::sleep_for(chrono::seconds(3));
//
//    cout << "[PI MAIN] Следующая операция" << endl;
//
//    raspberryOutgoing.send("READ_BOARD");
//
//    this_thread::sleep_for(chrono::seconds(2));
//
//    cout << "[PI MAIN] Завершение работы" << endl;
//}

struct Point
{
	int x;
	int y;

	Point(int x, int y) {
		this->x = x;
		this->y = y;
	}

};

vector<vector<Point>> ReadPaths()
{
	int pathCount;

	cout << "Введите количество путей: ";
	cin >> pathCount;

	vector<vector<Point>> paths(pathCount);

	for (int i = 0; i < pathCount; i++)
	{
		int pointCount;

		cout << "\nПуть " << i + 1 << ":\n";

		cout << "Введите количество точек: ";
		cin >> pointCount;

		paths[i].resize(pointCount);

		cout << "Введите координаты точек:\n";

		for (int j = 0; j < pointCount; j++)
		{
			cin >> paths[i][j].x
				>> paths[i][j].y;
		}
	}

	return paths;
}

//отправка сообщений с ардуино
void ArduinoSender(Communication& channel)
{
	string message;

	while (arduinoOutgoing.receive(message))
	{
		channel.arduinoToRaspberry.send(message);

		cout << "[ARDUINO SEND] " << message << endl;
	}
}

//прием сообщений
void ArduinoReceiver(Communication& channel)
{
	string message;

	while (channel.raspberryToArduino.receive(message))
	{
		arduinoIncoming.send(message);
	}
}


//обработка сообщений приходящих на ардуино
void ArduinoProcessor()
{
	string message;

	while (arduinoIncoming.receive(message))
	{
		cout << "[ARDUINO PROCESS] Получено: "
			<< message << endl;

		// Здесь обработка команд Raspberry Pi
	}
}


//основная программа ардуинки
void ArduinoMain()
{
	//TODO основной цикл работы ардуины
	/*cout << "[ARDUINO MAIN] Запуск" << endl;

	this_thread::sleep_for(chrono::seconds(1));

	cout << "[ARDUINO MAIN] Опрос датчиков" << endl;

	arduinoOutgoing.send("SENSOR_DATA_1");

	this_thread::sleep_for(chrono::seconds(4));

	cout << "[ARDUINO MAIN] Обработка состояния" << endl;

	arduinoOutgoing.send("SENSOR_DATA_2");

	this_thread::sleep_for(chrono::seconds(2));

	cout << "[ARDUINO MAIN] Завершение" << endl;*/
}



int main()
{
	Communication channel;

	// Запускаем фоновые потоки Raspberry Pi
	thread piSender(RaspberrySender, ref(channel));
	thread piReceiver(RaspberryReceiver, ref(channel));
	thread piProcessor(RaspberryProcessor);

	// Запускаем фоновые потоки Arduino
	thread arduinoSender(ArduinoSender, ref(channel));
	thread arduinoReceiver(ArduinoReceiver, ref(channel));
	thread arduinoProcessor(ArduinoProcessor);

	// Основные потоки выполняются последовательно
	thread piMain(RaspberryMain);
	thread arduinoMain(ArduinoMain);

	// Ждём завершения основных программ
	piMain.join();
	arduinoMain.join();

	// Завершаем фоновые потоки
	raspberryOutgoing.close();
	arduinoOutgoing.close();

	channel.raspberryToArduino.close();
	channel.arduinoToRaspberry.close();

	// Дожидаемся завершения приёма
	piReceiver.join();
	arduinoReceiver.join();

	raspberryIncoming.close();
	arduinoIncoming.close();

	piSender.join();
	arduinoSender.join();

	piProcessor.join();
	arduinoProcessor.join();

	cout << "Система завершила работу" << endl;

	return 0;
}