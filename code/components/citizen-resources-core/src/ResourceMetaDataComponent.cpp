/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ResourceMetaDataComponent.h"

#include <Resource.h>
#include <VFSManager.h>

#include <ManifestVersion.h>

#include <regex>
#include <iomanip>
#include <sstream>
#include <stack>
#include <boost/algorithm/string.hpp>

std::string path_normalize(const std::string& pathRef);

namespace fx
{
ResourceMetaDataComponent::ResourceMetaDataComponent(Resource* resourceRef)
	: m_resource(resourceRef), m_metaDataLoader(nullptr)
{
}

boost::optional<std::string> ResourceMetaDataComponent::LoadMetaData(const std::string& resourcePath)
{
	assert(m_metaDataLoader.GetRef());

	m_metaDataEntries.clear();

	return m_metaDataLoader->LoadMetaData(this, resourcePath);
}

static constexpr const ManifestVersion g_manifestVersionOrder[] = {
	guid_t{ 0 },
#include <ManifestVersions.h>
};

static constexpr const std::string_view g_manifestVersionOrderV2[] = {
	"",
#include <ManifestVersionsV2.h>
};

template<typename TSearch, typename T, unsigned int N>
static size_t FindManifestVersionIndex(const T (&list)[N], const TSearch& guid)
{
	auto begin = list;
	auto end = list + std::size(list);
	auto found = std::find(begin, end, guid);

	if (found == end)
	{
		return -1;
	}

	return (found - list);
}

// TODO: clean up to be a templated func
std::optional<bool> ResourceMetaDataComponent::IsManifestVersionBetween(const guid_t& lowerBound, const guid_t& upperBound)
{
	auto entries = this->GetEntries("resource_manifest_version");

	guid_t manifestVersion = { 0 };

	// if there's a manifest version
	if (entries.begin() != entries.end())
	{
		// parse it
		manifestVersion = ParseGuid(entries.begin()->second);
	}

	// find the manifest version in the manifest version stack
	auto resourceVersion = FindManifestVersionIndex(g_manifestVersionOrder, manifestVersion);

	// if not found, return failure
	if (resourceVersion == -1)
	{
		return {};
	}

	// test lower/upper bound
	static const guid_t nullGuid = { 0 };
	bool matches = true;

	if (lowerBound != nullGuid)
	{
		auto lowerVersion = FindManifestVersionIndex(g_manifestVersionOrder, lowerBound);

		if (resourceVersion < lowerVersion)
		{
			matches = false;
		}
	}

	if (matches && upperBound != nullGuid)
	{
		auto upperVersion = FindManifestVersionIndex(g_manifestVersionOrder, upperBound);

		if (resourceVersion >= upperVersion)
		{
			matches = false;
		}
	}

	return matches;
}

std::optional<bool> ResourceMetaDataComponent::IsManifestVersionBetween(const std::string& lowerBound, const std::string& upperBound)
{
	auto entries = this->GetEntries("fx_version");

	std::string manifestVersion;

	// if there's a manifest version
	if (entries.begin() != entries.end())
	{
		// parse it
		manifestVersion = entries.begin()->second;
	}

	// find the manifest version in the manifest version stack
	auto resourceVersion = FindManifestVersionIndex(g_manifestVersionOrderV2, manifestVersion);

	// if not found, return failure
	if (resourceVersion == -1)
	{
		return {};
	}

	// test lower/upper bound
	static const std::string emptyString = "";
	bool matches = true;

	if (lowerBound != emptyString)
	{
		auto lowerVersion = FindManifestVersionIndex(g_manifestVersionOrderV2, lowerBound);

		if (resourceVersion < lowerVersion)
		{
			matches = false;
		}
	}

	if (matches && upperBound != emptyString)
	{
		auto upperVersion = FindManifestVersionIndex(g_manifestVersionOrderV2, upperBound);

		if (resourceVersion >= upperVersion)
		{
			matches = false;
		}
	}

	return matches;
}

static std::string getDirectory(const std::string& in)
{
	auto i = in.find_last_of('/');

	if (i != std::string::npos)
	{
		return in.substr(0, i);
	}
	else
	{
		return ".";
	}
}

struct Match
{
	Match(const fwRefContainer<vfs::Device>& device, const std::string& pattern)
	{
		auto slashPos = pattern.find_last_of('/');
		auto root = pattern.substr(0, slashPos) + "/";
		auto after = pattern.substr(slashPos + 1);

		this->findHandle = device->FindFirst(root, &findData);

		this->device = device;
		this->pattern = after;
		this->root = root;
		this->end = (this->findHandle == INVALID_DEVICE_HANDLE);

		auto patternCopy = after;

		boost::replace_all(patternCopy, "\\", "\\\\");
		boost::replace_all(patternCopy, "^", "\\^");
		boost::replace_all(patternCopy, ".", "\\.");
		boost::replace_all(patternCopy, "$", "\\$");
		boost::replace_all(patternCopy, "|", "\\|");
		boost::replace_all(patternCopy, "(", "\\(");
		boost::replace_all(patternCopy, ")", "\\)");
		boost::replace_all(patternCopy, "[", "\\[");
		boost::replace_all(patternCopy, "]", "\\]");
		boost::replace_all(patternCopy, "*", "\\*");
		boost::replace_all(patternCopy, "+", "\\+");
		boost::replace_all(patternCopy, "?", "\\?");
		boost::replace_all(patternCopy, "/", "\\/");
		boost::replace_all(patternCopy, "\\?", ".");
		boost::replace_all(patternCopy, "\\*", ".*");

		this->re = std::regex{ "^" + patternCopy + "$" };

		while (!Matches() && !end)
		{
			FindNext();
		}

		this->has = false;

		if (Matches())
		{
			this->has = true;
		}
	}

