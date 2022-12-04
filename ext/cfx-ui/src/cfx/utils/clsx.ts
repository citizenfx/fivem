/**
 * Adopted from MIT-licensed https://github.com/lukeed/clsx/blob/08a5a7f3f61fd05036c8599e4c4a4e5cdef43d7a/src/index.js
 */

type TPrim =
  | string
  | number
  | boolean
  | void
  | undefined
  | null

type TVal =
  | TPrim
  | Record<string, TPrim>
  | Array<TPrim>

function toVal(mix: TVal): string {
  let y: string;
  let str = '';

	if (typeof mix === 'string' || typeof mix === 'number') {
		str += mix;
	} else if (typeof mix === 'object') {
		if (Array.isArray(mix)) {
      const length = mix.length;

			for (let k=0; k < length; k++) {
				if (mix[k]) {
					if (y = toVal(mix[k])) {
						str && (str += ' ');
						str += y;
					}
				}
			}
		} else {
			for (const k in mix) {
				if (mix[k]) {
					str && (str += ' ');
					str += k;
				}
			}
		}
	}

	return str;
}

export function clsx(...args: TVal[]): string {
  let i = 0;
  let x: string;
  let tmp: TVal;
  let str = '';

  const length = args.length;

	while (i < length) {
		if (tmp = arguments[i++]) {
			if (x = toVal(tmp)) {
				str && (str += ' ');
				str += x;
			}
		}
	}
	return str;
}
