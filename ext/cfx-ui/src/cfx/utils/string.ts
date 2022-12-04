export function unicodeCharAt(string: string, index: number): string {
  var first = string.charCodeAt(index);
  var second;
  if (first >= 0xD800 && first <= 0xDBFF && string.length > index + 1) {
    second = string.charCodeAt(index + 1);
    if (second >= 0xDC00 && second <= 0xDFFF) {
      return string.substring(index, index + 2);
    }
  }
  return string[index];
}

function normalCharAt(string: string, index: number): string {
  return string[index];
}

function indexSorter(a: number, b: number) {
  return a - b;
}

export function splitByIndices(text: string, indices: number[], unicodeIndices = false): Map<number, string> {
  const charAt = unicodeIndices
    ? unicodeCharAt
    : normalCharAt;

  const realIndices = indices
    .filter((index, idx, arr) => {
      // Dedupe
      if (arr.lastIndexOf(index) !== idx) {
        return false;
      }

      return index < text.length && index > 0;
    })
    .sort(indexSorter);

  if (realIndices.length === 0) {
    return new Map([[0, text]]);
  }

  const slices: Map<number, string> = new Map();

  let part = '';

  let strPtr = 0;
  let utfPtr = 0;
  let indexPtr = 0;
  let startIndex = 0;

  while (utfPtr < text.length) {
    const char = charAt(text, utfPtr);

    part += char;
    utfPtr += char.length;
    strPtr++;

    if (realIndices[indexPtr] === strPtr) {
      slices.set(startIndex, part);
      part = '';

      startIndex = realIndices[indexPtr];

      indexPtr++;
    }
  }

  if (part) {
    slices.set(startIndex, part);
  }

  return slices;
}
try {
  (window as any).__splitByIndices = splitByIndices;
} catch {}

export function replaceRange(str: string, replacement: string, start: number, end: number): string {
  const before = str.substring(0, start);
  const after = str.substring(end);

  return `${before}${replacement}${after}`;
}

export function normalizeSlashes(str: string): string {
  return str.replaceAll('\\', '/');
}

export function isTrueString(str: string): boolean {
  return !!str || str === 'true' || str === '1';
}

export function isFalseString(str: string): boolean {
  return !str || str === 'false' || str === '0';
}
