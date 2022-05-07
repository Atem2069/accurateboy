#include<iostream>
#include"Logger.h"
#include"Gameboy.h"
#include"Testing/gbclient_testing.h"

int main(int argc, char**argv)
{
	if (argc)
	{
		Logger::getInstance()->msg(LoggerSeverity::Info, "Starting in debug mode! Ensure mooneye test-suite is located within current directory");
		if (argv[0] == "-test")
		{
			GBTestClient m_gameBoy;
			m_gameBoy.run();
		}
	}
	Logger::getInstance()->msg(LoggerSeverity::Info, "Hello world!");
	GameBoy m_gameBoy;
	m_gameBoy.run();
	return 0;
}