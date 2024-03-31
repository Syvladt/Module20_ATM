#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>

// Процедура записи состояния банкомата
bool writeState(int init[])
{
	bool result = false;
	std::ofstream file("State.bin", std::ios::binary);
	if (file.is_open())
	{
		for (int i = 0; i < 6; ++i)
			file.write((char*)&init[i], sizeof(init[i]));
		result = true;
	}
	file.close();
	return result;
}

// Процедура выводит содержимое банкомата и сумму всех денег
void showState(int state[][2])
{
	int summ = 0;
	std::cout << "\n\nCurrent status ATM:\n";
	for (int i = 0; i < 6; ++i)
	{
		std::cout << state[i][0] << '\t' << state[i][1] << '\n';
		summ += state[i][0] * state[i][1];
	}
	std::cout << "\nTotal: " << summ << " rubles.\n";
}


int main()
{
	/* Изначально банкомат считаем полным, т.е. 1000 купюр.
	   Мелкие купюры используются чаще поэтому 
	   100  - 250 штук,
	   200  - 250 штук,
	   500  - 150 штук,
	   1000 - 150 штук,
	   2000 - 100 штук,
	   5000 - 100 штук. 
	   Именно эти данные и запишем в файл. */
	int bank[6][2] = { {100, 0}, {200, 0}, {500, 0}, {1000, 0}, {2000, 0}, {5000, 0} };
	int init[] = { 250, 250, 150, 150, 100, 100 }, summ = 0, inSumm, temp = -1;
	int delta[] = {0, 0, 0, 0, 0, 0}, multiple = -1;
	
	// Запись состояния банкомата в файл
	if (writeState(init))
		std::cout << "Writing to the file is successful.\n";
	else
		std::cout << "Writing to the file is not successful.\n";

	std::ifstream state("State.bin", std::ios::binary);
	if (state.is_open())
	{
		// Чтение наполненности банкомата из файла и подсчёт суммы в рублях.
		for (int i = 0; i < 6; ++i)
		{
			state.read((char*)&bank[i][1], sizeof(bank[i][1]));
			std::cout << bank[i][0] << '\t' << bank[i][1] << '\n';
			summ += bank[i][0] * bank[i][1];
		}
		state.close();
		// Если денег нет, то выход из программы.
		if (summ == 0)
			std::cout << "The ATM is empty.\n";
		// В банкомате - таки есть деньги
		else
		{
			while(temp != 0)
			{
				std::cout << "Enter a amount (0 - Shutdown ATM): ";
				std::cin >> temp;
				inSumm = temp;
				// Если введённая сумма отрицательная, то выдаём клиенту
				if (inSumm < 0)
				{
					// Если введённая сумма больше, чем денег в банкомате - выход из программы
					if (summ >= abs(inSumm))
					{
						// Пытаемся выдать запрошенную сумму
						for (int i = 5; i >= 0; --i)
						{
							if (bank[i][1] != 0 && inSumm / bank[i][0] != 0) // Если купюры i-го номинала есть и оставшаяся сумма к выдаче не 0
							{
								multiple = i;                            // Запоминаем индекс купюры которая является кратной
								delta[i] = abs(inSumm / bank[i][0]);     // Рассчитываем количество купюр i-го номинала
								if (delta[i] > bank[i][1])               // Если требуемое количество купюр в банкомате меньше
									delta[i] = bank[i][1];               // То забираем все купюры
								inSumm += delta[i] * bank[i][0];         // Вычитаем выданные деньги
							}
							else
							{
								// Если выбрали на выдачу купюры, но не получучается выдать всю сумму, но деньги в банкомате есть.
								if (i == 0 && abs(inSumm) > bank[0][0] || i == 3 && (abs(inSumm) / 1000) > bank[0][0])
								{
									inSumm -= bank[multiple][0] * delta[multiple];	// Добавляем последние выданные купюры
									delta[multiple] = 0;                            // Обнуляем массив выданных купюр
									i = multiple;
								}
							}
						}
						if (inSumm != 0 && multiple >= 0)  // Не получилось выдать сумму, т.к. в банкомате не оказалось некоторых купюр
						{
							std::cout << "Enter a multiple of the amount " << bank[multiple][0] << ".\n";
							for (int i = 0; i < 6; ++i)
								delta[i] = 0;
						}
						else
							if (inSumm != 0)  // Выдать деньги не получилось. Сумма не кратна минимальной купюре
							{
								std::cout << "The amount is not a multiple of the available bills. Repeat the input.\n";
								for (int i = 0; i < 6; ++i)
									delta[i] = 0;
							}
					}
					else
						std::cout << "There is not enough money.\n";
					// Вычитаем выбранные купюры из кассет, рассчитываем оставшуюся сумму и выводим состояние банкомата на экран
					summ = 0;
					for (int i = 0; i < 6; ++i)
					{
						bank[i][1] -= delta[i];
						summ += bank[i][0] * bank[i][1];
					}
					showState(bank);	// Выводим состояние банкомата
				}
				else
				{
					// Добавление купюр
					if (inSumm > 0)
					{
						// Инициализация массива delta
						for (int i = 0; i < 6; ++i)
							delta[i] = 0;
						// Введённая сумма должна быть кратна минимальной купюре
						if (inSumm % bank[0][0] == 0)
						{
							while (inSumm > 0)
							{
								// Так как купюры могут быть разные, а емкость банкомата ограничена, т.е. не все купюры могут быть приняты.
								summ = 0; // Возвращаемая сумма непринятых купюр
								for(int i = 5; i >= 0; --i)
								{
									if((inSumm / bank[i][0]) > 0)
									{
										// Рандомно определяем количество купюр i-го номинала.
										delta[i] = std::rand() % (inSumm / bank[i][0] + 1);
										if (i == 0)
											delta[i] = inSumm / bank[i][0];
										inSumm -= bank[i][0] * delta[i];
										
										if (bank[i][1] < init[i])
										{
											bank[i][1] += delta[i];
											delta[i] = 0;
										}
										summ += bank[i][0] * delta[i];
									}
								}
							}
							if (summ > 0)
							// Сумма возвращаемыч денег не 0
							{
								std::cout << "We are returning the money " << summ << " rubles.\n";
								for(int i = 0; i < 6; ++i)
									std::cout << bank[i][0] << '\t' << delta[i] << '\n';
							}
						}
						else
							// Введенная сумма не кратна минимальной купюре
							std::cout << "The entered amount is not a multiple of the minimum bill.\n";
						showState(bank);
					}
					else
					{
						std::cout << "\n\nBye.\n\n";
					}
				}
			}
		}
	}
	showState(bank);
}