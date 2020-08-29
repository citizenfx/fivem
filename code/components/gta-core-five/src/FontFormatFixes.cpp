#include "StdInc.h"

#include <LaunchMode.h>
#include <Hooking.h>

#include <regex>

#include <sstream>

#include <nutsnbolts.h>

#include <MinHook.h>

static void(*g_parseHtml)(void* styledText, const wchar_t* str, int64_t length, void* pImgInfoArr, bool multiline, bool condenseWhite, void* styleMgr, void* txtFmt, void* paraFmt);

static std::wregex emojiRegEx{ L"(?:\xd83d\xdc68\xd83c\xdffc\x200d\xd83e\xdd1d\x200d\xd83d\xdc68\xd83c\xdffb|\xd83d\xdc68\xd83c\xdffd\x200d\xd83e\xdd1d\x200d\xd83d\xdc68\xd83c[\xdffb\xdffc]|\xd83d\xdc68\xd83c\xdffe\x200d\xd83e\xdd1d\x200d\xd83d\xdc68\xd83c[\xdffb-\xdffd]|\xd83d\xdc68\xd83c\xdfff\x200d\xd83e\xdd1d\x200d\xd83d\xdc68\xd83c[\xdffb-\xdffe]|\xd83d\xdc69\xd83c\xdffb\x200d\xd83e\xdd1d\x200d\xd83d\xdc68\xd83c[\xdffc-\xdfff]|\xd83d\xdc69\xd83c\xdffc\x200d\xd83e\xdd1d\x200d\xd83d\xdc68\xd83c[\xdffb\xdffd-\xdfff]|\xd83d\xdc69\xd83c\xdffc\x200d\xd83e\xdd1d\x200d\xd83d\xdc69\xd83c\xdffb|\xd83d\xdc69\xd83c\xdffd\x200d\xd83e\xdd1d\x200d\xd83d\xdc68\xd83c[\xdffb\xdffc\xdffe\xdfff]|\xd83d\xdc69\xd83c\xdffd\x200d\xd83e\xdd1d\x200d\xd83d\xdc69\xd83c[\xdffb\xdffc]|\xd83d\xdc69\xd83c\xdffe\x200d\xd83e\xdd1d\x200d\xd83d\xdc68\xd83c[\xdffb-\xdffd\xdfff]|\xd83d\xdc69\xd83c\xdffe\x200d\xd83e\xdd1d\x200d\xd83d\xdc69\xd83c[\xdffb-\xdffd]|\xd83d\xdc69\xd83c\xdfff\x200d\xd83e\xdd1d\x200d\xd83d\xdc68\xd83c[\xdffb-\xdffe]|\xd83d\xdc69\xd83c\xdfff\x200d\xd83e\xdd1d\x200d\xd83d\xdc69\xd83c[\xdffb-\xdffe]|\xd83e\xddd1\xd83c\xdffb\x200d\xd83e\xdd1d\x200d\xd83e\xddd1\xd83c\xdffb|\xd83e\xddd1\xd83c\xdffc\x200d\xd83e\xdd1d\x200d\xd83e\xddd1\xd83c[\xdffb\xdffc]|\xd83e\xddd1\xd83c\xdffd\x200d\xd83e\xdd1d\x200d\xd83e\xddd1\xd83c[\xdffb-\xdffd]|\xd83e\xddd1\xd83c\xdffe\x200d\xd83e\xdd1d\x200d\xd83e\xddd1\xd83c[\xdffb-\xdffe]|\xd83e\xddd1\xd83c\xdfff\x200d\xd83e\xdd1d\x200d\xd83e\xddd1\xd83c[\xdffb-\xdfff]|\xd83e\xddd1\x200d\xd83e\xdd1d\x200d\xd83e\xddd1|\xd83d\xdc6b\xd83c[\xdffb-\xdfff]|\xd83d\xdc6c\xd83c[\xdffb-\xdfff]|\xd83d\xdc6d\xd83c[\xdffb-\xdfff]|\xd83d[\xdc6b-\xdc6d])|(?:\xd83d[\xdc68\xdc69])(?:\xd83c[\xdffb-\xdfff])?\x200d(?:\x2695\xfe0f|\x2696\xfe0f|\x2708\xfe0f|\xd83c[\xdf3e\xdf73\xdf93\xdfa4\xdfa8\xdfeb\xdfed]|\xd83d[\xdcbb\xdcbc\xdd27\xdd2c\xde80\xde92]|\xd83e[\xddaf-\xddb3\xddbc\xddbd])|(?:\xd83c[\xdfcb\xdfcc]|\xd83d[\xdd74\xdd75]|\x26f9)((?:\xd83c[\xdffb-\xdfff]|\xfe0f)\x200d[\x2640\x2642]\xfe0f)|(?:\xd83c[\xdfc3\xdfc4\xdfca]|\xd83d[\xdc6e\xdc71\xdc73\xdc77\xdc81\xdc82\xdc86\xdc87\xde45-\xde47\xde4b\xde4d\xde4e\xdea3\xdeb4-\xdeb6]|\xd83e[\xdd26\xdd35\xdd37-\xdd39\xdd3d\xdd3e\xddb8\xddb9\xddcd-\xddcf\xddd6-\xdddd])(?:\xd83c[\xdffb-\xdfff])?\x200d[\x2640\x2642]\xfe0f|(?:\xd83d\xdc68\x200d\x2764\xfe0f\x200d\xd83d\xdc8b\x200d\xd83d\xdc68|\xd83d\xdc68\x200d\xd83d\xdc68\x200d\xd83d\xdc66\x200d\xd83d\xdc66|\xd83d\xdc68\x200d\xd83d\xdc68\x200d\xd83d\xdc67\x200d\xd83d[\xdc66\xdc67]|\xd83d\xdc68\x200d\xd83d\xdc69\x200d\xd83d\xdc66\x200d\xd83d\xdc66|\xd83d\xdc68\x200d\xd83d\xdc69\x200d\xd83d\xdc67\x200d\xd83d[\xdc66\xdc67]|\xd83d\xdc69\x200d\x2764\xfe0f\x200d\xd83d\xdc8b\x200d\xd83d[\xdc68\xdc69]|\xd83d\xdc69\x200d\xd83d\xdc69\x200d\xd83d\xdc66\x200d\xd83d\xdc66|\xd83d\xdc69\x200d\xd83d\xdc69\x200d\xd83d\xdc67\x200d\xd83d[\xdc66\xdc67]|\xd83d\xdc68\x200d\x2764\xfe0f\x200d\xd83d\xdc68|\xd83d\xdc68\x200d\xd83d\xdc66\x200d\xd83d\xdc66|\xd83d\xdc68\x200d\xd83d\xdc67\x200d\xd83d[\xdc66\xdc67]|\xd83d\xdc68\x200d\xd83d\xdc68\x200d\xd83d[\xdc66\xdc67]|\xd83d\xdc68\x200d\xd83d\xdc69\x200d\xd83d[\xdc66\xdc67]|\xd83d\xdc69\x200d\x2764\xfe0f\x200d\xd83d[\xdc68\xdc69]|\xd83d\xdc69\x200d\xd83d\xdc66\x200d\xd83d\xdc66|\xd83d\xdc69\x200d\xd83d\xdc67\x200d\xd83d[\xdc66\xdc67]|\xd83d\xdc69\x200d\xd83d\xdc69\x200d\xd83d[\xdc66\xdc67]|\xd83c\xdff3\xfe0f\x200d\xd83c\xdf08|\xd83c\xdff4\x200d\x2620\xfe0f|\xd83d\xdc15\x200d\xd83e\xddba|\xd83d\xdc41\x200d\xd83d\xdde8|\xd83d\xdc68\x200d\xd83d[\xdc66\xdc67]|\xd83d\xdc69\x200d\xd83d[\xdc66\xdc67]|\xd83d\xdc6f\x200d\x2640\xfe0f|\xd83d\xdc6f\x200d\x2642\xfe0f|\xd83e\xdd3c\x200d\x2640\xfe0f|\xd83e\xdd3c\x200d\x2642\xfe0f|\xd83e\xddde\x200d\x2640\xfe0f|\xd83e\xddde\x200d\x2642\xfe0f|\xd83e\xdddf\x200d\x2640\xfe0f|\xd83e\xdddf\x200d\x2642\xfe0f)|[#*0-9]\xfe0f?\x20e3|(?:[©®\x2122\x265f]\xfe0f)|(?:\xd83c[\xdc04\xdd70\xdd71\xdd7e\xdd7f\xde02\xde1a\xde2f\xde37\xdf21\xdf24-\xdf2c\xdf36\xdf7d\xdf96\xdf97\xdf99-\xdf9b\xdf9e\xdf9f\xdfcd\xdfce\xdfd4-\xdfdf\xdff3\xdff5\xdff7]|\xd83d[\xdc3f\xdc41\xdcfd\xdd49\xdd4a\xdd6f\xdd70\xdd73\xdd76-\xdd79\xdd87\xdd8a-\xdd8d\xdda5\xdda8\xddb1\xddb2\xddbc\xddc2-\xddc4\xddd1-\xddd3\xdddc-\xddde\xdde1\xdde3\xdde8\xddef\xddf3\xddfa\xdecb\xdecd-\xdecf\xdee0-\xdee5\xdee9\xdef0\xdef3]|[\x203c\x2049\x2139\x2194-\x2199\x21a9\x21aa\x231a\x231b\x2328\x23cf\x23ed-\x23ef\x23f1\x23f2\x23f8-\x23fa\x24c2\x25aa\x25ab\x25b6\x25c0\x25fb-\x25fe\x2600-\x2604\x260e\x2611\x2614\x2615\x2618\x2620\x2622\x2623\x2626\x262a\x262e\x262f\x2638-\x263a\x2640\x2642\x2648-\x2653\x2660\x2663\x2665\x2666\x2668\x267b\x267f\x2692-\x2697\x2699\x269b\x269c\x26a0\x26a1\x26aa\x26ab\x26b0\x26b1\x26bd\x26be\x26c4\x26c5\x26c8\x26cf\x26d1\x26d3\x26d4\x26e9\x26ea\x26f0-\x26f5\x26f8\x26fa\x26fd\x2702\x2708\x2709\x270f\x2712\x2714\x2716\x271d\x2721\x2733\x2734\x2744\x2747\x2757\x2763\x2764\x27a1\x2934\x2935\x2b05-\x2b07\x2b1b\x2b1c\x2b50\x2b55\x3030\x303d\x3297\x3299])(?:\xfe0f|(?!\xfe0e))|(?:(?:\xd83c[\xdfcb\xdfcc]|\xd83d[\xdd74\xdd75\xdd90]|[\x261d\x26f7\x26f9\x270c\x270d])(?:\xfe0f|(?!\xfe0e))|(?:\xd83c[\xdf85\xdfc2-\xdfc4\xdfc7\xdfca]|\xd83d[\xdc42\xdc43\xdc46-\xdc50\xdc66-\xdc69\xdc6e\xdc70-\xdc78\xdc7c\xdc81-\xdc83\xdc85-\xdc87\xdcaa\xdd7a\xdd95\xdd96\xde45-\xde47\xde4b-\xde4f\xdea3\xdeb4-\xdeb6\xdec0\xdecc]|\xd83e[\xdd0f\xdd18-\xdd1c\xdd1e\xdd1f\xdd26\xdd30-\xdd39\xdd3d\xdd3e\xddb5\xddb6\xddb8\xddb9\xddbb\xddcd-\xddcf\xddd1-\xdddd]|[\x270a\x270b]))(?:\xd83c[\xdffb-\xdfff])?|(?:\xd83c\xdff4\xdb40\xdc67\xdb40\xdc62\xdb40\xdc65\xdb40\xdc6e\xdb40\xdc67\xdb40\xdc7f|\xd83c\xdff4\xdb40\xdc67\xdb40\xdc62\xdb40\xdc73\xdb40\xdc63\xdb40\xdc74\xdb40\xdc7f|\xd83c\xdff4\xdb40\xdc67\xdb40\xdc62\xdb40\xdc77\xdb40\xdc6c\xdb40\xdc73\xdb40\xdc7f|\xd83c\xdde6\xd83c[\xdde8-\xddec\xddee\xddf1\xddf2\xddf4\xddf6-\xddfa\xddfc\xddfd\xddff]|\xd83c\xdde7\xd83c[\xdde6\xdde7\xdde9-\xddef\xddf1-\xddf4\xddf6-\xddf9\xddfb\xddfc\xddfe\xddff]|\xd83c\xdde8\xd83c[\xdde6\xdde8\xdde9\xddeb-\xddee\xddf0-\xddf5\xddf7\xddfa-\xddff]|\xd83c\xdde9\xd83c[\xddea\xddec\xddef\xddf0\xddf2\xddf4\xddff]|\xd83c\xddea\xd83c[\xdde6\xdde8\xddea\xddec\xdded\xddf7-\xddfa]|\xd83c\xddeb\xd83c[\xddee-\xddf0\xddf2\xddf4\xddf7]|\xd83c\xddec\xd83c[\xdde6\xdde7\xdde9-\xddee\xddf1-\xddf3\xddf5-\xddfa\xddfc\xddfe]|\xd83c\xdded\xd83c[\xddf0\xddf2\xddf3\xddf7\xddf9\xddfa]|\xd83c\xddee\xd83c[\xdde8-\xddea\xddf1-\xddf4\xddf6-\xddf9]|\xd83c\xddef\xd83c[\xddea\xddf2\xddf4\xddf5]|\xd83c\xddf0\xd83c[\xddea\xddec-\xddee\xddf2\xddf3\xddf5\xddf7\xddfc\xddfe\xddff]|\xd83c\xddf1\xd83c[\xdde6-\xdde8\xddee\xddf0\xddf7-\xddfb\xddfe]|\xd83c\xddf2\xd83c[\xdde6\xdde8-\xdded\xddf0-\xddff]|\xd83c\xddf3\xd83c[\xdde6\xdde8\xddea-\xddec\xddee\xddf1\xddf4\xddf5\xddf7\xddfa\xddff]|\xd83c\xddf4\xd83c\xddf2|\xd83c\xddf5\xd83c[\xdde6\xddea-\xdded\xddf0-\xddf3\xddf7-\xddf9\xddfc\xddfe]|\xd83c\xddf6\xd83c\xdde6|\xd83c\xddf7\xd83c[\xddea\xddf4\xddf8\xddfa\xddfc]|\xd83c\xddf8\xd83c[\xdde6-\xddea\xddec-\xddf4\xddf7-\xddf9\xddfb\xddfd-\xddff]|\xd83c\xddf9\xd83c[\xdde6\xdde8\xdde9\xddeb-\xdded\xddef-\xddf4\xddf7\xddf9\xddfb\xddfc\xddff]|\xd83c\xddfa\xd83c[\xdde6\xddec\xddf2\xddf3\xddf8\xddfe\xddff]|\xd83c\xddfb\xd83c[\xdde6\xdde8\xddea\xddec\xddee\xddf3\xddfa]|\xd83c\xddfc\xd83c[\xddeb\xddf8]|\xd83c\xddfd\xd83c\xddf0|\xd83c\xddfe\xd83c[\xddea\xddf9]|\xd83c\xddff\xd83c[\xdde6\xddf2\xddfc]|\xd83c[\xdccf\xdd8e\xdd91-\xdd9a\xdde6-\xddff\xde01\xde32-\xde36\xde38-\xde3a\xde50\xde51\xdf00-\xdf20\xdf2d-\xdf35\xdf37-\xdf7c\xdf7e-\xdf84\xdf86-\xdf93\xdfa0-\xdfc1\xdfc5\xdfc6\xdfc8\xdfc9\xdfcf-\xdfd3\xdfe0-\xdff0\xdff4\xdff8-\xdfff]|\xd83d[\xdc00-\xdc3e\xdc40\xdc44\xdc45\xdc51-\xdc65\xdc6a-\xdc6d\xdc6f\xdc79-\xdc7b\xdc7d-\xdc80\xdc84\xdc88-\xdca9\xdcab-\xdcfc\xdcff-\xdd3d\xdd4b-\xdd4e\xdd50-\xdd67\xdda4\xddfb-\xde44\xde48-\xde4a\xde80-\xdea2\xdea4-\xdeb3\xdeb7-\xdebf\xdec1-\xdec5\xded0-\xded2\xded5\xdeeb\xdeec\xdef4-\xdefa\xdfe0-\xdfeb]|\xd83e[\xdd0d\xdd0e\xdd10-\xdd17\xdd1d\xdd20-\xdd25\xdd27-\xdd2f\xdd3a\xdd3c\xdd3f-\xdd45\xdd47-\xdd71\xdd73-\xdd76\xdd7a-\xdda2\xdda5-\xddaa\xddae-\xddb4\xddb7\xddba\xddbc-\xddca\xddd0\xddde-\xddff\xde70-\xde73\xde78-\xde7a\xde80-\xde82\xde90-\xde95]|[\x23e9-\x23ec\x23f0\x23f3\x267e\x26ce\x2705\x2728\x274c\x274e\x2753-\x2755\x2795-\x2797\x27b0\x27bf\xe50a])|\xfe0f" };
static std::wregex feofRegEx{ L"\xFE0F" };

