export function randomByte(): string {
  return Math.floor(Math.random() * 16).toString(16);
}

export function randomBytes(length: number): string {
  return Array(length).fill(0).map(randomByte).join('');
}

export function fastRandomId(): string {
  // eslint-disable-next-line no-bitwise
  return (Math.random() * 0x7fffff | 0x100000).toString(16);
}
