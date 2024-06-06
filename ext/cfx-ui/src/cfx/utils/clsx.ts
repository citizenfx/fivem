/**
 * Adopted from MIT-licensed https://github.com/lukeed/clsx/blob/08a5a7f3f61fd05036c8599e4c4a4e5cdef43d7a/src/index.js
 */

type TPrim = string | number | boolean | void | undefined | null;

type TVal = TPrim | Record<string, TPrim> | TPrim[];

function toVal(mix: TVal): string {
  if (typeof mix === 'string' || typeof mix === 'number') {
    return mix.toString();
  }

  if (!mix) {
    return '';
  }

  if (typeof mix === 'object') {
    if (Array.isArray(mix)) {
      let y: string;
      const strs: string[] = [];

      for (const item of mix) {
        if (item) {
          y = toVal(item);

          if (y) {
            strs.push(y);
          }
        }
      }

      return strs.join(' ');
    }

    const strs: string[] = [];
    Object.keys(mix).forEach((k) => {
      if (mix[k]) {
        strs.push(k);
      }
    });

    return strs.join(' ');
  }

  return '';
}

export function clsx(...args: TVal[]): string {
  let i = 0;
  let x: string;
  let tmp: TVal;
  const strs: string[] = [];

  while (i < args.length) {
    // eslint-disable-next-line prefer-rest-params
    tmp = arguments[i++];

    if (tmp) {
      x = toVal(tmp);

      if (x) {
        strs.push(x);
      }
    }
  }

  return strs.join(' ');
}