// from https://stackoverflow.com/a/45314422 but adapted to support wide chars
const unsigned char calcFinalSize[] =
{
	1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	1,   1,   6,   1,   1,   1,   5,   6,   1,   1,   1,   1,   1,   1,   1,   1,
	1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   4,   1,   4,   1,
	1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1
};

template<typename TChar>
struct Entities
{

};

template<>
struct Entities<char>
{
	static constexpr std::string_view amp = "&amp;";
	static constexpr std::string_view apos = "&apos;";
	static constexpr std::string_view quot = "&quot;";
	static constexpr std::string_view gt = "&gt;";
	static constexpr std::string_view lt = "&lt;";
};

template<>
struct Entities<wchar_t>
{
	static constexpr std::wstring_view amp = L"&amp;";
	static constexpr std::wstring_view apos = L"&apos;";
	static constexpr std::wstring_view quot = L"&quot;";
	static constexpr std::wstring_view gt = L"&gt;";
	static constexpr std::wstring_view lt = L"&lt;";
};

template<typename TChar>
void EscapeXml(std::basic_string<TChar>& in)
{
	const TChar* dataIn = in.data();
	size_t sizeIn = in.size();

	const TChar* dataInCurrent = dataIn;
	const TChar* dataInEnd = dataIn + sizeIn;
	size_t outSize = 0;
	while (dataInCurrent < dataInEnd)
	{
		auto ch = static_cast<uint8_t>(*dataInCurrent);

		// wide characters need specific branching, try to eliminate this at compile time
		if constexpr (sizeof(TChar) > sizeof(uint8_t))
		{
			// any >8-bit character is also 1 in size
			// this adds an additional branch, sadly.
			if (ch > std::numeric_limits<uint8_t>::max())
			{
				outSize += 1;
				dataInCurrent++;

				continue;
			}
		}

		outSize += calcFinalSize[ch];
		dataInCurrent++;
	}

	if (outSize == sizeIn)
	{
		return;
	}

	std::basic_string<TChar> out;
	out.resize(outSize);

	dataInCurrent = dataIn;
	TChar* dataOut = &out[0];
	while (dataInCurrent < dataInEnd)
	{
		using TEntities = Entities<TChar>;

		switch (*dataInCurrent) {
		case '&':
			memcpy(dataOut, TEntities::amp.data(), TEntities::amp.size() * sizeof(TChar));
			dataOut += TEntities::amp.size();
			break;
		case '\'':
			memcpy(dataOut, TEntities::apos.data(), TEntities::apos.size() * sizeof(TChar));
			dataOut += TEntities::apos.size();
			break;
		case '\"':
			memcpy(dataOut, TEntities::quot.data(), TEntities::quot.size() * sizeof(TChar));
			dataOut += TEntities::quot.size();
			break;
		case '>':
			memcpy(dataOut, TEntities::gt.data(), TEntities::gt.size() * sizeof(TChar));
			dataOut += TEntities::gt.size();
			break;
		case '<':
			memcpy(dataOut, TEntities::lt.data(), TEntities::lt.size() * sizeof(TChar));
			dataOut += TEntities::lt.size();
			break;
		default:
			*dataOut++ = *dataInCurrent;
		}
		dataInCurrent++;
	}
	in.swap(out);
}


