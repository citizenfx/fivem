export function randomBytes(length: number): string {
  return Array(length).fill(0).map(() => Math.floor(Math.random() * 16).toString(16)).join();
}

export function fastRandomId(): string {
  return (Math.random() * 0x7fffff | 0x100000).toString(16);
}
