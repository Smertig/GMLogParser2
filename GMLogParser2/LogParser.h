#pragma once

#include <cstdint>
#include <string>
#include <fstream>
#include <vector>

namespace LogParser {
	template <size_t N>
	struct skip_t {};

	struct bstring : public std::wstring {
		using std::wstring::wstring;
	};

	template <class T, class = std::enable_if_t<std::is_fundamental_v<T>>>
	static void Read(std::istream& is, T& out) {
		is.read(reinterpret_cast<char*>(&out), sizeof(T));
	}

	static void Read(std::istream& is, bstring& out) {
		uint32_t len = 0;
		Read(is, len);
		if (len > 0x1000) {
			throw std::runtime_error("corrupted log: too big string (" + std::to_string(len) + ")");
		}
		out.resize(len);
		is.read((char*)&out[0], len * 2);
	}

	template <size_t N>
	static void Read(std::istream& is, skip_t<N>&) {
		is.seekg(N, std::ios::cur);
	}

	class Record
	{
		skip_t<1> _1;
		bstring desc; skip_t<1> _2;
		bstring info; skip_t<1> _3;
		uint32_t gm_id; skip_t<1> _4;
		uint16_t year, month, day; skip_t<2> _5;
		uint16_t hour, min, sec; skip_t<2> _6;

		std::ifstream& ParseBinary(std::ifstream& is) {
			auto read = [&](auto& value) { Read(is, value); };
			read(_1); read(desc);
			read(_2); read(info);
			read(_3); read(gm_id);
			read(_4); read(year); read(month); read(day);
			read(_5); read(hour); read(min); read(sec);
			read(_6);
			return is;
		}

		std::wofstream& WriteAsText(std::wofstream& os) const {
			wchar_t buf[0x400];
			swprintf_s(buf, L"[%04d.%02d.%02d %02d:%02d:%02d] %35s | %80s | %6d ",
				year, month, day, hour, min, sec, desc.data(), info.empty() ? L"[empty]" : info.data(), gm_id);
			os << buf;
			return os;
		}
	public:
		Record() = default;

		friend std::ifstream& operator>>(std::ifstream& is, Record& rhs) {
			return rhs.ParseBinary(is);
		}

		friend std::wofstream& operator<<(std::wofstream& os, const Record& rhs) {
			return rhs.WriteAsText(os);
		}
	};

	class Parser {
		std::vector<Record> _records;
	public:
		Parser() = default;

		void Parse(const std::string& path) {
			std::ifstream is(path, std::ios::binary);
			if (!is) {
				throw std::invalid_argument("unable to open file: " + path);
			}
			is.exceptions(is.exceptions() | std::ios::failbit);

			uint32_t size = 0;
			try {
				// read size
				Read(is, size);

				// read records
				Record rec;
				for (size_t i = 0; i < size; i++) {
					is >> rec;
					_records.push_back(std::move(rec));
				}
			}
			catch (std::exception& e) {
				throw std::runtime_error(std::string("corrupted log file: ") + e.what());
			}

			if (_records.size() != size) {
				throw std::runtime_error("expected size " + std::to_string(size) + ", got " + std::to_string(_records.size()));
			}
		}

		void Convert(const std::string& out) {
			std::wofstream os(out);
			wchar_t buf[0x400];
			swprintf_s(buf, L"[%04s.%02s.%02s %02s:%02s:%02s] %35s | %80s | %6s ", L"YYYY", L"MM", L"DD", L"HH", L"MM", L"SS", L"Description", L"Additional info", L"GM id");
			os << buf << std::endl;
			os << L"-----------------------------------------------------------------------------------------------------------------------------------------------------" << std::endl;
			for (const auto& rec : _records) {
				os << rec << std::endl;
			}
		}
	};

}