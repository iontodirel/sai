// sai - General purpose self-contained C++ libraries.
//
// version.h
// Stores version information.
// 
// MIT License
// 
// Copyright (c) 2017 Ion Todirel
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef SAI_CORE_VERSION_H
#define SAI_CORE_VERSION_H

#include <string>
#include <stdexcept>
#include <sstream>

class version_parse_exception : public std::exception
{
public:
	version_parse_exception() : std::exception() {}
	version_parse_exception(const char* message) : std::exception(message) { message_ = message; }
	version_parse_exception(const std::string& message) : std::exception(message.c_str()) { message_ = message; }

	std::string message() { return message_; }

private:
	std::string message_;
};

class version
{
public:
	static version parse(const std::string& version_string)
	{
		version v;
		if (!try_parse(version_string, v))
		{
			throw version_parse_exception("Could not parse version.");
		}
		return v;
	}

	static bool try_parse(const std::string& version_string, version& result) noexcept
	{
		version v;

		if (version_string.length() == 0)
		{
			return false;
		}

		int i = 0;
		std::stringstream ss(version_string);
		std::string s;
		while (std::getline(ss, s, '.'))
		{
			size_t release_index = s.find('-');
			if (release_index != s.npos)
			{
				v.release_ = s.substr(release_index + 1);
				v.release_set_ = true;
				if (v.seq_field_count_ == 4)
				{
					v.seq_field_count_++;
				}
				s = s.substr(0, release_index);
			}

			int n = 0;
			try
			{
				n = std::stoi(s);
			}
			catch (std::invalid_argument&)
			{
				return false;
			}
			catch (std::out_of_range&)
			{
				return false;
			}

			if (i == 0)
				v.major_ = n;
			else if (i == 1)
				v.minor_ = n;
			else if (i == 2)
			{
				v.revision_ = n;
				v.seq_field_count_++;
			}
			else if (i == 3)
			{
				v.build_ = n;
				v.seq_field_count_++;
			}

			i++;
		}

		// Only assign if successful
		result = v;

		return true;
	}

	version() {}
	version(const char* version_string) : version(std::string(version_string)) {}
	version(std::string version_string)
	{
		*this = parse(version_string);
	}
	version(int major, int minor) : major_(major), minor_(minor) {}
	version(int major, int minor, std::string release) : major_(major), minor_(minor), release_(release), seq_field_count_(2), release_set_(true) {}
	version(int major, int minor, int revision) : major_(major), minor_(minor), revision_(revision), seq_field_count_(3) {}
	version(int major, int minor, int revision, std::string release) : major_(major), minor_(minor), revision_(revision), seq_field_count_(3), release_(release), release_set_(true) {}
	version(int major, int minor, int revision, int build) : major_(major), minor_(minor), revision_(revision), build_(build), seq_field_count_(4) {}
	version(int major, int minor, int revision, int build, std::string release) : major_(major), minor_(minor), revision_(revision), build_(build), release_(release), seq_field_count_(5), release_set_(true) {}

	version(const version& other)
	{
		major_ = other.major_;
		minor_ = other.minor_;
		revision_ = other.revision_;
		build_ = other.build_;
		release_ = other.release_;
		seq_field_count_ = other.seq_field_count_;
		release_set_ = other.release_set_;
	}

	version& operator=(const version& other)
	{
		major_ = other.major_;
		minor_ = other.minor_;
		revision_ = other.revision_;
		build_ = other.build_;
		release_ = other.release_;
		seq_field_count_ = other.seq_field_count_;
		release_set_ = other.release_set_;
		return *this;
	}

	version(version&& other)
	{
		major_ = other.major_;
		minor_ = other.minor_;
		revision_ = other.revision_;
		build_ = other.build_;
		release_ = std::move(other.release_);
		seq_field_count_ = other.seq_field_count_;
		release_set_ = other.release_set_;
	}

	version& operator=(version&& other)
	{
		major_ = other.major_;
		minor_ = other.minor_;
		revision_ = other.revision_;
		build_ = other.build_;
		release_ = std::move(other.release_);
		seq_field_count_ = other.seq_field_count_;
		release_set_ = other.release_set_;
		return *this;
	}

	~version() {}

	int major() const { return major_; }
	int minor() const { return minor_; }
	int revision() const { return revision_; }
	int build() const { return build_; }
	std::string release() const { return release_; }

	int fields() const
	{
		return (seq_field_count_ < 5 && release_set_) ? seq_field_count_ + 1 : seq_field_count_;
	}

	bool empty() const
	{
		return major_ == 0 &&
			minor_ == 0 &&
			revision_ == 0 &&
			build_ == 0 &&
			release_.length() == 0 &&
			seq_field_count_ == 2 &&
			release_set_ == false;
	}

	void clear()
	{
		major_ = {};
		minor_ = {};
		revision_ = {};
		build_ = {};
		release_ = {};
		seq_field_count_ = 2;
		release_set_ = false;
	}

	std::string to_string() const
	{
		return to_string(seq_field_count_);
	}

	std::string to_string(int field_count) const
	{
		return to_string(seq_field_count_, release_set_);
	}

	std::string to_string(int field_count, bool include_release) const
	{
		return to_string(field_count, include_release, "_");
	}

	std::string to_string(int field_count, bool include_release, const std::string& release_separator) const
	{
		return to_string(field_count, include_release, release_separator.c_str());
	}

	std::string to_string(int field_count, bool include_release, const char* release_separator) const
	{
		std::string str = std::to_string(major_) + "." + std::to_string(minor_);
		if (field_count >= 3)
			str += "." + std::to_string(revision_);
		if (field_count >= 4)
			str += "." + std::to_string(build_);
		if ((field_count >= 5 && release_.length() > 0) || include_release)
			str += release_separator + release_;
		return str;
	}

	bool operator==(const version& other) const { return compare(other) == 0; }
	bool operator!=(const version& other) const { return compare(other) == -1; }
	bool operator<(const version& other) const { return compare(other) < 0; }
	bool operator>(const version& other) const { return compare(other) > 0; }
	bool operator<=(const version& other) const { return compare(other) <= 0; }
	bool operator>=(const version& other) const { return compare(other) >= 0; }

	int compare(const version& other) const
	{
		if (major_ != other.major_)
		{
			if (major_ < other.major_)
				return -1;
			return 1;
		}

		if (minor_ != other.minor_)
		{
			if (minor_ < other.minor_)
				return -1;
			return 1;
		}

		if (revision_ != other.revision_)
		{
			if (revision_ < other.revision_)
				return -1;
			return 1;
		}

		if (build_ != other.build_)
		{
			if (build_ < other.build_)
				return -1;
			return 1;
		}

		if (release_set_ != other.release_set_ && release_ != other.release_)
		{
			return -1;
		}

		return 0;
	}

private:
	int major_ = 0;
	int minor_ = 0;
	int revision_ = 0;
	int build_ = 0;
	int seq_field_count_ = 2;
	std::string release_;
	bool release_set_ = false;
};

#endif