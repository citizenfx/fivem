export function joaat(key: string): number {
  const skey: any = key.toLowerCase();
  const hash = new Uint32Array(1);

  for (const i in skey) {
      hash[0] += skey.charCodeAt(i as any);
      hash[0] += hash[0] << 10;
      hash[0] ^= hash[0] >>> 6;
  }

  hash[0] += hash[0] << 3;
  hash[0] ^= hash[0] >>> 11;
  hash[0] += hash[0] << 15;

  return hash[0];
}

export function joaatString(key: string): string {
  return '0x' + joaat(key).toString(16).toUpperCase();
}