	const vfs::FindData& Get()
	{
		return findData;
	}

	bool Matches()
	{
		if (findData.name != "." && findData.name != "..")
		{
			return std::regex_match(findData.name, re);
		}

		return false;
	}

	void Next()
	{
		if (!end)
		{
			do
			{
				FindNext();
			} while (!Matches() && !end);

			has = !end && Matches();
		}
		else
		{
			has = false;
		}
	}

	operator bool()
	{
		return findHandle != INVALID_DEVICE_HANDLE && has;
	}

	~Match()
	{
		if (findHandle != INVALID_DEVICE_HANDLE)
		{
			device->FindClose(findHandle);
		}

		findHandle = INVALID_DEVICE_HANDLE;
	}

private:
	void FindNext()
	{
		end = !device->FindNext(findHandle, &findData);
	}

private:
	fwRefContainer<vfs::Device> device;
	std::string root;
	std::string pattern;
	vfs::Device::THandle findHandle;
	vfs::FindData findData;
	std::regex re;
	bool end;
	bool has;
};

struct Glob
{
	Glob(const fwRefContainer<vfs::Device>& device, const std::string& pattern)
	{
		this->device = device;

		// normalise slashes
		static const std::regex slashSplit{ "/+" };
		this->pattern = std::regex_replace(boost::trim_copy(pattern), slashSplit, "/");

		ParseNegate();

		// Bash 4.3 preserves the first two bytes that start with {}
		if (this->pattern.substr(0, 2) == "{}")
			this->pattern = "\\{\\}" + this->pattern.substr(2);
		globSet = BraceExpand(this->pattern);

		for (std::string& p : globSet)
		{
			std::vector<std::string> parts;
			boost::split(parts, p, boost::is_any_of("/"));
			globParts.push_back(parts);
		}

		for (std::vector<std::string>& globPart : globParts)
		{
			std::vector<ParseItem*> buffer;
			for (std::string& part : globPart)
				if (auto parsed = Parse(part, false))
					buffer.push_back(parsed->first);
				else
					goto failure;

			set.push_back(buffer);
			continue;
		failure:
			// something broke, ignore this glob
			continue;
		}
	}

