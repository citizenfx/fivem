/* eslint-disable no-bitwise */
const joaatU32Array = new Uint32Array(1);

export function joaat(key: string): number {
  const skey: any = key.toLowerCase();

  joaatU32Array[0] = 0;

  for (const i in skey) {
    if (Object.prototype.hasOwnProperty.call(skey, i)) {
      joaatU32Array[0] += skey.charCodeAt(i as any);
      joaatU32Array[0] += joaatU32Array[0] << 10;
      joaatU32Array[0] ^= joaatU32Array[0] >>> 6;
    }
  }

  joaatU32Array[0] += joaatU32Array[0] << 3;
  joaatU32Array[0] ^= joaatU32Array[0] >>> 11;
  joaatU32Array[0] += joaatU32Array[0] << 15;

  return joaatU32Array[0];
}
