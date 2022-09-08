export function randomBytes(length: number): string {
	return Array(length + 1)
		.join('x')
		.replace(/x/g, c => {
			return Math.floor(Math.random() * 16).toString(16);
		});
}

export function fastRandomId(): string {
  return (Math.random() * 0x7fffff | 0x100000).toString(16);
}