// from https://stackoverflow.com/a/37516316
template<class BidirIt, class Traits, class CharT, class UnaryFunction>
std::basic_string<CharT> regex_replace(BidirIt first, BidirIt last,
	const std::basic_regex<CharT, Traits>& re, UnaryFunction f)
{
	std::basic_string<CharT> s;

	typename std::match_results<BidirIt>::difference_type
		positionOfLastMatch = 0;
	auto endOfLastMatch = first;

	auto callback = [&](const std::match_results<BidirIt> & match)
	{
		auto positionOfThisMatch = match.position(0);
		auto diff = positionOfThisMatch - positionOfLastMatch;

		auto startOfThisMatch = endOfLastMatch;
		std::advance(startOfThisMatch, diff);

		s.append(endOfLastMatch, startOfThisMatch);
		s.append(f(match));

		auto lengthOfMatch = match.length(0);

		positionOfLastMatch = positionOfThisMatch + lengthOfMatch;

		endOfLastMatch = startOfThisMatch;
		std::advance(endOfLastMatch, lengthOfMatch);
	};

	std::regex_iterator<BidirIt> begin(first, last, re), end;
	std::for_each(begin, end, callback);

	s.append(endOfLastMatch, last);

	return s;
}

template<class Traits, class CharT, class UnaryFunction>
std::string regex_replace(const std::string& s,
	const std::basic_regex<CharT, Traits>& re, UnaryFunction f)
{
	return regex_replace(s.cbegin(), s.cend(), re, f);
}