	void ParseNegate()
	{
		auto it = pattern.begin();
		auto end = pattern.end();
		for (; it != end && *it == '!'; ++it)
			;
		negate = std::distance(pattern.begin(), it) & 1;
		pattern = std::string(it, end);
	}

	// https://www.gnu.org/software/bash/manual/html_node/Brace-Expansion.html
	std::vector<std::string> BraceExpand(const std::string& pattern)
	{
		auto begin = pattern.begin();
		auto it = pattern.begin();
		auto end = pattern.end();

		if (pattern[0] != '{')
		{
			for (; it != end; ++it)
			{
				switch (*it)
				{
					case ('$'):
						if (std::distance(it, end) <= 4)
							break;
						if (*(it + 1) == '{')
						{
							for (++it; it == end && *it != '}'; ++it)
								;
							if (it == end)
								break;
						}
						continue;
					case ('\\'):
						if (++it == end)
							break;
						continue;
					case ('{'):
					{
						std::string prefix(begin, it);
						std::string postfix(it, end);
						std::vector<std::string> e = BraceExpand(postfix);
						for (auto& s : e)
							s = prefix + s;
						return e;
					}
					default:
						continue;
				}

				break;
			}

			return { pattern };
		}

		static const std::regex numericSequence(R"(^\{(-?[[:d:]]+)\.\.(-?[[:d:]]+)(?:\.\.(-?[[:d:]]+))?\})");
		static const std::regex alphaSequence(R"(^\{([[:alpha:]])\.\.([[:alpha:]])(?:\.\.(-?[[:d:]]+))?\})");
		std::smatch seq;
		bool isNumeric = std::regex_search(pattern, seq, numericSequence);
		if (isNumeric || std::regex_search(pattern, seq, alphaSequence))
		{
			std::vector<std::string> suf = BraceExpand(seq.suffix());
			auto $1 = seq[1].str();
			auto $2 = seq[2].str();
			size_t pad = 0;
			int inc = seq[3].matched == false ? 1 : std::abs(std::stoi(seq[3].str())),
				start,
				end;

			if (isNumeric)
			{
				size_t $1_size;
				size_t $2_size;
				start = std::stoi($1, &$1_size);
				end = std::stoi($2, &$2_size);
				if ($1[$1.length() - $1_size] == '0' || $2[$2.length() - $2_size] == '0')
					pad = std::max($1_size, $2_size);
			}
			else
				start = $1[0], end = $2[0];
			if (start > end)
				std::swap(start, end);

			std::vector<std::string> ret;
			for (; start <= end; start += inc)
			{
				std::ostringstream o;
				if (isNumeric)
					o << std::setfill('0') << std::setw(pad) << start;
				else
					o << (char)start;
				for (auto& s : suf)
					ret.push_back(o.str() + s);
			}

			return ret;
		}

		int depth = 1;
		std::vector<std::string> set;
		auto member = pattern.begin();

		for (++member, ++it; it != end && depth; ++it)
		{
			switch (*it)
			{
				case ('\\'):
					if (++it == end)
						break;
					continue;
				case ('{'):
					depth++;
					continue;
				case ('}'):
					if (--depth == 0)
					{
						set.push_back(std::string(member, it));
						std::advance(member, std::distance(member, it) + 1);
					}
					continue;
				case (','):
					if (depth == 1)
					{
						set.push_back(std::string(member, it));
						std::advance(member, std::distance(member, it) + 1);
					}
					continue;
				default:
					continue;
			}

			break;
		}

		if (depth != 0)
			return BraceExpand("\\" + pattern);

		std::vector<std::string> nestedSet;
		for (auto& s : set)
			for (auto& e : BraceExpand(s))
				nestedSet.push_back(e);

		std::string post(it, end);
		if (set.size() == 1)
		{
			static const std::regex hasOptions(R"(,.*\})");
			if (std::regex_search(post, hasOptions))
				return BraceExpand(std::string(begin, it - 1) + "\\}" + post);
			for (auto& s : nestedSet)
				s = "{" + s + "}";
		}

		// add suffixes
		std::vector<std::string> result;
		for (auto& postfix : BraceExpand(post))
			for (auto& suffix : nestedSet)
				result.push_back(suffix + postfix);

		return result;
	}

