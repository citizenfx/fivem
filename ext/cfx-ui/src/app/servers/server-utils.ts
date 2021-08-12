import emojiRegex from 'emoji-regex';

const emojiPreRe = new RegExp('^(?:' + emojiRegex().source + ')', '');
const SPLIT_RE = /((?<!\.(?:[a-zA-Z]{2,6}))\s?\/+\s?|\||\s[-~]+\s|\s[Il]\s|[\sㅤ¦]+\p{Emoji}|ㅤ)/u;
const COMMA_SPLIT_RE = /,\s*/u;

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

export function filterProjectName(a: string) {
	if (a.length >= 50) {
		a = a.substring(0, 50);
	}

	let colorPrefix = '';

	const n = filterSplit(equalReplace(
		a,
		[/^[\sㅤ]+/, ''],
		[/^\p{So}/u, ''],
		[/(\s|\u2800)+/gu, ' '],
		[/^\^[0-9]/, (regs) => { colorPrefix = regs; return ''; }],
		[/\^[0-9]/, ''], // any non-prefixed color codes
		[/^\[[a-zA-Z]{2,3}(?:\/...?)*(?:\s.+?)?\]/, ''], // country prefixes
		[emojiPreRe, ''])) // emoji prefixes
	.normalize('NFKD');

	return colorPrefix + n;
}

export function filterProjectDesc(a: string) {
	if (a.length >= 125) {
		a = a.substring(0, 125);
	}

	return filterCommas(filterSplit(a.replace(/\^[0-9]/g, ''))).replace(/(\s|\u2800)+/gu, ' ').normalize('NFKD');
}
