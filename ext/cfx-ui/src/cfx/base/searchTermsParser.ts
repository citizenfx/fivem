import { validLocales, languageMap, countryMap } from 'cfx/common/services/intl/resources/cldr';

type BaseSearchTerm<Type extends string, Extra extends object = object> = {
  type: Type;
  source: string;
  value: string;
  invert: boolean;
  offset: number;
  regexp: boolean;
} & Extra;

export type IAddressSearchTerm = BaseSearchTerm<'address'>;
export type ILocaleSearchTerm = BaseSearchTerm<'locale'>;
export type INameSearchTerm = BaseSearchTerm<'name', { matchLocale?: { at: 'start' | 'end'; with: string } }>;
export type ICategorySearchTerm = BaseSearchTerm<'category', { category: string }>;

export type ISearchTerm = IAddressSearchTerm | ILocaleSearchTerm | INameSearchTerm | ICategorySearchTerm;

// const reRe = /^\/(.+)\/$/;
// const searchRe = /((?:~?\/.*?\/)|(?:[^\s]+))\s?/g;
// const categoryRe = /^([^:]*?):(.*)$/;

function encodeTermBit(bit: string): string {
  // eslint-disable-next-line @stylistic/quotes
  const encoded = bit.replace('"', '\\"').replace("'", "\\'").replace('`', '\\`').replace('~', '\\~');

  if (encoded.includes(' ')) {
    return `"${encoded}"`;
  }

  return encoded;
}
export function searchTermToString(term: ISearchTerm): string {
  let str = '';

  if (term.invert) {
    str += '~';
  }

  if (term.type === 'category') {
    str += `${encodeTermBit(term.category)}:`;
  }

  if (term.regexp) {
    str += `/${term.value}/`;
  } else {
    str += encodeTermBit(term.value);
  }

  return str;
}

export function isAddressSearchTerm(st: string | ISearchTerm[]): boolean {
  if (typeof st === 'string') {
    return st[0] === '>';
  }

  return st[0]?.type === 'address';
}

export function parseSearchTerms2(searchTerms: string): ISearchTerm[] {
  // Address is a special snowflake
  if (isAddressSearchTerm(searchTerms)) {
    return [
      {
        type: 'address',
        source: searchTerms,
        value: searchTerms.substring(1).trim(),
        offset: 0,
        invert: false,
        regexp: false,
      },
    ];
  }

  if (searchTerms.length === 1) {
    return [];
  }

  const terms: ISearchTerm[] = [];

  let ptr = 0;
  let sQuoteOrRegExpNow = false;
  let sQuoteOrRegExpStartChar = '';
  let sNextCharIsNoSpecial = false;

  let term: ISearchTerm = emptyTerm(ptr);

  function finalizeTerm(newTermPtr = ptr) {
    // Something very empty - ignore
    if (!term.source.trim()) {
      term = emptyTerm(newTermPtr);

      return;
    }

    // Validate if actual regexp
    if (term.regexp) {
      try {
        // eslint-disable-next-line no-new
        new RegExp(term.value, 'i');
      } catch (e) {
        term.regexp = false;
        term.value = quoteRe(term.value);
      }
    }

    if (term.type === 'name') {
      // Too short of a term - ignore
      if (term.source.length === 1) {
        term = emptyTerm(newTermPtr);

        return;
      }

      if (term.value.length === 2 && validLocales.has(term.value)) {
        term.type = 'locale' as any;
      } else {
        // This "name" could be a part of locale, right
        const lowerCaseValue = term.value.toLowerCase();

        if (countryMap[lowerCaseValue]) {
          term.matchLocale = {
            at: 'end',
            with: `-${countryMap[lowerCaseValue]}`,
          };
        } else if (languageMap[lowerCaseValue]) {
          term.matchLocale = {
            at: 'start',
            with: `${languageMap[lowerCaseValue]}-`,
          };
        }
      }
    }

    terms.push(term);
    term = emptyTerm(newTermPtr);
  }

  while (ptr < searchTerms.length) {
    const char = searchTerms[ptr];
    const termSourceLength = term.source.length;
    const termValueLength = term.value.length;

    ptr++;

    if (sNextCharIsNoSpecial) {
      sNextCharIsNoSpecial = false;

      term.value += char;
      term.source += char;
      continue;
    }

    // Ignore next char unless in regexp
    if (char === '\\') {
      sNextCharIsNoSpecial = true;

      term.source += char;

      // If in regexp - also append to the value
      if (term.regexp) {
        term.value += char;
      }
      continue;
    }

    // Quote middle and end
    if (sQuoteOrRegExpNow) {
      term.source += char;

      if (char === sQuoteOrRegExpStartChar) {
        sQuoteOrRegExpNow = false;
        sQuoteOrRegExpStartChar = '';

        // If it is the end of category term OR term's value is regexp - finalize
        if (term.regexp || term.type === 'category') {
          finalizeTerm();
        }
        continue;
      }

      term.value += char;
      continue;
    }

    if (isEmptyChar(char)) {
      finalizeTerm();
      continue;
    }

    // Quote start
    if (isQuotationChar(char)) {
      // If current terms' value isn't empty - that's the end for that term
      if (termValueLength > 0) {
        finalizeTerm(ptr - 1);
      }

      term.source += char;

      if (char === '/') {
        term.regexp = true;
      }
      sQuoteOrRegExpNow = true;
      sQuoteOrRegExpStartChar = char;
      continue;
    }

    term.source += char;

    // Inversion
    if (char === '~') {
      if (termSourceLength === 0) {
        term.invert = true;
        continue;
      }
    }

    // Category - term.value -> term.category
    if (char === ':') {
      if (termValueLength >= 2) {
        term.type = 'category';
        (term as any).category = term.value;
        term.value = '';
        continue;
      }
    }

    term.value += char;
  }

  finalizeTerm();

  return terms;
}
const quotationChar = {
  '"': true,
  '\'': true,
  '`': true,
  '/': true,
};
function isQuotationChar(char: string): boolean {
  return quotationChar[char] || false;
}
function isEmptyChar(char: string): boolean {
  return !char.trim();
}
function emptyTerm(offset: number): ISearchTerm {
  return {
    type: 'name',
    invert: false,
    regexp: false,
    offset,
    source: '',
    value: '',
  };
}

function quoteRe(text: string) {
  return text.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
}

try {
  (window as any).__parseSearchTerms = parseSearchTerms2;
} catch (e) {
  // Do nothing
}