	class ParseItem
	{
	protected:
		std::string m_source;

	public:
		std::string source() const
		{
			return m_source;
		};
		virtual bool match(const std::string& input) = 0;
	};

	class LiteralItem : public ParseItem
	{
	public:
		LiteralItem(const std::string& src)
		{
			m_source = src;
		};
		bool match(const std::string& input) override
		{
			return input == m_source;
		};
	};

	class MagicItem : public ParseItem
	{
	protected:
		std::regex regex;

	public:
		MagicItem(const std::string& src)
		{
			m_source = src;
			regex = std::regex{ "^" + src + "$" };
		};
		bool match(const std::string& input) override
		{
			return std::regex_match(input, regex);
		};
	};

	struct PatternListEntry
	{
		char type;
		size_t start;
		size_t reStart;
		size_t reEnd;
	};

	boost::optional<std::pair<Glob::ParseItem*, bool>> Parse(std::string& pattern, bool isSub)
	{
		if (pattern.empty() || pattern == "**")
			return std::pair<ParseItem*, bool>{ new LiteralItem{ pattern }, false };

		static const std::string qmark = "[^/]";
		static const std::string star = qmark + "*?";
		static const std::string reSpecials = "()[]{}?*+^$\\.&~# \t\n\r\v\f";

		std::string re;
		bool hasMagic = false, escaping = false, inClass = false;
		std::stack<PatternListEntry> patternListStack;
		std::vector<PatternListEntry> negativeLists;
		char stateChar = 0;

		size_t reClassStart = 0, classStart = 0;

		std::string patternStart = pattern[0] == '.' ? "" : R"((?!\.))";

		auto clearStateChar = [&]()
		{
			switch (stateChar)
			{
				case 0:
					return;
				case '*':
					re += star;
					hasMagic = true;
					break;
				case '?':
					re += qmark;
					hasMagic = true;
					break;
				default:
					re += "\\";
					re += stateChar;
					break;
			}

			stateChar = 0;
		};

		for (size_t i = 0; i < pattern.length(); i++)
		{
			char c = pattern[i];

			if (escaping)
			{
				if (c == '/')
					return boost::optional<std::pair<ParseItem*, bool>>{};
				if (reSpecials.find(c) != std::string::npos)
					re += "\\";
				re += c;
				escaping = false;
				continue;
			}

			switch (c)
			{
				case '/':
					return boost::optional<std::pair<ParseItem*, bool>>{};

				case '\\':
					clearStateChar();
					escaping = true;
					continue;

				case '?':
				case '*':
				case '+':
				case '@':
				case '!':
					if (inClass)
					{
						if (c == '!' && i == classStart + 1)
							c = '^';
						re += c;
						continue;
					}

					clearStateChar();
					stateChar = c;
					continue;

				case '(':
					if (inClass)
					{
						re += "(";
						continue;
					}

					if (!stateChar)
					{
						re += "\\(";
						continue;
					}

					patternListStack.push(PatternListEntry{ stateChar, i - 1, re.length() });
					re += stateChar == '!' ? "(?:(?!(?:" : "(?:";
					stateChar = 0;
					continue;

				case ')':
					if (inClass || patternListStack.empty())
					{
						re += "\\)";
						continue;
					}

					clearStateChar();
					hasMagic = true;
					auto pl = patternListStack.top();
					patternListStack.pop();

					re += ')';
					switch (pl.type)
					{
						case '!':
							re += ")[^/]*?)";
							pl.reEnd = re.length();
							negativeLists.push_back(pl);
							break;
						case '?':
						case '+':
						case '*':
							re += pl.type;
							break;
							// case '@':
							//   break;
					}

					continue;

				case '|':
					if (inClass || patternListStack.empty())
					{
						re += "\\|";
						continue;
					}

					clearStateChar();
					re += "|";
					continue;

				case '[':
					clearStateChar();

					if (inClass)
					{
						re += "\\[";
						continue;
					}

					inClass = true;
					classStart = i;
					reClassStart = re.length();
					re += c;
					continue;

				case ']':
				{
					if (i == classStart + 1 || !inClass)
					{
						re += "\\]";
						continue;
					}

					std::string cs = pattern.substr(classStart + 1, i - classStart - 1);
					try
					{
						auto escaped = boost::replace_all_copy(cs, "\\", "");
						auto s = std::regex_replace(escaped, std::regex{ "[\\[\\]]" }, "\\$&");
						std::regex reClass{ "[" + s + "]" };
					}
					catch (const std::regex_error&)
					{
						std::pair<Glob::ParseItem*, bool> sp = *Parse(cs, true);
						re = re.substr(0, reClassStart) + "\\[" + (*sp.first).source() + "\\]";
						hasMagic = hasMagic || sp.second;
						inClass = false;
						continue;
					}

					hasMagic = true;
					inClass = false;
					re += c;
					continue;
				}

				default:
					clearStateChar();

					if (reSpecials.find(c) != std::string::npos && !(c == '^' && inClass))
						re += "\\";

					re += c;
					break;
			}
		}

		if (inClass)
		{
			std::string cs = pattern.substr(classStart + 1);
			std::pair<Glob::ParseItem*, bool> sp = *Parse(cs, true);
			re = re.substr(0, reClassStart) + "\\[" + (*sp.first).source();
			hasMagic = hasMagic || sp.second;
		}

		for (size_t p = patternListStack.size(); p-- > 0;)
		{
			auto pl = patternListStack.top();
			patternListStack.pop();

			std::string tail;
			std::string tailStr = re.substr(pl.reStart + 3);
			ptrdiff_t lastMatch = 0;
			auto lastMatchEnd = tailStr.cbegin();
			static const std::regex escapeCheck(R"(((?:\\{2}){0,64})(\\?)\|)");
			std::sregex_iterator begin(tailStr.cbegin(), tailStr.cend(), escapeCheck),
			end;

			for (; begin != end; ++begin)
			{
				const std::smatch& match = *begin;
				auto pos = match.position();
				auto start = lastMatchEnd;
				std::advance(start, pos - lastMatch);
				tail.append(lastMatchEnd, start);
				auto $1 = match[1].str();
				auto $2 = match[2].str();

				if ($2.empty())
					$2 = "\\";
				tail.append($1 + $1 + $2 + "|");

				auto length = match.length();
				lastMatch = pos + length;
				lastMatchEnd = start;
				std::advance(lastMatchEnd, length);
			}

			tail.append(lastMatchEnd, tailStr.cend());

			std::string t = pl.type == '*'   ? star
							: pl.type == '?' ? qmark
											 : "\\" + std::string(1, pl.type);

			hasMagic = true;
			re = re.substr(0, pl.reStart) + t + "\\(" + tail;
		}

		clearStateChar();
		if (escaping)
			re += "\\\\";

		bool addPatternStart = false;
		switch (re[0])
		{
			case '[':
			case '.':
			case '(':
				addPatternStart = true;
				break;
		}

		for (size_t n = negativeLists.size(); n-- > 0;)
		{
			auto nl = negativeLists[n];

			auto nlBefore = re.substr(0, nl.reStart);
			auto nlFirst = re.substr(nl.reStart, nl.reEnd - nl.reStart - 8);
			auto nlAfter = re.substr(nl.reEnd);
			auto nlLast = re.substr(nl.reEnd - 8, 8) + nlAfter;

			auto openParensBefore = std::count(nlBefore.begin(), nlBefore.end(), '(');
			while (openParensBefore--)
				nlAfter = std::regex_replace(nlAfter, std::regex("\\)[+*?]?"), "");

			std::string dollar = nlAfter.empty() && !isSub ? "$" : "";
			re = nlBefore + nlFirst + nlAfter + dollar + nlLast;
		}

		if (!re.empty() && hasMagic)
			re = "(?=.)" + re;

		if (addPatternStart)
			re = patternStart + re;

		if (isSub)
			return std::pair<ParseItem*, bool>{ new LiteralItem{ re }, hasMagic };

		if (!hasMagic)
		{
			static const std::regex globUnescape(R"(\\(.))");
			return std::pair<ParseItem*, bool>{ new LiteralItem{ std::regex_replace(re, globUnescape, "$1") }, false };
		}

		return std::pair<ParseItem*, bool>{ new MagicItem{ re }, false };
	}