static uint32_t curTime;

static void ParseHtmlStub(void* styledText, const wchar_t* str, int64_t length, void* pImgInfoArr, bool multiline, bool condenseWhite, void* styleMgr, void* txtFmt, void* paraFmt)
{
	if (!txtFmt)
	{
		txtFmt = *(void**)((char*)styledText + 56); // default text format
	}

	int sz = ceil(*(uint16_t*)((char*)txtFmt + 62) / 20.0f * 1.2f);

	static thread_local std::map<std::wstring, std::wstring, std::less<>> replacedText;
	static thread_local uint32_t nextClear;

	if (curTime > nextClear)
	{
		replacedText.clear();
		nextClear = curTime + 5000;
	}

	auto it = replacedText.find(std::wstring_view{ str, size_t(length) });

	if (it == replacedText.end())
	{
		auto emojifiedText = regex_replace(str, str + length, emojiRegEx, [sz](const auto& m)
		{
			auto s = m.str();

			if (s.find(L"\x200D") == std::string::npos)
			{
				s = std::regex_replace(s, feofRegEx, L"");
			}

			std::wstringstream codePointString;

			int i = 0;
			int p = 0;

			while (i < s.length())
			{
				auto c = s[i++];
				if (p)
				{
					codePointString << std::hex << (0x10000 + ((p - 0xD800) << 10) + (c - 0xDC00)) << L"_";
					p = 0;
				}
				else if (0xD800 <= c && c <= 0xDBFF)
				{
					p = c;
				}
				else
				{
					codePointString << std::hex << uint32_t(c) << L"_";
				}
			}

			auto cps = codePointString.str();

			return fmt::sprintf(L"<img src='flex_img_%s' width='%d' height='%d'/>", cps.substr(0, cps.length() - 1), sz, sz);
		});

		it = replacedText.emplace(str, std::move(emojifiedText)).first;
	}

	g_parseHtml(styledText, it->second.c_str(), it->second.size(), pImgInfoArr, multiline, condenseWhite, styleMgr, txtFmt, paraFmt);
}

