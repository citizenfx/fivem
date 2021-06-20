import emojiRegex from 'emoji-regex';

const emojiPreRe = new RegExp('^(?:' + emojiRegex().source + ')', '');

function filterSplit(a: string) {
	const bits = a.split(/(\s\/+\s|\||\s-+\s|\s[Il]\s)/u)
		.map(b => b.trim())
		.filter(b => b !== '');

	return bits.length > 0 ? bits[0] : '';
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
		[/^\s+/, ''],
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

	return filterSplit(a).replace(/(\s|\u2800)+/gu, ' ').replace(/\^[0-9]/g, '').normalize('NFKD');
}