	std::set<std::string> Matches(const std::string& root, std::vector<std::string> file = {})
	{
		vfs::FindData find;
		auto handle = device->FindFirst(root, &find);
		bool end = handle == INVALID_DEVICE_HANDLE;

		std::set<std::string> results;

		for (; !end; end = !device->FindNext(handle, &find))
		{
			bool isdir = find.attributes & FILE_ATTRIBUTE_DIRECTORY;
			auto full = root + "/" + find.name;

			file.push_back(find.name);

			for (auto& pattern : set)
			{
				if (MatchOne(file, pattern, isdir) != negate)
				{
					if (isdir)
						for (auto& result : Matches(full, file))
							results.insert(std::move(result));
					else
						results.insert(full);
					break;
				}
			}

			file.pop_back();
		};

		device->FindClose(handle);

		return results;
	}

	bool MatchOne(const std::vector<std::string>& file, const std::vector<ParseItem*>& pattern, bool partial)
	{
		size_t fi = 0, pi = 0;
		auto fl = file.size();
		auto pl = pattern.size();

		for (; fi < fl && pi < pl; fi++, pi++)
		{
			auto part = file[fi];
			auto item = pattern[pi];

			if (item->source() == "**")
			{
				size_t fr = fi, pr = pi + 1;

				if (pr == pl)
				{
					for (; fi < fl; fi++)
						if (file[fi][0] == '.')
							return false;

					return true;
				}

				while (fr < fl)
				{
					std::vector<std::string> frSlice(file.begin() + fr, file.end());
					std::vector<ParseItem*> prSlice(pattern.begin() + pr, pattern.end());
					if (MatchOne(frSlice, prSlice, partial))
						return true;
					if (file[fr][0] == '.')
						break;
					fr++;
				}

				if (partial && fr == file.size())
					return true;

				return false;
			}

			if (!item->match(part))
				return false;
		}

		if (fi == file.size() && pi == pattern.size())
			return true;
		else if (fi == file.size())
			return partial;
		else if (pi == pattern.size())
			return fi == file.size() - 1 && file[fi] == "";

		throw std::runtime_error("Glob matching passthrough.");
	}