static void ParseHtmlUtf8(void* styledText, const char* str, uint64_t length, void* pImgInfoArr, bool multiline, bool condenseWhite, void* styleMgr, void* txtFmt, void* paraFmt)
{
	auto u16 = ToWide(std::string{ str, length });

	ParseHtmlStub(styledText, u16.c_str(), u16.size(), pImgInfoArr, multiline, condenseWhite, styleMgr, txtFmt, paraFmt);
}

static void GfxLog(void* sfLog, int messageType, const char* pfmt, va_list argList)
{
	static char buf[32768];
	vsnprintf(buf, sizeof(buf) - 1, pfmt, argList);

	trace("GFx log: %s\n", buf);
}

static void(*g_origGFxEditTextCharacterDef__SetTextValue)(void* self, const char* newText, bool html, bool notifyVariable);

static void GFxEditTextCharacterDef__SetTextValue(void* self, const char* newText, bool html, bool notifyVariable)
{
	std::string textRef;

	if (!html)
	{
		html = true;
	}

	g_origGFxEditTextCharacterDef__SetTextValue(self, newText, html, notifyVariable);
}

struct GSizeF
{
	float x;
	float y;
};

static GSizeF (*getHtmlTextExtent)(void* self, const char* putf8Str, float width, const void* ptxtParams);

