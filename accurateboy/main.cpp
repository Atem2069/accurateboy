#include<iostream>
#include"Logger.h"
#include"Gameboy.h"
#include"Testing/gbclient_testing.h"

int main(int argc, char**argv)
{
	if (argc)
	{
		Logger::getInstance()->msg(LoggerSeverity::Info, "Starting in debug mode! Ensure mooneye test-suite is located within current directory");
		std::cout << argv[1] << '\n';
		if (argv[1]==std::string("-test"))
		{
			GBTestClient m_gameBoy;
			m_gameBoy.run();
			return 0;
		}
		else
			Logger::getInstance()->msg(LoggerSeverity::Warn, "Unknown parameter passed..");
	}
	Logger::getInstance()->msg(LoggerSeverity::Info, "Hello world!");
	GameBoy m_gameBoy;
	m_gameBoy.run();
	return 0;
}