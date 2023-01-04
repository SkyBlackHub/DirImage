#ifndef LOGGER_H
#define LOGGER_H

#include "Singleton.h"

#include <iostream>
#include <stdarg.h>
#include <unordered_map>

namespace DIP {

	class Logger : public SingletonDefault<Logger>
	{
	public:
		enum Level : unsigned int {
			LEVEL_NONE = 0,
			LEVEL_ERROR,
			LEVEL_INFO,
			LEVEL_DEBUG
		};

		void message(const wchar_t *prefix, const wchar_t *message, Level level = LEVEL_NONE);

		template <typename... Args>
		void debug(const wchar_t *message, Args... arguments)
		{
			this->levelMessage(LEVEL_DEBUG, message, arguments...);
		}

		template <typename... Args>
		void info(const wchar_t *message, Args... arguments)
		{
			this->levelMessage(LEVEL_INFO, message, arguments...);
		}

		template <typename... Args>
		void error(const wchar_t *message, Args... arguments)
		{
			this->levelMessage(LEVEL_ERROR, message, arguments...);
		}

		Level level() const;
		void setLevel(Level level);

		bool isOutputToFile() const;
		void setOutputToFile(bool output_to_file);
		void setOutputToFile(const wchar_t *output_filename);
		void setOutputToFile(const std::wstring &output_filename);

		bool isOutputToStream() const;
		void setOutputToStream(bool output_to_stream);

		const std::wstring &outputFilename() const;
		void setOutputFilename(const std::wstring &output_filename);

	private:
		template <typename... Args>
		void levelMessage(Level level, const wchar_t *message, Args... arguments)
		{
			if (m_level >= level) {
				swprintf(m_buffer, 512, message, arguments...);
				this->message(s_level_prefixes.at(level), m_buffer, level);
			}
		}

		static const std::unordered_map<Level, const wchar_t *> s_level_prefixes;

		Level m_level = Logger::LEVEL_NONE;

		bool m_output_to_file = false;
		bool m_output_to_stream = true;

		std::wstring m_output_filename;

		wchar_t m_buffer[512];
	};

}

#define Log DIP::Logger::instance()

#endif // LOGGER_H