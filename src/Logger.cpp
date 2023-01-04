#include "Logger.h"

#include <chrono>

const std::unordered_map<DIP::Logger::Level, const wchar_t *> DIP::Logger::s_level_prefixes = {
	{DIP::Logger::LEVEL_NONE, L""},
	{DIP::Logger::LEVEL_DEBUG, L"DEBUG"},
	{DIP::Logger::LEVEL_INFO, L"INFO"},
	{DIP::Logger::LEVEL_ERROR, L"ERROR"}
};

void DIP::Logger::message(const wchar_t *prefix, const wchar_t *message, Level level)
{
	if (m_output_to_file && m_output_filename.empty() == false) {
		FILE *file;
		// open file in append mode with UTF-16 BOM header
		if (_wfopen_s(&file, m_output_filename.data(), L"a, ccs=UTF-16LE") != 0) {
			return;
		}

		{
			std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			struct tm time;
			localtime_s(&time, &now);

			wchar_t buffer[100];
			std::wcsftime(buffer, sizeof(buffer), L"[%Y-%m-%d %H:%M:%S] ", &time);
			fwrite(buffer, sizeof(wchar_t), wcslen(buffer), file);
		}

		fwrite(prefix, sizeof(wchar_t), wcslen(prefix), file);
		fwrite(L": ", sizeof(wchar_t), 2, file);
		fwrite(message, sizeof(wchar_t), wcslen(message), file);
		fwrite(L"\n", sizeof(wchar_t), 1, file);
		fclose(file);
	}
	if (m_output_to_stream) {
		std::wostream &stream = level == LEVEL_ERROR ? std::wcerr : std::wcout;
		stream << "[DirImage] " << prefix << ": " << message << "\n";
	}
}

DIP::Logger::Level DIP::Logger::level() const
{
	return m_level;
}

void DIP::Logger::setLevel(Level level)
{
	m_level = level;
}

bool DIP::Logger::isOutputToFile() const
{
	return m_output_to_file;
}

void DIP::Logger::setOutputToFile(bool output_to_file)
{
	m_output_to_file = output_to_file;
}

void DIP::Logger::setOutputToFile(const wchar_t *output_filename)
{
	m_output_to_file = true;
	m_output_filename = output_filename;
}

void DIP::Logger::setOutputToFile(const std::wstring &output_filename)
{
	m_output_to_file = true;
	m_output_filename = output_filename;
}

bool DIP::Logger::isOutputToStream() const
{
	return m_output_to_stream;
}

void DIP::Logger::setOutputToStream(bool output_to_stream)
{
	m_output_to_stream = output_to_stream;
}

const std::wstring &DIP::Logger::outputFilename() const
{
	return m_output_filename;
}

void DIP::Logger::setOutputFilename(const std::wstring &output_filename)
{
	m_output_filename = output_filename;
}