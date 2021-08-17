import emojiRegex from 'emoji-regex';

const emojiPreRe = new RegExp('^(?:' + emojiRegex().source + ')', '');
const SPLIT_RE = /((?<!\.(?:[a-zA-Z]{2,6}))\s?\/+\s?|\||\s[-~]+\s|\s[Il]\s|[\s⠀ㅤ¦]+(?![#0-9])\p{Emoji}|(?<=(?!^)(?![#0-9])\p{Emoji})[\s⠀ㅤ¦]+|・|ㅤ)/u;
const COMMA_SPLIT_RE = /(?:(?<!(?:\d+|Q))\+|,\s*|\.\s+)/u;

function filterSplit(a: string) {
	const bits = a.split(SPLIT_RE)
		.map(b => b.trim())
		.filter(b => b !== '');

	return bits.length > 0 ? bits[0] : '';
}

function filterCommas(a: string) {
	const bits = a.split(COMMA_SPLIT_RE)
		.map(b => b.trim())
		.filter(b => b !== '');

	return bits.slice(0, 3).join(', ');
}

function equalReplace(a: string, ...res: [any, any][]) {
	let lastA: string;

	do {
		lastA = a;

		for (const re of res) {
			a = a.replace(re[0], re[1]);
		}
	} while (a !== lastA);

	return a;
}

const COUNTRY_PREFIX_RE = /^[[{(][a-zA-Z]{2,}(?:\/...?)*(?:\s.+?)?[\]})]/;

export function filterProjectName(a: string) {
	if (a.length >= 50) {
		a = a.substring(0, 50);
	}

	let colorPrefix = '';

	const n = filterSplit(equalReplace(
		equalReplace(
			a,
			[/^[\sㅤ]+/, ''],
			[/(?<=(?!(\d|#))\p{Emoji})(?!(\d|#))\p{Emoji}/u, ''],
			[/^\p{So}/u, ''],
			[/(\s|\u2800)+/gu, ' '],
			[/^\^[0-9]/, (regs) => { colorPrefix = regs; return ''; }],
			[/\^[0-9]/, ''], // any non-prefixed color codes
			[/[\])]\s*[[(].*$/, ']'], // suffixes after a tag
			[/,.*$/, ''], // a name usually doesn't contain a comma
			[COUNTRY_PREFIX_RE, ''], // country prefixes
			[emojiPreRe, '']), // emoji prefixes
		[/[\p{Pe}】]/gu, ''],
		[/(?<!\d)[\p{Ps}【]/gu, '']))
	.normalize('NFKD');

	return colorPrefix + n;
}

export function filterProjectDesc(a: string) {
	if (a.length >= 125) {
		a = a.substring(0, 125);
	}

	return filterCommas(filterSplit(equalReplace(
		a,
		[/\^[0-9]/g, ''],
		[/^[\sㅤ]+/, ''],
		[COUNTRY_PREFIX_RE, ''],
	))).replace(/(\s|\u2800)+/gu, ' ').normalize('NFKD');
}