static GSizeF GetHtmlTextExtentWrap(void* self, const char* putf8Str, float width, const void* ptxtParams)
{
	// escape (since this is actually non-HTML text extent)
	//std::string textRef = putf8Str;
	//EscapeXml<char>(textRef);

	return getHtmlTextExtent(self, putf8Str, width, ptxtParams);
}

static void(*g_origFormatGtaText)(const char* in, char* out, bool a3, void* a4, float size, bool* html, int maxLength, bool a8);

static std::regex condRe{"&lt;(/?C)&gt;"};

static void FormatGtaTextWrap(const char* in, char* out, bool a3, void* a4, float size, bool* html, int maxLength, bool a8)
{
	g_origFormatGtaText(in, out, a3, a4, size, html, maxLength, a8);

	if (!*html)
	{
		*html = true;
	}
}

static HookFunction hookFunction([]()
{
	if (CfxIsSinglePlayer())
	{
		return;
	}

	MH_Initialize();
	MH_CreateHook(hook::get_call(hook::get_pattern("40 88 6C 24 28 44 88 44 24 20 4C 8B C3 48 8B D6", 16)), ParseHtmlStub, (void**)&g_parseHtml);
	MH_CreateHook(hook::get_pattern("48 8B F1 44 8D 6F 01 48", -48), GFxEditTextCharacterDef__SetTextValue, (void**)&g_origGFxEditTextCharacterDef__SetTextValue);

	// GTA text formatting function
	MH_CreateHook(hook::get_pattern("4C 8B FA 4C 8B D9 F3 0F 59 0D", -0x3A), FormatGtaTextWrap, (void**)& g_origFormatGtaText);

	MH_EnableHook(MH_ALL_HOOKS);

	// parse UTF-8 HTML
	hook::jump(hook::get_pattern("49 8B D8  45 33 C0 49 8B E9 FF 50", -0x31), ParseHtmlUtf8);

	// GFxDrawTextImpl(?)

	// GetTextExtent
	hook::set_call(&getHtmlTextExtent, hook::get_pattern("48 8B 55 60 45 33 E4 4C 89", -0x5B));
	hook::jump(hook::get_pattern("0F 29 70 D8 4D 8B F0 48 8B F2 0F 28 F3", -0x1F), GetHtmlTextExtentWrap);

	OnGameFrame.Connect([]()
	{
		curTime = GetTickCount();
	});

	// 1604 unused
	//*(void**)0x1419F4858 = GfxLog;
});