	bool IsGlob()
	{
		if (set.size() != 1 || globParts.size() != 1 || set[0].size() != globParts[0].size())
			return true;
		for (auto& item : set[0])
			if (dynamic_cast<MagicItem*>(item))
				return true;
		return false;
	}

private:
	bool negate = false;
	std::vector<std::string> globSet;
	std::vector<std::vector<std::string>> globParts;
	std::string pattern;
	std::vector<std::vector<ParseItem*>> set;
	fwRefContainer<vfs::Device> device;
};

static std::vector<std::string> MatchFiles(const fwRefContainer<vfs::Device>& device, const std::string& pattern)
{
	auto patternNorm = path_normalize(pattern);

	auto starPos = patternNorm.find('*');
	auto before = getDirectory((starPos != std::string::npos) ? patternNorm.substr(0, starPos) : patternNorm);
	auto slashPos = (starPos != std::string::npos) ? patternNorm.find('/', starPos) : std::string::npos;
	auto after = (slashPos != std::string::npos) ? patternNorm.substr(slashPos + 1) : "";

	bool recurse = (starPos != std::string::npos && patternNorm.substr(starPos + 1, 1) == "*" && (starPos == 0 || patternNorm.substr(starPos - 1, 1) == "/"));

	std::set<std::string> results;

	if (recurse)
	{
		// 1 is correct behavior, 2 is legacy behavior we have to retain(...)
		for (auto submaskOff : { 1, 2 })
		{
			auto submask = patternNorm.substr(0, starPos) + patternNorm.substr(starPos + submaskOff);

			auto rawResults = MatchFiles(device, submask);

			for (auto& result : rawResults)
			{
				results.insert(std::move(result));
			}
		}

		auto findPattern = patternNorm.substr(0, starPos + 1);

		for (Match match{ device, findPattern }; match; match.Next())
		{
			if (match.Get().attributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				auto matchPath = before + "/" + match.Get().name + "/" + patternNorm.substr(starPos);
				auto resultsSecond = MatchFiles(device, matchPath);

				for (auto& result : resultsSecond)
				{
					results.insert(std::move(result));
				}
			}
		}
	}
	else
	{
		auto findPattern = patternNorm.substr(0, slashPos != std::string::npos ? slashPos - 1 : std::string::npos);

		for (Match match{ device, findPattern }; match; match.Next())
		{
			bool isfile = !(match.Get().attributes & FILE_ATTRIBUTE_DIRECTORY);
			bool hasSlashPos = slashPos != std::string::npos;

			if (!(hasSlashPos && isfile))
			{
				auto matchPath = before + "/" + match.Get().name;

				if (!after.empty())
				{
					auto resultsSecond = MatchFiles(device, matchPath + "/" + after);

					for (auto& result : resultsSecond)
					{
						results.insert(std::move(result));
					}
				}
				else if (isfile)
				{
					results.insert(matchPath);
				}
			}
		}
	}

	std::vector<std::string> resultsVec{ results.begin(), results.end() };
	return resultsVec;
}

static std::vector<std::string> MatchFilesV2(const fwRefContainer<vfs::Device>& device, const std::string& root, const std::string& pattern)
{
	Glob glob(device, pattern);

	if (!glob.IsGlob())
	{
		std::vector<std::string> results;
		if (!(device->GetAttributes(root + "/" + pattern) & FILE_ATTRIBUTE_DIRECTORY))
			results.push_back(root + "/" + pattern);
		return results;
	}
	else
	{
		auto matches = glob.Matches(root);
		return { matches.begin(), matches.end() };
	}
}

void ResourceMetaDataComponent::GlobEntries(const std::string& key, const std::function<void(const std::string&)>& entryCallback)
{
	for (auto& entry : GetEntries(key))
	{
		GlobValue(entry.second, entryCallback);
	}
}

void ResourceMetaDataComponent::GlobValue(const std::string& value, const std::function<void(const std::string&)>& entryCallback)
{
	// why... would anyone pass an empty value?!
	// this makes the VFS all odd so let's ignore it
	if (value.empty())
	{
		return;
	}

	const auto& rootPath = m_resource->GetPath() + "/";
	fwRefContainer<vfs::Device> device = vfs::GetDevice(rootPath);

	if (!device.GetRef())
	{
		return;
	}

	auto relRoot = path_normalize(rootPath);

	std::string pattern = value;

	// @ prefixes for files are special and handled later on
	if (pattern.length() >= 1 && pattern[0] == '@')
	{
		entryCallback(pattern);
		return;
	}

	auto rmvRes = this->IsManifestVersionBetween("diabolical", "");
	auto mf = (!rmvRes || !*rmvRes) ? MatchFiles(device, rootPath + pattern) : MatchFilesV2(device, relRoot, pattern);

	for (auto& file : mf)
	{
		if (file.length() < (relRoot.length() + 1))
		{
			continue;
		}

		entryCallback(file.substr(relRoot.length() + 1));
	}
}
}
